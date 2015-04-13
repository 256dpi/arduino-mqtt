#!/bin/bash

git clone http://git.eclipse.org/gitroot/paho/org.eclipse.paho.mqtt.python.git ./paho

cp ./paho/src/paho/mqtt/client.py ./mqtt.py

rm -rf ./paho
