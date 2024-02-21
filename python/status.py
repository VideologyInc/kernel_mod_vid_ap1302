#!/usr/bin/env python3

import gsi2c


CAMERA_STATE       = 0x01
CAMERA_PASSWORD    = 0x02
MCU_TEMP           = 0x03
SENSOR_TEMP_EN     = 0x04
GET_SENSOR_TEMP    = 0x05


if(gsi2c.bootid() == 0x5AA5): 
    print("Bootloader is active!")

print("MCU version = %d.%d" %(gsi2c.read8(0xFF),gsi2c.read8(0xFE)))

print("NVM version = %d.%d" %(gsi2c.read8(0xEF),gsi2c.read8(0xEE)))

print("ISP version = %d" %(gsi2c.read8(0xED)<<8 | gsi2c.read8(0xEC)))

gsi2c.write8(0xe8,CAMERA_STATE)
print("CAMERA_STATE = %d" %(gsi2c.read8(0xe9)))

gsi2c.write8(0xe8,CAMERA_PASSWORD)
print("CAMERA_PASSWORD = %d" %(gsi2c.read8(0xe9)))

gsi2c.write8(0xe8,MCU_TEMP)
print("MCU_TEMP = %d" %(gsi2c.read8(0xe9)))

gsi2c.write8(0xe8,SENSOR_TEMP_EN) #enable the temprature-sensor in the sensor
gsi2c.write8(0xe8,GET_SENSOR_TEMP)
print("GET_SENSOR_TEMP = %d" %(gsi2c.read8(0xe9)))


def main():
    print()

if __name__ == "__main__":
    main()