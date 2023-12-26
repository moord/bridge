#include "dyn_array.h"
#include "filelogger.h"
#include "sqlite_logger.h"
#include "device_poll.h"
#include "server1.h"
#include "server2.h"

#include <boost/program_options.hpp>

namespace po = boost::program_options;

DynamicArray dyn;  // сделать статиком .. singletone

int main(int argc, char** argv){
    po::options_description desc ("Доступные опции");
    desc.add_options ()
            ("help,h", "описание доступных опций")
            ("logger,L", po::value<std::string>(), "тип логера (file|sqlite)")
            ("param-file,F", po::value<std::string>(), "параметры динамического массива (*.json)")
            ("serial,s", po::value<std::string>(), "последовательный порт")
            ("port1,1", po::value<int>(), "порт режима <запись>-<ответ> (по умолчанию 9000)")
            ("port2,2", po::value<int>(), "порт режима <подписка> (по умолчанию 9001)");

    po::variables_map vm;
    po::store (po::command_line_parser (argc, argv).options (desc).run (), vm);
    po::notify (vm);

    if (vm.count ("help") ) {
        std::cerr << desc << "\n";
        return 1;
    }

    std::string serial_port;
    if (vm.count ("serial") ) {
        serial_port = vm["serial"].as<std::string>();
    } else{
        std::cerr << "не задан последовательный порт\n";
        return 1;
    }

    if (vm.count ("param-file") ) {
        if( dyn.load(vm["param-file"].as<std::string>()) ){
            std::cout << "STATE_CNT : " << dyn.get_state_size() << "\n";
            std::cout << "MANAGE_CNT: " << dyn.get_manage_size() << "\n";
            std::cout << "SENSOR_CNT: " << dyn.get_sensor_size() << "\n";
        } else{
            std::cerr << "ошибка загрузки параметров динамического массива\n";
            return 1;
        }
    } else{
        std::cerr << "не заданы параметры динамического массива\n";
        return 1;
    }

   std::shared_ptr<Logger> log;

   std::string logger_type = "file";

   if (vm.count ("logger") ) {
       logger_type = vm["logger"].as<std::string>();
   }

   if( logger_type == "file"){
       log.reset(new FileLogger());
   } else if( logger_type == "sqlite"){
       log.reset(new SqliteLogger());
   }else{
       std::cerr << "Данный тип логера не поддерживается\n";
       return 1;
   }

   int port1{9000}, port2{9001};

   if (vm.count ("port1") ) {
       port1 = vm["port1"].as<int>();
   }

   if (vm.count ("port2") ) {
       port2 = vm["port2"].as<int>();
   }

   log->open();

   dyn.subscribe_manage( log->manage_log );
   dyn.subscribe_sensor( log->sensor_log );

   Device dev(DEVICE_ADDR, serial_port);

   dyn.subscribe_manage( &dev );

   // запуск опроса устройства
   std::thread t(&Device::poll, &dev);

   boost::asio::io_service io_context1;

   server1 server1(io_context1, port1);

   // запустить сервер1
   std::thread t1([&io_context1](){
       try
       {
           io_context1.run();
       }
       catch (const std::exception& ex)
       {
           std::cerr << "Exception: " << ex.what() << "\n";
       }
   });

   boost::asio::io_service io_context2;

   server2 server2(io_context2, port2);

   // запустить сервер1
   std::thread t2([&io_context2](){
       try
       {
           io_context2.run();
       }
       catch (const std::exception& ex)
       {
           std::cerr << "Exception: " << ex.what() << "\n";
       }
   });

   std::string user_cmd;
   std::cout << "Enter the 'exit' to stop: \n";
   while(std::getline(std::cin, user_cmd) ) {
       if( user_cmd == "exit"){
          dev.terminate();
          server1.stop();
          io_context1.stop();
          server2.stop();
          io_context2.stop();
          break;
       }
   }

   if(t.joinable()) t.join();   
   if(t1.joinable()) t1.join();
   if(t2.joinable()) t2.join();

   log->close();

   return 0;
}

