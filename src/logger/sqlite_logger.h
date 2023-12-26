#pragma once

#include "sqlite/sqlite3.h"

#include <string>
#include <mutex>

#include "logger.h"

#include <fstream>
#include <mutex>

#include "dyn_array.h"
#include <ctime>
#include <exception>

// Класс DbObserver с интерефейсом "наблюдатель" 
// контролирует изменения в "динамическом массиве" и сохраняет изменения в таблицу sqlite базы данных

template< class T>
class DbObserver:public Observer<T>{
private:
    sqlite3 **db;
    std::mutex *mtx;
    std::string table;
    char *errMsg = nullptr;
public:
    DbObserver(sqlite3 **db_, std::mutex *mtx_, std::string table_name):db(db_), mtx(mtx_), table(table_name){};

    void update(T &rec){
        if(db){
            std::lock_guard lck(*mtx);
            std::time_t t = std::chrono::system_clock::to_time_t(rec.age);
            std::string sql ="INSERT INTO " + table + " VALUES(" + std::to_string(rec.index)  + ", '" + rec.alias + "', " + std::to_string(t) + ", " + std::to_string(static_cast<int>(rec.value)) + ");";
            sql = "BEGIN; " + sql;
            try{
                if( auto rc = sqlite3_exec(*db, sql.c_str(), nullptr, nullptr, &errMsg)){

                    std::cout << errMsg << "\n";
                    sqlite3_free(errMsg);

                    sqlite3_exec(*db, "ROLLBACK", nullptr, nullptr, &errMsg);

                } else {
                    sqlite3_exec(*db, "COMMIT", nullptr, nullptr, &errMsg);
                }
            } catch( std::exception &ex){
                std::cout <<  ex.what() << "\n";
            }
        }
    }
};

class SqliteLogger:public Logger{
private:
    sqlite3 *db;
    std::mutex db_mtx;
    char *errMsg = nullptr;
public:
    SqliteLogger();
    ~SqliteLogger();
    void open();
    void close();
};
