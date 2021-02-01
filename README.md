# arduino-mqtt

[![Build Status](https://travis-ci.org/256dpi/arduino-mqtt.svg?branch=master)](https://travis-ci.org/256dpi/arduino-mqtt)
[![GitHub release](https://img.shields.io/github/release/256dpi/arduino-mqtt.svg)](https://github.com/256dpi/arduino-mqtt/releases)

This library bundles the [lwmqtt](https://github.com/256dpi/lwmqtt) MQTT 3.1.1 client and adds a thin wrapper to get an Arduino like API.

Download the latest version from the [release](https://github.com/256dpi/arduino-mqtt/releases) section. Or even better use the builtin Library Manager in the Arduino IDE and search for "MQTT".

The library is also available on [PlatformIO](https://platformio.org/lib/show/617/MQTT). You can install it by running: `pio lib install "MQTT"`. 

## Compatibility

The following examples show how you can use the library with various Arduino compatible hardware:

- [Arduino Yun & Yun-Shield](https://github.com/256dpi/arduino-mqtt/blob/master/examples/ArduinoYun/ArduinoYun.ino) ([Secure](https://github.com/256dpi/arduino-mqtt/blob/master/examples/ArduinoYunSecure/ArduinoYunSecure.ino))
- [Arduino Ethernet Shield](https://github.com/256dpi/arduino-mqtt/blob/master/examples/ArduinoEthernetShield/ArduinoEthernetShield.ino)    
- [Arduino WiFi Shield](https://github.com/256dpi/arduino-mqtt/blob/master/examples/ArduinoWiFiShield/ArduinoWiFiShield.ino)
- [Adafruit HUZZAH ESP8266](https://github.com/256dpi/arduino-mqtt/blob/master/examples/AdafruitHuzzahESP8266/AdafruitHuzzahESP8266.ino) ([Secure](https://github.com/256dpi/arduino-mqtt/blob/master/examples/AdafruitHuzzahESP8266Secure/AdafruitHuzzahESP8266Secure.ino))
- [Arduino/Genuino WiFi101 Shield](https://github.com/256dpi/arduino-mqtt/blob/master/examples/ArduinoWiFi101/ArduinoWiFi101.ino) ([Secure](https://github.com/256dpi/arduino-mqtt/blob/master/examples/ArduinoWiFi101Secure/ArduinoWiFi101Secure.ino))
- [Arduino MKR GSM 1400](https://github.com/256dpi/arduino-mqtt/blob/master/examples/ArduinoMKRGSM1400/ArduinoMKRGSM1400.ino) ([Secure](https://github.com/256dpi/arduino-mqtt/blob/master/examples/ArduinoMKRGSM1400Secure/ArduinoMKRGSM1400Secure.ino))
- [ESP32 Development Board](https://github.com/256dpi/arduino-mqtt/blob/master/examples/ESP32DevelopmentBoard/ESP32DevelopmentBoard.ino) ([Secure](https://github.com/256dpi/arduino-mqtt/blob/master/examples/ESP32DevelopmentBoardSecure/ESP32DevelopmentBoardSecure.ino))

Other shields and boards should also work if they provide a [Client](https://www.arduino.cc/en/Reference/ClientConstructor) based network implementation.

**Check out the [Wiki](https://github.com/256dpi/arduino-mqtt/wiki) to find more examples.**

## Notes

- The maximum size for packets being published and received is set by default to 128 bytes. To change the buffer sizes, you need to use `MQTTClient client(256)` instead of just `MQTTClient client` on the top of your sketch. The passed value denotes the read and write buffer size.

- On the ESP8266 it has been reported that an additional `delay(10);` after `client.loop();` fixes many stability issues with WiFi connections.

- To use the library with shiftr.io, you need to provide the token key (username) and token secret (password) as the second and third argument to `client.connect(name, key, secret)`. 

## Example

The following example uses an Arduino MKR1000 to connect to the public shiftr.io instance. You can check on your device after a successful connection here: https://www.shiftr.io/try.

```c++
#include <SPI.h>
#include <WiFi101.h>
#include <MQTT.h>

const char ssid[] = "ssid";
const char pass[] = "pass";

WiFiClient net;
MQTTClient client;

unsigned long lastMillis = 0;

void connect() {
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.print("\nconnecting...");
  while (!client.connect("arduino", "public", "public")) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nconnected!");

  client.subscribe("/hello");
  // client.unsubscribe("/hello");
}

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);

  // Note: Do not use the client in the callback to publish, subscribe or
  // unsubscribe as it may cause deadlocks when other things arrive while
  // sending and receiving acknowledgments. Instead, change a global variable,
  // or push to a queue and handle it in the loop after calling `client.loop()`.
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, pass);

  // Note: Local domain names (e.g. "Computer.local" on OSX) are not supported
  // by Arduino. You need to set the IP address directly.
  client.begin("public.cloud.shiftr.io", net);
  client.onMessage(messageReceived);

  connect();
}

void loop() {
  client.loop();

  if (!client.connected()) {
    connect();
  }

  // publish a message roughly every second.
  if (millis() - lastMillis > 1000) {
    lastMillis = millis();
    client.publish("/hello", "world");
  }
}
```

## API

Initialize the object using the hostname of the broker, the brokers port (default: `1883`) and the underlying Client class for network transport:

```c++
void begin(Client &client);
void begin(const char hostname[], Client &client);
void begin(const char hostname[], int port, Client &client);
void begin(IPAddress address, Client &client);
void begin(IPAddress address, int port, Client &client);
```

- Specify port `8883` when using secure clients for encrypted connections.
- Local domain names (e.g. `Computer.local` on OSX) are not supported by Arduino. You need to set the IP address directly.

The hostname and port can also be changed after calling `begin()`:

```c++
void setHost(const char hostname[]);
void setHost(const char hostname[], int port);
void setHost(IPAddress address);
void setHost(IPAddress address, int port);
```

Set a will message (last testament) that gets registered on the broker after connecting. `setWill()` has to be called before calling `connect()`:

```c++
void setWill(const char topic[]);
void setWill(const char topic[], const char payload[]);
void setWill(const char topic[], const char payload[], bool retained, int qos);
void clearWill();
```

Register a callback to receive messages:

```c++
void onMessage(MQTTClientCallbackSimple);
// Callback signature: void messageReceived(String &topic, String &payload) {}

void onMessage(MQTTClientCallbackSimpleFunction cb);
// Callback signature: std::function<void(String &topic, String &payload)>

void onMessageAdvanced(MQTTClientCallbackAdvanced);
// Callback signature: void messageReceived(MQTTClient *client, char topic[], char bytes[], int length) {}

void onMessageAdvanced(MQTTClientCallbackAdvancedFunction cb);
// Callback signature: std::function<void(MQTTClient *client, char topic[], char bytes[], int length)>
```

- The set callback is mostly called during a call to `loop()` but may also be called during a call to `subscribe()`, `unsubscribe()` or `publish() // QoS > 0` if messages have been received before receiving the required acknowledgement. Therefore, it is strongly recommended to not call `subscribe()`, `unsubscribe()` or `publish() // QoS > 0` directly in the callback.
- In case you need a reference to an object that manages the client, use the `void * ref` property on the client to store a pointer, and access it directly from the advanced callback.
- If the platform supports `<functional>` you can directly register a function wrapper.

Set more advanced options:

```c++
void setKeepAlive(int keepAlive);
void setCleanSession(bool cleanSession);
void setTimeout(int timeout);
void setOptions(int keepAlive, bool cleanSession, int timeout);
```

- The `keepAlive` option controls the keep alive interval in seconds (default: 10).
- The `cleanSession` option controls the session retention on the broker side (default: true).
- The `timeout` option controls the default timeout for all commands in milliseconds (default: 1000).

Set a custom clock source "custom millis" callback to enable deep sleep applications:

```c++
void setClockSource(MQTTClientClockSource);
// Callback signature: uint32_t clockSource() {}
```

- The specified callback is used by the internal timers to get a monotonic time in milliseconds. Since the clock source for the built-in `millis` is stopped when the the Arduino goes into deep sleep, you need to provide a custom callback that first syncs with a built-in or external Real Time Clock (RTC). You can pass `NULL` to reset to the default implementation.

Connect to broker using the supplied client id and an optional username and password:

```c++
bool connect(const char clientID[], bool skip = false);
bool connect(const char clientID[], const char username[], bool skip = false);
bool connect(const char clientID[], const char username[], const char password[], bool skip = false);
```

- If the `skip` option is set to true, the client will skip the network level connection and jump to the MQTT level connection. This option can be used in order to establish and verify TLS connections manually before giving control to the MQTT client. 
- The functions return a boolean that indicates if the connection has been established successfully (true).

Publishes a message to the broker with an optional payload:

```c++
bool publish(const String &topic);
bool publish(const char topic[]);
bool publish(const String &topic, const String &payload);
bool publish(const String &topic, const String &payload, bool retained, int qos);
bool publish(const char topic[], const String &payload);
bool publish(const char topic[], const String &payload, bool retained, int qos);
bool publish(const char topic[], const char payload[]);
bool publish(const char topic[], const char payload[], bool retained, int qos);
bool publish(const char topic[], const char payload[], int length);
bool publish(const char topic[], const char payload[], int length, bool retained, int qos);
```

- The functions return a boolean that indicates if the publish has been successful (true).

Subscribe to a topic:

```c++
bool subscribe(const String &topic);
bool subscribe(const String &topic, int qos); 
bool subscribe(const char topic[]);
bool subscribe(const char topic[], int qos);
```

- The functions return a boolean that indicates if the subscribe has been successful (true).

Unsubscribe from a topic:

```c++
bool unsubscribe(const String &topic);
bool unsubscribe(const char topic[]);
```

- The functions return a boolean that indicates if the unsubscribe has been successful (true).

Sends and receives packets:

```c++
bool loop();
```

- This function should be called in every `loop`.
- The function returns a boolean that indicates if the loop has been successful (true).

Check if the client is currently connected:

```c++
bool connected();
```

Access low-level information for debugging:

```c++
lwmqtt_err_t lastError();
lwmqtt_return_code_t returnCode();
```

- The error codes can be found [here](https://github.com/256dpi/lwmqtt/blob/master/include/lwmqtt.h#L15).
- The return codes can be found [here](https://github.com/256dpi/lwmqtt/blob/master/include/lwmqtt.h#L260).

Disconnect from the broker:

```c++
bool disconnect();
```

- The function returns a boolean that indicates if the disconnect has been successful (true).

## Release Management

- Update version in `library.properties`.
- Create release on GitHub.
