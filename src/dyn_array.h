#pragma once

#include <iostream>

#include <string>
#include <vector>
#include <chrono>
#include <set>


using timestamp =  std::chrono::time_point<std::chrono::system_clock>;

// дискретное состояние агрегата/датчика или управляющий сигнал
struct state_rec{
    unsigned char index;    // порядковый номер
    std::string alias;      // название
    timestamp age;          // время последнего изменения
    unsigned char value;    // значение 0 выкл 1 выкл
    std::string to_string(){
        return "STATE" + std::to_string(index)+"="+std::to_string(value);
    }
};

// аналоговые значение датчика
struct sensor_rec{         
    unsigned char index;     // порядковый номер
    std::string alias;      // название
    timestamp age;          // время последнего изменения
    int value,              // значение
        prev_value;
    int delta;              // уведомлять подписчиков при изменениии значения на delta 
    std::string to_string(){
        return "SENSOR" + std::to_string(index)+"="+std::to_string(value);
    }
};

// интерфейс наблюдатель
template<class T>
class Observer {
public:
    virtual ~Observer() = default;

    virtual void update(T &) = 0;
};

// Динамический массив - отображает структуру технологической линии
// массив дискретных состояний агрегата/датчика
// массив дискретных сигналы на управление агрегатами
// массив аналоговых значений датчиков

class DynamicArray{
private:
    std::set<Observer<state_rec> *> manage_subscribers;   // "подписчики" на изменение сигналов управления
    std::set<Observer<sensor_rec> *> sensor_subscribers;  // "подписчики" на изменение дискретных состояний
    std::set<Observer<state_rec> *> state_subscribers;    // "подписчики" на изменение аналоговых показаний

public:
    std::vector< state_rec > state;                       // массив дискретных состояний агрегата/датчика
    std::vector< sensor_rec > sensor;                     // массив аналоговых значений датчиков
    std::vector< state_rec > manage;                      // массив дискретных сигналы на управление агрегатами

   DynamicArray();
   DynamicArray(std::string filename);

   bool load(std::string filename);	                // загрузить структуру технологической линии представленную в json формате
   
   int get_state_size();
   int get_sensor_size();
   int get_manage_size();

   void set_state( int index, unsigned char new_value );  // обновить дискретное состояние 
   void set_sensor( int index, int new_value );           // обновить показание аналогового датчика
   void set_manage( int index, unsigned char new_value ); // обновить дискретное управление ( при любом обновление уведомляются "наблюдтели" при их наличии

   void subscribe_manage(Observer<state_rec> *);        // добавить подписчика на изменение управления
   void subscribe_sensor(Observer<sensor_rec> *);	// добавить подписчика на изменение аналогового значения
   void subscribe_state(Observer<state_rec> *);  	// добавить подписчика на изменение дискретных состояний

   void unsubscribe_manage(Observer<state_rec> *);      
   void unsubscribe_sensor(Observer<sensor_rec> *);	
   void unsubscribe_state(Observer<state_rec> *);	

   // для поддержки старых программ
   int copy( unsigned char *data, int max_size );       // скопировать все состояния и аналоговые значения в буфер

};
