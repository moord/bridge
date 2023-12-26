#include "sqlite_logger.h"

const std::string sql_create{"BEGIN;"
                             "CREATE TABLE IF NOT EXISTS STATE_LOG( id INT, alias TEXT, age TIMESTAMP, value INT );" \
                             "CREATE TABLE IF NOT EXISTS MANAGE_LOG( id INT, alias TEXT, age TIMESTAMP, value INT );"\
                             "CREATE TABLE IF NOT EXISTS SENSOR_LOG( id INT, alias TEXT, age TIMESTAMP, value INT );"\
                             "COMMIT;"};

SqliteLogger::SqliteLogger()
{
    state_log = new DbObserver<state_rec>(&db,&db_mtx,"STATE_LOG");
    manage_log = new DbObserver<state_rec>(&db,&db_mtx,"MANAGE_LOG");
    sensor_log = new DbObserver<sensor_rec>(&db,&db_mtx,"SENSOR_LOG");
}

SqliteLogger::~SqliteLogger()
{

    delete state_log;
    delete manage_log;
    delete sensor_log;
}


void SqliteLogger::open(){
    // установить соединение с базой данных
    // открыть/создать
    if(sqlite3_open("log.db", &db)){
        db = nullptr;
    }
    else{

        if( sqlite3_exec(db, sql_create.c_str(), nullptr, nullptr, &errMsg)){
            sqlite3_free(errMsg);
            std::cout << errMsg << "\n";
        }

    }

};

void SqliteLogger::close(){
    if( db ){
        sqlite3_close(db);
		db = nullptr;
    }
}
