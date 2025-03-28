#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later

from decimal import Decimal
from periphery import I2C
import argparse
from .  import gsi2c
#import gsi2c

def main():
    parser = argparse.ArgumentParser(description="Sync Trigger",prog="sync_trigger")
    parser.add_argument('-f', dest='fps', metavar='fps', help='set frame rate')
    parser.add_argument('-b', dest='bus', metavar='bus', default = 0, help='bus 0 or 1 select for trigger')
    parser.add_argument('-c', dest='config', action='store_true', help='enable sync input')
    args = parser.parse_args()

    if args.fps != None :
        if args.bus == '1':
            gsi2c.i2c = I2C("/dev/links/csi1_i2c")
            print("Tigger form bus 1\n")
        else:
            gsi2c.i2c = I2C("/dev/links/csi0_i2c")
            print("Tigger form bus 0\n")
        gsi2c.write32(0x64,round(256 * 1000_000 / Decimal(args.fps))) # set fps in microseconds
        #print("%8X"%round(256 * 1000_000 / Decimal(args.fps)))
        print("%3.2f fps (%7.2f us)\n"%(Decimal(args.fps),(1000_000 / Decimal(args.fps))))
        gsi2c.write16(0x60,0x3FFF) # set duty cycle to 0.25 ~0.5
        gsi2c.write8(0x68,0x07) # enable sync (pwm)
    else: 
        gsi2c.write8(0x68,0x06) # disable sync (pwm)

    if args.config == True:
        print("sync on Trigger\n")
        gsi2c.i2c = I2C("/dev/links/csi0_i2c")
        gsi2c.write16(0x58,0x302) # wait for trigger
        gsi2c.i2c = I2C("/dev/links/csi1_i2c")
        gsi2c.write16(0x58,0x302) # wait for trigger
    else:   
        print("Camera framerate\n") 
        gsi2c.i2c = I2C("/dev/links/csi0_i2c")
        gsi2c.write16(0x58,0x300) # wait for trigger
        gsi2c.i2c = I2C("/dev/links/csi1_i2c")
        gsi2c.write16(0x58,0x300) # wait for trigger

    print()

if __name__ == "__main__":
    main()