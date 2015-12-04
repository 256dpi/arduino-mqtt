#include <Arduino.h>

template <class T>
class MQTTTest {
public:
    void run(T *client);
    void message(String topic, String payload);
private:
    T *client;
    void printResult(boolean res);
    boolean testMessage(const char * topic, const char * payload);
    boolean newMessage = false;
    boolean passedTest = false;
    String testTopic;
    String testPayload;
    boolean testConnectivity(boolean test);
};

/* Methods */

template <class T>
void MQTTTest<T>::run(T *client) {
  this->client = client;

  Serial.println("Starting tests...");

  Serial.print("[Test 1] Connect: ");
  this->printResult(this->client->connect("arduino-mqtt-test", "try", "try") && this->testConnectivity(true));

  Serial.print("[Test 2] Subscribe & Publish: ");
  this->client->subscribe("arduino-mqtt-test/topic1");
  this->client->publish("arduino-mqtt-test/topic1", "test");
  this->printResult(this->testMessage("arduino-mqtt-test/topic1", "test"));

  Serial.print("[Test 3] Unsubscribe: ");
  this->client->subscribe("arduino-mqtt-test/topic2");
  this->client->subscribe("arduino-mqtt-test/topic3");
  this->client->unsubscribe("arduino-mqtt-test/topic2");
  this->client->publish("arduino-mqtt-test/topic2", "test");
  this->client->publish("arduino-mqtt-test/topic3", "test");
  this->printResult(this->testMessage("arduino-mqtt-test/topic3", "test"));

  Serial.print("[Test 4] Disconnect: ");
  this->client->disconnect();
  this->printResult(this->testConnectivity(false));

  Serial.println("Tests finished!");
}

template <class T>
void MQTTTest<T>::message(String topic, String payload) {
  this->newMessage = true;
  this->passedTest = topic.equals(this->testTopic) && payload.equals(this->testPayload);
}

/* Helpers */

template <class T>
void MQTTTest<T>::printResult(boolean res) {
  res ? Serial.println("SUCCESS") : Serial.println("FAILED");
}

template <class T>
boolean MQTTTest<T>::testMessage(const char *topic, const char *payload) {
  this->testTopic = String(topic);
  this->testPayload = String(payload);

  while(!this->newMessage) {
    this->client->loop();
  }

  boolean ret = this->passedTest;
  this->newMessage = false;
  this->passedTest = false;

  return ret;
}

template <class T>
boolean MQTTTest<T>::testConnectivity(boolean test) {
  while(this->client->connected() != test) {
    this->client->loop();
  }

  return this->client->connected() == test;
}
