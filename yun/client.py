import sys
import paho.mqtt.client as mqtt

# Client Callbacks

def on_connect(_, __, ___, rc):
    if rc == 0:
        send_command("ca")
    else:
        send_command("cd")

def on_message(_, __, msg):
    send_command("m:" + msg.topic + ":" + str(msg.payload))

# Command Helpers

def parse_command(client, line):
    segments = line.split(":")
    cmd = segments[0]
    remaining = segments[1:]
    if cmd == 'c':
        do_connect(client, remaining)
    elif cmd == 's':
        do_subscribe(client, remaining)
    elif cmd == 'u':
        do_unsubscribe(client, remaining)
    elif cmd == 'p':
        do_publish(client, remaining)
    elif cmd == 'd':
        do_disconnect(client)

def send_command(line):
    sys.stdout.write(line + "\n")

# Command Handlers

def do_connect(client, args):
    if len(args) >= 4:
        client.username_pw_set(args[2], args[3])
    if len(args) >= 2:
        client.connect(args[0], int(args[1]))
        client.loop_start()

def do_subscribe(client, args):
    if len(args) >= 1:
        client.subscribe(args[0])

def do_unsubscribe(client, args):
    if len(args) >= 1:
        client.unsubscribe(args[0])

def do_publish(client, args):
    if len(args) >= 2:
        client.publish(args[0], args[1])

def do_disconnect(client):
    client.disconnect()
    client.loop_stop()

# Main Loop

c = mqtt.Client()
c.on_connect = on_connect
c.on_message = on_message

send_command("ok")

while True:
    parse_command(c, sys.stdin.readline()[0:-1])

# TODO: better error handling
