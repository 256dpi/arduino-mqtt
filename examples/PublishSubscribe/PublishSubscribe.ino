#include <Bridge.h>
#include <YunClient.h>
#include <MQTTClient.h>

void callback(String topic, String payload) {
  Serial.println(String("incoming: ") + payload);
}

YunClient net;
MQTTClient client("connect.shiftr.io", 1883, callback, net);

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
