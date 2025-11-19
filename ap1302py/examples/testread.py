#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later

from periphery import I2C
from time import sleep
from ap1302py import gsi2c


CAMERA_STATE       = 0x01
CAMERA_PASSWORD    = 0x02
MCU_TEMP           = 0x03
SENSOR_TEMP_EN     = 0x04
GET_SENSOR_TEMP    = 0x05


video_format = {
	0: [1920,1200],
	1: [1600,1200],
	2: [1200,1200],
	3: [1920,1080],
	4: [1440,1080],
	5: [1080,1080],
	6: [1366,1024],
	7: [1280,1024],
	8: [1024,1024],
	9: [1280,960],
	10: [1366,768],
	11: [1024,768],
	12: [1280,720],
	13: [960,720],
	14: [1024,576],
	15: [768,576],
	16: [960,540],
	17: [720,540],
	18: [854,480],
	19: [640,480],
}


def main():
    gsi2c.write8(0xe8,CAMERA_STATE)
    print("CAMERA_STATE = %d" %(gsi2c.read8(0xe9)))

    format = gsi2c.read8(0x10)
    dim = video_format[format] if format>=0 and format<=19 else [0,0]
    print(f"Video Format = {format}: {dim}")


if __name__ == "__main__":
    # i2c device path
    gsi2c.i2c = I2C("/dev/links/csi0_i2c")
    main()
