#!/usr/bin/env python3

import gsi2c


import argparse


parser = argparse.ArgumentParser(description="Set password ",prog="password")
parser.add_argument('-p',  dest='password', metavar='password', type=lambda x: int(x,0), default=0, help='set password')
args = parser.parse_args()


def main():
    gsi2c.set_password(args.password)
    print()

if __name__ == "__main__":
    main()