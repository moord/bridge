#pragma once

#include <mutex>
#include <condition_variable>
#include <string>
#include <queue>
#include "dyn_array.h"
#include <tuple>
#include <atomic>

#include "metra_protocol.h"
#include "TimeoutSerial.h"

#include "include.h"

#define USART_GET_PARAM    	      0x0001
#define USART_SET_PARAM    	      0x0002
#define USART_GET_DATA     	      0x0004
#define USART_SET_MANAGE	      0x0008

// управляющий сигнал
struct ManageCommand{
    ManageCommand(){};
    ManageCommand(unsigned char type_, unsigned char index_, unsigned char set_value_):type(type_), index(index_), set_value(set_value_){};
    unsigned char type, index, set_value;
};

// очередь управляющих сигналов для отправки в контроллер
class QueueManage{
private:
    std::mutex mtx;
    std::queue<ManageCommand>q_manage;
public:
    size_t size(){
        std::lock_guard lck(mtx);
        return q_manage.size();
    }

    bool empty(){
        std::lock_guard lck(mtx);
        return q_manage.empty();
    }

    // добавить команду управления в очередь
    void add( ManageCommand &&val){
        std::lock_guard lck(mtx);
        q_manage.emplace(std::move(val));
    }

    // извлечь команду управления из очереди
    std::tuple<ManageCommand,bool> get(){
        std::lock_guard lck(mtx);
        if( q_manage.size() ){
            auto val = q_manage.front();
            q_manage.pop();
            return std::make_tuple(val, true);
        } else{
            return std::make_tuple(ManageCommand(), false);
        }
    }
};

// Класс "управляющий контроллер"
// с интерфесом для подписки на изменение управляющих сигналов в динамическом массиве
class Device:public Observer<state_rec>{
private:
   std::condition_variable cv;
   std::mutex cv_m;

   QueueManage q_manage;

   bool terminated{false};

   // связь
   int addr;                       // адрес устройства для MetraProtocol
   MetraProtocol rs{512};

   std::string port;               // имя последовательного порта     
   TimeoutSerial serial;

   void send_S();                  // отправить управляющий сигнал
   void send_G();                  // отправить запрос на получение данных

   void update_G( unsigned char *buf);   // обработать ответ с данными 

   void send_ack(unsigned char command, unsigned char *mes, unsigned int mes_size);     // отправка запроса MetraProtocol 
   void receive_answer();                                                               // получение ответа MetraProtocol
public:
   std::atomic<unsigned long> control_tech{0};

   Device(int addr_, const std::string &port_):addr(addr_),port(port_){};

   void update(state_rec &manage_rec);

   void poll();                                                                         // рабочий цикл запрос-ответ

   void terminate();   
};
