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
#include <LowPower.h>
#include <EEPROM.h>

void do_send(osjob_t* j);

//////////////////Board configuration////////////////
// Pin mapping for the board model provided
// Need to be change if the mapping of the lora module change
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

// For normal use, we require that you edit the sketch to replace FILLMEIN
// with values assigned by the TTN console. However, for regression tests,
// we want to be able to compile these scripts. The regression tests define
// COMPILE_REGRESSION_TEST, and in that case we define FILLMEIN to a non-
// working but innocuous value.
//
#ifdef COMPILE_REGRESSION_TEST
# define FILLMEIN 0
#else
# warning "You must replace the values marked FILLMEIN with real values from the TTN control panel!"
# define FILLMEIN (#dont edit this, edit the lines that use FILLMEIN)
#endif

// This EUI must be in little-endian format, so least-significant-byte
// first. When copying an EUI from ttnctl output, this means to reverse
// the bytes. For TTN issued EUIs the last bytes should be 0xD5, 0xB3,
// 0x70.
//static const u1_t PROGMEM APPEUI[8]= { FILLMEIN };
static const u1_t PROGMEM APPEUI[8]= { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
void os_getArtEui (u1_t* buf) { memcpy_P(buf, APPEUI, 8);}

// This should also be in little endian format, see above.
//06 e7 06 90 9e e2 7f 7d

static const u1_t PROGMEM DEVEUI[8]= {0x06, 0xe7, 0x06, 0x90, 0x9e, 0xe2, 0x7f, 0x7d};
void os_getDevEui (u1_t* buf) { memcpy_P(buf, DEVEUI, 8);}

// This key should be in big endian format (or, since it is not really a
// number but a block of memory, endianness does not really apply). In
// practice, a key taken from the TTN console can be copied as-is.
// f7 fb 0a d1 ab dc 5e 54 35 f7 a6 cd 34 d1 de e0
static const u1_t PROGMEM APPKEY[16] = {0xf7, 0xfb, 0x0a, 0xd1, 0xab, 0xdc, 0x5e, 0x54, 0x35, 0xf7, 0xa6, 0xcd, 0x34, 0xd1, 0xde, 0xe0};
void os_getDevKey (u1_t* buf) {  memcpy_P(buf, APPKEY, 16);}

static osjob_t sendjob;

// INTERRUPTION PIN
volatile uint8_t flags_pinIrq = 0b00000000;

#define FLAG_IRQ2 (1 << 0)
void interrupt_pin2(){
    flags_pinIrq |= FLAG_IRQ2;
    os_clearCallback(&sendjob);
    os_setCallback(&sendjob, do_send);
}

uint8_t i_burst = -1;
uint8_t i_id;

#ifdef LOW_POWER

void lowPower_withInts() {
    if((flags_pinIrq & FLAG_IRQ2) > 0) return;

  Serial.flush();
  delay(5);

  while (!os_queryTimeCriticalJobs(ms2osticks(1158))) {
      if((flags_pinIrq & FLAG_IRQ2) > 0) break;

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

#define LED_PIN 5
void led_blink(int n){
    for(int i = 0; i < n ; i++){
        digitalWrite(LED_PIN, HIGH);
        delay(250);
        digitalWrite(LED_PIN, LOW);
        delay(250);
    }

}

const int payload_length = 2;
uint8_t payload[payload_length];
const uint8_t payload_size = sizeof(payload);

void printHex2(unsigned v) {
    v &= 0xff;
    if (v < 16)
        Serial.print('0');
    Serial.print(v, HEX);
}

ostime_t t_queued, t_complete;
ostime_t t_nextTx;

bool goDeepSleep = false;

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

            led_blink(2);

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
            
            led_blink(1);

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
            if(i_burst<9) os_setCallback(&sendjob, do_send);
            else goDeepSleep = true;

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

    if((flags_pinIrq & FLAG_IRQ2) > 0){
        Serial.print(F("Int on pin 2\n"));
        flags_pinIrq = 0b00000000;
        EEPROM.put(0, ++i_id);
        i_burst = 0;
    }
    else i_burst++;

    payload[0] = i_id;
    payload[1] = i_burst;
        
    Serial.print(F("Sending (hex) : "));
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
    
    //Counters
    i_burst = 254;
    EEPROM.get(0, i_id); 

    //Interruption pin
    attachInterrupt(digitalPinToInterrupt(2), interrupt_pin2, RISING);

    //Blink-led
    pinMode(LED_PIN, OUTPUT);
    
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
    if (goDeepSleep){
        if(!os_queryTimeCriticalJobs(ms2osticks(8158))) {
            Serial.println(F("Entering sleep mode..."));
            lowPower_withInts();
            Serial.println(F("\nWaking up !"));
        }
        goDeepSleep = false;
    }
    os_runloop_once();
}
