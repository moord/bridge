#pragma once

#include <string>
#include <memory>
#include "dyn_array.h"

// класс родитель для реализаций логеров

// state_log - "подписывается на изменения" дискретных состояний в динамическом массиве 
// manage_log - "подписывается на изменения" дискретных состояний в динамическом массиве 
// sensor_log - "подписывается на изменения" дискретных состояний в динамическом массиве 

class Logger{
public:
   Observer<state_rec> *state_log;
   Observer<state_rec> *manage_log;
   Observer<sensor_rec> *sensor_log;

   virtual void open() = 0;

   virtual void close() = 0;
};

