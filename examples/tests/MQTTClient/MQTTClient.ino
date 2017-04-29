#include <Bridge.h>
#include <YunClient.h>
#include <MQTTClient.h>
#include <MQTTTest.h>

YunClient net;
MQTTClient client;
MQTTTest<MQTTClient> test;

void setup() {
  Bridge.begin();
  Serial.begin(115200);

  // run a very basic automated test
  client.begin("broker.shiftr.io", net);
  client.onMessage(messageReceived);
  test.run(&client);
}

void loop() {}

void messageReceived(String topic, String payload, char * bytes, unsigned int length) {
  test.message(topic, payload);
}
