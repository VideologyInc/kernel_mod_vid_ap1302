#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later

from periphery import I2C
from . import gsi2c
from . import crc
import os
import argparse
from time import sleep


def percprint(start, appsize, current):
	perc = round(100*(current-start)/appsize)
	print("Progress: %2d%%\r"%(perc),end="",flush=True)

def readfile(filename):
	mybytebuff = []
	#load the img file in byte array
	myfile = open(filename, "r")
	lines = myfile.readlines()
	myfile.close()
	for myline in lines:
		if myline.find('//') < 0:
			mybytebuff.append([int(i, 16) for i in myline.split()])
	ispsize = sum((len(x)-1) for x in mybytebuff) # calculate appsize
	return ispsize, mybytebuff

def isperase():
	erasing = True
	gsi2c.isp_erase_all()
	while(erasing):
		status = gsi2c.isp_get_spi_status() # get status
		print(". ",end="",flush=True)
		if(status[0] == 0) &(status[1] == 0): erasing = False
		sleep(0.01)
	print()

# write isp buffer to camera
def ispwrite(mybytebuff, ispsize):
	for block in mybytebuff:
		gsi2c.isp_write(block[0], block[1:])
		percprint(0, ispsize, block[0])
		writing = True
		timeout = 0
		while(writing): #wait for ready
			status = gsi2c.isp_get_spi_status() # get status
			if(status[0] == 0) & (status[1] == 0): writing = False
			timeout += 1
			if timeout > 1000:
				print("[ERROR] Timeout whil writing spiflash!")
				quit()
	print()

 # verify by comparing the crc calculated from file and crc calculated by mcu
def ispverify(mybytebuff, ispsize):
	mycrc=0xFFFF
	for block in mybytebuff: #calculate CRC over buffer
		size  = len(block) - 1
		for n in range(1, size+1, 1):
			mycrc = crc.UpdateCRC(mycrc, block[n])
	#print("CRC 1 = 0x%04X\n"%mycrc)
	mycrc2 = gsi2c.isp_calc_crc(0, ispsize - 1) # request mcu to calculate CRC
	#print("CRC 2 = 0x%04X\n"%mycrc2)
	print()
	if(mycrc == mycrc2):
		return True
	else:
		return False


def main():
	#load the img file in byte array
	mybytebuff = []
	ispsize = 0

	parser = argparse.ArgumentParser(description="Write ISP image ",prog="flashisp")
	parser.add_argument('-f', dest='filename', metavar='filename',  required=True, help='Image filename')
	parser.add_argument('-p',  dest='password', metavar='password', type=lambda x: int(x,0), default=0, help='set password')
	parser.add_argument('-D', dest='dummy', action='store_true', help='Dummy I2C ransfers')
	parser.add_argument('-i', dest='iic', metavar='iic',type=lambda p: p if os.path.exists(p) else FileNotFoundError(p), default='/dev/i2c0', help='i2c dev path')
	args = parser.parse_args()

	gsi2c.dummy(args.dummy)

	if args.iic == 0:
		gsi2c.i2c = I2C("/dev/links/csi0_i2c")
	elif args.iic == 1:
		gsi2c.i2c = I2C("/dev/links/csi1_i2c")
	else:
		print("wrong i2c bus!\n")
		return

	ispsize, mybytebuff = readfile(args.filename)

	gsi2c.set_password(args.password) # set the password to update

	gsi2c.write8(0xEB,0x82) # set camera to upgrader mode

	# get the spiflash id
	id = gsi2c.isp_get_spi_id(0x9F) # get SPI-FLASH  (0x9F = JEDEC) or (0x90 =ID)
	print("SPI ID: ",end="")
	print(id)
	if((id[0] != 0xEF) or (id[1] != 0x40) or (id[2] != 0x14)):
		print("[ERROR] JEDEC ID")
		quit()

	# get the spiflash status
	status = gsi2c.isp_get_spi_status() # get status
	if(status[0] != 0) | (status[1] != 0):
		print("[ERROR] status")
		quit()

	# erase the spi flash
	print("Erasing")
	isperase()

	# programm the spiflash
	print("Programming")
	ispwrite(mybytebuff, ispsize)

	# verify
	print("Verifying")
	if(ispverify(mybytebuff, ispsize) == True): print("Verify OK")
	else: print("Verify Failed !!!")

	gsi2c.set_password(0) # unset the password

	print("Done\n")

if __name__ == "__main__":
    main()

