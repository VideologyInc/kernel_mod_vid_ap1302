#!/usr/bin/env python3

from periphery import I2C
import argparse
import gsi2c

def main():
    parser = argparse.ArgumentParser(description="I2C readreg",prog="readreg")
    parser.add_argument('-r', dest='register', metavar='n', type=lambda x: int(x,0), required=True, help='register address')
    parser.add_argument('-s', dest='size', metavar='n', type=lambda x: int(x,0), default=8,help='register size',choices=[8,16,32])
    parser.add_argument('-c', dest='count', metavar='count',type=int, default=1, help='number of sequential registers to read')
    parser.add_argument('-i', dest='iic', metavar='iic',type=int, default=0, help='i2c bus 0 or 1')
    args = parser.parse_args()

    regaddr = args.register

    if args.iic == 0:
        gsi2c.i2c = I2C("/dev/links/csi0_i2c")
    elif args.iic == 1:
        gsi2c.i2c = I2C("/dev/links/csi1_i2c")
    else:
        print("wrong i2c bus!\n")
        return

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
