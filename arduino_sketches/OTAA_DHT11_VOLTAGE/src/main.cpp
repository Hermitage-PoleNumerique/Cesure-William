/*******************************************************************************
 * Copyright (c) 2015 Thomas Telkamp and Matthijs Kooijman
 * Copyright (c) 2018 Terry Moore, MCCI
 *
 * Permission is hereby granted, free of charge, to anyone
 * obtaining a copy of this document and accompanying files,
 * to do whatever they want with them without any restriction,
 * including, but not limited to, copying, modification and redistribution.
 * NO WARRANTY OF ANY KIND IS PROVIDED.
 *
 * This example sends a valid LoRaWAN packet with payload "Hello,
 * world!", using frequency and encryption settings matching those of
 * the The Things Network. It's pre-configured for the Adafruit
 * Feather M0 LoRa.
 *
 * This uses OTAA (Over-the-air activation), where where a DevEUI and
 * application key is configured, which are used in an over-the-air
 * activation procedure where a DevAddr and session keys are
 * assigned/generated for use with all further communication.
 *
 * Note: LoRaWAN per sub-band duty-cycle limitation is enforced (1% in
 * g1, 0.1% in g2), but not the TTN fair usage policy (which is probably
 * violated by this sketch when left running for longer)!

 * To use this sketch, first register your application and device with
 * the things network, to set or generate an AppEUI, DevEUI and AppKey.
 * Multiple devices can use the same AppEUI, but each device has its own
 * DevEUI and AppKey.
 *
 * Do not forget to define the radio type correctly in
 * arduino-lmic/project_config/lmic_project_config.h or from your BOARDS.txt.
 *
 *******************************************************************************/

#include <Arduino.h>
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

void do_send(osjob_t* j);

//////////////////Sensors configuration////////////////

// Include sensors
#include "DHT11_Temperature.h"
#include "DHT11_Humidity.h"
#include "VOLT.h"

// CHANGE HERE THE NUMBER OF SENSORS, SOME CAN BE NOT CONNECTED
const int number_of_sensors = 3;

///////////////////////////////////////////////////////


//////////////////Board configuration////////////////
// Pin mapping for the board model provided
// Need to be change if the mapping to the lora module change
const lmic_pinmap lmic_pins = {
    .nss = 10,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 6,
    .dio = {7, 8, LMIC_UNUSED_PIN},
};
/////////////////////////////////////////////////////

//////////////////Sleep mode configuration////////////////
// Sleep mode management with Low-power librarie
#define LOW_POWER
#define SHOW_LOW_POWER_CYCLE                  

#if defined LOW_POWER
#include <LowPower.h>
#endif

//////////////////////////////////////////////////////////

//////////////////INTERRUPTION CONFIGURATION/////////////////
// INTERRUPTIONS ON PIN 2 AND/OR 3 : Allow to send a payload instantly if a signal (HIGH, LOW, RISING, FALLING) is sense on Pin 2 or 3
#define IRQ_PIN2
//#define IRQ_PIN3
//If IRQ_OVERRIDE defined, the other sensors won't be evaluate before sending, in case of interruption
//#define IRQ_OVERRIDE

// Citation from Arduino doc :
// The interruption mode defines when the interrupt should be triggered. Four constants are predefined as valid values:
//    -LOW to trigger the interrupt whenever the pin is low,
//    -CHANGE to trigger the interrupt whenever the pin changes value
//    -RISING to trigger when the pin goes from low to high,
//    -FALLING for when the pin goes from high to low.
#define IRQMODE_PIN2 RISING
#define IRQMODE_PIN3 RISING

/////////////////////////////////////////////////////////////

//////////////////Node configuration////////////////
// This EUI must be in little-endian format, so least-significant-byte
// first. When copying an EUI from ttnctl output, this means to reverse
// the bytes. For TTN issued EUIs the last bytes should be 0xD5, 0xB3,
// 0x70.
static const u1_t PROGMEM APPEUI[8]= { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
void os_getArtEui (u1_t* buf) { memcpy_P(buf, APPEUI, 8);}

// This should also be in little endian format, see above.
//f7 ec c3 88 30 db 06 f4
static const u1_t PROGMEM DEVEUI[8]= {0xf7, 0xec, 0xc3, 0x88, 0x30, 0xdb, 0x06, 0xf4};
void os_getDevEui (u1_t* buf) { memcpy_P(buf, DEVEUI, 8);}

// This key should be in big endian format (or, since it is not really a
// number but a block of memory, endianness does not really apply). In
// practice, a key taken from the TTN console can be copied as-is.
static const u1_t PROGMEM APPKEY[16] = {0xd1, 0xd2, 0x3a, 0x45, 0x03, 0x7c, 0x20, 0x58, 0xf5, 0x9b, 0x2c, 0x5f, 0x27, 0xef, 0xf8, 0x4d};
void os_getDevKey (u1_t* buf) {  memcpy_P(buf, APPKEY, 16);}

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 20;
////////////////////////////////////////////////////

static osjob_t sendjob;

// INTERRUPTION PINS
#if defined IRQ_PIN2 || defined IRQ_PIN3
volatile uint8_t flags_pinIrq = 0b00000000;
#endif

#if defined IRQ_PIN2
#define FLAG_IRQ2 (1 << 0)
void interrupt_pin2(){
    flags_pinIrq |= FLAG_IRQ2;
    os_clearCallback(&sendjob);
    os_setCallback(&sendjob, do_send);
}
#endif

#if defined IRQ_PIN3
#define FLAG_IRQ3 (1 << 1)
void interrupt_pin3(){
    flags_pinIrq |= FLAG_IRQ3;
    os_clearCallback(&sendjob);
    os_setCallback(&sendjob, do_send);
}
#endif

#if defined IRQ_PIN2 && defined IRQ_PIN3
const int irq_payload_offset = 2;
#elif defined IRQ_PIN2 || defined IRQ_PIN3
const int irq_payload_offset = 1;
#else
const int irq_payload_offset = 0;
#endif

#ifdef IRQ_PIN2
const char irq_pin2_nomenclature = 'I';
#endif
#ifdef IRQ_PIN3
const char irq_pin3_nomenclature = 'J';
#endif

#ifdef LOW_POWER

bool goDeepSleep = false;

void lowPower_withInts() {
    #if defined IRQ_PIN2 && defined IRQ_PIN3
    if((flags_pinIrq & (FLAG_IRQ2||FLAG_IRQ3)) > 0) return;
    #elif defined IRQ_PIN2
    if((flags_pinIrq & FLAG_IRQ2) > 0) return;
    #elif defined IRQ_PIN3
    if((flags_pinIrq & FLAG_IRQ3) > 0) return;
    #endif

  Serial.flush();
  delay(5);

  while (!os_queryTimeCriticalJobs(ms2osticks(1158))) {
      #if defined IRQ_PIN2 && defined IRQ_PIN3
      if((flags_pinIrq & (FLAG_IRQ2||FLAG_IRQ3)) > 0) break;
      #elif defined IRQ_PIN2
      if((flags_pinIrq & FLAG_IRQ2) > 0) break;
      #elif defined IRQ_PIN3
      if((flags_pinIrq & FLAG_IRQ3) > 0) break;
      #endif

  
      // ATmega2560, ATmega328P, ATmega168, ATmega32U4
      // each wake-up introduces an overhead of about 158ms
      if (!os_queryTimeCriticalJobs(ms2osticks(8158))) {
        LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF); 
        // MCU has just woken up, but `micros()` has the same value as before deep
        // sleep. So in order to not wait another `txIntervalSeconds`, we jump to the future:
        hal_jump_to_the_future_us((uint32_t) 8158000UL);
#ifdef SHOW_LOW_POWER_CYCLE                  
              Serial.print(F("8"));
#endif              
      }
      else if (!os_queryTimeCriticalJobs(ms2osticks(4158))) {
        LowPower.powerDown(SLEEP_4S, ADC_OFF, BOD_OFF); 
        hal_jump_to_the_future_us((uint32_t) 4158000UL);
#ifdef SHOW_LOW_POWER_CYCLE                  
              Serial.print(F("4"));
#endif 
      }
      else if (!os_queryTimeCriticalJobs(ms2osticks(2158))) {
        LowPower.powerDown(SLEEP_2S, ADC_OFF, BOD_OFF); 
        hal_jump_to_the_future_us((uint32_t) 2158000UL);
#ifdef SHOW_LOW_POWER_CYCLE                  
            Serial.print(F("2"));
#endif 
      }
      else if (!os_queryTimeCriticalJobs(ms2osticks(1158))) {
          LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF); 
          hal_jump_to_the_future_us((uint32_t) 1158000UL);
#ifdef SHOW_LOW_POWER_CYCLE                  
              Serial.print(F("1"));
#endif 
      }      

#ifdef SHOW_LOW_POWER_CYCLE
      Serial.flush();
      delay(1);
#endif
   }
}
#endif

const int payload_length = irq_payload_offset + number_of_sensors*(sizeof(float)+1);
uint8_t payload[payload_length];
const uint8_t payload_size = sizeof(payload);

// array containing sensors pointers
Sensor* sensor_ptrs[number_of_sensors];


void printHex2(unsigned v) {
    v &= 0xff;
    if (v < 16)
        Serial.print('0');
    Serial.print(v, HEX);
}

ostime_t t_queued, t_complete;
ostime_t t_nextTx;


void onEvent (ev_t ev) {
    t_complete = os_getTime();
    Serial.print(t_complete);
    Serial.print(F("["));
    Serial.print(osticks2ms(t_complete));
    Serial.print(F("]: "));
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            Serial.println(F("EV_SCAN_TIMEOUT"));
            break;
        case EV_BEACON_FOUND:
            Serial.println(F("EV_BEACON_FOUND"));
            break;
        case EV_BEACON_MISSED:
            Serial.println(F("EV_BEACON_MISSED"));
            break;
        case EV_BEACON_TRACKED:
            Serial.println(F("EV_BEACON_TRACKED"));
            break;
        case EV_JOINING:
            Serial.println(F("EV_JOINING"));
            break;
        case EV_JOINED:
            Serial.println(F("EV_JOINED"));
            {
              u4_t netid = 0;
              devaddr_t devaddr = 0;
              u1_t nwkKey[16];
              u1_t artKey[16];
              LMIC_getSessionKeys(&netid, &devaddr, nwkKey, artKey);
              Serial.print(F("netid: "));
              Serial.println(netid, DEC);
              Serial.print(F("devaddr: "));
              Serial.println(devaddr, HEX);
              Serial.print(F("AppSKey: "));
              for (size_t i=0; i<sizeof(artKey); ++i) {
                if (i != 0)
                  Serial.print(F("-"));
                printHex2(artKey[i]);
              }
              Serial.println("");
              Serial.print(F("NwkSKey: "));
              for (size_t i=0; i<sizeof(nwkKey); ++i) {
                      if (i != 0)
                              Serial.print(F("-"));
                      printHex2(nwkKey[i]);
              }
              Serial.println();
            }
            
            // Disable link check validation (automatically enabled
            // during join, but because slow data rates change max TX
	          // size, we don't use it in this example.
            LMIC_setLinkCheckMode(0);
            break;
        /*
        || This event is defined but not used in the code. No
        || point in wasting codespace on it.
        ||
        || case EV_RFU1:
        ||     Serial.println(F("EV_RFU1"));
        ||     break;
        */
        case EV_JOIN_FAILED:
            Serial.println(F("EV_JOIN_FAILED"));
            break;
        case EV_REJOIN_FAILED:
            Serial.println(F("EV_REJOIN_FAILED"));
            break;
            break;
        case EV_TXCOMPLETE:
            Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));

            Serial.print(F("Diff (sec): "));
            Serial.println(osticks2ms(t_complete-t_queued)/1000);
            Serial.println("");
            
            if (LMIC.txrxFlags & TXRX_ACK)
              Serial.println(F("Received ack"));
            if (LMIC.dataLen) {
              Serial.println(F("Received "));
              Serial.println(LMIC.dataLen);
              Serial.println(F(" bytes of payload"));
            }
            // Schedule next transmission
            t_nextTx = os_getTime()+sec2osticks(TX_INTERVAL);
            os_setTimedCallback(&sendjob, t_nextTx, do_send);
            goDeepSleep = true;
            break;

        case EV_LOST_TSYNC:
            Serial.println(F("EV_LOST_TSYNC"));
            break;
        case EV_RESET:
            Serial.println(F("EV_RESET"));
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            Serial.println(F("EV_RXCOMPLETE"));
            break;
        case EV_LINK_DEAD:
            Serial.println(F("EV_LINK_DEAD"));
            break;
        case EV_LINK_ALIVE:
            Serial.println(F("EV_LINK_ALIVE"));
            break;
        /*
        || This event is defined but not used in the code. No
        || point in wasting codespace on it.
        ||
        || case EV_SCAN_FOUND:
        ||    Serial.println(F("EV_SCAN_FOUND"));
        ||    break;
        */
        case EV_TXSTART:
            Serial.println(F("EV_TXSTART"));
            break;
        case EV_TXCANCELED:
            Serial.println(F("EV_TXCANCELED"));
            break;
        case EV_RXSTART:
            /* do not print anything -- it wrecks timing */
            break;
        case EV_JOIN_TXCOMPLETE:
            Serial.println(F("EV_JOIN_TXCOMPLETE: no JoinAccept"));
            break;

        default:
            Serial.print(F("Unknown event: "));
            Serial.println((unsigned) ev);
            break;
    }
}

void do_send(osjob_t* j){

int i_offset = 0;
#if defined IRQ_PIN2
    detachInterrupt(digitalPinToInterrupt(2));
    if((flags_pinIrq & FLAG_IRQ2) > 0){
        Serial.print(F("Int on pin 2\n"));
        payload[0] = irq_pin2_nomenclature ;
    }
    else payload[0] = 0, i_offset++;
#endif
#if defined IRQ_PIN3
      detachInterrupt(digitalPinToInterrupt(3));
      if((flags_pinIrq & FLAG_IRQ3) > 0){
        Serial.print(F("Int on pin 3\n"));
#if defined IRQ_PIN2
        payload[1] = irq_pin3_nomenclature;
      }
      else payload[1] = 0, i_offset++;
#else
        payload[0] = irq_pin3_nomenclature;
      }
      else payload[0] = 0, i_offset++;
#endif
#endif
        
#if defined IRQ_OVERRIDE && (defined IRQ_PIN2 || defined IRQ_PIN3)

#if defined IRQ_PIN2 && defined IRQ_PIN3
      if((flags_pinIrq & (FLAG_IRQ2|FLAG_IRQ3)) > 0) {
#elif defined IRQ_PIN2
      if((flags_pinIrq & FLAG_IRQ2) > 0) {
#elif defined IRQ_PIN3
      if((flags_pinIrq & FLAG_IRQ3) > 0) {
#endif
      for (int i=irq_payload_offset; i<payload_length ; i++) payload[i] = 0;
      }
      else{

#endif

          // Main loop for sensors, actually, you don't have to edit anything here
          for (int i=0, j=irq_payload_offset; i<number_of_sensors; i++, j += 1 + sizeof(float)) {

              //For each sensor, a char for defining the variable and a float for the value is added to the payload 
              if (sensor_ptrs[i]->get_is_connected() || sensor_ptrs[i]->has_fake_data()) {
                  payload[j] = (uint8_t)sensor_ptrs[i]->get_nomenclature();
                  float payloadData = (float) sensor_ptrs[i]->get_value();
                  memcpy(payload+j+1, &payloadData, sizeof(float));

              }
          }

#if defined IRQ_OVERRIDE && (defined IRQ_PIN2 || defined IRQ_PIN3)        
      }
#endif
    
    Serial.print(F("Sending : "));
    Serial.print((char*)(payload+i_offset));
    Serial.println("");

    Serial.print(F("Payload hex : "));
    for (int i=0; i<payload_length;i++) {
        printHex2(payload[i]);
        Serial.print(F(" "));
    }
    Serial.println("");

    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
    } else {
        // Prepare upstream data transmission at the next possible time.
        LMIC_setTxData2(1, payload, payload_size, 0);

        t_queued = os_getTime();
        Serial.print((uint32_t)t_queued);
        Serial.print(F(": "));
        
        Serial.println(F("Packet queued"));
        Serial.print(F("FREQ="));
        Serial.println(LMIC.freq);              
    }
    // Next TX is scheduled after TX_COMPLETE event.

    //Reattach the interruptions
#if defined IRQ_PIN2 || defined IRQ_PIN3
      flags_pinIrq = 0b00000000;
#endif

#if defined IRQ_PIN2
      attachInterrupt(digitalPinToInterrupt(2), interrupt_pin2, IRQMODE_PIN2);
#endif

#if defined IRQ_PIN3
      attachInterrupt(digitalPinToInterrupt(3), interrupt_pin3, IRQMODE_PIN3);
#endif
}


void setup() {

    delay(5000);
    while (! Serial);
    Serial.begin(9600);
    Serial.println(F("Starting"));

    // LMIC init
    os_init();
    // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();

    // Let LMIC compensate for +/- 10% clock error
    // we take 10% error to better handle downlink messages    
    LMIC_setClockError(MAX_CLOCK_ERROR * 10 / 100);
    
    #ifdef LOW_POWER
      bool low_power_status = IS_LOWPOWER;
    #else
      bool low_power_status = IS_NOT_LOWPOWER;
    #endif
    
    //////////////////////////////////////////////////////////////////
    // ADD YOUR SENSORS HERE   
    // Sensor(nomenclature, is_analog, is_connected, is_low_power, pin_read, pin_power, pin_trigger=-1)   
    sensor_ptrs[0] = new DHT11_Temperature('T', IS_NOT_ANALOG, IS_CONNECTED, low_power_status, 5,4);
    sensor_ptrs[1] = new DHT11_Humidity('H', IS_NOT_ANALOG, IS_CONNECTED, low_power_status, 5,4);
    sensor_ptrs[2] = new VOLT('V', IS_NOT_ANALOG, IS_CONNECTED, low_power_status, A0);


    ////////////////////////////////////////////////////////////////// 
    
    #if defined(CFG_eu868)
    // Set up the channels used by the Things Network, which corresponds
    // to the defaults of most gateways. Without this, only three base
    // channels from the LoRaWAN specification are used, which certainly
    // works, so it is good for debugging, but can overload those
    // frequencies, so be sure to configure the full frequency range of
    // your network here (unless your network autoconfigures them).
    // Setting up channels should happen after LMIC_setSession, as that
    // configures the minimal channel set.
    // NA-US channels 0-71 are configured automatically
    
    LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI);      // g-band
    LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK,  DR_FSK),  BAND_MILLI);      // g2-band
    #endif

    // Start job (sending automatically starts OTAA too)
    do_send(&sendjob);
}

void loop() {

#if defined LOW_POWER
    if (goDeepSleep){
        //Sleep mode doesn't start if there is a job running in less than 8s
        if(!os_queryTimeCriticalJobs(ms2osticks(8158))) {
            Serial.println(F("Entering sleep mode..."));
            lowPower_withInts();
            Serial.println(F("\nWaking up !"));
        }
        goDeepSleep = false;
    }
#endif

    os_runloop_once();
}
