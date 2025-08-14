#==================================================================================================================
#File Name: scanner_bridge.py
# Author: Vraj Patel, Samip Patel, Mihir Jariwala, Vamseedhar Reddy
# Date:		17/07/2025
# Modified:	None
# © Fanshawe College, 2025

# Description: This file contains the implementation of the scanner bridge,
# which facilitates communication between the barcode scanner and the ESP32.

#!/usr/bin/env python3
import serial
import socket
import time

SERIAL_PORT     = r'\\.\COM9'
BAUD_RATE       = 9600
TCP_IP, TCP_PORT = '192.168.4.1', 3334
RECONNECT_DELAY = 5  # seconds

#>>> open_serial===============================================================================================
#Author:		Vamseedhar Reddy, Samip Patel, Mihir Jariwala, Vraj Patel
#Date:		17/07/2025
#Modified:	None
#Desc:		This function will open the serial port for communication.
#Input: 	- None
#Returns:	A serial.Serial object on success, or None on failure.

def open_serial():
    while True:
        try:
            ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
            print(f"[+] Opened serial port {SERIAL_PORT}@{BAUD_RATE}")
            return ser
        except Exception as e:
            print(f"[!] Serial open failed: {e}")
            time.sleep(RECONNECT_DELAY)

#>>> send_barcode=============================================================================================
#Author:		Vamseedhar Reddy, Samip Patel, Mihir Jariwala, Vraj Patel
#Date:		17/07/2025
#Modified:	None
#Desc:		This function will send a barcode string to the ESP32 over TCP.
#Input: 	- barcode: The barcode string to send
#Returns:	None

def send_barcode(barcode: str):
    """Open a new socket, send one barcode, then close."""
    try:
        with socket.create_connection((TCP_IP, TCP_PORT), timeout=5) as sock:
            print(f"[+] Connected to {TCP_IP}:{TCP_PORT}")
            sock.sendall((barcode + "\n").encode('utf-8'))
    except Exception as e:
        print(f"[!] TCP send failed: {e}")

#>>> bridge==================================================================================================
#Author:		Vamseedhar Reddy, Samip Patel, Mihir Jariwala, Vraj Patel
#Date:		17/07/2025
#Modified:	None
#Desc:		This function will bridge the serial and TCP connections.
#Input: 	- None
#Returns:	None

def bridge():
    ser = open_serial()
    try:
        while True:
            raw = ser.readline()
            if not raw:
                continue
            text = raw.decode('utf-8', 'ignore').strip()
            if text:
                print(f"→ Scanned: {text}")
                send_barcode(text)
    except KeyboardInterrupt:
        print("\n[!] Interrupted by user, exiting…")
    finally:
        ser.close()

if __name__ == "__main__":
    print("[*] Scanner→ESP bridge starting…")
    bridge()
