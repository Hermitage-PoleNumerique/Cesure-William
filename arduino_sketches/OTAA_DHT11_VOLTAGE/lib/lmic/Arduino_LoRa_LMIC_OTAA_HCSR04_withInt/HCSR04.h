/*
* Copyright (C) 2016 Nicolas Bertuol, University of Pau, France
*
* nicolas.bertuol@etud.univ-pau.fr
*/

#ifndef HCSR04_H
#define HCSR04_H

#include "Sensor.h"

class HCSR04 : public Sensor {
  public:
    HCSR04(char nomenclature, bool is_analog, bool is_connected, bool is_low_power, uint8_t pin_read, uint8_t pin_power, int pin_trigger);
    double get_value();
    void update_data();
};

#endif
