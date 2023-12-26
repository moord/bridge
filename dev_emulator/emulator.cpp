#include "TimeoutSerial.h"
#include "../src/metra_protocol.h"
#include "../src/dyn_array.h"
#include "../src/include.h"
#include <thread>

class EmulatorDynamicArray: public DynamicArray{
public:

    void set_manage( int index, unsigned char new_value ){
        DynamicArray::set_manage(index, new_value);

        for( auto & rec: state){
            if( rec.alias == manage[index].alias){
                set_state(rec.index, new_value);
                break;
            }

        }
    }
};

EmulatorDynamicArray dyn;

bool terminated{false};

void device_exchange( std::string port){

    unsigned char tmp = 0;
    // установить соединение
    MetraProtocol rs{512};

    TimeoutSerial serial;

    try {
        serial.open(port, BAUD_RATE);
        serial.setTimeout(boost::posix_time::seconds(5));

    } catch(boost::system::system_error& e) {
        std::cout<<"Open error: "<<e.what()<<std::endl;
    }

    s_mpr_param prm;

    prm.data = new unsigned char[512];

    unsigned char data;

    while( !terminated ){
        try {
            serial.read(reinterpret_cast<char*>(&data),1);
             if( rs.received_byte(data)  ){

                prm.rx_address = DEVICE_ADDR;
                if( rs.get_packet(&prm,512) ){
                    switch(prm.command){
                    case 'S':
                        if( prm.data[2] == ManageType::TYPE_MANAGE){
                            std::cout << "Set manage. index = " << (int)prm.data[0] << " value = " << (int)prm.data[1] << std::endl;
                            dyn.set_manage(prm.data[0],prm.data[1]);
                        }
                        prm.data_size = 0;
                        break;
                    case 'G':
                        prm.data_size= dyn.copy(prm.data, 512);
                        break;
                    default:
                        break;
                    }

                    std::swap(prm.rx_address, prm.tx_address);
                    rs.set_packet(&prm);

                    serial.write(reinterpret_cast<char*>(rs.packet),rs.size);
                }
            }

        } catch(boost::system::system_error& e)
        {
            std::cout<<"Read/write error: "<<e.what()<<std::endl;
            break;
        }
        catch(timeout_exception& e){
            std::cout<< e.what()<<std::endl;
            //break;
        }
    }

    delete[] prm.data;

    // закрыть соединение
    serial.close();
}

int main(int argc, char* argv[]){

    if (argc != 3)
    {
      std::cerr << "Usage: device_emulatorr <serial port> <param file name>\n";
      return 1;
    }

    dyn.load(argv[2]);

    auto sensors_change = [](){
        std::srand(std::time(0));
        while( !terminated){
            for( auto & rec: dyn.sensor){
                if( rec.value < 1000){
                    rec.value += std::rand() % 100;
                } else{
                    rec.value = 0;
                }
            }
            std::this_thread::sleep_for(std::chrono::seconds(3));
        }
    };

    std::thread t1(sensors_change);
    std::thread t2(device_exchange, argv[1]);



    std::string user_cmd;
    std::cout << "Enter the 'exit' to stop: \n";
    while(std::getline(std::cin, user_cmd) ) {
        if( user_cmd == "exit"){
            terminated = true;
            break;
        }
    }

    if(t1.joinable()) t1.join();
    if(t2.joinable()) t2.join();

    return 0;
}
