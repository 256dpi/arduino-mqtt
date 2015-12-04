import sys
import mqtt

class Bridge:
    COMMAND_END = b"\n"
    ARGUMENTS_SEPERATOR = b":"
    PAYLOAD_SEPERATOR = b";"

    # Constructor
    def __init__(self):
        self.client = None
        self.will_topic = ""
        self.will_payload = ""
        self.stopped = False

    # Bridge Callbacks
    def on_connect(self, _, __, ___, rc):
        self.send_command("a;" if rc == 0 else "r;")
    def on_message(self, _, __, msg):
        self.send_command("m:" + msg.topic + ":" + str(len(msg.payload)) + ";" + str(msg.payload))
    def on_disconnect(self, _, __, ___):
        self.send_command("e;")
        self.stopped = True

    # Command Helpers
    def parse_command(self, line):
        segments = line.split(self.ARGUMENTS_SEPERATOR)
        cmd = segments[0]
        remaining = segments[1:]
        if cmd == 'w':
            self.do_will(remaining)
        elif cmd == 'c':
            self.do_connect(remaining)
        elif cmd == 's':
            self.do_subscribe(remaining)
        elif cmd == 'u':
            self.do_unsubscribe(remaining)
        elif cmd == 'p':
            self.do_publish(remaining)
        elif cmd == 'd':
            self.do_disconnect()
        self.read_until(self.COMMAND_END)
    def send_command(self, line):
        sys.stdout.write(line + self.COMMAND_END)

    # Command Handlers
    def do_will(self, args):
        self.will_topic = args[0]
        self.will_payload = self.read_chunk(int(args[1]))

    def do_connect(self, args):
        self.client = mqtt.Client(args[2])
        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message
        self.client.on_disconnect = self.on_disconnect
        if len(args) >= 5:
            self.client.username_pw_set(args[3], args[4])
        if len(self.will_topic) > 0:
            self.client.will_set(self.will_topic, self.will_payload)
        try:
            self.client.connect(args[0], int(args[1]))
            self.client.loop_start()
        except:
            self.send_command("r;")

    def do_subscribe(self, args):
        self.client.subscribe(args[0])
    def do_unsubscribe(self, args):
        self.client.unsubscribe(args[0])
    def do_publish(self, args):
        self.client.publish(args[0], self.read_chunk(int(args[1])))
    def do_disconnect(self):
        self.client.loop_stop()
        self.client.disconnect()

    # Main
    def run(self):
        self.send_command("b;")
        while not self.stopped:
            self.parse_command(self.read_until(self.PAYLOAD_SEPERATOR))

    # Low Level Helpers
    def read_until(self, end):
        s = b""
        c = sys.stdin.read(1)
        while c not in end:
            s += c
            c = sys.stdin.read(1)
        return s
    def read_chunk(self, length):
        return sys.stdin.read(length)

# Main Loop
Bridge().run()
