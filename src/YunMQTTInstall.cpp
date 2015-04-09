#include "YunMQTTInstall.h"

#include <Process.h>

void sendCommand(const char *name, const char * cmd, const char * param1, const char * param2) {
  Serial.print("=== ");
  Serial.print(name);
  Serial.println(" ===");

  Process p;
  p.begin(cmd);
  p.addParameter(param1);

  if(param2 != NULL) {
    p.addParameter(param2);
  }

  Serial.print("> running '");
  Serial.print(cmd);
  Serial.print(' ');
  Serial.print(param1);

  if(param2 != NULL) {
    Serial.print(' ');
    Serial.print(param2);
  }

  Serial.println("'");

  p.run();

  while (p.available() > 0) {
    char c = p.read();
    Serial.print(c);
  }

  Serial.flush();

  Serial.println("> finished!");
  Serial.println("");
}

void YunMQTTInstall() {
  Serial.println("Starting Installation!");
  Serial.println("");

  sendCommand("Update", "opkg", "update", NULL);
  sendCommand("Install 'easy_install'", "opkg", "install", "distribute");
  sendCommand("Install python SSL support", "opkg", "install", "python-openssl");
  sendCommand("Install 'pip'", "easy_install", "pip", NULL);
  sendCommand("Install 'paho-mqtt'", "pip", "install", "paho-mqtt");
  sendCommand("Download script", "wget", "https://raw.githubusercontent.com/256dpi/arduino-mqtt/yun-client/yun/client.py", NULL);

  Serial.println("Installation finished!");
}
