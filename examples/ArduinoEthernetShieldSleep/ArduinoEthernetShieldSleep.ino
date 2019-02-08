/**
 * Hardware Used:
 *  - Arduino UNO
 *  - Ethernet Shield (W5100 model)
 *  - DS1307 RTC 
 *  
 *  I have tested it with mosquitto and emqx brokers installed in my local
 *  network but it should work with any broker in cloud.
 *  
 *  Device will go to sleep when it detects no MQTT activity during 2.5 seconds.
 *  
 *  Device will wake up periodically by WDT, in order to, if no MQTT activity,
 *  do keep_alive if necessary. You should take care of keep alive time and wdt
 *  period configuration. If your wdt period is too much high than keep alive, you
 *  could lost connection because keep alive is not sent.
 *  
 *  Furthermore, device could be wake up if ethernet traffic is detected. Ethernet
 *  shield Interrupt is enabled (ensure you have soldered ethernet shield /INT to INT0 or INT1).
 *  
 *  For testing purpose, every minute, a packet will be publish to test/period 
 *  topic. If you want to test ethernet interruption, you could publish any message
 *  through test/receive topic.
 * 
 *  
 *  by Jose Manuel Perez
 *  https://github.com/jmpmscorp *  
 */

#include <Ethernet.h>
#include <utility/w5100.h>
#include <MQTT.h>

#include <Wire.h>
#include "RTClib.h"   // From https://github.com/adafruit/RTClib

#include <avr/sleep.h>
#include <avr/wdt.h>

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
byte ip[] = {192, 168, 1, 66};  // <- change to match your network

const char * mqttBrokerAddress = "192.168.1.65";
const char * mqttClientId = "arduino-ethernet";
const char * mqttUsername = "";
const char * mqttPassword = "";

EthernetClient net;
MQTTClient client;
RTC_DS1307 rtc;

volatile bool ethShieldInterruptFlag = false;
volatile bool wdtInterruptFlag = false;
bool didWakeup = true;

unsigned long mqttLastActivityMillis = 0;

void connect() {
  Serial.print("connecting...");
  while (!client.connect(mqttClientId, mqttUsername, mqttPassword)) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nconnected!");

  client.subscribe("test/receive");
}

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
}

void setup() {
  wdt_disable();
  Serial.begin(19200);
  Ethernet.begin(mac, ip);
  while(!rtc.begin()) {
    Serial.println(F("Couldn't find RTC"));
    delay(2500);
  }

  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  Serial.println(F("Start!"));
  
  // Note: Local domain names (e.g. "Computer.local" on OSX) are not supported by Arduino.
  // You need to set the IP address directly.
  client.begin(mqttBrokerAddress, net);
  client.setClockSource(customMillis);
  client.setOptions(30, true, 1500);
  client.onMessage(messageReceived);
  
  // enable interrupts for Socket 0
  W5100.writeIMR(0x01);
  attachInterrupt(digitalPinToInterrupt(2), [](){ ethShieldInterruptFlag = true; }, FALLING );
  
  connect();
  enableWdt(WDTO_8S);
}

void loop() {
  if(wdtInterruptFlag) {
    wdt_reset();
    // Clear WDT Int Flag
    WDTCSR |= _BV(WDIE);
    wdtInterruptFlag = false;
  }
  
  client.loop();

  if (!client.connected()) {
    connect();
  }
  else {
    periodicallySent();
  }

  if(ethShieldInterruptFlag) {
    ethShieldInterruptFlag = false;
    //Serial.println("Ethernet Shield Interrupt");
    uint8_t irState = W5100.readSnIR(0);
    //Serial.println(irState, HEX);
    W5100.writeSnIR(0, irState);
    mqttLastActivityMillis = customMillis();
  }

  if(customMillis() - mqttLastActivityMillis > 2500) {
    Serial.println(F("To Sleep!"));
    Serial.flush();
    enterSleep();
  }
}

void periodicallySent() {
  static unsigned long lastSentMillis = 0;

  if(customMillis() - lastSentMillis > 60000) {
    if(client.publish("test/period", "Periodically packet")) {
      lastSentMillis = customMillis();
    }    
  }
}

uint32_t customMillis() {
  static uint32_t offset = 0;

  if(didWakeup) {
    offset = rtcMillis() - millis();
    didWakeUp = false;
  }

  return millis() + offset;
}

uint32_t rtcMillis() {
  return rtc.now().secondstime() * 1000; //We return epoch time in milliseconds
}

void enterSleep() {
  ADCSRA &= ~_BV(ADEN);

  set_sleep_mode(SLEEP_MODE_STANDBY);
  noInterrupts();
  sleep_enable();
  interrupts();
  sleep_mode();

  sleep_disable();
  

  ADCSRA |= _BV(ADEN);

  doAfterSleep();
}

void doAfterSleep() {
  didWakeup = true;
  Serial.println(F("Wake Up!"));
}

void enableWdt(uint16_t period) {
    #ifdef ARDUINO_ARCH_AVR

    // Both WDE and WDIE
    __asm__ __volatile__ (  \
        "in __tmp_reg__,__SREG__" "\n\t"    \
        "cli" "\n\t"    \
        "wdr" "\n\t"    \
        "sts %0,%1" "\n\t"  \
        "out __SREG__,__tmp_reg__" "\n\t"   \
        "sts %0,%2" "\n\t" \
        : /* no outputs */  \
        : "M" (_SFR_MEM_ADDR(_WD_CONTROL_REG)), \
          "r" (_BV(_WD_CHANGE_BIT) | _BV(WDE)), \
          "r" ((uint8_t) (((period & 0x08) ? _WD_PS3_MASK : 0x00) | \
              _BV(WDE) | _BV(WDIE) | (period & 0x07)) ) \
        : "r0"  \
    );
  #endif
}

ISR(WDT_vect) {
  wdtInterruptFlag = true;
}
