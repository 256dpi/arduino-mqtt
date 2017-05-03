# Uncompilable CMake File to enable project editing with CLion IDE

cmake_minimum_required(VERSION 2.8.4)
project(arduino-mqtt)

include_directories(/Applications/Arduino.app/Contents/Java/hardware/arduino/avr/cores/arduino/)

include_directories(src/)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    examples/AdafruitHuzzahESP8266/AdafruitHuzzahESP8266.ino
    examples/AdafruitHuzzahESP8266_SSL/AdafruitHuzzahESP8266_SSL.ino
    examples/ArduinoEthernetShield/ArduinoEthernetShield.ino
    examples/ArduinoWiFi101/ArduinoWiFi101.ino
    examples/ArduinoWiFi101_SSL/ArduinoWiFi101_SSL.ino
    examples/ArduinoWiFiShield/ArduinoWiFiShield.ino
    examples/ArduinoYun_MQTTClient/ArduinoYun_MQTTClient.ino
    examples/ESP32DevelopmentBoard/ESP32DevelopmentBoard.ino
    examples/ESP32DevelopmentBoard_SSL/ESP32DevelopmentBoard_SSL.ino
    src/MQTTClient.h
    src/system.cpp
    src/system.h)

add_executable(arduino-mqtt ${SOURCE_FILES})
