// This example uses an Arduino MKR NB 1500 board
// to connect to shiftr.io.
//
// You can check on your device after a successful
// connection here: https://www.shiftr.io/try.
//
// by Max Kriegleder
// https://github.com/256dpi/arduino-mqtt

#include <MKRNB.h>
#include <MQTT.h>

const char pin[]      = "";
const char apn[]      = "apn";
const char login[]    = "login";
const char password[] = "password";

NBClient net;
NB nbAccess;
MQTTClient client;

unsigned long lastMillis = 0;

void connect() {
  // connection state
  bool connected = false;

  Serial.print("connecting to cellular network ...");

  while (!connected) {
    if ((nbAccess.begin(pin, apn, login, password) == NB_READY)) {
      connected = true;
    } else {
      Serial.print(".");
      delay(1000);
    }
  }

  Serial.print("\nconnecting...");
  while (!client.connect("arduino", "public", "public")) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nconnected!");

  client.subscribe("/hello");
  // client.unsubscribe("/hello");
}

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);

  // Note: Do not use the client in the callback to publish, subscribe or
  // unsubscribe as it may cause deadlocks when other things arrive while
  // sending and receiving acknowledgments. Instead, change a global variable,
  // or push to a queue and handle it in the loop after calling `client.loop()`.
}

void setup() {
  Serial.begin(115200);

  // Note: Local domain names (e.g. "Computer.local" on OSX) are not supported
  // by Arduino. You need to set the IP address directly.
  client.begin("public.cloud.shiftr.io", net);
  client.onMessage(messageReceived);

  connect();
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
