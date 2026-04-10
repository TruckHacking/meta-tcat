#!/usr/bin/env python3
import socket
import threading
import subprocess
import time
import sys

def udp_listener():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.bind(('127.0.0.1', 6971))
    s.settimeout(2.0)
    try:
        data, addr = s.recvfrom(255)
        return data.hex()
    except Exception as e:
        return None
    finally:
        s.close()

class ListenerThread(threading.Thread):
    def __init__(self):
        super().__init__()
        self.result = None
    def run(self):
        self.result = udp_listener()

# Start listener
t = ListenerThread()
t.start()
time.sleep(0.1) # wait for bind

# Run the C binary
subprocess.run(["./j1708send_c", "--checksums=false", "0a00f5"])

t.join()

if t.result == "0a00f5":
    print("SUCCESS: 0a00f5 was sent without modification.")
    sys.exit(0)
else:
    print(f"FAIL: Expected 0a00f5, but got {t.result}")
    sys.exit(1)
