#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later

from periphery import I2C
import argparse
import gsi2c

def main():
    parser = argparse.ArgumentParser(description="Set password ",prog="password")
    parser.add_argument('-p',  dest='password', metavar='password', type=lambda x: int(x,0), default=0, help='set password')
    parser.add_argument('-i', dest='iic', metavar='iic',type=int, default=0, help='i2c bus 0 or 1')
    args = parser.parse_args()

    if args.iic == 0:
        gsi2c.i2c = I2C("/dev/links/csi0_i2c")
    elif args.iic == 1:
        gsi2c.i2c = I2C("/dev/links/csi1_i2c")
    else:
        print("wrong i2c bus!\n")
        return

    gsi2c.set_password(args.password)
    print()

if __name__ == "__main__":
    main()