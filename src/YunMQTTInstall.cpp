#include "YunMQTTInstall.h"

#include <Process.h>

void sendCommand(const char *name, const char * cmd) {
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

void YunMQTTInstall() {
  Serial.println("Starting Installation!\n");

  sendCommand("Update", "opkg update");
  sendCommand("Install 'easy_install'", "opkg install distribute");
  sendCommand("Install python SSL support", "opkg install python-openssl");
  sendCommand("Install 'pip'", "easy_install pip");
  sendCommand("Install 'paho-mqtt'", "pip install paho-mqtt");
  sendCommand("Download script", "wget https://raw.githubusercontent.com/256dpi/arduino-mqtt/yun-client/yun/client.py --no-check-certificate -O /usr/client.py");

  Serial.println("Installation finished!");
}
