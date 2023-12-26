#pragma once

#include "logger.h"

#include <fstream>
#include <mutex>

#include "dyn_array.h"
#include <ctime>
#include <iomanip>

// Класс FileObserver с интерефейсом "наблюдатель" 
// контролирует изменения в "динамическом массиве" и сохраняет изменения в файл

template< class T>
class FileObserver:public Observer<T>{
private:
    std::mutex mtx;
    std::ofstream ofs;
public:
    FileObserver(){}

    ~FileObserver(){

        if(ofs.is_open()){
            ofs.close();
        }
    }

    void open(const std::string filename){
       ofs.open(filename, std::ios_base::app);
    };

    void close(){
         if(ofs.is_open()){
             ofs.close();
         }
    }

    void update(T &rec){
        if(ofs.is_open()){
            std::lock_guard lck(mtx);
            std::time_t t = std::chrono::system_clock::to_time_t(rec.age);
            ofs << std::put_time(std::localtime(&t), "%Y.%m.%d %H:%M:%S ") << rec.alias << " = " << static_cast<int>(rec.value) << "\n";
        }
    }
};

class FileLogger : public Logger{
private:
public:
    FileLogger();
    ~FileLogger();

   void open();
   void close();
};
