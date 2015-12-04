#include <Arduino.h>

template <class T>
class MQTTTest {
    T *client;
public:
    void run(T *client);
    void loop();
    void message(String topic, String payload);
};

template <class T>
void MQTTTest<T>::run(T *client) {
  this->client = client;
}

template <class T>
void MQTTTest<T>::loop() {
  this->client->loop();
}

template <class T>
void MQTTTest<T>::message(String topic, String payload) {
  Serial.println(topic);
  Serial.println(payload);
}
