#include <Bridge.h>
#include <YunMQTTClient.h>

YunMQTTClient client("broker.shiftr.io");

unsigned long lastMillis = 0;

void setup() {
  Bridge.begin();
  Serial.begin(9600);

  // This will install the required python files (pass true to force update).
  // Line can also be commented out to save program space.
  // If you update the library you also need to update the bridge!
  switch(client.installBridge(false)) {
    case 0: Serial.println("error while installing bridge!"); break;
    case 1: Serial.println("bridge already installed!"); break;
    case 2: Serial.println("bridge updated!"); break;
  }

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
  // Publish a message roughly every second.
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
