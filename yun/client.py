import sys
import paho.mqtt.client as mqtt

def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    client.subscribe("$SYS/#")

def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload))

client = mqtt.Client()

client.on_connect = on_connect
client.on_message = on_message

client.username_pw_set("demo", "demo")
client.connect("connect.shiftr.io", 1883, 60)

client.loop_start()

line = True
while line:
    line = sys.stdin.readline()
    sys.stdout.write(line)
