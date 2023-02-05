all: fmt

fmt:
	clang-format -i src/*.h src/*.cpp -style="{BasedOnStyle: Google, ColumnLimit: 120}"

update:
	rm -rf ./lwmqtt
	git clone --branch v0.9.0 https://github.com/256dpi/lwmqtt.git ./lwmqtt
	mkdir -p ./src/lwmqtt
	cp -r ./lwmqtt/src/*.c ./src/lwmqtt/
	cp -r ./lwmqtt/src/*.h ./src/lwmqtt/
	cp -r ./lwmqtt/include/*.h ./src/lwmqtt/
	rm ./src/lwmqtt/posix.c
	rm -rf ./lwmqtt
	sed -i '' "s/<lwmqtt.h>/\"lwmqtt.h\"/g" ./src/lwmqtt/*

install:
	# expects arduino-cli to be installed and configured with esp8266 and esp32 cores
	arduino-cli update
	# ensure cores
	arduino-cli core install esp8266:esp8266
	arduino-cli core install esp32:esp32
	arduino-cli core install arduino:samd
	arduino-cli core install arduino:avr
	# ensure libraries
	arduino-cli lib install WiFi
	arduino-cli lib install WiFi101
	arduino-cli lib install MKRGSM
	arduino-cli lib install MKRNB
	arduino-cli lib install Ethernet
	arduino-cli lib install Bridge

test:
	# expects repository to be linked to libraries
	arduino-cli compile --fqbn "esp32:esp32:esp32:FlashFreq=80" ./examples/ESP32DevelopmentBoard

build:
	# expects repository to be linked to libraries
	arduino-cli compile --fqbn "esp8266:esp8266:huzzah:eesz=4M3M,xtal=80" ./examples/AdafruitHuzzahESP8266
	arduino-cli compile --fqbn "esp8266:esp8266:huzzah:eesz=4M3M,xtal=80" ./examples/AdafruitHuzzahESP8266Secure
	arduino-cli compile --fqbn "arduino:avr:uno" ./examples/ArduinoEthernetShield
	arduino-cli compile --fqbn "arduino:samd:mkrgsm1400" ./examples/ArduinoMKRGSM1400
	arduino-cli compile --fqbn "arduino:samd:mkrgsm1400" ./examples/ArduinoMKRGSM1400Secure
	arduino-cli compile --fqbn "arduino:samd:mkrnb1500" ./examples/ArduinoMKRNB1500
	arduino-cli compile --fqbn "arduino:avr:uno" ./examples/ArduinoWiFi101
	arduino-cli compile --fqbn "arduino:avr:uno" ./examples/ArduinoWiFi101Secure
	arduino-cli compile --fqbn "arduino:avr:uno" ./examples/ArduinoWiFiShield
	arduino-cli compile --fqbn "arduino:avr:yun" ./examples/ArduinoYun
	arduino-cli compile --fqbn "arduino:avr:yun" ./examples/ArduinoYunSecure
	arduino-cli compile --fqbn "esp32:esp32:esp32:FlashFreq=80" ./examples/ESP32DevelopmentBoard
	arduino-cli compile --fqbn "esp32:esp32:esp32:FlashFreq=80" ./examples/ESP32DevelopmentBoardSecure
