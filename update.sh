#!/bin/bash

git clone git://git.eclipse.org/gitroot/paho/org.eclipse.paho.mqtt.embedded-c.git ./lib

cp -r ./lib/MQTTPacket/src/* ./src/lib
cp -r ./lib/MQTTClient/src/* ./src/lib

rm -rf ./src/lib/arduino
rm -rf ./src/lib/linux
rm -rf ./src/lib/mbed
rm ./src/lib/*Server.c
rm ./src/lib/MQTTFormat.*
rm ./src/lib/MQTTLogging.h

sed -i '' '/#include "MQTTLogging.h"/d' ./src/lib/MQTTClient.h

rm -rf ./lib
