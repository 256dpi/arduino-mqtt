// This example uses an Adafruit Huzzah ESP8266
// to connect to shiftr.io.
//
// You can check on your device after a successful
// connection here: https://shiftr.io/try.
//
// by Joël Gähwiler
// https://github.com/256dpi/arduino-mqtt

#include <ESP8266WiFi.h>
#include <MQTTClient.h>

const char *ssid = "ssid";
const char *pass = "password";

WiFiClientSecure net;
MQTTClient client;

unsigned long lastMillis = 0;

void connect(); // <- predefine connect() for setup()

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, pass);
  client.begin("domain.com",8883, net); // MQTT broker documents to use port 8883 for SSL, change as necessary

  connect();
}

void connect() {
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.print("\nconnecting...");
  while (!client.connect("arduino", "admin", "admin")) // order: clientid,username,password; change as necessary
{
    Serial.print(".");
  }

  Serial.println("\nconnected!");

  client.subscribe("/example/+/+");
  // client.unsubscribe("/example");
}

void loop() {
  client.loop();
  delay(10); // <- fixes some issues with WiFi stability

  if(!client.connected()) {
    connect();
  }

  // publish a message roughly every second.
  if(millis() - lastMillis > 1000) {
    lastMillis = millis();
    client.publish("/hello", "world");
    Serial.println("tets");
  }
}

void messageReceived(String topic, String payload, char * bytes, unsigned int length) {
  Serial.print("incoming: ");
  Serial.print(topic);
  Serial.print(" - ");
  Serial.print(payload);
  Serial.println();
}
