#!/usr/bin/env python3

import gsi2c


# examples

print("Format Type = 0x%02X" % gsi2c.read8(0x11)) # read the format type

#gsi2c.write16(0x02, 0x0000) # brighntness 0x0000 is center
print("Brightness = 0x%04X" % gsi2c.read16(0x02))

gsi2c.write16(0x04, 0x0000) # contrast 0x0000 is center
print("Contrast = 0x%04X" % gsi2c.read16(0x04))

gsi2c.write16(0x06, 0x1000) # saturation 0x1000 is 1.0
print("Saturation = 0x%04X" % gsi2c.read16(0x06))

gsi2c.write16(0x0A, 0x0000) # sharpen 0x0000 is center
print("Sharpness = 0x%04X" % gsi2c.read16(0x0A))

gsi2c.write16(0x0C, 0x0000) # noise 0x0000 is center, max = 0x7FFF
print("Noise Reduction = 0x%04X" % gsi2c.read16(0x0C))

gsi2c.write8(0x1F, 0x00) # mirror bit[0] / flip bit[1]

print("MCU version = %d.%d" %(gsi2c.read8(0xFF),gsi2c.read8(0xFE)))

print("NVM version = %d.%d" %(gsi2c.read8(0xEF),gsi2c.read8(0xEE)))

print("ISP version = %d" %(gsi2c.read8(0xED)<<8 | gsi2c.read8(0xEC)))

gsi2c.write16(0x16, 0x01E00)

def main():
    print()

if __name__ == "__main__":
    main()
