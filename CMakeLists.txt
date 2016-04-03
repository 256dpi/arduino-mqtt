# Uncompilable CMake File to enable project editing with CLion IDE

cmake_minimum_required(VERSION 2.8.4)
project(arduino_mqtt)

include_directories(/Applications/Arduino.app/Contents/Java/hardware/arduino/avr/cores/arduino/)
include_directories(/Applications/Arduino.app/Contents/Java/libraries/Bridge/src)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    src/lib/FP.h
    src/lib/MQTTClient.h
    src/lib/MQTTConnect.h
    src/lib/MQTTConnectClient.c
    src/lib/MQTTDeserializePublish.c
    src/lib/MQTTPacket.c
    src/lib/MQTTPacket.h
    src/lib/MQTTPublish.h
    src/lib/MQTTSerializePublish.c
    src/lib/MQTTSubscribe.h
    src/lib/MQTTSubscribeClient.c
    src/lib/MQTTUnsubscribe.h
    src/lib/MQTTUnsubscribeClient.c
    src/MQTTClient.cpp
    src/MQTTClient.h
    src/MQTTTest.h
    src/Network.cpp
    src/Network.h
    src/Timer.cpp
    src/Timer.h
    src/YunMQTTClient.cpp
    src/YunMQTTClient.h)

set(ARDUINO_AVR_YUN 1)

add_executable(arduino_mqtt ${SOURCE_FILES})
