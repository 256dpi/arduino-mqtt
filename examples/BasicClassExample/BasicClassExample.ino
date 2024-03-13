/*
* Basic example of using the library arduino-mqtt by 256dpi as a class
* 
* Edit platformio.ini to match your board. By default it's set to be 
* used with an Adafruit Feather Esp32-S3 with no psram.
* 
* Edit the server settings in Connectivity.h to the Broker/server you
* want to use. There's more details in the README.md file.
*/

#include <Arduino.h>
#include "Connectivity.h"

Connectivity connection;

void setup() {
  Serial.begin(115200);
  delay(3000); // Give Serial a little time to connect
  connection.begin();
}

void loop() {
  connection.loop(); // Stay connected
}
