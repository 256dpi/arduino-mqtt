#!/bin/bash

git clone git://git.eclipse.org/gitroot/paho/org.eclipse.paho.mqtt.embedded-c.git ./lib

cp -r ./lib/MQTTPacket/src/* ./src/lib
cp -r ./lib/MQTTClient/src/* ./src/lib

# remove examples
rm -rf ./src/lib/arduino
rm -rf ./src/lib/linux
rm -rf ./src/lib/mbed

# remove unneeded server files
rm ./src/lib/*Server.c

# remove unneeded format stuff
rm ./src/lib/MQTTFormat.*

# remove the logging calls
rm ./src/lib/MQTTLogging.h
sed -i '' '/#include "MQTTLogging.h"/d' ./src/lib/MQTTClient.h

# remove all that stack trace stuff
rm ./src/lib/StackTrace.h
sed -i '' '/#include "StackTrace.h"/d' ./src/lib/MQTTConnectClient.c ./src/lib/MQTTDeserializePublish.c ./src/lib/MQTTPacket.c ./src/lib/MQTTSerializePublish.c ./src/lib/MQTTSubscribeClient.c ./src/lib/MQTTUnsubscribeClient.c
sed -i '' '/FUNC_ENTRY/d' ./src/lib/MQTTConnectClient.c ./src/lib/MQTTDeserializePublish.c ./src/lib/MQTTPacket.c ./src/lib/MQTTSerializePublish.c ./src/lib/MQTTSubscribeClient.c ./src/lib/MQTTUnsubscribeClient.c
sed -i '' 's/exit: FUNC_EXIT_RC(rc);/exit:/' ./src/lib/MQTTConnectClient.c
sed -i '' '/FUNC_EXIT/d' ./src/lib/MQTTConnectClient.c ./src/lib/MQTTDeserializePublish.c ./src/lib/MQTTPacket.c ./src/lib/MQTTSerializePublish.c ./src/lib/MQTTSubscribeClient.c ./src/lib/MQTTUnsubscribeClient.c

rm -rf ./lib
