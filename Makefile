
vid_isp_ar0234-objs = cam_ar0234.o gs_ap1302.o gs_image_update.o
obj-m += vid_isp_ar0234.o

# EXTRA_CFLAGS += -DDEBUG

KERNEL_VERSION ?= $(shell uname -r)
KERNEL_SRC ?= /usr/src/kernel
INSTALL_FW_PATH ?= $(DESTDIR)/lib/firmware

default:
	make -C ${KERNEL_SRC} M=$(CURDIR) modules

install_firmware:
	install -d ${INSTALL_FW_PATH}
	install -Dm0600 firmware/* ${INSTALL_FW_PATH}/

modules_install: install_firmware
	make -C ${KERNEL_SRC} M=$(CURDIR) KERNELRELEASE=$(KERNEL_VERSION) modules_install

clean:
	make -C ${KERNEL_SRC} M=$(CURDIR) clean

mod:
	rmmod -f vid_isp_ar0234 || echo cant remove
	modprobe vid_isp_ar0234

everything: default modules_install
	rmmod -f vid_isp_ar0234 || echo cant remove
	modprobe vid_isp_ar0234

