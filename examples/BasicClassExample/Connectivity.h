
#ifndef MQTT_CONNECTIVITY_h
#define MQTT_CONNECTIVITY_h

#include <WiFi.h>
#include <MQTT.h>

class Connectivity {
    public:
        Connectivity();
        void begin();
        void loop();
        void sendStatus();
        bool reconnect();
        static MQTTClientCallbackAdvancedFunction messageReceived(MQTTClient *client, char topic[], char bytes[], int length);
    private:
        // Edit your server settings here
        WiFiClient networkClient;
        String ssid =     "EnterYourSsidHere"; 
        String password = "EnterYourWifiPasswordHere";

        MQTTClient mqttClient;
        String mqttServer = "test.mosquitto.org";
        String mqttUsername = "";
        String mqttPassword = "";
        String mqttClientID = "undefined";
        int mqttPort = 1883;

        unsigned long nextStatusMessage = 0;
};

#endif // MQTT_CONNECTIVITY_h