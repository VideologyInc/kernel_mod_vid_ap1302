#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later

from periphery import I2C
from time import sleep

# use logger because print wont work for apps run without a shell env

i2c = None
#i2c = I2C("/dev/links/csi1_i2c")

_maxretries = 150
_retrytime = 0.005 #5ms

FLASH_APP_START          = 0x1A00
FLASH_APP_MAX            = 0xF3FF
FLASH_APP_SIZE           = FLASH_APP_MAX - FLASH_APP_START + 1
FLASH_NVM_START          = 0xF400
FLASH_NVM_MAX            = 0xF7FF
FLASH_NVM_SIZE           = FLASH_NVM_MAX - FLASH_NVM_START + 1

_FLASH_CRC_ADDRESS       = 0xF3F0
_FLASH_APP_SIZE_ADDRESS  = 0xF3F4
_FLASH_APP_MAX_ADDRESS   = 0xF3E0
_FLASH_APP_START_ADDRESS = 0x1A00
_FLASH_MAX               = 0xF9FF
_FLASH_PAGE_SIZE         = 512


NVM_USER_REG = 0
NVM_USER_CAL = 1
NVM_FACTORY_REG = 2
NVM_FACTORY_CAL = 3
#NVM_TEMP = 4 (don't use)

def dprint(a):
    b=a
    #print (a,end="")

_dummy: bool = False
def dummy(onoff: bool):
    global _dummy
    _dummy = onoff

def tranfer(msgs):
    retries = 0
    err = 0
    if _dummy == False:
        while (retries < _maxretries):
            try:
                err = i2c.transfer(0x38, msgs)
            except:
                dprint("%d "%(retries))
            if(err == None): break
            retries += 1
            sleep(_retrytime)
    else:
        print (msgs[-1].data)
    return msgs[-1].data


#
# read /write commands for the mainapp
#


# Wait for I2C bus to become available
def i2ccheck():
    retries = 0
    err = 0
    msgs = I2C.Message([0], read=False)
    if _dummy == False:
        while (retries < 100): # max 10 seconds
            try:
                err = i2c.transfer(0x38)
            except:
                dprint("%d "%(retries))
            if(err == None): break
            retries += 1
            sleep(0.01) #10ms
            print(". " ,end="",flush=True)
    else:
        print (msgs[-1].data)

def read8(addr):
    dprint("\tread8 %02X = "%(addr))
    msgs = [I2C.Message([0x31, addr]), I2C.Message([0], read=True)]
    data = tranfer(msgs)
    dprint("%02X\n"%(data[0]))
    return data[0]

def read16(addr):
    dprint("\tread16 %02X = "%(addr))
    msgs = [I2C.Message([0x33, addr]), I2C.Message([0,0], read=True)]
    data = tranfer(msgs)
    dprint("%04X\n"%(data[0] | (data[1])<<8))
    return data[0] | (data[1])<<8

def read32(addr):
    dprint("\tread32 %02X = "%(addr))
    msgs = [I2C.Message([0x35, addr]), I2C.Message([0,0,0,0], read=True)]
    data = tranfer(msgs)
    dprint("%08X\n"%(data[0] | (data[1])<<8 | (data[2])<<16 | (data[3])<<24))
    return data[0] | (data[1])<<8 | (data[2])<<16 | (data[3])<<24

def write8(addr, val):
    dprint("\twrite8 ")
    msgs = [I2C.Message([0x30, addr, val], read=False)]
    data = tranfer(msgs)
    dprint("%02X = %02X\n"%(addr,val))

def write16(addr, val):
    dprint("\twrite16 ")
    msgs = [I2C.Message([0x32, addr, val&0xFF, (val>>8)&0xFF], read=False)]
    data = tranfer(msgs)
    dprint("%02X = %04X\n"%(addr,val))

def write32(addr, val):
    dprint("\twrite32 ")
    msgs = [I2C.Message([0x34, addr, val&0xFF, (val>>8)&0xFF, (val>>16)&0xFF, (val>>24)&0xFF], read=False)]
    data = tranfer(msgs)
    dprint("%02X = %08X\n"%(addr,val))

# read the 16 byte unique serial number
def read_serial():
    msgs = [I2C.Message([0x61]), I2C.Message([0 for i in range(16)], read=True)]
    data = tranfer(msgs)
    dprint("\tserial: ")
    dprint(data )
    dprint("\n")
    return data

# set/unset the password, use True or False
def set_password(val):
    msgs = [I2C.Message([0x30, 0xFC, (val & 0xFF)], read=False)]
    data = tranfer(msgs)
    msgs = [I2C.Message([0x30, 0xFD, ((val>>8) & 0xFF)], read=False)]
    data = tranfer(msgs)


# restart = reboot
def restart():
	write8(0xF0, 0x99)

#
# NVM read / wrtie commands for the mainapp
#

# read data from nvm space
# NVM_USER_REG = page 0
# NVM_USER_CAL = page 1
# NVM_FACTORY_REG = page 2
# NVM_FACTORY_CAL = page 3
# NVM_TEMP = page 4 (don't use)
def read_nvm(page, address, size):
    if page >= 5: return # never use page >= 5 here
    msgs = [I2C.Message([0x51, page, address, size]), I2C.Message([0 for i in range(size)], read=True)]
    data = tranfer(msgs)
    dprint("\tread nvm: ")
    dprint(data )
    dprint("\n")
    return data

# Write to NVM space, maximum buf size is 16 bytes
# NVM_USER_REG = page 0
# NVM_USER_CAL = page 1
# NVM_FACTORY_REG = page 2 (need password)
# NVM_FACTORY_CAL = page 3 (need password)
def write_nvm(page, address, buf):
    if page >= 4: return # never use page = 4 here
    dprint("\write nvm: ")
    dprint(buf)
    dprint("\n")
    msgs = [I2C.Message([0x50, page, address] + buf, read=False)]
    data = tranfer(msgs)

#
# ISP SPI update commands for the mainapp
#

# write 16, 32 or 64 byte buffer to flash
def isp_write(address, buf):
    dprint("\tispwrite %04X " %(address))
    dprint(buf)
    dprint("\n")
    msgs = [I2C.Message([0x40, address&0xFF, (address>>8)&0xFF, (address>>16)&0xFF] +buf, read=False)]
    data = tranfer(msgs)

def isp_calc_crc(address1, address2):
    dprint("\tisp_calc_crc %08X %08X\n" %(address1, address2))
    msgs = [I2C.Message([0x47, address1 & 0xFF, (address1 >> 8) & 0xFF, (address1 >> 16) &0xFF, address2 & 0xFF, (address2 >> 8) & 0xFF, (address2 >> 16) & 0xFF], read=False)]
    data = tranfer(msgs)
    i2ccheck()
    msgs = [I2C.Message([0,0], read=True)]
    data = tranfer(msgs)
    return data[0] | (data[1] <<8)

def isp_erase_page(address):
    dprint("\tisp_erase_page %08X\n" %(address))
    msgs = [I2C.Message([0x44, address&0xFF, (address>>8)&0xFF, (address>>16)&0xFF], read=False)]
    data = tranfer(msgs)
    i2ccheck()

def isp_erase_all():
    dprint("\tisp_erase_all\n")
    msgs = [I2C.Message([0x42, 0x01], read=False)]
    data = tranfer(msgs)
    #i2ccheck()

#command = 0x9F (= JEDEC) or 0x90 (=ID)
def isp_get_spi_id(command):
    dprint("\tisp_get_spi_status\n")
    msgs = [I2C.Message([0x43, command]), I2C.Message([0,0,0], read=True)]
    data = tranfer(msgs)
    return data

def isp_get_spi_status():
    msgs = [I2C.Message([0x45]), I2C.Message([0,0], read=True)]
    data = tranfer(msgs)
    return data

#
# Flash commands for the bootloader
#


# read 16, 32 or 64 byte buffer from flash
def flashread(address,size):
    msgs = [I2C.Message([0x39, address&0xFF, (address>>8)&0xFF, size]), I2C.Message([0 for i in range(size)], read=True)]
    data = tranfer(msgs)
    dprint("\tread %04X : "%(address))
    dprint(data)
    dprint("\n")
    return data

# write 16, 32 or 64 byte buffer to flash
def flashwrite(address, buf):
    dprint("\twrite %04X " %(address))
    dprint(buf)
    dprint("\n")
    msgs = [I2C.Message([0x38, address&0xFF, (address>>8)&0xFF] +buf, read=False)]
    data = tranfer(msgs)

# Wait for I2C bus to become available
def check():
    retries = 0
    err = 0
    msgs = I2C.Message([0], read=False)
    while (retries < 100): # max 10 seconds
        try:
            err = i2c.transfer(0x38)
        except:
            dprint("%d "%(retries))
        print(". " ,end="",flush=True)
        if(err == None) or (err == 0): break
        retries += 1
        sleep(0.01) #10ms


# get crc calculated by mcu
def calc_crc():
    msgs = [I2C.Message([0x41, 0x00], read=False)]
    data = tranfer(msgs)
    check()
    msgs = [I2C.Message([0x00, 0x00], read=True)]
    data = tranfer(msgs)
    return data[0] | (data[1])<<8

# get crc from mcu
def read_crc():
    data = flashread(_FLASH_CRC_ADDRESS, 2)
    return data[0] | (data[1] << 8)

# write the crc to flash
def write_crc(crc):
    data = [crc & 0xFF, crc >> 8]
    flashwrite(_FLASH_CRC_ADDRESS, 2, data)

# erase the crc stored in flash
def erase_crc():
	erase_page(_FLASH_CRC_ADDRESS, 2)

# get the image size stored in flash
def read_size():
	data = flashread(_FLASH_APP_SIZE_ADDRESS,2)
	return data[0] | (data[1] << 8)

# write the image size to flash
def write_size(size):
	data = [size & 0xFF, size >> 8]
	flashwrite(_FLASH_APP_SIZE_ADDRESS, 2, data)

# erase the image size stored in flash
def erase_size():
	erase_page(_FLASH_APP_SIZE_ADDRESS, 2)

# erase data in flash page
def erase_page(address, size):
    if(size > _FLASH_PAGE_SIZE): # MCU Bootloader bug!
        print("[ERROR] size must be smaller or equal to pagesize.\n")
    else:
        msgs = [I2C.Message([0x44, address&0xFF, (address>>8)&0xFF, size&0xFF, (size>>8)&0xFF], read=False)]
        data = tranfer(msgs) # do we need more time?

# erase mainapp
def erase_app():
    for n in range(_FLASH_APP_START_ADDRESS, FLASH_APP_MAX, _FLASH_PAGE_SIZE):
        erase_page(n, _FLASH_PAGE_SIZE)

# erase nvm
def erase_nvm():
    for n in range(FLASH_NVM_START, FLASH_NVM_MAX, _FLASH_PAGE_SIZE):
        erase_page(n, _FLASH_PAGE_SIZE)

# erase flash
def erase_all():
    msgs = [I2C.Message([0x44, 0x01], read=False)]
    data = tranfer(msgs)
    check()

# reboot
def reboot():
    msgs = [I2C.Message([0x46, 0x01], read=False)]
    data = tranfer(msgs)
    sleep(0.01)
    check()
    sleep(1) #wait 1 second after reboot

# get the bootloader id, to check if the mcu is running bootloader
def bootid():
    msgs = [I2C.Message([0x47]), I2C.Message([0,0], read=True)]
    data = tranfer(msgs)
    return data[0]| (data[1] <<8)

def main():
    print()


if __name__ == "__main__":
    main()

