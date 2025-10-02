#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later

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


# Set / Get Zoom and Pan
# read zoom and get zoom use different registers: 0x18 (RW) and 0x1A (RO)
def read_zoom()->float:
    """
    16 bits = s7.8: 1-bit sign, 7-bit integer, 8-bit fraction
    """

    val = gsi2c.read16(0x18)
    sign = val >> 15
    val = val & 0x7F
    v = val / 256.0
    return -v if sign else v


def get_zoom():
    """
    16 bits = s7.8: 1-bit sign, 7-bit integer, 8-bit fraction
    """

    val = gsi2c.read16(0x1A)
    sign = val >> 15
    val = val & 0x7F
    v = val / 256.0
    return -v if sign else v


def set_zoom(scale: float):
    """
    (0, 128):   Use fov.
    (-128, 0):  User set scale.
    """

    scale = min(max(scale, -40), 40)
    sign = scale < 0
    vali = int(abs(scale)*256)
    valw = vali + (sign <<15)
    gsi2c.write16(0x18, valw)

def reset_zoom():
    gsi2c.write16(0x18, 0x0100)
    

def get_zoom_speed():
    """
    0x80 = Immediate
    0x00 = stop
    1(0x01) ~ 127(0x7F) = Linear
    (neg.)1(0xFF) ~ (neg.)127(0x81) = Fractional
    (Becomes slower when approaching zoom target)

    """

    val = gsi2c.read8(0x1C)
    return val if val <= 128 else 1.0 / (256 - val)


def pan_horizontal():
    """
    0 = center
    [-64, -1] = pan left
    [1, 64] = pan right
    """
    val = gsi2c.read8(0x1D) - 0x40

    return val


def pan_vertical():
    """
    0 = center
    [-64, -1] = pan up
    [1, 64] = pan down
    """
    val = gsi2c.read8(0x1E) - 0x40

    return val


# Get / Set mirror and flip.
def get_mirror_flip():
    """
    0 = Normal
    1 = Flip vertical
    2 = Mirror horizontal
    3 = Flip vertical flip and mirror horizontal

    return mirror, flip
    """
    val = gsi2c.read8(0x1F)
    mirror = val >= 2
    flip = (val == 1) or (val == 3)
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
        gsi2c.write8(0x1F, flip + mode * 2)  # mirror bit[1] / flip bit[0]


def set_flip(mode):
    """
    mode = 1 (On) or 0 (Off)
    """

    if (mode != 1) and (mode != 0):
        mode = 1
    # Get current mode
    mirror, flip = get_mirror_flip()
    if flip != mode:
        gsi2c.write8(0x1F, mode + mirror * 2)  # mirror bit[1] / flip bit[0]


def reset_mirror_flip():
    gsi2c.write8(0x1F, 0x00)


# Test zoom.
def test_zoom():

    set_zoom(2.0)

    print("Zoom Read = ", read_zoom())
    print("Get Zoom = ", get_zoom())
    print("Zoom Speed = ", get_zoom_speed())

    print("Pan Horizontal = ", pan_horizontal())
    print("Pan Vertical = ", pan_vertical())

    time.sleep(2)
    set_zoom(0.75)

    print("Zoom Read = ", read_zoom())
    print("Get Zoom = ", get_zoom())


# Test mirror and flip.
def test_mirror_flip():

    mirror, flip = get_mirror_flip()
    print("Mirror = ", mirror)
    print("Flip = ", flip)

    set_mirror(1)
    time.sleep(1)
    set_mirror(0)
    time.sleep(1)

    set_flip(1)
    time.sleep(1)
    set_flip(0)
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
    # i2c device path
    gsi2c.i2c = I2C("/dev/links/csi0_i2c")

    test_zoom()

    # test_mirror_flip()
