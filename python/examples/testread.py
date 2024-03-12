#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later

from time import sleep
import gsi2c


CAMERA_STATE       = 0x01
CAMERA_PASSWORD    = 0x02
MCU_TEMP           = 0x03
SENSOR_TEMP_EN     = 0x04
GET_SENSOR_TEMP    = 0x05




print("Format = %02x" %(gsi2c.read8(0x10)))




def main():
    print()

if __name__ == "__main__":
    main()