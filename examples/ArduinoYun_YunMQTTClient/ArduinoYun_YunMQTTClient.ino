// This example uses an Arduino Yun or a Yun-Shield
// and the YunMQTTClient to connect to shiftr.io.
//
// The YunMQTTClient uses a Linux side python
// script to manage the connection which results
// in less program space and memory used on the Arduino.
//
// You can check on your device after a successful
// connection here: https://shiftr.io/try.
//
// by Joël Gähwiler
// https://github.com/256dpi/arduino-mqtt

#include <Bridge.h>
#include <YunMQTTClient.h>

YunMQTTClient client;

unsigned long lastMillis = 0;

void setup() {
  Bridge.begin();
  SerialUSB.begin(9600);
  client.begin("broker.shiftr.io");

  connect();
}

void connect() {
  SerialUSB.print("connecting...");
  while (!client.connect("arduino", "try", "try")) {
    SerialUSB.print(".");
    delay(1000);
  }

  SerialUSB.println("\nconnected!");

  client.subscribe("/example");
  // client.unsubscribe("/example");
}

void loop() {
  client.loop();

  if (!client.connected()) {
    connect();
  }

  // publish a message roughly every second.
  if (millis() - lastMillis > 1000) {
    lastMillis = millis();
    client.publish("/hello", "world");
  }
}

void messageReceived(String topic, String payload, char * bytes, unsigned int length) {
  SerialUSB.print("incoming: ");
  SerialUSB.print(topic);
  SerialUSB.print(" - ");
  SerialUSB.print(payload);
  SerialUSB.println();
}
