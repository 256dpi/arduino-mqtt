// This example uses an Arduino Yun or a Yun-Shield
// and the YunMQTTClient to connect to shiftr.io.
//
// Note: You need to upgrade the Arduino Yun to the
// latest firmware version to have the certificates
// present.
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
  Serial.begin(115200);
  client.begin("broker.shiftr.io", 8883); // MQTT brokers usually use port 8883 for secure connections
  client.setTls("/etc/ssl/certs/AddTrust_External_Root.crt"); // select the CA for the broker

  connect();
}

void connect() {
  Serial.print("connecting...");
  while (!client.connect("arduino", "try", "try")) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nconnected!");

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
  Serial.print("incoming: ");
  Serial.print(topic);
  Serial.print(" - ");
  Serial.print(payload);
  Serial.println();
}
