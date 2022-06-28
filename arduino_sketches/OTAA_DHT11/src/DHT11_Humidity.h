/*
* Copyright (C) 2016 Nicolas Bertuol, University of Pau, France
*
* nicolas.bertuol@etud.univ-pau.fr
*/

#ifndef DHT11_HUMIDITY_H
#define DHT11_HUMIDITY_H

#include "Sensor.h"
//#include "DHT11.h"
#include "DHT.h"

class DHT11_Humidity : public Sensor {
  public:    
    DHT11_Humidity(char* nomenclature, bool is_analog, bool is_connected, bool is_low_power, uint8_t pin_read, uint8_t pin_power);
    double get_value();
    void update_data();
    
  private:
    DHT* dht = NULL;
    //DHT11* dht = NULL;
    //DHT11_ERROR_t errorCode;
    uint8_t _dht_type;
    
};

#endif
