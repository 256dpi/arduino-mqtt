# Uncompilable CMake File to enable project editing with CLion IDE

cmake_minimum_required(VERSION 2.8.4)
project(arduino-mqtt)

include_directories(/Applications/Arduino.app/Contents/Java/hardware/arduino/avr/cores/arduino/)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    src/MQTTClient.h
    src/MQTTTest.h
    src/system.cpp
    src/system.h)

add_executable(arduino-mqtt ${SOURCE_FILES})
