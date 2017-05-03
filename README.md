# arduino-mqtt

[![Build Status](https://travis-ci.org/256dpi/arduino-mqtt.svg?branch=master)](https://travis-ci.org/256dpi/arduino-mqtt)

**MQTT library for Arduino based on the lwmqtt project**

This library bundles the [lwmqtt](https://github.com/256dpi/lwmqtt) client and adds a thin wrapper to get an Arduino like API.

Download the latest version from the [release](https://github.com/256dpi/arduino-mqtt/releases) section.

*Or even better use the builtin Library Manager in the Arduino IDE and search for "MQTT".*

## Compatibility

The following examples show how you can use the library with various Arduino compatible hardware:

- [Arduino Yun & Yun-Shield](https://github.com/256dpi/arduino-mqtt/blob/master/examples/ArduinoYun_MQTTClient/ArduinoYun_MQTTClient.ino)
- [Arduino Ethernet Shield](https://github.com/256dpi/arduino-mqtt/blob/master/examples/ArduinoEthernetShield/ArduinoEthernetShield.ino)
- [Arduino WiFi Shield](https://github.com/256dpi/arduino-mqtt/blob/master/examples/ArduinoWiFiShield/ArduinoWiFiShield.ino)
- [Adafruit HUZZAH ESP8266](https://github.com/256dpi/arduino-mqtt/blob/master/examples/AdafruitHuzzahESP8266/AdafruitHuzzahESP8266.ino) ([SSL](https://github.com/256dpi/arduino-mqtt/blob/master/examples/AdafruitHuzzahESP8266_SSL/AdafruitHuzzahESP8266_SSL.ino))
- [Arduino/Genuino WiFi101 Shield](https://github.com/256dpi/arduino-mqtt/blob/master/examples/ArduinoWiFi101/ArduinoWiFi101.ino) ([SSL](https://github.com/256dpi/arduino-mqtt/blob/master/examples/ArduinoWiFi101_SSL/ArduinoWiFi101_SSL.ino))
- [ESP32 Development Board](https://github.com/256dpi/arduino-mqtt/blob/master/examples/ESP32DevelopmentBoard/ESP32DevelopmentBoard.ino) ([SSL](https://github.com/256dpi/arduino-mqtt/blob/master/examples/ESP32DevelopmentBoard_SSL/ESP32DevelopmentBoard_SSL.ino))

Other shields and boards should also work if they provide a [Client](https://www.arduino.cc/en/Reference/ClientConstructor) based network implementation.

## Caveats

- The maximum size for packets being published and received is set by default to 128 bytes. To change the buffer sizes, you need to use `AdvancedMQTTClient<256> client` instead of just `MQTTClient client` on the top of your sketch. The value in the angle brackets denotes the buffer sizes.

- On the ESP8266 it has been reported that an additional `delay(10);` after `client.loop();` fixes many stability issues with WiFi connections.

## API

Initialize the object using the hostname of the broker, the brokers port (default: `1883`) and the underlying Client class for network transport:

```c++
void begin(Client& client);
void begin(const char hostname[], Client &client);
void begin(const char hostname[], int port, Client &client);
```

- Specify port `8883` when using SSL clients for secure connections.
- Local domain names (e.g. `Computer.local` on OSX) are not supported by Arduino. You need to set the IP address directly.

The hostname and port can also be changed after calling `begin()`:

```c++
void setHost(const char hostname[]);
void setHost(const char hostname[], int port);
```

Set a will message (last testament) that gets registered on the broker after connecting:

```c++
void setWill(const char topic[]);
void setWill(const char topic[], const char payload[]);
void setWill(const char topic[], const char payload[], bool retained, int qos);
void clearWill();
```

Connect to broker using the supplied client id and an optional username and password:

```c++
boolean connect(const char clientId[]);
boolean connect(const char clientId[], const char username[], const char password[]);
```

- This functions returns a boolean that indicates if the connection has been established successfully.

Publishes a message to the broker with an optional payload:

```c++
boolean publish(String topic);
boolean publish(const char topic[]);
boolean publish(String topic, String payload);
boolean publish(String topic, String payload, bool retained, int qos);
boolean publish(const char topic[], String payload);
boolean publish(const char topic[], String payload, bool retained, int qos);
boolean publish(const char topic[], const char payload[]);
boolean publish(const char topic[], const char payload[], bool retained, int qos);
boolean publish(const char topic[], char payload[], unsigned int length);
boolean publish(const char topic[], char payload[], unsigned int length, bool retained, int qos);
```

Subscribe to a topic:

```c++
boolean subscribe(String topic);
boolean subscribe(String topic, int qos); 
boolean subscribe(const char topic[]);
boolean subscribe(const char topic[], int qos);
```

Unsubscribe from a topic:

```c++
boolean unsubscribe(String topic);
boolean unsubscribe(const char topic[]);
```

Sends and receives packets:

```c++
boolean loop();
```

- This function should be called in every `loop`.

Check if the client is currently connected:

```c++
boolean connected();
```

Access low-level information for debugging:

```c++
lwmqtt_err_t lastError();
lwmqtt_return_code_t returnCode();
```

Disconnect from the broker:

```c++
boolean disconnect();
```
