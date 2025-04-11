#!/usr/bin/env python3
"""
GPIO Synchronization Trigger
Toggles multiple GPIO pins at a specified frequency for camera synchronization.
"""

import argparse
import sys, os, time
import re
from periphery import GPIO
import timerfd

def find_gpio_by_name(gpio_name):
    with open("/sys/kernel/debug/gpio", "r") as f:
        chip = ""
        offset = 0
        for line in f:
            if line.startswith("gpiochip"):
                match = re.match(r"(gpiochip\d+): GPIOs (\d+)-\d+", line)
                if match:
                    chip = "/dev/" + match.group(1)
                    offset = int(match.group(2))
            if gpio_name in line:
                parts = line.strip().split()
                for part in parts:
                    if part.startswith("gpio-"):
                        gpio_num = int(part.replace("gpio-", ""))
                        return chip, gpio_num - offset
    return None, None

def toggle_sync_gpios(gpios, freq):
    print(f"GPIO pins: {gpios}")
    print(f"Frequency: {freq} Hz (period: {1000/freq:.2f}ms)")
    timer = timerfd.create(timerfd.CLOCK_REALTIME, 0)
    timerfd.settime(timer, 0, 0.1, 1.0 / freq)

    # Initialize GPIO lines
    gpio_pins = []
    try:
        for gpio_name in gpios:
            chip, num = find_gpio_by_name(gpio_name)
            if chip:
                gpio = GPIO(chip, num, "out")
                gpio.write(False)  # Initialize to low
                gpio_pins.append(gpio)
    except Exception as e:
        print(f"Failed to initialize GPIO lines: {e}")
        # Clean up any pins that were opened
        for gpio in gpio_pins:
            gpio.close()
        return 1

    # Main loop
    print("Running... Press Ctrl+C to exit.")
    while True:
        try:
            os.read(timer, 100)
            # time.sleep(0.5/freq)

            # Set all GPIOs high
            for gpio in gpio_pins:
                gpio.write(True)

            # Wait for pulse width duration
            # time.sleep(0.5/freq)
            time.sleep(0.002)

            # Set all GPIOs low
            for gpio in gpio_pins:
                gpio.write(False)

        except KeyboardInterrupt:
            break
        except Exception as e:
            print(f"Error: {e}")
            break

    # Cleanup
    for gpio in gpio_pins:
        gpio.write(False)
        gpio.close()

    return 0

def main():
    parser = argparse.ArgumentParser(description='GPIO Synchronization Trigger')
    parser.add_argument('--gpios', type=str, nargs='+', default=['CSI0-TRIGGER', 'CSI1-TRIGGER'], help='Names of GPIO lines (default: CSI0-TRIGGER CSI1-TRIGGER)')
    parser.add_argument('--freq', type=float, default=16.5, help='Frequency in Hz (default: 16.5)')

    args = parser.parse_args()

    toggle_sync_gpios(args.gpios, args.freq)

if __name__ == "__main__":
    sys.exit(main())
