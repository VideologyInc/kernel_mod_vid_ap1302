#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later

from periphery import I2C
from time import sleep
from ap1302py import gsi2c


def main():
    #x8 = read8_test(0x00)
    #print("read: 0x%02X" % x8)
    print("read8: 0x%02X" % gsi2c.read8(0x00))
    gsi2c.write8(0x00,0x55)
    gsi2c.write8(0x00,0xAA)
    print("read8: 0x%02X" % gsi2c.read8(0x00))
    print("read8: 0x%02X" % gsi2c.read8(0x00))
    print("read8: 0x%02X" % gsi2c.read8(0x00))

    print("read16: 0x%04X" % gsi2c.read16(0x14))
    gsi2c.write16(0x14,0x02D1)

    gsi2c.write16(0x14,0x02D2)
    gsi2c.write16(0x14,0x02D3)
    gsi2c.write16(0x14,0x02D0)
    print("read16: 0x%04X" % gsi2c.read16(0x14))
    print("read16: 0x%04X" % gsi2c.read16(0x14))
    print("read16: 0x%04X" % gsi2c.read16(0x14))

    gsi2c.write32(0x24,33333)
    print("read32: 0x%08X" % gsi2c.read32(0x24))
    gsi2c.write32(0x24,22222)
    print("read32: 0x%08X" % gsi2c.read32(0x24))

    print(gsi2c.read_serial())
    print(gsi2c.read_nvm(gsi2c.NVM_USER_REG, 0, 16))


if __name__ == "__main__":
    # i2c device path
    gsi2c.i2c = I2C("/dev/links/csi0_i2c")
    main()

