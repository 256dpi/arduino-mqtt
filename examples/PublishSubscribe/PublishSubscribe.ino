#include <Bridge.h>
#include <YunClient.h>
#include <MQTTClient.h>

YunClient net;
MQTTClient client("connect.shiftr.io", 1883, net);

void setup() {
  Serial.begin(9600);
  Bridge.begin();
  if (client.connect("arduino", "demo", "demo")) {
    client.publish("/topic", "Hello world!");
    client.subscribe("/another/topic");
  }
}

void loop() {
  client.loop();
}

void messageReceived(String topic, String payload) {
  Serial.println(String("incoming: ") + payload);
}
