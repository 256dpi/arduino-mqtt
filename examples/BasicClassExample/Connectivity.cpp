#include "Connectivity.h"

Connectivity::Connectivity(){
    
}

void Connectivity::begin(){
    // Setup connection
    mqttClientID = WiFi.macAddress();
    mqttClient.ref = this;
    mqttClient.begin(mqttServer.c_str(), mqttPort, networkClient);
    reconnect();
}

void Connectivity::loop(){
    mqttClient.loop();

    // Stay connected
    if (!mqttClient.connected()) {
        reconnect();
    }
    
    // Send ping now and then
    if (millis() > nextStatusMessage) {
        nextStatusMessage = millis()+10000;
        sendStatus();
    }
}

bool Connectivity::reconnect(){
    if( WiFi.status() != WL_CONNECTED )
    {
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid.c_str(), password.c_str());
        while (WiFi.status() != WL_CONNECTED){
            Serial.print(".");
            delay(500);
        }
        log_i("Connected to %s", WiFi.SSID().c_str());
    }
    
    // Connect to MQTT
    mqttClient.onMessageAdvanced( messageReceived );
    bool ret = mqttClient.connect(mqttClientID.c_str(), mqttUsername.c_str(), mqttPassword.c_str());
    if( ret )
    {
        String topic = String("/device/") + mqttClientID+"/update";
        Serial.print("Subscribing to ");
        Serial.println(topic.c_str());
        mqttClient.subscribe( topic.c_str() );

        Serial.print("Open a tool like MQTT Explorer and send a message to ");
        Serial.println(topic.c_str());

        Serial.println("You should then see the message on the Serial Monitor (if you open it)");

        sendStatus();
    }
    return ret;
}

void Connectivity::sendStatus(){
    String topic = String("/device/") + mqttClientID+"/status";
    String payload = "{\"status\": \"online\"}";
    mqttClient.publish(topic.c_str(), payload.c_str());
}

// See https://github.com/256dpi/arduino-mqtt/pull/220 for details
 MQTTClientCallbackAdvancedFunction Connectivity::messageReceived(MQTTClient *client, char topic[], char bytes[], int length)
 {
    String cTopic = topic;
    String cPayload = bytes;
    Serial.print("messageReceived on topic: ");
    Serial.print(cTopic);
    Serial.print(" payload: ");
    Serial.println(cPayload);

    auto parent = (Connectivity *)client->ref;
    if( parent != nullptr )
    {
        Serial.print("RefTest: ");
        Serial.println(parent->mqttServer);
    }
    return NULL;
}
