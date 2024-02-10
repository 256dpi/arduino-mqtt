# Uncompilable CMake File to enable project editing with CLion IDE

cmake_minimum_required(VERSION 3.13)
project(arduino-mqtt)

include_directories(
        /Applications/Arduino.app/Contents/Java/hardware/arduino/avr/cores/arduino
        /Users/256dpi/Documents/Arduino/libraries/Ethernet/src
        /Users/256dpi/Documents/Arduino/libraries/WiFi101/src
        /Users/256dpi/Documents/Arduino/libraries/MKRGSM/src
        /Users/256dpi/Documents/Arduino/libraries/MKRNB/src
        /Applications/Arduino.app/Contents/Java/libraries/Bridge/src
        /Users/256dpi/Library/Arduino15/packages/esp8266/hardware/esp8266/3.1.2/libraries/ESP8266WiFi/src
        /Users/256dpi/Library/Arduino15/packages/esp32/hardware/esp32/2.0.11/libraries/WiFi/src
        /Users/256dpi/Library/Arduino15/packages/esp32/hardware/esp32/2.0.11/libraries/WiFiClientSecure/src
        src)

include_directories(src/)

set(CMAKE_CXX_FLAGS -std=c++11)

set(SOURCE_FILES
    examples/AdafruitHuzzahESP8266/AdafruitHuzzahESP8266.ino
        examples/AdafruitHuzzahESP8266Secure/AdafruitHuzzahESP8266Secure.ino
    examples/ArduinoEthernetShield/ArduinoEthernetShield.ino
    examples/ArduinoMKRGSM1400/ArduinoMKRGSM1400.ino
        examples/ArduinoMKRGSM1400Secure/ArduinoMKRGSM1400Secure.ino
    examples/ArduinoMKRNB1500/ArduinoMKRNB1500.ino
    examples/ArduinoWiFi101/ArduinoWiFi101.ino
        examples/ArduinoWiFi101Secure/ArduinoWiFi101Secure.ino
    examples/ArduinoWiFiShield/ArduinoWiFiShield.ino
    examples/ArduinoYun/ArduinoYun.ino
        examples/ArduinoYunSecure/ArduinoYunSecure.ino
    examples/ESP32DevelopmentBoard/ESP32DevelopmentBoard.ino
        examples/ESP32DevelopmentBoardSecure/ESP32DevelopmentBoardSecure.ino
    src/lwmqtt
    src/MQTT.h
    src/MQTTClient.h
    src/MQTTClient.cpp)

add_executable(arduino-mqtt ${SOURCE_FILES})
