# -*- coding: utf-8 -*-
import serial
import requests

ser = serial.Serial('/dev/tty.usbserial-MW2MRTQL',115200)
baseurl = "http://52.197.122.211/api/dices?"


conv = ""
ID = ""
DIAL = ""

def receive():
	rec = ser.readline()
	global conv
	conv = rec.decode()
	# print (conv)

def Send_to_AWS():
	global ID, DIAL
	ID = conv[11:19]
	DIAL = conv[34:35]
	DIAL_Convert()
	print ("識別番号: %s" % ID)
	print ("値: %s" % DIAL)
	# print(len(conv))
	if len(conv) == 51:
		if not DIAL == "E":
			appurl = baseurl + "dice_id=" + ID + "&angle=" + DIAL
			print(appurl)
			requests.post(appurl)
			result = requests.get(appurl)
			print(result)

def DIAL_Convert():
	global DIAL
	if DIAL == "F":
		DIAL = "0"
	elif DIAL == "E":
		DIAL = "1"
	elif DIAL == "D":
		DIAL = "2"
	elif DIAL == "C":
		DIAL = "3"
	elif DIAL == "B":
		DIAL = "4"
	else:
		DIAL = "E"

def main():
	receive()
	Send_to_AWS()

if __name__ == '__main__':
	while True:
		main()
