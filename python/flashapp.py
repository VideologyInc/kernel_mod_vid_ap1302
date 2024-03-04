#!/usr/bin/env python3

from periphery import I2C
import os, sys, struct
from time import sleep
import gsi2c
import argparse


def percprint(start,appsize,current):
	perc = round(100*(current-start)/appsize)
	print("\rProgress: %2d%% "%(perc),end="",flush=True)

def arrprint(address,arr,size):
	print("\n0x%04X: "%(address),end="",flush=True)
	for n in range (0,size,1):
		print("%02X "%(arr[n]),end="",flush=True)

#load the img file in byte array
mybytebuff = []
appsize = 0

def readfile(filename):
    #load the img file in byte array
    global mybytebuff
    global appsize
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
    appsize = sum((len(x)-1) for x in mybytebuff) # calculate appsize        




def main():
    parser = argparse.ArgumentParser(description="Update mainapp img file",prog="flashapp")
    parser.add_argument('-f', dest='filename', metavar='filename',  required=True, help='Image filename')
    parser.add_argument('-p', dest='password', metavar='password', type=lambda x: int(x,0), default=0, help='set password')
    parser.add_argument('-a', dest='writeapp', action='store_true', help='Write app only')
    parser.add_argument('-n', dest='writenvm', action='store_true', help='Write nvm only')
    parser.add_argument('-D', dest='dummy', action='store_true', help='Dummy I2C ransfers')
    parser.add_argument('-i', dest='iic', metavar='iic',type=int, default=0, help='i2c bus 0 or 1')
    args = parser.parse_args()

    
    gsi2c.dummy(args.dummy)
    
    if args.iic == 0:
        gsi2c.i2c = I2C("/dev/links/csi0_i2c")
    elif args.iic == 1:
        gsi2c.i2c = I2C("/dev/links/csi1_i2c")
    else:
        print("wrong i2c bus!\n")
        return
    



    readfile(args.filename)
    gsi2c.set_password(args.password) # set the password to update
    sleep(1) 
    
    print("Start Bootloader ",end="",flush=True)
    gsi2c.write8(0xF0, 0xA5) # start the bootloader
    sleep(1) # wait for bootloader
    gsi2c.check() # check if i2c is up"
    print("")

    #print("bootid: %04X" %(gsi2c.bootid()))

    print("Erase flash ",end="",flush=True)
    if args.writeapp :
        gsi2c.erase_app()
    elif args.writenvm :
        gsi2c.erase_nvm()
    else:
        gsi2c.erase_all()
    gsi2c.check() # wait for erase to finish
    print("")

    # write to flash
    if args.writeapp:  # app only
        for block in mybytebuff:
            if block[0] > gsi2c.FLASH_APP_MAX: 
                break # exit
            gsi2c.flashwrite(block[0], block[1:])
            #print(block[0],block[1:])
            percprint(gsi2c.FLASH_APP_START, gsi2c.FLASH_APP_SIZE, block[0])
    elif args.writenvm: # nvm only
        for block in mybytebuff:
            if block[0] > gsi2c.FLASH_NVM_MAX: 
                break # exit
            if block[0] >= gsi2c.FLASH_NVM_START:
                gsi2c.flashwrite(block[0], block[1:])
                #print(block[0],block[1:])
                percprint(gsi2c.FLASH_NVM_START, gsi2c.FLASH_NVM_SIZE, block[0])
    else:
        print("Writing %d bytes" %(appsize))
        for block in mybytebuff:
            gsi2c.flashwrite(block[0], block[1:])
            #print(block[0],block[1:])
            percprint(gsi2c.FLASH_APP_START, appsize, block[0])
    gsi2c.check() # wait for write to finish
    print("")

    # get the crc's
    crc_read = gsi2c.read_crc() # read CRC from mcu 
    print("crc_read %04X " %(crc_read) ,end="",flush=True)
    gsi2c.check() # wait for crc_calc to finish
    print("")

    app_size = gsi2c.read_size() # read CRC from mcu 
    print("app_size %04X " %(app_size),end="",flush=True)
    gsi2c.check() # wait for crc_calc to finish
    print("")

    crc_calc = gsi2c.calc_crc() # have mcu calculate CRC
    print("\rcrc_calc %04X " %(crc_calc),end="",flush=True)
    gsi2c.check() # wait for crc_calc to finish
    print("")

    # check CRC
    if(crc_read == crc_calc):
        print("Progamming was successful!")
        print("Now Rebooting . . . ",end="",flush=True)
        gsi2c.reboot() # reboot, main app should start
        sleep(1) # wait for boot
        print("")	
    else:
        print("\nProgamming Failed!")
        print("CRC stored in flash:       0x%04X"%(crc_read))
        print("CRC calculated from flash: 0x%04X"%(crc_calc))
        print("\n")

        gsi2c.check()
        sleep(0.001)
    print("Done\n")



if __name__ == "__main__":
    main()

