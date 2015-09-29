# arduino-mqtt

**MQTT library for Arduino based on the Eclipse Paho projects**

This library bundles the [Embedded MQTT C/C++ Client](https://eclipse.org/paho/clients/c/embedded/) library of the Eclipse Paho project and adds a thin wrapper to get an Arduino like API. Additionally there is an drop-in alternative for the Arduino Yùn that uses a python based client on the linux processor and a binary interface to lower program space usage on the Arduino side.

The first release of the library only supports QoS0 and the basic features to get going. In the next releases more of the features will be available. Please create an issue if you need a specific functionality.

This library is an alternative to the [pubsubclient](https://github.com/knolleary/pubsubclient) library by [knolleary](https://github.com/knolleary) which uses a custom protocol implementation.

[Download version 1.8.0 of the library.](https://github.com/256dpi/arduino-mqtt/releases/download/v1.8.0/mqtt.zip)

*Or even better use the newly available Library Manager in the Arduino IDE.*

## Caveats

- The maximum size for packets being published and received is set by default to 128 bytes. To change that value, you need to download the library manually and change the value in the following file: https://github.com/256dpi/arduino-mqtt/blob/master/src/MQTTClient.h#L5.

## Compatibility

This library has been officially tested on the **Arduino Yùn**. Other boards and shields should work, but may need custom initialization of the Client class.

Here is a list of platforms that are supported:

- [Arduino Yùn](https://www.arduino.cc/en/Main/ArduinoBoardYun)
- [ESP8266](https://github.com/esp8266/Arduino)

## Example

- [Example using the alternative YunMQTTClient](https://github.com/256dpi/arduino-mqtt/blob/master/examples/YunMQTTClient/YunMQTTClient.ino).

```c++
#include <Bridge.h>
#include <YunClient.h>
#include <MQTTClient.h>

YunClient net;
MQTTClient client;

unsigned long lastMillis = 0;

void setup() {
  Bridge.begin();
  Serial.begin(9600);
  client.begin("broker.shiftr.io", net);
  
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
  // publish a message roughly every second.
  if(millis() - lastMillis > 1000) {
    lastMillis = millis();
    client.publish("/hello", "world");
  }
}

void messageReceived(String topic, String payload, char * bytes, unsigned int length) {
  Serial.print("incoming: ");
  Serial.print(topic);
  Serial.print(" - ");
  Serial.print(payload);
  Serial.println();
}
```

## API

Initialize the object using the hostname of the broker, the brokers port (default: `1883`) and the underlying Client class for network transport:

```c++
void begin(const char * hostname, Client& client);
void begin(const char * hostname, int port, Client& client);
```

_The special`YunMQTTClient` does not need the `client` parameter._

Set the will message that gets registered on a connect:

```c++
void setWill(const char * topic);
void setWill(const char * topic, const char * payload);
```

Connect to broker using the supplied client id and an optional username and password:

```c++
boolean connect(const char * clientId);
boolean connect(const char * clientId, const char * username, const char * password);
```

_This functions returns a value that indicates if the connection has been established successfully._

Publishes a message to the broker with an optional payload:

```c++
void publish(String topic);
void publish(String topic, String payload);
void publish(const char * topic, String payload);
void publish(const char * topic, const char * payload);
```

Subscribe to a topic: 

```c++
void subscribe(String topic);
void subscribe(const char * topic);
```

Unsubscribe from a topic:

```c++
void unsubscribe(String topic);
void unsubscribe(const char * topic);
```

Sends and receives packets: 

```c++
void loop();
```

_This function should be called in every `loop`._

Check if the client is currently connected:

```c++
boolean connected();
```

Disconnects from the broker:

```c++
void disconnect();
```
