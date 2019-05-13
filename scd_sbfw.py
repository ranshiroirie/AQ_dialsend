# -*- coding: utf-8 -*-
import serial
import requests

ser = serial.Serial('/dev/tty.usbserial-MW2MRTQL',115200)
baseurl = "http://52.197.122.211/api/dices?"

conv = ""
ID = ""
DIAL = ""
SW = ""
VOL = ""
DIAL_VALUE = 0
SW_VALUE = 0

counts = 0

def receive():
	rec = ser.readline()
	global conv
	conv = rec.decode()
	# print (conv)

def Send_to_AWS():
	global ID, DIAL,counts,SW, VALUE
	counts += 1
	ID = conv[11:19]
	DIAL = conv[34:35]
	SW = conv[37:45]
	DIAL_Convert()
	SW_convert()
	print ("カウント: %s" % counts)
	print ("識別番号: %s" % ID)
	print ("ダイヤル: %s" % DIAL_VALUE)
	print ("スイッチ: %s" % SW_VALUE)
	voltage()
	
	TASK = SW_VALUE * 10 + DIAL_VALUE + 1
	if len(conv) == 51:
		if not DIAL == "ERR":
			if not SW == "ERR":
				appurl = baseurl + "dice_id=" + ID + "&angle=" + str(TASK)
				print(appurl)
				requests.post(appurl)
				result = requests.get(appurl)
				print(result)

def DIAL_Convert():
	global DIAL,DIAL_VALUE
	if DIAL == "F":
		DIAL_VALUE = 0
	elif DIAL == "E":
		DIAL_VALUE = 1
	elif DIAL == "D":
		DIAL_VALUE = 2
	elif DIAL == "C":
		DIAL_VALUE = 3
	elif DIAL == "B":
		DIAL_VALUE = 4
	elif DIAL == "A":
		DIAL_VALUE = 5
	elif DIAL == "9":
		DIAL_VALUE = 6
	elif DIAL == "8":
		DIAL_VALUE = 7
	elif DIAL == "7":
		DIAL_VALUE = 8
	elif DIAL == "6":
		DIAL_VALUE = 9
	else:
		DIAL = "ERR"

def SW_convert():
	global SW,SW_VALUE
	if SW == "00FFFFFF":
		SW_VALUE = 0
	elif SW == "FF00FFFF":
		SW_VALUE = 1
	elif SW == "FFFF00FF":
		SW_VALUE = 2 
	else:
		SW_VALUE = -1
		SW = "ERR"


def voltage():
	global VOL
	VOL = conv[28:31]
	converted_vol = int(VOL, 16)
	print ("駆動電圧: %dmV" % converted_vol)


def main():
	receive()
	Send_to_AWS()
	print ("")

if __name__ == '__main__':
	while True:
		main()
