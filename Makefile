all: fmt

fmt:
	clang-format -i src/*.h src/*.cpp -style="{BasedOnStyle: Google, ColumnLimit: 120}"

update:
	rm -rf ./lwmqtt
	git clone --branch v0.6.4 https://github.com/256dpi/lwmqtt.git ./lwmqtt
	mkdir -p ./src/lwmqtt
	cp -r ./lwmqtt/src/*.c ./src/lwmqtt/
	cp -r ./lwmqtt/src/*.h ./src/lwmqtt/
	cp -r ./lwmqtt/include/*.h ./src/lwmqtt/
	rm -rf ./lwmqtt
	sed -i '' "s/<lwmqtt.h>/\"lwmqtt.h\"/g" ./src/lwmqtt/*
