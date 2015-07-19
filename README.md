# arduino-mqtt

**MQTT library for Arduino based on the Eclipse Paho projects**

This library bundles the [Embedded MQTT C/C++ Client](https://eclipse.org/paho/clients/c/embedded/) library of the Eclipse Paho project and adds a thin wrapper to get an Arduino like API. Additionally there is an drop-in alternative for the Arduino Yùn that uses a python based client on the linux processor and a binary interface to lower program space usage on the Arduino side.

The first release of the library only supports QoS0 and the basic features to get going. In the next releases more of the features will be available. Please create an issue if you need a specific functionality.

This library is an alternative to the [pubsubclient](https://github.com/knolleary/pubsubclient) library by [knolleary](https://github.com/knolleary) which uses a custom protocol implementation.

[Download version 1.6.0 of the library.](https://github.com/256dpi/arduino-mqtt/releases/download/v1.6.0/mqtt.zip)

*Or even better use the newly available Library Manager in the Arduino IDE.*

## Caveats

- The maximum size for packets being published and received is set by default to 128 bytes. To change that value, you need to download the library manually and change the value in the following file: https://github.com/256dpi/arduino-mqtt/blob/master/src/MQTTClient.h#L5.

## Compatibility

This library has been officially tested on the **Arduino Yùn**. Other boards and shields should work, but may need custom initialization of the Client class.

## Example

- [Example using the alternative YunMQTTClient](https://github.com/256dpi/arduino-mqtt/blob/master/examples/YunMQTTClient/YunMQTTClient.ino).

```c++
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
    client.subscribe("/example");
    // client.unsubscribe("/example");
  } else {
    Serial.println("not connected!");
  }
}

void loop() {
  client.loop();
  // publish message roughly every second
  if(millis() - lastMillis > 1000) {
    lastMillis = millis();
    client.publish("/hello", "world");
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
