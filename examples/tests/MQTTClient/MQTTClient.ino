#include <Bridge.h>
#include <YunClient.h>
#include <MQTTClient.h>
#include <MQTTTest.h>

YunClient net;
MQTTClient client;
MQTTTest<MQTTClient> test;

void setup() {
  Bridge.begin();
  Serial.begin(9600);

  client.begin("broker.shiftr.io", net);
  test.run(&client);
}

void loop() {
  test.loop();
}

void messageReceived(String topic, String payload, char * bytes, unsigned int length) {
  test.message(topic, payload);
}
