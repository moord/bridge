#include "filelogger.h"

FileLogger::FileLogger(){
    state_log = new FileObserver<state_rec>();
    manage_log = new FileObserver<state_rec>();
    sensor_log = new FileObserver<sensor_rec>();
}

FileLogger::~FileLogger(){
    delete state_log;
    delete manage_log;
    delete sensor_log;
}

void FileLogger::open(){
    static_cast<FileObserver<state_rec>*>(state_log)->open("state.txt");
    static_cast<FileObserver<state_rec>*>(manage_log)->open("manage.txt");
    static_cast<FileObserver<sensor_rec>*>(sensor_log)->open("sensor.txt");
};

void FileLogger::close(){
    static_cast<FileObserver<state_rec>*>(state_log)->close();
    static_cast<FileObserver<state_rec>*>(manage_log)->close();
    static_cast<FileObserver<sensor_rec>*>(sensor_log)->close();
}
