#!/bin/bash

git clone git://git.eclipse.org/gitroot/paho/org.eclipse.paho.mqtt.embedded-c.git ./lib

cp -r ./lib/MQTTPacket/src/* ./src/lib
cp -r ./lib/MQTTClient/src/* ./src/lib

rm -rf ./src/lib/arduino
rm -rf ./src/lib/linux
rm -rf ./src/lib/mbed
rm ./src/lib/*Server.c

rm -rf ./lib
