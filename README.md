# arduino-mqtt

**paho mqtt library wrapper for arduino**

## Download

[Download version 0.1 of the library.](https://github.com/256dpi/arduino-mqtt/archive/master.zip)

## Example

```cplusplus
#include <Bridge.h>
#include <YunClient.h>
#include <MQTTClient.h>

void callback(String topic, String payload) {
  Serial.println(String("incoming: ") + payload);
}

YunClient net;
MQTTClient client("connect.shiftr.io", 1883, callback, net);

void setup() {
  Serial.begin(9600);
  Bridge.begin();
  if (client.connect("arduino", "demo", "demo")) {
    client.publish("/topic", "Hello world!");
    client.subscribe("/another/topic");
  }
}

void loop() {
  client.loop();
}
```