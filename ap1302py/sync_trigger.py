#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later

from decimal import Decimal
from periphery import I2C
import argparse
from . import gsi2c
from .sync_gpio_trigger import toggle_sync_gpios
import glob

def main():
    busses = glob.glob('/dev/links/csi*_i2c')
    parser = argparse.ArgumentParser(description="Sync Trigger",prog="sync_trigger")
    parser.add_argument('-i', dest='iic', metavar='iic', nargs='*', default=list(busses), help=f'one or more /dev/i2c* devices with sensors. Default {busses}')
    parser.add_argument('-f', dest='fps', metavar='fps', help='Set PWM output on sensor sync-out pin to frequency.')
    parser.add_argument('-c', dest='sync_in', action='store_true', help='enable sync input. will wait indefinitely until sync-in signal is received')
    parser.add_argument('-p', dest='gpiofps', type=float, help='Toggle processor named GPIOs listed in "gpios" argument to sync video at frequency specified in Hz. Selects -c (default: Dont toggle GPIOs)')
    parser.add_argument('-g', dest='gpios', type=str, nargs='+', default=['CSI0-TRIGGER', 'CSI1-TRIGGER'], help='Names of GPIO lines used with -p (default: CSI0-TRIGGER CSI1-TRIGGER)')

    args = parser.parse_args()

    i2cs = [I2C(iic) for iic in args.iic]

    if args.fps != None :
        print("Tigger set fps")
        print("%3.2f fps (%7.2f us)\n"%(Decimal(args.fps),(1000_000 / Decimal(args.fps))))
        for i2c in i2cs:
            gsi2c.i2c = i2c
            gsi2c.write32(0x64,round(256 * 1000_000 / Decimal(args.fps))) # set fps in microseconds
            gsi2c.write16(0x60,0x3FFF) # set duty cycle to 0.25 ~0.5
            gsi2c.write8(0x68,0x07) # enable sync-out (pwm)
    else:
        for i2c in i2cs:
            gsi2c.i2c = i2c
            gsi2c.write8(0x68,0x06) # disable sync-out (pwm)

    if args.sync_in or args.gpiofps != None:
        print("sync-in on Trigger")
        for i2c in i2cs:
            gsi2c.i2c = i2c
            gsi2c.write16(0x58,0x302) # wait for trigger
    else:
        print("trigger internal")
        for i2c in i2cs:
            gsi2c.i2c = i2c
            gsi2c.write16(0x58,0x300) # wait for trigger

    if args.gpiofps != None:
        toggle_sync_gpios(args.gpios, args.gpiofps)

if __name__ == "__main__":
    main()
