#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later

import argparse
import math
import time

from periphery import I2C
from ap1302py import gsi2c


video_format = {
    0: [1920, 1200],
    1: [1600, 1200],
    2: [1200, 1200],
    3: [1920, 1080],
    4: [1440, 1080],
    5: [1080, 1080],
    6: [1366, 1024],
    7: [1280, 1024],
    8: [1024, 1024],
    9: [1280, 960],
    10: [1366, 768],
    11: [1024, 768],
    12: [1280, 720],
    13: [960, 720],
    14: [1024, 576],
    15: [768, 576],
    16: [960, 540],
    17: [720, 540],
    18: [854, 480],
    19: [640, 480],
}


# Set / Get Zoom.
# read zoom and get zoom use different registers: 0x18 (RW) and 0x1A (RO)
def read_zoom() -> float:
    """
    16 bits = s7.8: 1-bit sign, 7-bit integer, 8-bit fraction
    Treat value as a standard int16 type.
    """

    val = gsi2c.read16(0x18)
    v = val / 256.0
    return v if v <= 128 else v - 256


def get_zoom():
    """
    16 bits = s7.8: 1-bit sign, 7-bit integer, 8-bit fraction
    Treat value as a standard int16 type.
    """

    val = gsi2c.read16(0x1A)
    v = val / 256.0
    return v


def set_zoom(scale: float):
    """
    (0, 128):   Use fov.
    (-128, 0):  User set scale.
    """

    scale = min(max(scale, -40), 40)
    vali = int(scale * 256)
    gsi2c.write16(0x18, vali)


def reset_zoom():
    gsi2c.write16(0x18, 0x0100)
    # Also reset zoom speed.
    gsi2c.write8(0x1C, 0x80)
    # as well as pan and tilt
    gsi2c.write8(0x1D, 0x40)
    gsi2c.write8(0x1E, 0x40)
    


# Set / Get zoom speed, effective when zoom-in or zoom-out.
def get_zoom_speed():
    """
    0x80 = Immediate    (This should be default value instead of 0 ;-)
    0x00 = stop
    1(0x01) ~ 127(0x7F) = Linear
    (neg.)1(0xFF) ~ (neg.)127(0x81) = Fractional
    (Becomes slower when approaching zoom target)

    """

    val = gsi2c.read8(0x1C)
    return val if val <= 128 else val - 256


def set_zoom_speed(val):
    """
    val in [-127, 128]
    """

    val = min(max(val, -127), 128)
    gsi2c.write8(0x1C, val)


# Set / Get Pan / Tilt in zoom-in mode.
def get_pan():
    """
    Horizontal pan in zoom-in mode.

    0 = center
    [-64, -1] = pan left
    [1, 64] = pan right
    """
    val = gsi2c.read8(0x1D) - 0x40

    return val


def set_pan(val):
    """
    val in [-64, 64]
    """
    val = min(max(val, -64), 64) + 64
    gsi2c.write8(0x1D, val)


def get_tilt():
    """
    Vertical Tilt in zoom-in mode.

    0 = center
    [-64, -1] = pan up
    [1, 64] = pan down
    """
    val = gsi2c.read8(0x1E) - 0x40

    return val


def set_tilt(val):
    """
    val in [-64, 64]
    """
    val = min(max(val, -64), 64) + 64
    gsi2c.write8(0x1E, val)


# Get / Set mirror and flip.
def get_mirror_flip():
    """
    0 = Normal
    1 = Mirror horizontal
    2 = Flip vertical
    3 = Flip vertical and mirror horizontal

    return mirror, flip
    """
    val = gsi2c.read8(0x1F)
    flip = (val >= 2)
    mirror = (val == 1) or (val == 3)
    return mirror, flip


def set_mirror(mode):
    """
    mode = 1 (On) or 0 (Off)
    """

    if (mode != 1) and (mode != 0):
        mode = 1
    # Get current mode
    mirror, flip = get_mirror_flip()
    if mirror != mode:
        gsi2c.write8(0x1F, mode + flip * 2)  # mirror bit[0] / flip bit[1]


def set_flip(mode):
    """
    mode = 1 (On) or 0 (Off)
    """

    if (mode != 1) and (mode != 0):
        mode = 1
    # Get current mode
    mirror, flip = get_mirror_flip()
    if flip != mode:
        gsi2c.write8(0x1F, mirror + mode * 2)  # mirror bit[0] / flip bit[1]


def reset_mirror_flip():
    gsi2c.write8(0x1F, 0x00)


# Test zoom.
def test_zoom():

    reset_zoom()
    print("Zoom Read = ", read_zoom())
    print("Get Zoom = ", get_zoom())
    print("Zoom Speed = ", get_zoom_speed())

    print("Pan Horizontal = ", get_pan())
    print("Tilt Vertical = ", get_tilt())

    # Test some zoom scales (>0 by fov, <0 by user).
    for s in [2.0, -1.5, -1.0, -0.5, 1.5]:
        time.sleep(2)
        print(f"\nSet Zoom: {s}")
        set_zoom(s)
        print("Zoom Read = ", read_zoom())
        print("Get Zoom = ", get_zoom())


# Test zoom speed.
def test_zoom_speed():

    reset_zoom()
    print("Zoom = ", get_zoom(), ", Zoom Speed = ", get_zoom_speed())
    # Use defaukt zoom speed.
    set_zoom(1.5)
    time.sleep(1)
    print("Zoom = ", get_zoom(), ", Zoom Speed = ", get_zoom_speed())

    set_zoom_speed(10)
    set_zoom_speed(-10)
    set_zoom(2.0)
    time.sleep(4)
    print("Zoom = ", get_zoom(), ", Zoom Speed = ", get_zoom_speed())

    set_zoom_speed(100)
    set_zoom_speed(-20)
    set_zoom(0.5)
    time.sleep(4)
    print("Zoom = ", get_zoom(), ", Zoom Speed = ", get_zoom_speed())


# Test zoom + pan or tilt.
def test_zoom_pan_tilt():
    reset_zoom()
    
    set_zoom(2.0)
    set_zoom_speed(10)
    print(f"Pan = {get_pan()}, Tilt = {get_tilt()}")

    set_pan(-10)
    time.sleep(2)
    print(f"Pan = {get_pan()}, Tilt = {get_tilt()}")
    
    set_pan(20)
    time.sleep(2)
    print(f"Pan = {get_pan()}, Tilt = {get_tilt()}")
    
    set_tilt(-30)
    time.sleep(2)
    print(f"Pan = {get_pan()}, Tilt = {get_tilt()}")
    
    set_tilt(40)
    time.sleep(2)
    print(f"Pan = {get_pan()}, Tilt = {get_tilt()}")

    set_zoom(1.0)
    
# Test zoom + pan + mirror.
def test_zoom_pan_mirror():
    reset_zoom()
    reset_mirror_flip()
    
    set_zoom(2.0)
    set_zoom_speed(10)
    mirror, flip = get_mirror_flip()
    print(f"Pan = {get_pan()}, Mirror = {mirror}")

    set_pan(40)
    time.sleep(2)
    set_mirror(1)
    mirror, flip = get_mirror_flip()
    print(f"Pan = {get_pan()}, Mirror = {mirror}")

    time.sleep(2)
    reset_mirror_flip()


# Test zoom + tilt + flip.
def test_zoom_tilt_flip():
    reset_zoom()
    reset_mirror_flip()
    
    set_zoom(2.0)
    set_zoom_speed(10)
    mirror, flip = get_mirror_flip()
    print(f"Tilt = {get_tilt()}, Mirror = {mirror}, Flip = {flip}")

    set_tilt(40)
    time.sleep(2)
    set_flip(1)
    mirror, flip = get_mirror_flip()
    print(f"Tilt = {get_tilt()}, Mirror = {mirror}, Flip = {flip}")

    time.sleep(2)
    reset_mirror_flip()


# Test mirror and flip.
def test_mirror_flip():

    mirror, flip = get_mirror_flip()
    print("Mirror = ", mirror)
    print("Flip = ", flip)

    set_mirror(1)
    mirror, flip = get_mirror_flip()
    print("Mirror = ", mirror)
    time.sleep(1)

    set_mirror(0)
    mirror, flip = get_mirror_flip()
    print("Mirror = ", mirror)
    time.sleep(1)

    set_flip(1)
    mirror, flip = get_mirror_flip()
    print("Flip = ", flip)
    time.sleep(1)

    set_flip(0)
    mirror, flip = get_mirror_flip()
    print("Flip = ", flip)
    time.sleep(1)

    reset_mirror_flip()
    mirror, flip = get_mirror_flip()
    print("Mirror = ", mirror)
    print("Flip = ", flip)


"""
    gsi2c.write16(0x04, 0x0000) # contrast 0x0000 is center
    print("Contrast = 0x%04X" % gsi2c.read16(0x04))

"""

if __name__ == "__main__":

    parser = argparse.ArgumentParser(
        description="Camera Control Test", prog="zoom_flip_mirror"
    )

    parser.add_argument(
        "-i", "--i2c", default="/dev/links/csi0_i2c", help="i2c device path"
    )

    parser.add_argument("-z", "--zoomtest", type=bool, default=False, help="Zoom Test")
    parser.add_argument(
        "-m", "--mirrortest", type=bool, default=False, help="Mirror / Flip Test"
    )

    parser.add_argument(
        "-s", "--speedtest", type=bool, default=False, help="Zoom Speed Test"
    )

    parser.add_argument(
        "-p", "--pantest", type=bool, default=False, help="Zoom Pan and Tilt Test"
    )

    parser.add_argument(
        "-r", "--panmirror", type=bool, default=False, help="Zoom Pan and Mirror Test"
    )
    parser.add_argument(
        "-t", "--tiltflip", type=bool, default=False, help="Zoom Tilt and Flip Test"
    )

    args = parser.parse_args()

    # i2c device init
    gsi2c.i2c = I2C(args.i2c)

    if args.zoomtest:
        test_zoom()
    elif args.mirrortest:
        test_mirror_flip()
    elif args.speedtest:
        test_zoom_speed()
    elif args.pantest:
        test_zoom_pan_tilt()
    elif args.panmirror:
        test_zoom_pan_mirror()
    elif args.tiltflip:
        test_zoom_tilt_flip()
