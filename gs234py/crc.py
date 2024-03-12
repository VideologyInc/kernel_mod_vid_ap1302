#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later

##
#  @file crc.py
#  @brief Brief description
#



POLY=0x1021
INITIAL=0xFFFF

def UpdateCRC (CRC_acc, CRC_input):
	# Create the CRC "dividend" for polynomial arithmetic (binary arithmetic with no carries)
	CRC_acc = CRC_acc ^ (CRC_input << 8)

	# "Divide" the poly into the dividend using CRC XOR subtraction
	# CRC_acc holds the "remainder" of each divide
	# Only complete this division for 8 bits since input is 1 byte
	for i in range(0,8,1):
		#Check if the MSB is set (if MSB is 1, then the POLY can "divide" into the "dividend")
		if ((CRC_acc & 0x8000) == 0x8000):
			#if so, shift the CRC value, and XOR "subtract" the poly
			CRC_acc = CRC_acc << 1
			CRC_acc ^= POLY;
		else:
			#if not, just shift the CRC value
			CRC_acc = CRC_acc << 1
	#Return the final remainder (CRC value)
	return CRC_acc&0xFFFF

def GetCRC(crc_init, input_buf, size):
	crc=crc_init
	for n in range(size):
		crc = UpdateCRC(crc, input_buf[n])
	return crc

def main():
    print()

if __name__ == "__main__":
    main()