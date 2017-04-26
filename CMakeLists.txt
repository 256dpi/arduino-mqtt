# Uncompilable CMake File to enable project editing with CLion IDE

cmake_minimum_required(VERSION 2.8.4)
project(arduino-mqtt)

include_directories(/Applications/Arduino.app/Contents/Java/hardware/arduino/avr/cores/arduino/)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(src/lwmqtt)

set(SOURCE_FILES
    src/MQTTClient.cpp
    src/MQTTClient.h
    src/MQTTTest.h
    src/Network.cpp
    src/Network.h
    src/Timer.cpp
    src/Timer.h
    src/system.c
    src/system.h)

add_executable(arduino-mqtt ${SOURCE_FILES})
