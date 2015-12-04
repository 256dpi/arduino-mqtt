#include <Bridge.h>
#include <YunMQTTClient.h>
#include <MQTTTest.h>

YunMQTTClient client;
MQTTTest<YunMQTTClient> test;

void setup() {
  Bridge.begin();
  Serial.begin(9600);

  client.begin("broker.shiftr.io");
  test.run(&client);
}

void loop() {
  test.loop();
}

void messageReceived(String topic, String payload, char * bytes, unsigned int length) {
  test.message(topic, payload);
}
