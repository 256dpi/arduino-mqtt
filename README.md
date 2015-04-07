# arduino-mqtt

**paho mqtt library wrapper for arduino**

This library bundles the [Embedded MQTT C/C++ Client](https://eclipse.org/paho/clients/c/embedded/) library of the eclipse paho project and adds a thin wrapper to get an Arduino like API.

The first release of the library only supports QoS0 and the basic features to get going. In the next releases more of the features will be available.

This library is an alternative to the [pubsubclient](https://github.com/knolleary/pubsubclient) library by [knolleary](https://github.com/knolleary) which only supports QoS0 and uses a custom protocol implementation.

[Download version 1.3.2 of the library.](https://github.com/256dpi/arduino-mqtt/releases/download/v1.3.2/mqtt.zip)

## Compatibility

This library has been only tested on the **Arduino Yùn** yet. Other boards and shields should work if they properly extend the Client API.

## Example

```c++
#include <Bridge.h>
#include <YunClient.h>
#include <MQTTClient.h>

YunClient net;
MQTTClient client("connect.shiftr.io", 1883, net);

unsigned long lastMillis = 0;

void setup() {
  Bridge.begin();
  Serial.begin(9600);
  Serial.println("connecting...");
  if (client.connect("arduino", "demo", "demo")) {
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
```

## Configuration

```c++
// this buffer gets allocated two times to hold the outgoing and incomming message
#define MQTT_BUFFER_SIZE 64 // default 128
```
