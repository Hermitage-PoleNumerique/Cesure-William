/*
* Copyright (C) William Michalski, Hermitage-PoleNumerique, France
*
*/

#include "VOLT.h"
#define R1 51000
#define R2 100000

VOLT::VOLT(char nomenclature, bool is_analog, bool is_connected, bool is_low_power, uint8_t pin_read):Sensor(nomenclature,is_analog, is_connected, is_low_power, pin_read){
  if (get_is_connected()){
    
  set_warmup_time(0);
  }
}

void VOLT::update_data()
{
  if (get_is_connected()) {

    int readPin = get_pin_read();

    long vcc = readVcc();
    double voltage = analogRead(readPin)* (vcc*0.001/1024.0) * ((R1 + R2)/R2);

    set_data(voltage);

  }
  else {
  	// if not connected, set a random value (for testing)  	
  	if (has_fake_data())	
    	set_data((float)random(0, 450));
  }

}

long VOLT::readVcc() {
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1125300L / result; // Back-calculate AVcc in mV
  return result;
}

double VOLT::get_value()
{
  uint8_t retry=4;
  
  do { 

    update_data();
    Serial.println(get_data());

    retry--;
    
  } while (retry && get_data()==-1.0);

  return get_data();
}
