#!/usr/bin/env python3
import os
import sys
import pty
import subprocess
import time
import socket

def test_j1708send_checksum():
    # 1. Setup a dummy UDP listener mimicking plc4trucksduck_host
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.bind(('127.0.0.1', 6971))
    s.settimeout(2.0)
    
    # 2. Run j1708send_c with --checksums=false 0a00f5
    proc = subprocess.run(["./src/arm/j1708send_c", "--checksums=false", "0a00f5"], capture_output=True)
    
    # 3. Read what it sent
    try:
        data, addr = s.recvfrom(255)
        hex_out = data.hex()
        if hex_out == "0a00f5":
            print(f"PASS: j1708send C binary sent {hex_out} exactly.")
        else:
            print(f"FAIL: j1708send C binary sent {hex_out} instead of 0a00f5.")
            sys.exit(1)
    except socket.timeout:
        print("FAIL: j1708send timeout")
        sys.exit(1)
    finally:
        s.close()

if __name__ == "__main__":
    test_j1708send_checksum()
