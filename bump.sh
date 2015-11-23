#!/usr/bin/env bash

old=$1
new=$2

echo "bump version from $old to $new..."

sed -i '.bak' "s/$old/$new/g" library.properties
rm library.properties.bak

sed -i '.bak' "s/$old/$new/g" README.md
rm README.md.bak

sed -i '.bak' "s/$old/$new/g" src/YunMQTTClient.cpp
rm src/YunMQTTClient.cpp.bak
