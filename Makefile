
vid_isp_ar0234-objs = cam_ar0234.o gs_ap1302.o
obj-m += vid_isp_ar0234.o

# EXTRA_CFLAGS += -DDEBUG

KERNEL_SRC ?= /usr/src/kernel

default:
	make -C ${KERNEL_SRC} M=$(CURDIR) modules

modules_install:
	make -C ${KERNEL_SRC} M=$(CURDIR) modules_install

clean:
	make -C ${KERNEL_SRC} M=$(CURDIR) clean

mod:
	rmmod -f vid_isp_ar0234 || echo cant remove
	modprobe vid_isp_ar0234

everything: default modules_install
	rmmod -f vid_isp_ar0234 || echo cant remove
	modprobe vid_isp_ar0234

