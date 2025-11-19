## Kernel V4L2 module and python helpers for Videology AP1302 based camera modules.

***
Note: this module is valid only for Linux kernels > 5.12

#### 1. Follow Scailx document online: scailx software manual / Developing Software on Scailx / Build loadable module on device.

#### https://videology-inc.atlassian.net/wiki/spaces/SUD/pages/18579457/Build+loadable+module+on+device

#### Prepare files to build Global shutter camera ar0234 module from C codes.

#### Follow steps as follows ...

#### `cd /usr/src/kernel`
#### `gunzip < /proc/config.gz > .config`
#### `make oldconfig`
#### `make prepare`
#### `make scripts`

#### 2. Check out this repository on Scailx camera.

#### 3. cd to local folder on camera, and build + install module.
####	`make` and `make modules_install`

#### 4. Check module is loaded `lsmod`.

#### 5. Check newly built module is in correct place.
####	'ls -lt /lib/modules/6*/updates*/vid_isp_ar0234.ko`

#### 6.	Reboot camera and run `dmesg` to see new messages you add to the C codes ;-)

===========================================================================

#### 7. To test python program to do vptz commands, go to ~/ap1302py/examples folder
####	`cd /root/kernel_mod_vid_ap1302/ap1302py/examples`

#### 8. On Windows, run VLC Player with correct camera stream path
####	rtsp://scailx-ai.local:8554/cam0-gs-AR0234_1080p

####	Here scailx-ai.local is the Scailx camera's host name. Make sure it matches your camera host name or just use the IP address instead.

#### 9.	Now on camera console, run python program zoom_flip_mirror.py to see camera stream effect on VLC player, for examples

####	`python3 zoom_flip_mirror.py -h`	to show help
####	`python3 zoom_flip_mirror.py -z 1`  to see zoom-in test on VLC player.
####	`python3 zoom_flip_mirror.py -m 1`  to see mirror test on VLC player.
####	`python3 zoom_flip_mirror.py -s 1`  to see zoom/pan/tilt speed test on VLC player.
####	`python3 zoom_flip_mirror.py -p 1`  to see pan test on VLC player.
####	`python3 zoom_flip_mirror.py -r 1`  to see pan + mirror test on VLC player.
####	`python3 zoom_flip_mirror.py -t 1`  to see tilt + flip test on VLC player.

***
