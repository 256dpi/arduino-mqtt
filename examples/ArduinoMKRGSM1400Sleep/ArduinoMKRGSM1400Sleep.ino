/**
 *  Device will go to sleep every loop if MQTT client is connected.
 *  
 *  Device will wake up periodically by WDT, in order to, if no MQTT activity,
 *  do keep_alive if necessary. You should take care of keep alive time and wdt
 *  period configuration. If your wdt period is too much high than keep alive, you
 *  could lost connection because keep alive is not sent.
 *    
 *  For testing purpose, every minute, a packet will be publish to test/period 
 *  topic.
 *  
 * 
 *  IMPORTANT
 * 
 *  - ABOUT SERIAL PORT
 *  
 *  Due to sleep funcionality, Serial (SerialUSB) can't be used for debugging.
 *  When board goes to sleep, SerialUSB is dettach and COM port is cleared, so
 *  next time board wakes up you won't see anything on your open port.
 *  In order to allow debug way, you could use Serial1 (pin 13, 14) with any
 *  UART to USB converter to see debug messages.
 *  
 *  You will find a #define debugSerial when you can change debug port.
 *  
 * 
 *  - ABOUT RECEPTION:
 *  
 *  If you read the code, you will see that MQTT client is subscribed to the topic
 *  test/receive. It's done for testing messages reception.
 *  If you test it, you will notice that if you publish on this topic through any
 *  other tool, you won't see message until WDT wakes up the board, so, MQTT
 *  reception don't wake up it. Why?
 *  
 *  1. Ublox chip could be configured to raise its RI(ring indicator) pin when it
 *     receives calls, sms, but also, when it recevies data through sockets.
 *     We could connect this pin to one of our SAMD chip and attach an interrupt
 *     to wake up. However, if you take a look on MKR schematic, you will notice 
 *     that Ublox RI is not connected to any SAMD chip. We can't use this technique.
 * 
 *  2. I know that SAMD chip can wake up from sleep mode when SERCOM (USART) 
 *     data is available. I was looking for SAMD registers configuration to achieve
 *     this funcionality over arduino forum, atmel forum, but I haven't found any
 *     information/example.
 *     If anyone knows how to achieve, please, I will be grateful if you show me any
 *     example.  
 *  
 * 
 *  
 *  by Jose Manuel Perez
 *  https://github.com/jmpmscorp *  
 */
#include <MKRGSM.h>
#include <MQTT.h>
#include "RTCZero.h"
#include "samd_wdt.h"

#define debugSerial Serial1

const char * pin      = "";
const char * apn      = "orangeworld";
const char * login    = "";
const char * password = "";

const char * mqttBrokerAddress = "antaresserver.ddns.net";
const char * mqttClientId = "mkrgsm1400";
const char * mqttUsername = "";
const char * mqttPassword = "";

RTCZero rtc;

GSMClient net;
GPRS gprs;
GSM gsmAccess;
MQTTClient client;

unsigned long lastMillis = 0;
bool didWakeUp = true;

void connect() {
  // connection state
  bool connected = false;

  // After starting the modem with gsmAccess.begin()
  // attach to the GPRS network with the APN, login and password
  while (!connected) {
    if (gsmAccess.begin(pin) == GSM_READY) {
        debugSerial.println("Trying to connect to cellular network...");
        // Reset WDT to prevent board reset if attachGPRS take too much time.
        resetWdt();
        if (gprs.attachGPRS(apn, login, password) == GPRS_READY) {
            connected = true;
        }        
    } else {
      debugSerial.print(".");
      delay(1000);
    }
    // Check if WDT must be reset
    checkWdt();
  }

  debugSerial.print("\nConnecting to MQTT Broker...");
  while (!client.connect(mqttClientId, mqttUsername, mqttPassword)) {
    debugSerial.print(".");
    delay(1000);
  }

  debugSerial.println("\nconnected!");

  client.subscribe("test/receive");
}

void messageReceived(String &topic, String &payload) {
  debugSerial.println("incoming: " + topic + " - " + payload);
}

void setup() {
  disableWdt();
  debugSerial.begin(115200);
  
  rtc.begin();

  // Note: Local domain names (e.g. "Computer.local" on OSX) are not supported by Arduino.
  // You need to set the IP address directly.
  client.begin(mqttBrokerAddress, net);
  client.setOptions(30, true, 1000);
  client.setClockSource(customMillis);
  client.onMessage(messageReceived);

  // Config SAMD Sleep Mode
  SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

  connect();
  enableWdt(WDT_PERIOD_4X);
}

void loop() {
  
  checkWdt();

  client.loop();  

  if (!client.connected()) {
    connect();
  }
  else {
      periodicallySent();
  }

  if(client.connected()) {
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

void checkWdt() {
    if(samdWdtFlag) {
      samdWdtFlag = false;
      resetWdt();
    } 
}

uint32_t customMillis() {
    static uint32_t offset = 0;

    if (didWakeUp) {
        offset = rtcMillis() - millis();
        didWakeUp = false;
    }

    return millis() + offset;
}

uint32_t rtcMillis() {
    return rtc.getEpoch() * 1000;
}

void enterSleep() {
    if(!samdWdtFlag) {
        __WFI();
    }

    doAfterSleep();
}

void doAfterSleep() {
    didWakeUp = true;
    debugSerial.println("Wake up!");
}
