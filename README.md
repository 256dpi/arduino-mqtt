# arduino-mqtt

**MQTT library for Arduino based on the Eclipse Paho projects**

This library bundles the [Embedded MQTT C/C++ Client](https://eclipse.org/paho/clients/c/embedded/) library of the Eclipse Paho project and adds a thin wrapper to get an Arduino like API. Additionally there is an drop-in alternative for the Arduino Yùn that uses a python based client on the linux processor and a binary interface to lower program space usage on the Arduino side.

The first release of the library only supports QoS0 and the basic features to get going. In the next releases more of the features will be available.

This library is an alternative to the [pubsubclient](https://github.com/knolleary/pubsubclient) library by [knolleary](https://github.com/knolleary) which only supports QoS0 and uses a custom protocol implementation.

[Download version 1.5.0 of the library.](https://github.com/256dpi/arduino-mqtt/releases/download/v1.5.0/mqtt.zip)

*Or even better use the newly available Library Manager in the Arduino IDE.*

## Compatibility

This library has been only tested on the **Arduino Yùn** yet. Other boards and shields should work if they properly extend the Client API.

## Examples

- [Example using the standard MQTTClient](https://github.com/256dpi/arduino-mqtt/blob/master/examples/MQTTClient/MQTTClient.ino).
- [Example using the alternative YunMQTTClient](https://github.com/256dpi/arduino-mqtt/blob/master/examples/YunMQTTClient/YunMQTTClient.ino).
