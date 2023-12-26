#include "nlohmann/json.hpp"
#include "dyn_array.h"
#include <fstream>
#include <algorithm>

using json = nlohmann::json;

DynamicArray::DynamicArray(){
}

DynamicArray::DynamicArray(std::string filename){
    load(filename);
}

bool DynamicArray::load(std::string filename){
	bool result = false;
 	try{
    std::ifstream ifs(filename);

    json j = json::parse(ifs);

	state.clear();
	if( j["state"].size() ){
	 	state.resize(j["state"].size());
		int index;		
	    for (auto it = j["state"].begin(); it != j["state"].end(); ++it) {
	        index = it->value("index",-1);
			if( index >= 0 ){
                state[index].index = index;
				state[index].alias =  it->value("alias","");
                state[index].age = {};
				state[index].value = 0;
			}
	    }
	}

	manage.clear();
	if( j["manage"].size() ){
        manage.resize(j["manage"].size());
		int index;		
	    for (auto it = j["manage"].begin(); it != j["manage"].end(); ++it) {
	        index = it->value("index",-1);
			if( index >= 0 ){
                manage[index].index = index;
                manage[index].alias =  it->value("alias","");
                manage[index].age = {};
				manage[index].value = 0;
			}
	    }
	}

	sensor.clear();
	if( j["sensor"].size() ){
	 	sensor.resize(j["sensor"].size());
		int index;		
	    for (auto it = j["sensor"].begin(); it != j["sensor"].end(); ++it) {
	        index = it->value("index",-1);
			if( index >= 0 ){
                sensor[index].index = index;
				sensor[index].alias =  it->value("alias","");
                sensor[index].age = {};
				sensor[index].value = 0;
                sensor[index].delta =  it->value("delta",0);
                sensor[index].prev_value = sensor[index].value - sensor[index].delta;
			}
	    }
	}

	 result = true;
    } catch(std::exception &e){
//        std::cout << e.what() << "\n";
	}

	return result;	
}


int DynamicArray::get_state_size(){
	return state.size();
}

int DynamicArray::get_sensor_size(){
	return sensor.size();
}

int DynamicArray::get_manage_size(){
	return manage.size();
}


void DynamicArray::set_state( int index, unsigned char new_value ){
 	if( index <= state.size() && state[index].value != new_value){
		state[index].value = new_value;
        state[index].age = std::chrono::system_clock::now();
        for( const auto s: state_subscribers){
            s->update(state[index]);
		}
	}
}

void DynamicArray::set_sensor(int index, int new_value){
 	if( index <= sensor.size() && sensor[index].value != new_value){
		sensor[index].value = new_value;
        sensor[index].age = std::chrono::system_clock::now();
        if( abs(sensor[index].value - sensor[index].prev_value) >= sensor[index].delta){
            sensor[index].prev_value = sensor[index].value;
            for( const auto s: sensor_subscribers){
                s->update(sensor[index]);
            }
        }
    }
}

void DynamicArray::set_manage( int index, unsigned char new_value ){
    if( index <= manage.size() /*&& manage[index].value != new_value*/){
		manage[index].value = new_value;
        manage[index].age = std::chrono::system_clock::now();        
        for( const auto s: manage_subscribers){
            s->update(manage[index]);
		}
	}
}

void DynamicArray::subscribe_manage( Observer<state_rec> * obs ){
    manage_subscribers.insert(obs);
}

void DynamicArray::subscribe_sensor( Observer<sensor_rec> * obs){
    sensor_subscribers.insert(obs);
}

void DynamicArray::subscribe_state( Observer<state_rec> * obs ){
    state_subscribers.insert(obs);
}

void DynamicArray::unsubscribe_manage(Observer<state_rec> *obs){
     manage_subscribers.erase(obs);
}
void DynamicArray::unsubscribe_sensor(Observer<sensor_rec> *obs){
    sensor_subscribers.erase(obs);
}
void DynamicArray::unsubscribe_state(Observer<state_rec> *obs){
    state_subscribers.erase(obs);
};

int DynamicArray::copy( unsigned char *data, int max_size ){
   // int count = std::min( static_cast<int>(state.size()), max_size);

    int index =0;
    for( auto i = 0; i < state.size(); ++i){
        if( index < max_size){
            data[index] = state[i].value;
            ++index;
        } else{
            break;
        }
    }

    int16_t val;
    for( auto i = 0; i < sensor.size(); ++i){
        if( index < max_size){
            val = sensor[i].value;
            data[index] = (unsigned char)val;
            data[index+1] = (unsigned char)(val >> 8);

            index += 2;
        } else{
            break;
        }
    }

    return index;
}
