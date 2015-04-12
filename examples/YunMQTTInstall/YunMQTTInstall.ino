#include <Bridge.h>
#include <YunMQTTInstall.h>

void setup() {
  Bridge.begin();
  Serial.begin(9600);

  while (!Serial);
  delay(1000);

  YunMQTTInstall();
}

void loop() {}
