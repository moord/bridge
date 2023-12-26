#include "device_poll.h"
#include <thread>

extern DynamicArray dyn;

void Device::terminate(){
    terminated = true;
    cv.notify_one();
}

void Device::update(state_rec &manage_rec){
    q_manage.add({ManageType::TYPE_MANAGE, manage_rec.index, manage_rec.value});
    control_tech.fetch_or(USART_SET_MANAGE);
    cv.notify_one();
}

void Device::poll(){

    unsigned long cmd{USART_GET_DATA};

    while(1){
        try{
            // если порт не открыт
            // открыть порт
            if( !serial.isOpen()){
                serial.open(port, BAUD_RATE);
                serial.setTimeout(boost::posix_time::seconds(1));
            }

            while (!terminated) {

                // опрос каждые 250 мс 
                std::unique_lock<std::mutex> lk(cv_m);
                if ( !cv.wait_for(lk,  std::chrono::milliseconds(250), [this]{ return (control_tech != 0 || terminated); })){
                    cmd |= USART_GET_DATA;

                } else{
                    cmd |= control_tech;
                    control_tech = 0;
                }

                while (cmd > 0 && !terminated) {
                    if ((cmd & USART_SET_MANAGE) > 0)
                        // включить/выключиь агрегат
                    {
                        send_S();
                        // проверка очереди управления
                        if (q_manage.empty()) {
                            cmd &= ~(USART_SET_MANAGE);
                        }
                    }
                    else if ((cmd & USART_GET_DATA) > 0)
                        // получить данные (датчики, состояние, токи)
                    {
                        send_G();
                        cmd &= ~USART_GET_DATA;
                    }

                    if (cmd ){
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    }
                } // while ( cmd > 0)
            }

        }
        catch(boost::system::system_error& e){
            //
            std::cout << "boost::system::system_error\n";
        }
        catch(...){
            std::cout << "other error\n";
        }

        // закрыть порт
        if( !serial.isOpen()){
            serial.close();
        }

        if (terminated){
            break;
        }

        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
}

void Device::send_S() {
    auto [val, success] = q_manage.get();
    if ( success ) {
        unsigned char mes[] = {val.index, val.set_value,val.type};

        send_ack('S', mes, std::size(mes));

        receive_answer();
    }
}

void Device::send_G() {

    send_ack('G', nullptr, 0);
    receive_answer();
}

void Device::send_ack(unsigned char command, unsigned char *mes, unsigned int mes_size){

    s_mpr_param prm;

    prm.data = mes;
    prm.data_size = mes_size;
    prm.rx_address = addr;
    prm.tx_address = MY_ADDR;
    prm.command = command;

    rs.set_packet(&prm);

    serial.write(reinterpret_cast<char*>(rs.packet),rs.size);
}

void Device::receive_answer(){
    unsigned char data;

    do
    {
        try {
            serial.read(reinterpret_cast<char*>(&data),1);

            if( rs.received_byte(data)  ){
                s_mpr_param prm;

                prm.data = new unsigned char[512];

                prm.rx_address = MY_ADDR;
                if( rs.get_packet(&prm,512) ){

                    switch(prm.command){
                    case 'S':
                        //update_S
                        break;
                    case 'G':
                        update_G(prm.data);
                        break;
                    default:
                        break;
                    }
                }

                delete[] prm.data;

                break;
            }

        } catch(boost::system::system_error& e)
        {
            std::cout<<"Read error: "<<e.what()<<std::endl;
            //throw дальшее
            break;
        }
        catch(timeout_exception& e){
            std::cout<<"Read error: "<<e.what()<<std::endl;
            break;
        }
    }
    while( true );
}

void Device::update_G( unsigned char *buf){
    // считываем состояния
    int cnt = dyn.get_state_size();

    int index = 0;

    for (auto i = 0; i < cnt; ++i) {
        dyn.set_state(i, buf[index]);
        ++index;
    }

    // считываем датчики
    cnt = dyn.get_sensor_size();

    int val;
    for (auto i = 0; i < cnt; ++i) {
        val = buf[index] + (int)buf[index+1]*256;
        dyn.set_sensor(i, val);
        index+=2;
    }
}
