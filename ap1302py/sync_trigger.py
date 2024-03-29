#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later

from decimal import Decimal
from periphery import I2C
import argparse
from .  import gsi2c


def main():
    parser = argparse.ArgumentParser(description="Sync Trigger",prog="sync_trigger")
    parser.add_argument('-f', dest='fps', metavar='fps', required=True, help='set frame rate')
    parser.add_argument('-c', dest='config', action='store_true', help='configurate sync and trigger')
    args = parser.parse_args()

    gsi2c.i2c = I2C("/dev/links/csi0_i2c")
    gsi2c.write32(0x64,round(256 * 1000_000 / Decimal(args.fps))) # set fps in microseconds
    print("%8X"%round(256 * 1000_000 / Decimal(args.fps)))

    print("%3.2f fps (%7.2f us)\n"%(Decimal(args.fps),(1000_000 / Decimal(args.fps))))

    if args.config == True:
        gsi2c.i2c = I2C("/dev/links/csi0_i2c")
        gsi2c.write16(0x60,0x3FFF) # set duty cycle to 0.25 ~0.5
        gsi2c.write8(0x68,0x07) # enable sync (pwm)
        gsi2c.write16(0x58,0x302) # wait for trigger

        gsi2c.i2c = I2C("/dev/links/csi1_i2c")
        gsi2c.write16(0x58,0x302) # wait for trigger

    print()

if __name__ == "__main__":
    main()