#include <Bridge.h>

void setup() {
  Bridge.begin();
  Serial.begin(9600);

  while (!Serial);
  delay(1000);

  Serial.println("Starting Installation!\n");

  run("Create directory", "mkdir -p /usr/mqtt");
  run("Download script", "wget https://raw.githubusercontent.com/256dpi/arduino-mqtt/yun-bridge/yun/mqtt.py --no-check-certificate -O /usr/mqtt/mqtt.py");
  run("Download script", "wget https://raw.githubusercontent.com/256dpi/arduino-mqtt/yun-bridge/yun/bridge.py --no-check-certificate -O /usr/mqtt/bridge.py");

  Serial.println("Installation finished!");
}

void loop() {}

void run(const char *name, const char * cmd) {
  Serial.print("=== ");
  Serial.println(name);

  Serial.print("--> running '");
  Serial.print(cmd);
  Serial.println("'");

  Process p;
  p.begin(cmd);
  p.runShellCommand(cmd);

  while (p.available() > 0) {
    char c = (char)p.read();
    Serial.print(c);
  }

  Serial.println("--> finished!\n");
}
