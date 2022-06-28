/*
* Copyright (C) 2016 Nicolas Bertuol, University of Pau, France
*
* nicolas.bertuol@etud.univ-pau.fr
*
* modified by Congduc Pham, University of Pau, France
*/

#include "HCSR04.h"

HCSR04::HCSR04(char nomenclature, bool is_analog, bool is_connected, bool is_low_power, uint8_t pin_read, uint8_t pin_power, int pin_trigger):Sensor(nomenclature,is_analog, is_connected, is_low_power, pin_read, pin_power, pin_trigger){
  if (get_is_connected()){
    
    pinMode(get_pin_power(),OUTPUT);
    pinMode(get_pin_read(),INPUT);
    pinMode(get_pin_trigger(), OUTPUT);
    
	if(get_is_low_power())
       digitalWrite(get_pin_power(),LOW);
    else
		   digitalWrite(get_pin_power(),HIGH);
   
  set_warmup_time(0);
  }
}

void HCSR04::update_data()
{
  if (get_is_connected()) {

    int triggerPin = get_pin_trigger();
    int echoPin = get_pin_read();

    //Clears the triggerPin condition
    digitalWrite(triggerPin, LOW);
    delayMicroseconds(2);
    // Sets the triggerPin HIGH (ACTIVE) for 10 microseconds
    digitalWrite(triggerPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(triggerPin, LOW);
    // Reads the echoPin, returns the sound wave travel time in microseconds
    long duration = pulseIn(echoPin, HIGH);
    // Calculating the distance
    double distance = duration * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)

    set_data(distance);

  }
  else {
  	// if not connected, set a random value (for testing)  	
  	if (has_fake_data())	
    	set_data((float)random(0, 450));
  }

}

double HCSR04::get_value()
{
  int triggerPin = get_pin_trigger();
  int echoPin = get_pin_read();
  uint8_t retry=4;
  Serial.print("dist -> ");
  
  // if we use a digital pin to power the sensor...
  if (get_is_low_power())
    digitalWrite(get_pin_power(),HIGH);

  do { 

    //don't need a warmup time but need a firt trigger
    digitalWrite(triggerPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(triggerPin, LOW);
    pulseIn(echoPin, HIGH);
        
    update_data();
    Serial.println(get_data());

    retry--;
    
  } while (retry && get_data()==-1.0);

  if (get_is_low_power())    
    digitalWrite(get_pin_power(),LOW);  
    
  return get_data();
}
