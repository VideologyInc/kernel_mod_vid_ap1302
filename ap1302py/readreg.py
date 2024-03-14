#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later

from periphery import I2C
import argparse
from .  import gsi2c
import os

def main():
    parser = argparse.ArgumentParser(description="I2C readreg",prog="readreg")
    parser.add_argument('-r', dest='register', metavar='n', type=lambda x: int(x,0), required=True, help='register address')
    parser.add_argument('-s', dest='size', metavar='n', type=lambda x: int(x,0), default=8,help='register size',choices=[8,16,32])
    parser.add_argument('-c', dest='count', metavar='count',type=int, default=1, help='number of sequential registers to read')
    parser.add_argument('-i', dest='iic', metavar='iic',type=lambda p: p if os.path.exists(p) else FileNotFoundError(p), default='/dev/i2c0', help='i2c dev path')
    args = parser.parse_args()

    regaddr = args.register

    gsi2c.i2c = I2C(args.iic)

    for n in range(0,args.count,1):
        if args.size == 8:
            print("0x%02X = 0x%02X"%(regaddr,gsi2c.read8(regaddr)))
            regaddr += 1
        if args.size == 16:
            print("0x%02X = 0x%04X"%(regaddr,gsi2c.read16(regaddr)))
            regaddr += 2
        if args.size == 32:
            print("0x%02X = 0x%08X"%(regaddr,gsi2c.read32(regaddr)))
            regaddr += 4


if __name__ == "__main__":
    main()
