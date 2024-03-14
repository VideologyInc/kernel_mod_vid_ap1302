#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later

from periphery import I2C
from time import sleep
import datetime
import os
from . import gsi2c
import argparse


def percprint(start,appsize,current):
	perc = round(100*(current-start)/appsize)
	print("Progress: %2d%%\r"%(perc),end="",flush=True)

def printbuf(mybytebuff):
    line=0
    for myline in mybytebuff:
        for mybyte in myline:
            print("%02X "%(mybyte),end="")
        print("")
        line += 1
        if(line % 16 == 0): print("")

def get_nvmspace_name(val):
    switcher={ 0:'User Registers',
               1:'User Calibration',
               2:'Factory Registers',
               3:'Factory Calibration', }
    return switcher.get(val,"Invalid")

# read file to buffer
def readfile(filename):
    mybytebuff = []
    myfile = open(filename, "r")
    lines = myfile.readlines()
    myfile.close()
    for myline in lines:
        if myline.find('TOTALSIZE') > -1:
            a=1 #dummy
        elif myline.find('BLOCKSIZE') > -1:
            a=2 #dummy
        elif myline.find('//') < 0:
            mybytebuff.append([int(i, 16) for i in myline.split()])
    nvmsize = sum((len(x)-1) for x in mybytebuff) # calculate appsize
    return nvmsize, mybytebuff

#write buffer to image file
def writefile(filename, mybytebuff):
    myfile = open(filename, "w")
    myfile.write("////////////////////////////////////////////////////\n")
    myfile.write("// NVM Contents\n")
    myfile.write("// Date & Time: %s\n" %(datetime.datetime.now().strftime("%d/%m/%Y %H:%M:%S")))
    myfile.write("// Factory version: %3d.%d\n"%(mybytebuff[0x2E][0xF],mybytebuff[0x2E][0xE]))
    myfile.write("// User version:    %3d.%d\n"%(mybytebuff[0xE][0xF],mybytebuff[0xE][0xE]))
    line=0
    for myline in mybytebuff:
        if(line % 16 == 0):
            myfile.write("////////////////////////////////////////////////////\n")
            myfile.write("// Start of page: %d (%s)\n"%(line>>4,get_nvmspace_name(line>>4)))
            myfile.write("//// 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n")
        myfile.write("%04X "%(line<<4))
        for mybyte in myline:
            myfile.write("%02X "%(mybyte))
        myfile.write("\n")
        line += 1
    myfile.write("//")
    myfile.close()

# write nvm buffer to camera
def writenvm(nvmblocknr, mybytebuff):
    if(nvmblocknr == 99):
        line = 0
        for myline in mybytebuff:
            gsi2c.write_nvm(line>>4, myline[0]&0xFF, myline[1:])
            line += 1 # increase page number
    else:
        line = nvmblocknr<<4
        for myline in mybytebuff[nvmblocknr<<4:nvmblocknr+1<<4]:
            gsi2c.write_nvm(line>>4, myline[0]&0xFF, myline[1:])
            line += 1 # increase page number

# read nvm from camera
def readnvm():
    mybytebuff = []
    for page in range(4):
        address = 0
        for n in range(16):
            mybytebuff.append(gsi2c.read_nvm(page, address, 16))
            address += 16
    return mybytebuff



def main():
    mybytebuff = []
    nvmsize = 0
    parser = argparse.ArgumentParser(description="Read or Write nvm ",prog="flashnvm")
    parser.add_argument('-f', dest='filename', metavar='filename', default="NONE", help='Image filename')
    parser.add_argument('-w', dest='writenvm', action='store_true', help='Write nvm')
    parser.add_argument('-n', dest='nvmblocknr', metavar='nvmblocknr', type=int, default=99, help='write nvm block: 0=userreg, 1=usercal, 2=factoryreg, 3=factorycal, 99=all')
    parser.add_argument('-p',  dest='password', metavar='password', type=lambda x: int(x,0), default=0, help='pasword in case of writing to factory space')
    parser.add_argument('-D', dest='dummy', action='store_true', help='Dummy I2C ransfers')
    parser.add_argument('-i', dest='iic', metavar='iic',type=lambda p: p if os.path.exists(p) else FileNotFoundError(p), default='/dev/i2c0', help='i2c dev path')
    args = parser.parse_args()

    gsi2c.dummy(args.dummy)

    gsi2c.i2c = I2C(args.iic)

    # in case of write read from file
    if(args.writenvm):
        if(args.filename == "NONE"):
            print("[ERROR] No filename given!")
            return
        nvmsize, mybytebuff = readfile(args.filename)

    #set password in case writing to factory space
    if(args.password > 0):
        gsi2c.set_password(args.password)
        sleep(0.1)

    # in case of write, update nvm
    if(args.writenvm):
         writenvm(args.nvmblocknr, mybytebuff)
    # in case of read, print or store to file
    else:
         mybytebuff = readnvm()
         if(args.filename == "NONE"):
            printbuf(mybytebuff)
            return
         else:
             writefile(args.filename, mybytebuff)

    #unset password
    if(args.password > 0):
        gsi2c.set_password(0)

    print("Done\n")

if __name__ == "__main__":
    main()

