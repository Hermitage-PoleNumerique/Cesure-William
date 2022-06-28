/*
* Copyright (C) 2016 Nicolas Bertuol, University of Pau, France
*
* nicolas.bertuol@etud.univ-pau.fr
*/

#ifndef VOLT_H
#define VOLT_H

#include "Sensor.h"

class VOLT : public Sensor {
  public:
    VOLT(char nomenclature, bool is_analog, bool is_connected, bool is_low_power, uint8_t pin_read);
    double get_value();
    void update_data();
    long readVcc();
};

#endif
