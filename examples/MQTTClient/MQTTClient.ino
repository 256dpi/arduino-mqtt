#include <Bridge.h>
#include <YunClient.h>
#include <MQTTClient.h>

YunClient net;
MQTTClient client("broker.shiftr.io", net);

unsigned long lastMillis = 0;

void setup() {
  Bridge.begin();
  Serial.begin(9600);
  Serial.println("connecting...");
  if (client.connect("arduino", "try", "try")) {
    Serial.println("connected!");
    client.subscribe("/another/topic");
    // client.unsubscribe("/another/topic");
  } else {
    Serial.println("not connected!");
  }
}

void loop() {
  client.loop();
  // publish message roughly every second
  if(millis() - lastMillis > 1000) {
    lastMillis = millis();
    client.publish("/topic", "Hello world!");
  }
}

void messageReceived(String topic, String payload, char * bytes, unsigned int length) {
  Serial.print("incomming: ");
  Serial.print(topic);
  Serial.print(" - ");
  Serial.print(payload);
  Serial.println();
}
