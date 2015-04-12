#include <Bridge.h>
#include <Process.h>

void setup() {
  Bridge.begin();
  Serial.begin(9600);

  while (!Serial);
  delay(1000);

  Serial.println("Starting Installation!\n");

  run("Update", "opkg update");
  run("Install 'easy_install'", "opkg install distribute");
  run("Install python SSL support", "opkg install python-openssl");
  run("Install 'pip'", "easy_install pip");
  run("Install 'paho-mqtt'", "pip install paho-mqtt");
  run("Download script", "wget https://raw.githubusercontent.com/256dpi/arduino-mqtt/yun-client/yun/client.py --no-check-certificate -O /usr/client.py");

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
    char c = p.read();
    Serial.print(c);
  }

  Serial.println("--> finished!\n");
}
