#include <Bridge.h>
#include <YunClient.h>
#include <MQTTClient.h>

YunClient net;
MQTTClient client("connect.shiftr.io", 1883, net);

void setup() {
  Bridge.begin();
  Serial.begin(9600);
  Serial.println("connecting...");
  if (client.connect("arduino", "demo", "demo")) {
    Serial.println("connected!");
    client.publish("/topic", "Hello world!");
    client.subscribe("/another/topic");
  } else {
    Serial.println("not connected!");
  }
}

void loop() {
  client.loop();
}

void messageReceived(String topic, char * payload, unsigned int length) {
  Serial.print("incomming: ");
  Serial.print(topic);
  Serial.print(" - ");
  Serial.write(payload, length);
  Serial.println();
}
