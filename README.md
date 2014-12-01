# arduino-mqtt

**paho mqtt library wrapper for arduino**

## Download

[Download version 0.1 of the library.](https://github.com/256dpi/arduino-mqtt/archive/master.zip)

## Example

```c++
#include <Bridge.h>
#include <YunClient.h>
#include <MQTTClient.h>

YunClient net;
MQTTClient client("connect.shiftr.io", 1883, net);

void setup() {
  Bridge.begin();
  Serial.begin(9600);
  Serial.println("connecting...");
  if (client.connect("arduino", "demo", "demo")) {
    Serial.println("connected!");
    client.publish("/topic", "Hello world!");
    client.subscribe("/another/topic");
  } else {
    Serial.println("not connected!");
  }
}

void loop() {
  client.loop();
}

void messageReceived(String topic, String payload) {
  Serial.println(String("incoming: ") + payload);
}
```