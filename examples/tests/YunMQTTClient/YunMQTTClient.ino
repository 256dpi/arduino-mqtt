#include <Bridge.h>
#include <YunMQTTClient.h>
#include <MQTTTest.h>

YunMQTTClient client;
MQTTTest<YunMQTTClient> test;

void setup() {
  Bridge.begin();
  Serial.begin(9600);

  // run a very basic automated test
  client.begin("broker.shiftr.io");
  test.run(&client);
}

void loop() {}

void messageReceived(String topic, String payload, char * bytes, unsigned int length) {
  test.message(topic, payload);
}
