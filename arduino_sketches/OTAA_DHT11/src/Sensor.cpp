/*
* Copyright (C) 2016 Nicolas Bertuol, University of Pau, France
*
* nicolas.bertuol@etud.univ-pau.fr
*
* Modified by C. Pham, University of Pau, France
* Modified by William Michalski, Hermitage-PoleNumerique, France
*/

#include "Sensor.h"
    
Sensor::Sensor(char nomenclature, bool is_analog, bool is_connected, bool is_low_power, uint8_t pin_read, uint8_t pin_power, int pin_trigger){
  
  _nomenclature = nomenclature;
  
  set_is_analog(is_analog);
  set_is_connected(is_connected);
  set_is_low_power(is_low_power);
  set_pin_read(pin_read);
  set_pin_power(pin_power);
  set_pin_trigger(pin_trigger);
  set_data(0);
  set_warmup_time(0);
  set_fake_data(false);
  set_n_sample(5);
  
  /*if(_pin_power != -1){
    set_power_set("LOW");
  }*/
}

/////////////
// GETTERS //
/////////////

char Sensor::get_nomenclature() {
  return _nomenclature;
}

bool Sensor::get_is_analog(){
  return _is_analog;
}

bool Sensor::get_is_connected(){
  return _is_connected;
}

bool Sensor::get_is_low_power(){
  return _is_low_power;
}

uint8_t Sensor::get_pin_read(){
  return _pin_read;
}

uint8_t Sensor::get_pin_power(){
  return _pin_power;
}

int Sensor::get_pin_trigger(){
  return _pin_trigger;
}

/*char* Sensor::get_power_set(){
  return _power_set;
}*/

double Sensor::get_data(){
  return _data;
}

uint16_t Sensor::get_warmup_time(){
  return _warmup_time;
}

bool Sensor::has_fake_data(){
  return _with_fake_data;
}

bool Sensor::has_pin_trigger(){
  return (_pin_trigger==-1)?false:true;
}

uint8_t Sensor::get_n_sample(){
  return _n_sample;
}
/////////////
// SETTERS //
/////////////
void Sensor::set_is_analog(bool b){
  _is_analog = b;
}

void Sensor::set_is_connected(bool b){
  _is_connected = b;
}

void Sensor::set_is_low_power(bool b){
  _is_low_power = b;
}

void Sensor::set_pin_read(uint8_t u){
  _pin_read = u;
}

void Sensor::set_pin_power(uint8_t u){
  _pin_power = u;
}

void Sensor::set_pin_trigger(int u){
  _pin_trigger = u;
}

/*void Sensor::set_power_set(char* c){
  if(_power_set != NULL){
    delete [] _power_set;
  }
  _power_set = new char[strlen(c)];
  strcpy(_power_set, c);
}*/

void Sensor::set_data(double d) {

	if (_is_connected || _with_fake_data)		
  		_data = d;
  	else
  		_data = -1.0;	
}

void Sensor::set_warmup_time(uint16_t t){
  _warmup_time = t;
}

void Sensor::set_fake_data(bool b) {
	_with_fake_data=b;
}		

void Sensor::set_n_sample(uint8_t n) {
  _n_sample=n;
}   


void Sensor::update_data(){
  
}

double Sensor::get_value(){
  return 0.0;
}
