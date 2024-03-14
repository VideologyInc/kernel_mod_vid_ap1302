#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later

import sys
from . import flashapp, flashisp, readreg, status, sync_trigger, writereg


def usage():
    print(f"Usage: {__name__} command [args] [--help]")
    print(f"Commands:")
    print(f"  readreg")
    print(f"  flashisp")
    print(f"  flashapp")
    print(f"  status")
    print(f"  sync_trigger")
    print(f"  writereg")

def main():
    # get the first argument
    if len(sys.argv) < 2:
        usage()
        sys.exit(1)
    first_arg = sys.argv[1]
    if first_arg in ["readreg", "flashisp", "flashapp", "status", "sync_trigger", "writereg"]:
        # remove the first argument
        sys.argv.pop(1)
        if first_arg == "readreg":
            readreg.main()
        elif first_arg == "flashisp":
            flashisp.main()
        elif first_arg == "flashapp":
            flashapp.main()
        elif first_arg == "status":
            status.main()
        elif first_arg == "sync_trigger":
            sync_trigger.main()
        elif first_arg == "writereg":
            writereg.main()
    else:
        usage()
        sys.exit(1)

if __name__ == "__main__":
    main()