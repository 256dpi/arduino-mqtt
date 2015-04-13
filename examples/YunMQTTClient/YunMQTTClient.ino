#include <Bridge.h>
#include <YunMQTTClient.h>

YunMQTTClient client("connect.shiftr.io", 1883);

unsigned long lastMillis = 0;

void setup() {
  Bridge.begin();
  Serial.begin(9600);

  // this will install the required python files (pass true to force download)
  // can also be commented out to save program space
  client.installBridge(false);

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
