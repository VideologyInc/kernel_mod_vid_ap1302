#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later

from periphery import I2C
import argparse
from .  import gsi2c
import os

def main():
    parser = argparse.ArgumentParser(description="Set password ",prog="password")
    parser.add_argument('-p',  dest='password', metavar='password', type=lambda x: int(x,0), default=0, help='set password')
    parser.add_argument('-i', dest='iic', metavar='iic',type=lambda p: p if os.path.exists(p) else FileNotFoundError(p), default='/dev/i2c-0', help='i2c dev path')
    args = parser.parse_args()

    gsi2c.i2c = I2C(args.iic)

    gsi2c.set_password(args.password)
    print()

if __name__ == "__main__":
    main()