/*
 * Copyright (C) 2023 Videology Inc, Inc. All Rights Reserved.
 */

#include <linux/clk.h>
#include <linux/ctype.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/regmap.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/firmware.h>
#include <linux/pinctrl/consumer.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/kmod.h>
#include <media/v4l2-async.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-device.h>
#include <media/v4l2-event.h>
#include <media/v4l2-fwnode.h>
#include <media/v4l2-subdev.h>
#include <linux/version.h>

#include "cam_ar0234.h"
#include "gs_ap1302.h"
#include "gs_image_update.h"

#define MCU_FIRMWARE_VERSION 0x001B // version = 0.27
#define NVM_FIRMWARE_VERSION 0x0001 // version = 0.1 (un-even version for GPIO 8)
//#define NVM_FIRMWARE_VERSION 0x0002 // version = 0.2 (even version for GPIO 7)
#define ISP_FIRMWARE_VERSION 443

// MCU firmware contains both mcu and nvm in a single image therefore it has two version numbers.
#define MCU_FIRMWARE_NAME "SFT-23361_mcu_0.26_0.1.img"   				// trigger on gpio 8
//#define MCU_FIRMWARE_NAME "SFT-23361_mcu_0.26_0.2.img" 				// trigger on gpio 7

#define NVM_FIRMWARE_NAME "SFT-23363_nvm_0.1.img" 						// trigger on gpio 8
//#define NVM_FIRMWARE_NAME "SFT-23363_nvm_0.2.img"						// trigger on gpio 7

#define ISP_COLOR_FIRMWARE_NAME "SFT-24147_full_color_443.img"	// color
#define ISP_MONO_FIRMWARE_NAME "SFT-24148_full_mono_443.img"		// monochrome


//get module parameter from /etc/modprobe.d/vid_isp_ar0234.conf file 
static int nvm_firmware_versions[MAX_CAMERA_DEVICES] = { [0 ...(MAX_CAMERA_DEVICES - 1)] = NVM_FIRMWARE_VERSION };
module_param_array(nvm_firmware_versions, int, NULL, 0444);
MODULE_PARM_DESC(devices, "nvm version numbers");

static char * nvm_firmware_names[MAX_CAMERA_DEVICES];
module_param_array(nvm_firmware_names, charp, NULL, 0444);
MODULE_PARM_DESC(devices, "nvm file names");

#if 0
static int mcu_firmware_versions[MAX_CAMERA_DEVICES] = { [0 ...(MAX_CAMERA_DEVICES - 1)] = NVM_FIRMWARE_VERSION };
module_param_array(mcu_firmware_versions, int, NULL, 0444);
MODULE_PARM_DESC(devices, "mcu version numbers");

static char * mcu_firmware_names[MAX_CAMERA_DEVICES];
module_param_array(mcu_firmware_names, charp, NULL, 0444);
MODULE_PARM_DESC(devices, "mcu file names");

static int isp_color_firmware_versions[MAX_CAMERA_DEVICES] = { [0 ...(MAX_CAMERA_DEVICES - 1)] = NVM_FIRMWARE_VERSION };
module_param_array(isp_color_firmware_versions, int, NULL, 0444);
MODULE_PARM_DESC(devices, "isp color version numbers");

static char * isp_color_firmware_names[MAX_CAMERA_DEVICES];
module_param_array(isp_color_firmware_names, charp, NULL, 0444);
MODULE_PARM_DESC(devices, "isp color file names");

static int isp_mono_firmware_versions[MAX_CAMERA_DEVICES] = { [0 ...(MAX_CAMERA_DEVICES - 1)] = NVM_FIRMWARE_VERSION };
module_param_array(isp_mono_firmware_versions, int, NULL, 0444);
MODULE_PARM_DESC(devices, "isp mono version numbers");

static char * isp_mono_firmware_names[MAX_CAMERA_DEVICES];
module_param_array(isp_mono_firmware_names, charp, NULL, 0444);
MODULE_PARM_DESC(devices, "isp mono file names");
#endif

#ifdef DEBUG
static int gs_print_params(void)
{
	for(int n = 0; n < MAX_CAMERA_DEVICES; n++)
	{
		if(nvm_firmware_names[n] == NULL) break;
		pr_debug("---%s: %s  (%x)\n",__func__,nvm_firmware_names[n], nvm_firmware_versions[n]);
		//pr_debug("---%s: %s  (%x)\n",__func__,mcu_firmware_names[n], mcu_firmware_versions[n]);
		//pr_debug("---%s: %s  (%x)\n",__func__,isp_color_firmware_names[n], isp_color_firmware_versions[n]);
		//pr_debug("---%s: %s  (%x)\n",__func__,isp_mono_firmware_names[n], isp_mono_firmware_versions[n]);
	}
	return 0;
}
#endif



static int gs_ar0234_i_cntrl(struct gs_ar0234_dev *sensor);


static struct resolution sensor_res_list[] = {
	{.width = 1280, .height = 720,  .framerate = 25, .frame_format_code = 12, .name="720p25"  },
	{.width = 1280, .height = 720,  .framerate = 30, .frame_format_code = 12, .name="720p30"  },
	{.width = 1280, .height = 720,  .framerate = 50, .frame_format_code = 12, .name="720p50"  },
	{.width = 1280, .height = 720,  .framerate = 60, .frame_format_code = 12, .name="720p60"  },

	{.width = 1280, .height = 960,  .framerate = 25, .frame_format_code = 9, .name="960p25"  },
	{.width = 1280, .height = 960,  .framerate = 30, .frame_format_code = 9, .name="960p30"  },
	{.width = 1280, .height = 960,  .framerate = 50, .frame_format_code = 9, .name="960p50"  },
	{.width = 1280, .height = 960,  .framerate = 60, .frame_format_code = 9, .name="960p60"  },

	{.width = 1920, .height = 1080, .framerate = 25, .frame_format_code = 3, .name="1080p25" },
	{.width = 1920, .height = 1080, .framerate = 30, .frame_format_code = 3, .name="1080p30" },
	{.width = 1920, .height = 1080, .framerate = 50, .frame_format_code = 3, .name="1080p50" },
	{.width = 1920, .height = 1080, .framerate = 60, .frame_format_code = 3, .name="1080p60" },

	{.width = 1440, .height = 1080, .framerate = 25, .frame_format_code = 4, .name="1080x1080@25" },
	{.width = 1440, .height = 1080, .framerate = 30, .frame_format_code = 4, .name="1080x1080@30" },
	{.width = 1440, .height = 1080, .framerate = 50, .frame_format_code = 4, .name="1080x1080@50" },
	{.width = 1440, .height = 1080, .framerate = 60, .frame_format_code = 4, .name="1080x1080@60" },

	{.width = 1080, .height = 1080, .framerate = 25, .frame_format_code = 5, .name="1080x1080@25" },
	{.width = 1080, .height = 1080, .framerate = 30, .frame_format_code = 5, .name="1080x1080@30" },
	{.width = 1080, .height = 1080, .framerate = 50, .frame_format_code = 5, .name="1080x1080@50" },
	{.width = 1080, .height = 1080, .framerate = 60, .frame_format_code = 5, .name="1080x1080@60" },

	{.width = 1024, .height = 1024, .framerate = 25, .frame_format_code = 11, .name="1024x1024@25" },
	{.width = 1024, .height = 1024, .framerate = 30, .frame_format_code = 11, .name="1024x1024@30" },
	{.width = 1024, .height = 1024, .framerate = 50, .frame_format_code = 11, .name="1024x1024@50" },
	{.width = 1024, .height = 1024, .framerate = 60, .frame_format_code = 11, .name="1024x1024@60" },

	{.width = 1280, .height = 1024, .framerate = 25, .frame_format_code = 7, .name="1280x1024@25" },
	{.width = 1280, .height = 1024, .framerate = 30, .frame_format_code = 7, .name="1280x1024@30" },
	{.width = 1280, .height = 1024, .framerate = 50, .frame_format_code = 7, .name="1280x1024@50" },
	{.width = 1280, .height = 1024, .framerate = 60, .frame_format_code = 7, .name="1280x1024@60" },
};

static inline struct gs_ar0234_dev *to_gs_ar0234_dev(struct v4l2_subdev *sd)
{
	return container_of(sd, struct gs_ar0234_dev, sd);
}

static inline struct v4l2_subdev *ctrl_to_sd(struct v4l2_ctrl *ctrl)
{
	return &container_of(ctrl->handler, struct gs_ar0234_dev,
			     ctrls.handler)->sd;
}

#if  0
// FIXME: implement everything in kernel I2C commands
//static char upcall_string[512];
static int gs_ar0234_upcall(struct gs_ar0234_dev *sensor, char *call_str)
{
	char *argv[5], *envp[5];

	dev_dbg_ratelimited(sensor->dev, "%s: \n", __func__);

	argv[0] = "/usr/bin/python3";
	argv[1] = "/bin/gs_ar0234_upcall.py";
	argv[2] = call_str;
	argv[3] = NULL;

	/* minimal command environment taken from cpu_run_sbin_hotplug */
	envp[0] = "HOME=/home/root/";
	envp[1] = "PATH=/sbin:/bin:/usr/sbin:/usr/bin";
	envp[2] = NULL;

	// call_usermodehelper(argv[0], argv, envp, UMH_WAIT_EXEC);
	return 0;
}
#endif

#if 0
static int print_upcall(char * print_str)
{
	char *argv[5], *envp[5];

	argv[0] = "/bin/echo.coreutils";
	argv[1] = print_str;
	argv[2] = ">>";
	argv[3] = "/home/root/gs0234_feature/log.txt";
	argv[4] = NULL;

	envp[0] = "HOME=/home/root/";
	envp[1] = "PATH=/sbin:/bin:/usr/sbin:/usr/bin";
	envp[2] = NULL;

	call_usermodehelper(argv[0], argv, envp, UMH_WAIT_EXEC);
	return 0;
}
#endif


/* --------------- Subdev Operations --------------- */

static int gs_ar0234_s_power(struct v4l2_subdev *sd, int on)
{
	int ret=0;
	struct gs_ar0234_dev *sensor = to_gs_ar0234_dev(sd);
	dev_dbg(sensor->dev, "%s: %s\n", __func__, on ? "up" : "down");
	mutex_lock(&sensor->lock);
	ret = gs_ar0234_power(sensor, on);
	mutex_unlock(&sensor->lock);
	return ret;
}

static int ops_get_fmt(struct v4l2_subdev *sub_dev, struct v4l2_subdev_state *sd_state, struct v4l2_subdev_format *format)
{
	struct gs_ar0234_dev *sensor = to_gs_ar0234_dev(sub_dev);
	struct v4l2_mbus_framefmt *fmt;

	dev_dbg(sub_dev->dev, "%s: \n", __func__);

	if (format->pad >= NUM_PADS)
		return -EINVAL;

	if (format->which == V4L2_SUBDEV_FORMAT_TRY)
		fmt = v4l2_subdev_get_try_format(&sensor->sd, sd_state, format->pad);
	else
		fmt = &sensor->fmt;

	format->format = *fmt;
	return 0;
}

static int gs_ar0234_set_fmt(struct v4l2_subdev *sd, struct v4l2_subdev_state *sd_state, struct v4l2_subdev_format *format)
{
	struct gs_ar0234_dev *sensor = to_gs_ar0234_dev(sd);
	const struct resolution *new_mode = NULL;
	struct v4l2_mbus_framefmt *fmt = &format->format;
	int ret=0, i;

	dev_info(sensor->dev, "%s \n",__func__);

	if (format->pad >= NUM_PADS)
		return -EINVAL;

	for (i = 0; i < ARRAY_SIZE(sensor_res_list); i++) {
		if (format->format.width == sensor_res_list[i].width && format->format.height == sensor_res_list[i].height) {
			new_mode = &sensor_res_list[i];		// set mode here so illegal framerate just gets assigned a legal one.
			// a previous call to ops_set_frame_interval would have set sensor->framerate. Otherwise its the existing FR.
			if (sensor->framerate == sensor_res_list[i].framerate)
				break;
		}
	}
	if (new_mode == NULL)
		return -EINVAL;



	// TODO : implement function to set media-bus format and colorspace based on fmt

	/////////////////////////////////////////////////////////////////////////////////


	sensor->framerate = new_mode->framerate;

	if (format->which == V4L2_SUBDEV_FORMAT_TRY)
		fmt = v4l2_subdev_get_try_format(sd, sd_state, 0);
	else
		fmt = &sensor->fmt;

	*fmt = format->format;

	// if ((new_mode != sensor->mode) && (format->which != V4L2_SUBDEV_FORMAT_TRY)) {
	sensor->mode = new_mode;
	// snprintf(upcall_string, sizeof(upcall_string), "0x%02x", resolution);
	//ret |= gs_ar0234_upcall(sensor, new_mode->name);

	pr_debug("%s: sensor->ep.bus_type =%d\n", __func__, sensor->ep.bus_type);
	pr_debug("%s: sensor->ep.bus      =%p\n", __func__, &sensor->ep.bus);
	pr_debug("%s: sensor->fmt->height =%d\n", __func__, format->format.height);
	pr_debug("%s: sensor->fmt->width  =%d\n", __func__, format->format.width);
	pr_debug("%s: end \n", __func__);
	return ret;
}

static int gs_ar0234_g_volatile_ctrl(struct v4l2_ctrl *ctrl)
{
	struct v4l2_subdev *sd = ctrl_to_sd(ctrl);
	struct gs_ar0234_dev *sensor = to_gs_ar0234_dev(sd);
	int ret;
	u16 shortval;

	/* v4l2_ctrl_lock() locks our own mutex */
	dev_dbg_ratelimited(sd->dev, "%s %x: \n", __func__,ctrl->id);

	switch (ctrl->id) {
		case V4L2_CID_BRIGHTNESS:
			ret = gs_ar0234_read_reg16(sensor, GS_REG_BRIGHTNESS, &shortval);
			if (ret < 0)
				return ret;
			sensor->ctrls.brightness->val = shortval;
			break;

		default:
			ret = -EINVAL;
	}

	return ret;
}

static int gs_ar0234_s_ctrl(struct v4l2_ctrl *ctrl)
{
	struct v4l2_subdev *sd = ctrl_to_sd(ctrl);
	struct gs_ar0234_dev *sensor = to_gs_ar0234_dev(sd);
	int ret, val;
	u8 val8;
	u16 tmp;

	// if (sensor->power_count == 0)
	// 	return 0;
	dev_dbg_ratelimited(sd->dev, "%s: \n", __func__);

	switch (ctrl->id) {
	case V4L2_CID_BRIGHTNESS:
		ret = gs_ar0234_write_reg16(sensor, GS_REG_BRIGHTNESS, ctrl->val);
		dev_dbg_ratelimited(sd->dev, "%s: set brightness to %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_CONTRAST:
		ret = gs_ar0234_write_reg16(sensor, GS_REG_CONTRAST, ctrl->val);
		dev_dbg_ratelimited(sd->dev, "%s: set contrast to %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_SATURATION:
		ret = gs_ar0234_write_reg16(sensor, GS_REG_SATURATION, ctrl->val);
		dev_dbg_ratelimited(sd->dev, "%s: set saturation to %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_AUTO_WHITE_BALANCE:
			ret = gs_ar0234_write_reg8(sensor, GS_REG_WHITEBALANCE, ctrl->val == 0 ? 0x7 : 0xF); // only OFF and ON
			dev_dbg_ratelimited(sd->dev, "%s: set white balance to %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_DO_WHITE_BALANCE:
			ret = gs_ar0234_write_reg8(sensor, GS_REG_WHITEBALANCE, 0x8); // push to white, when done AWB is in manual mode
			dev_dbg_ratelimited(sd->dev, "%s: set push_to_white to %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_WHITE_BALANCE_TEMPERATURE:
		ret = gs_ar0234_write_reg16(sensor, GS_REG_WB_TEMPERATURE, ctrl->val);
		dev_dbg_ratelimited(sd->dev, "%s: set white balance temperature to %d K\n", __func__, ctrl->val);
		break;
	case V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE:
		if(ctrl->val == V4L2_WHITE_BALANCE_MANUAL)
		{ 
			if (sensor->ctrls.auto_wb->cur.val == 0) // if WB is disabled
			{
				ret = gs_ar0234_write_reg8(sensor, GS_REG_WHITEBALANCE, 0x00); // Control AWB manualy using X, Y parameters
				dev_dbg_ratelimited(sd->dev, "%s: set white balance temperature to Manual\n", __func__);
			}
		}
		else 
		{
			if (sensor->ctrls.auto_wb->cur.val == 0) // if WB is disabled
			{
				ret = gs_ar0234_write_reg8(sensor, GS_REG_WHITEBALANCE, 0x07); // Control AWB using temperature
			}
		}
		switch(ctrl->val) {
			case V4L2_WHITE_BALANCE_MANUAL: 		val = 0; break;
			//case V4L2_WHITE_BALANCE_AUTO: 		val = 0; break;
			case V4L2_WHITE_BALANCE_INCANDESCENT: 	val = 3000; break;
			case V4L2_WHITE_BALANCE_FLUORESCENT: 	val = 4000; break;
			case V4L2_WHITE_BALANCE_FLUORESCENT_H: 	val = 5000; break;
			case V4L2_WHITE_BALANCE_HORIZON: 		val = 5000; break;
			case V4L2_WHITE_BALANCE_DAYLIGHT: 		val = 6500; break;
			case V4L2_WHITE_BALANCE_FLASH: 			val = 5500; break;
			case V4L2_WHITE_BALANCE_CLOUDY: 		val = 7500; break;
			case V4L2_WHITE_BALANCE_SHADE: 			val = 9500; break;
		}
		ret = gs_ar0234_write_reg16(sensor, GS_REG_WB_TEMPERATURE, val);
		dev_dbg_ratelimited(sd->dev, "%s: set white balance temperature to %d K\n", __func__, val);
		break;
	case V4L2_CID_RED_BALANCE: // Red gain in manual WB
		break;
	case V4L2_CID_BLUE_BALANCE: // Blue gain in manual WB
		break;
	case V4L2_CID_AWB_MAN_X:
		ret = gs_ar0234_write_reg16(sensor, GS_REG_AWB_MAN_X, ctrl->val);
		dev_dbg_ratelimited(sd->dev, "%s: set white balance manual X to %d \n", __func__, ctrl->val);
		break;	
	case V4L2_CID_AWB_MAN_Y:
		ret = gs_ar0234_write_reg16(sensor, GS_REG_AWB_MAN_Y, ctrl->val);
		dev_dbg_ratelimited(sd->dev, "%s: set white balance manual Y to %d \n", __func__, ctrl->val);
		break;	
	case V4L2_CID_GAMMA:
		ret = gs_ar0234_write_reg16(sensor, GS_REG_GAMMA, ctrl->val);
		dev_dbg_ratelimited(sd->dev, "%s: set gamma to %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_EXPOSURE_AUTO: // exposure menu selection
		switch(ctrl->val) {
			case V4L2_EXPOSURE_AUTO:
				val8 = 0xC; // Auto Brightness/Exposure/Gain
				break;
			case V4L2_EXPOSURE_MANUAL:
				val8 = 0x0; // Manual Exposure, manual Gain
				break;
			case V4L2_EXPOSURE_SHUTTER_PRIORITY:
				val8 = 0x9; // Auto BV, Manual Exposure, Auto Gain
				break;
			case V4L2_EXPOSURE_APERTURE_PRIORITY:
				val8 = 0xC; //  For now use same as Auto
				break;
			default:
				val8 = 0xFF; // ignore for others
				break;
		}
		if(val8 < 0xFF) 	{
			ret = gs_ar0234_write_reg8(sensor, GS_REG_EXPOSURE_MODE, val8);
			dev_dbg_ratelimited(sd->dev, "%s: set exposure mode to %d\n", __func__, val8);
		}
		break;
	case V4L2_CID_EXPOSURE_ABSOLUTE : // reference level in (us * 100) in order to fit in 16bit:333 = 33300us
		val = ctrl->val * 100;
		ret = gs_ar0234_write_reg32(sensor, GS_REG_EXPOSURE_ABS, val);
		dev_dbg_ratelimited(sd->dev, "%s: set exposure to %d us\n", __func__, val);
		break;
	case V4L2_CID_EXPOSURE : // for now use to adjust reference or target level
		val = ctrl->val;
		//need to convert val to format s7.8
		tmp = (u16) (val * 256 / 1000);  // reverse = val * 1000 / 256
		tmp = val < 0 ? tmp - ((val%1000) > -500 ? 0 : 1) : tmp + ((val%1000) < 500 ? 0 : 1); // rounding
		ret = gs_ar0234_write_reg16(sensor, GS_REG_AE_TARGET, tmp);
		dev_dbg_ratelimited(sd->dev, "%s: set autoe exposure target to 0x%04X\n", __func__, tmp);
		break;
	case V4L2_CID_EXPOSURE_UPPER:
		val = ctrl->val * 100;
		ret = gs_ar0234_write_reg32(sensor, GS_REG_EXPOSURE_UPPER, val);
		dev_dbg_ratelimited(sd->dev, "%s: set exposure upper to %d us\n", __func__, val);
		break;
	case V4L2_CID_EXPOSURE_MAX:
		val = ctrl->val * 100;
		ret = gs_ar0234_write_reg32(sensor, GS_REG_EXPOSURE_MAX, val);
		dev_dbg_ratelimited(sd->dev, "%s: set exposure max to %d us\n", __func__, val);
		break;
	case V4L2_CID_GAIN_UPPER:
		ret = gs_ar0234_write_reg16(sensor, GS_REG_GAIN_UPPER, ctrl->val);
		dev_dbg_ratelimited(sd->dev, "%s: set gain upper to %d \n", __func__, ctrl->val);
		break;
	case V4L2_CID_GAIN_MAX:
		ret = gs_ar0234_write_reg16(sensor, GS_REG_GAIN_MAX, ctrl->val);
		dev_dbg_ratelimited(sd->dev, "%s: set gain max to %d \n", __func__, ctrl->val);
		break;
	case V4L2_CID_GAIN: // gain level
		ret = gs_ar0234_write_reg16(sensor, GS_REG_GAIN, ctrl->val);
		dev_dbg_ratelimited(sd->dev, "%s: set gain to %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_AUTOGAIN: //  ??????<tbd>
		break;
	case V4L2_CID_AUTOBRIGHTNESS: //  ?????? <tbd>
		break;
	case V4L2_CID_ISO_SENSITIVITY: //  ?????? <tbd>
		break;
	case V4L2_CID_ISO_SENSITIVITY_AUTO: //  ?????? <tbd>
		break;

	case V4L2_CID_EXPOSURE_METERING:
		switch(ctrl->val) {
			case V4L2_EXPOSURE_METERING_AVERAGE: 			val8 = 0; break;
			case V4L2_EXPOSURE_METERING_CENTER_WEIGHTED: 	val8 = 1; break;
			case V4L2_EXPOSURE_METERING_SPOT: 				val8 = 2; break;
			case V4L2_EXPOSURE_METERING_MATRIX: 			val8 = 3; break; // apointer to 8 weight table
		}
		ret = gs_ar0234_write_reg8(sensor, GS_REG_BLC_MODE, val8);
		dev_dbg_ratelimited(sd->dev, "%s: set exopure metering to %d\n", __func__, val8);
		break;
	case V4L2_CID_BACKLIGHT_COMPENSATION:
		ret = gs_ar0234_write_reg8(sensor, GS_REG_BLC_LEVEL, ctrl->val);
		dev_dbg_ratelimited(sd->dev, "%s: set blc level to %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_BLC_WINDOW_X0:
		ret = gs_ar0234_write_reg8(sensor, GS_REG_BLC_WINDOW_X0, ctrl->val);
		dev_dbg_ratelimited(sd->dev, "%s: set blc window x0 to %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_BLC_WINDOW_Y0:
		ret = gs_ar0234_write_reg8(sensor, GS_REG_BLC_WINDOW_Y0, ctrl->val);
		dev_dbg_ratelimited(sd->dev, "%s: set blc window y0 to %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_BLC_WINDOW_X1:
		ret = gs_ar0234_write_reg8(sensor, GS_REG_BLC_WINDOW_X1, ctrl->val);
		dev_dbg_ratelimited(sd->dev, "%s: set blc window x1 to %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_BLC_WINDOW_Y1:
		ret = gs_ar0234_write_reg8(sensor, GS_REG_BLC_WINDOW_Y1, ctrl->val);
		dev_dbg_ratelimited(sd->dev, "%s: set blc window y1 to %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_BLC_RATIO:
		ret = gs_ar0234_write_reg8(sensor, GS_REG_BLC_RATIO, ctrl->val);
		dev_dbg_ratelimited(sd->dev, "%s: set blc ratio to %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_BLC_FACE_LEVEL:
		ret = gs_ar0234_write_reg8(sensor, GS_REG_BLC_FACE_LEVEL, ctrl->val);
		dev_dbg_ratelimited(sd->dev, "%s: set blc face level to %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_BLC_FACE_WEIGHT:
		ret = gs_ar0234_write_reg8(sensor, GS_REG_BLC_FACE_WEIGHT, ctrl->val);
		dev_dbg_ratelimited(sd->dev, "%s: set blc face weight to %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_BLC_ROI_LEVEL:
		ret = gs_ar0234_write_reg8(sensor, GS_REG_BLC_ROI_LEVEL, ctrl->val);
		dev_dbg_ratelimited(sd->dev, "%s: set blc roi level to %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_FACE_DETECT_0:
		ret = gs_ar0234_read_reg8(sensor, GS_REG_FACE_DETECT, &val8);
		if(ret) break;
		ret = gs_ar0234_write_reg8(sensor, GS_REG_FACE_DETECT, (val8 & (~(1<<0))) | ctrl->val<<0);
		dev_dbg_ratelimited(sd->dev, "%s: set face detect b0 to %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_FACE_DETECT_4:
		ret = gs_ar0234_read_reg8(sensor, GS_REG_FACE_DETECT, &val8);
		if(ret) break;
		ret = gs_ar0234_write_reg8(sensor, GS_REG_FACE_DETECT, (val8 & (~(1<<4))) | ctrl->val<<4);
		dev_dbg_ratelimited(sd->dev, "%s: set face detect b4 to %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_FACE_DETECT_5:
		ret = gs_ar0234_read_reg8(sensor, GS_REG_FACE_DETECT, &val8);
		if(ret) break;
		ret = gs_ar0234_write_reg8(sensor, GS_REG_FACE_DETECT, (val8 & (~(1<<5))) | ctrl->val<<5);
		dev_dbg_ratelimited(sd->dev, "%s: set face detect b5 to %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_FACE_DETECT_SPEED:
		ret = gs_ar0234_write_reg8(sensor, GS_REG_FACE_DETECT_SPEED, ctrl->val);
		dev_dbg_ratelimited(sd->dev, "%s: set face detect speed to %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_FACE_DETECT_THRESHOLD:
		ret = gs_ar0234_write_reg8(sensor, GS_REG_FACE_DETECT_THRESHOLD, ctrl->val);
		dev_dbg_ratelimited(sd->dev, "%s: set face detect threshold to %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_FACE_CHROMA_THRESHOLD:
		ret = gs_ar0234_write_reg8(sensor, GS_REG_FACE_CHROMA_THRESHOLD, ctrl->val);
		dev_dbg_ratelimited(sd->dev, "%s: set face chroma threshold to %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_FACE_MIN_SIZE:
		ret = gs_ar0234_write_reg16(sensor, GS_REG_FACE_MIN_SIZE, ctrl->val);
		dev_dbg_ratelimited(sd->dev, "%s: set face min size to %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_FACE_MAX_SIZE:
		ret = gs_ar0234_write_reg16(sensor, GS_REG_FACE_MAX_SIZE, ctrl->val);
		dev_dbg_ratelimited(sd->dev, "%s: set face max size to %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_POWER_LINE_FREQUENCY:
		ret = gs_ar0234_read_reg8(sensor, GS_REG_ANTIFLICKER_MODE, &val8); // set power line frequency
		if(ret) break;
		val8 = val8 & 0xFC; // make bit[1..0] zero
		switch(ctrl->val) {
			case V4L2_CID_POWER_LINE_FREQUENCY_DISABLED:
				ret = gs_ar0234_write_reg8(sensor, GS_REG_ANTIFLICKER_MODE, val8 );
				break;
			case V4L2_CID_POWER_LINE_FREQUENCY_50HZ:
				ret = gs_ar0234_write_reg8(sensor, GS_REG_ANTIFLICKER_MODE, val8 | 1);
				if(ret) break;
				ret = gs_ar0234_write_reg8(sensor, GS_REG_ANTIFLICKER_FREQ, 50);
				break;
			case V4L2_CID_POWER_LINE_FREQUENCY_60HZ:
				ret = gs_ar0234_write_reg8(sensor, GS_REG_ANTIFLICKER_MODE, val8 | 1);
				if(ret) break;
				ret = gs_ar0234_write_reg8(sensor, GS_REG_ANTIFLICKER_FREQ, 60);
				break;
			case V4L2_CID_POWER_LINE_FREQUENCY_AUTO:
			default:
				ret = gs_ar0234_write_reg8(sensor, GS_REG_ANTIFLICKER_MODE, val8 | 2);
				break;
		}
		dev_dbg_ratelimited(sd->dev, "%s: set anti flicker  to %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_HFLIP:
		// mirror is bit[0], flip is bit[1]. Read vflip value and write both
		ret = gs_ar0234_write_reg8(sensor, GS_REG_MIRROR_FLIP, (sensor->ctrls.vflip->val << 1 | ctrl->val));
		dev_dbg_ratelimited(sd->dev, "%s: set hflip to %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_VFLIP:
		// mirror is bit[0], flip is bit[1]. Read hflip value and write both
		ret = gs_ar0234_write_reg8(sensor, GS_REG_MIRROR_FLIP, ctrl->val << 1 | sensor->ctrls.hflip->val);
		dev_dbg_ratelimited(sd->dev, "%s: set hflip to %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_SHARPNESS:
		ret = gs_ar0234_write_reg16(sensor, GS_REG_SHARPNESS, ctrl->val);
		dev_dbg_ratelimited(sd->dev, "%s: set sharpness to %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_COLORFX: // color effect
		switch (ctrl->val) {
			case V4L2_COLORFX_NONE: 		val8=0x00; break;
			case V4L2_COLORFX_BW: 			val8=0x03; break;
			case V4L2_COLORFX_SEPIA:		val8=0x0D; break;
			case V4L2_COLORFX_NEGATIVE: 	val8=0x07; break;
			case V4L2_COLORFX_EMBOSS: 		val8=0x05; break;
			case V4L2_COLORFX_SKETCH: 		val8=0x0F; break;
			case V4L2_COLORFX_SKY_BLUE: 	val8=0x08; break;
			case V4L2_COLORFX_GRASS_GREEN: 	val8=0x09; break;
			case V4L2_COLORFX_SKIN_WHITEN: 	val8=0x00; break; //?
			case V4L2_COLORFX_VIVID: 		val8=0x00; break; // ?
			case V4L2_COLORFX_AQUA: 		val8=0x00; break; // ?
			case V4L2_COLORFX_ART_FREEZE: 	val8=0x11; break; // foggy
			case V4L2_COLORFX_SILHOUETTE: 	val8=0x04; break; //emboss B/W
			case V4L2_COLORFX_SOLARIZATION: val8=0x10; break;
			case V4L2_COLORFX_ANTIQUE: 		val8=0x02; break;
			case V4L2_COLORFX_SET_CBCR: 	val8=0x00; break;
			default: val8 = 0;	break;
		}
		ret = gs_ar0234_write_reg8(sensor, GS_REG_COLORFX, val8);
		dev_dbg_ratelimited(sd->dev, "%s: set colorfx to %d\n", __func__, val8);
		break;
	case V4L2_CID_TEST_PATTERN:
		ret = gs_ar0234_write_reg8(sensor, GS_REG_TESTPATTERN, ctrl->val);
		dev_dbg_ratelimited(sd->dev, "%s: set testpattern %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_PAN_ABSOLUTE:
		ret = gs_ar0234_write_reg8(sensor, GS_REG_PAN, ctrl->val);
		dev_dbg_ratelimited(sd->dev, "%s: set pan to %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_TILT_ABSOLUTE:
		ret = gs_ar0234_write_reg8(sensor, GS_REG_TILT, ctrl->val);
		dev_dbg_ratelimited(sd->dev, "%s: set tilt to %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_ZOOM_ABSOLUTE:
		ret = gs_ar0234_write_reg16(sensor, GS_REG_ZOOM, ctrl->val);
		dev_dbg_ratelimited(sd->dev, "%s: set zoom to %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_ZOOM_SPEED:
		ret = gs_ar0234_write_reg8(sensor, GS_REG_ZOOM_SPEED, ctrl->val);
		dev_dbg_ratelimited(sd->dev, "%s: set zoom speed to %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_NOISE_RED:
		ret = gs_ar0234_write_reg16(sensor, GS_REG_NOISE_RED, ctrl->val);
		dev_dbg_ratelimited(sd->dev, "%s: set noise reduction to %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_ROI_MODE_0:
	  	ret = gs_ar0234_read_reg8(sensor, GS_REG_ROI_MODE, &val8); // set power line frequency
		if(ret) break;
		ret = gs_ar0234_write_reg8(sensor, GS_REG_ROI_MODE, (val8 & (~(1<<0))) | ctrl->val<<0);
		dev_dbg_ratelimited(sd->dev, "%s: set roi mode b0 to %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_ROI_MODE_1:
	  	ret = gs_ar0234_read_reg8(sensor, GS_REG_ROI_MODE, &val8); // set power line frequency
		if(ret) break;
		ret = gs_ar0234_write_reg8(sensor, GS_REG_ROI_MODE, (val8 & (~(1<<1))) | ctrl->val<<1);
		dev_dbg_ratelimited(sd->dev, "%s: set roi mode b1 to %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_ROI_MODE_2:
	  	ret = gs_ar0234_read_reg8(sensor, GS_REG_ROI_MODE, &val8); // set power line frequency
		if(ret) break;
		ret = gs_ar0234_write_reg8(sensor, GS_REG_ROI_MODE, (val8 & (~(1<<2))) | ctrl->val<<2);
		dev_dbg_ratelimited(sd->dev, "%s: set roi mode b2 to %d\n", __func__, ctrl->val);
		break;
	case V4L2_CID_STORE_REGISTERS:
		ret = gs_ar0234_write_reg8(sensor, GS_REG_SAVE_RESTART, 0x01);
		dev_dbg_ratelimited(sd->dev, "%s: set store registers\n", __func__);
		break;
	case V4L2_CID_RESTORE_REGISTERS:
		ret = gs_ar0234_write_reg8(sensor, GS_REG_SAVE_RESTART, 0x05);
		if(ret) break;
		msleep(5); //wait 5 ms for recovery of factory registers
		dev_dbg_ratelimited(sd->dev, "%s: set restore registers\n", __func__);
		ret = gs_ar0234_i_cntrl(sensor);
		break;
	case V4L2_CID_RESTORE_FACTORY:
		ret = gs_ar0234_write_reg8(sensor, GS_REG_SAVE_RESTART, 0x07);
		if(ret) break;
		msleep(10); //wait 5 ms for recovery of factory registers
		ret = gs_ar0234_write_reg8(sensor, GS_REG_SAVE_RESTART, 0x08);
		if(ret) break;
		msleep(10); //wait 5 ms for recovery of factory registers
		dev_dbg_ratelimited(sd->dev, "%s: set restore factory\n", __func__);
		ret = gs_ar0234_i_cntrl(sensor);
		break;
	case V4L2_CID_REBOOT:
		ret = gs_ar0234_write_reg8(sensor, GS_REG_SAVE_RESTART, 0x99);
		dev_dbg_ratelimited(sd->dev, "%s: set reboot\n", __func__);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int gs_ar0234_i_cntrl(struct gs_ar0234_dev *sensor)
{
	int ret=0;
	u32 uval32;
	u16 uval;
	u8 uval8;
	union  myunion {
		u16 uval;
		s16 sval;
	} val_t;

	dev_dbg(sensor->dev, "%s: \n", __func__);

	ret = gs_ar0234_read_reg16(sensor, GS_REG_BRIGHTNESS, (short *) &sensor->ctrls.brightness->cur.val);
	//dev_dbg(sensor->dev, "%s: brighness = %x \n", __func__, sensor->ctrls.brightness->cur.val);
	if (ret < 0) return ret;

	ret = gs_ar0234_read_reg16(sensor, GS_REG_CONTRAST, (short *)&sensor->ctrls.contrast->cur.val);
	if (ret < 0) return ret;

	ret = gs_ar0234_read_reg16(sensor, GS_REG_SATURATION, (short *)&sensor->ctrls.saturation->cur.val);
	if (ret < 0) return ret;

	ret = gs_ar0234_read_reg16(sensor, GS_REG_GAMMA, (short *) &sensor->ctrls.gamma->cur);
	if (ret < 0) return ret;

	ret = gs_ar0234_read_reg16(sensor, GS_REG_SHARPNESS, (short *)&sensor->ctrls.sharpness->cur.val);
	if (ret < 0) return ret;

	ret = gs_ar0234_read_reg16(sensor, GS_REG_NOISE_RED, (short *)&sensor->ctrls.noise_red->cur.val);
	if (ret < 0) return ret;

	ret = gs_ar0234_read_reg16(sensor, GS_REG_SHARPNESS, (short *) &sensor->ctrls.blc_level->cur.val);
	if (ret < 0) return ret;

	ret = gs_ar0234_read_reg16(sensor, GS_REG_WB_TEMPERATURE, (short *) &sensor->ctrls.wb_temp->cur.val);
	if (ret < 0) return ret;

	ret = gs_ar0234_read_reg16(sensor, GS_REG_GAIN, (short *) &sensor->ctrls.gain->cur.val);
	if (ret < 0) return ret;

	ret = gs_ar0234_read_reg16(sensor, GS_REG_ZOOM, (short *) &sensor->ctrls.zoom->cur.val);
	if (ret < 0) return ret;

	ret = gs_ar0234_read_reg8(sensor, GS_REG_ZOOM_SPEED,(u8 *) &sensor->ctrls.zoom_speed->cur.val);
	if (ret < 0) return ret;

	ret = gs_ar0234_read_reg8(sensor, GS_REG_PAN, (u8 *) &sensor->ctrls.pan->cur.val);
	if (ret < 0) return ret;

	ret = gs_ar0234_read_reg8(sensor, GS_REG_TILT, (u8 *) &sensor->ctrls.tilt->cur.val);
	if (ret < 0) return ret;

	ret = gs_ar0234_read_reg8(sensor, GS_REG_TESTPATTERN, (u8 *) &sensor->ctrls.testpattern->cur.val);
	if (ret < 0) return ret;

	ret = gs_ar0234_read_reg16(sensor, GS_REG_AE_TARGET, &val_t.uval);
	if (ret < 0) return ret;
	sensor->ctrls.exposure->cur.val = (s32) (((s32) val_t.sval) * 1000 / 256);

	ret = gs_ar0234_read_reg8(sensor, GS_REG_EXPOSURE_MODE, &uval8);
	if (ret < 0) return ret;
	switch(uval8) {
		case 0x0:
			sensor->ctrls.auto_exp->cur.val = V4L2_EXPOSURE_MANUAL;
			break;
		case 0x9:
			sensor->ctrls.auto_exp->cur.val = V4L2_EXPOSURE_SHUTTER_PRIORITY;
			break;
		case 0xC:
			sensor->ctrls.auto_exp->cur.val = V4L2_EXPOSURE_AUTO;
			break;
		default:
			sensor->ctrls.auto_exp->cur.val = V4L2_EXPOSURE_AUTO;
			dev_dbg(sensor->dev, "%s: exposure mode = %x is this correct?\n", __func__, uval8);
			break;
	}

	ret = gs_ar0234_read_reg32(sensor, GS_REG_EXPOSURE_ABS, &uval32);
	if (ret < 0) return ret;
	sensor->ctrls.exposure_absolute->cur.val = uval32/100; // 100us
	//dev_dbg(sensor->dev, "%s: exposure abs = %x %d is this correct?\n", __func__, uval32,sensor->ctrls.exposure_absolute->cur.val);

	ret = gs_ar0234_read_reg32(sensor, GS_REG_EXPOSURE_UPPER, &uval32);
	sensor->ctrls.exposure_upper->cur.val = uval32/100; // 100us
	if (ret < 0) return ret;

	ret = gs_ar0234_read_reg32(sensor, GS_REG_EXPOSURE_MAX, &uval32);
	sensor->ctrls.exposure_max->cur.val = uval32/100; // 100us
	if (ret < 0) return ret;

	ret = gs_ar0234_read_reg16(sensor, GS_REG_GAIN_UPPER, (u16 *) &sensor->ctrls.gain_upper->cur.val);
	if (ret < 0) return ret;

	ret = gs_ar0234_read_reg16(sensor, GS_REG_GAIN_MAX, (u16 *) &sensor->ctrls.gain_max->cur.val);
	if (ret < 0) return ret;

	ret = gs_ar0234_read_reg8(sensor, GS_REG_ROI_MODE, &uval8);
	if (ret < 0) return ret;
	sensor->ctrls.roi_mode_0->cur.val = ((uval8>>0) & 0x01);
	sensor->ctrls.roi_mode_1->cur.val = ((uval8>>1) & 0x01);
	sensor->ctrls.roi_mode_2->cur.val = ((uval8>>2) & 0x01);

	ret = gs_ar0234_read_reg8(sensor, GS_REG_BLC_MODE, &uval8);
	if (ret < 0) return ret;
	switch(uval8) {
		case 0x0:
			sensor->ctrls.exposure_metering->cur.val = V4L2_EXPOSURE_METERING_AVERAGE;
			break;
		case 0x1:
			sensor->ctrls.exposure_metering->cur.val = V4L2_EXPOSURE_METERING_CENTER_WEIGHTED;
			break;
		case 0x2:
			sensor->ctrls.exposure_metering->cur.val = V4L2_EXPOSURE_METERING_SPOT;
			break;
		case 0x3:
			sensor->ctrls.exposure_metering->cur.val = V4L2_EXPOSURE_METERING_MATRIX;
			break;
		default:
			sensor->ctrls.exposure_metering->cur.val = V4L2_EXPOSURE_METERING_CENTER_WEIGHTED;
			dev_dbg(sensor->dev, "%s: exposure_metering = %x is this correct?\n", __func__, uval8);
			break;
	}

	ret = gs_ar0234_read_reg8(sensor, GS_REG_BLC_WINDOW_X0, (u8 *) &sensor->ctrls.blc_window_x0->cur.val);
	if (ret < 0) return ret;

	ret = gs_ar0234_read_reg8(sensor, GS_REG_BLC_WINDOW_Y0, (u8 *) &sensor->ctrls.blc_window_y0->cur.val);
	if (ret < 0) return ret;

	ret = gs_ar0234_read_reg8(sensor, GS_REG_BLC_WINDOW_X1, (u8 *) &sensor->ctrls.blc_window_x1->cur.val);
	if (ret < 0) return ret;

	ret = gs_ar0234_read_reg8(sensor, GS_REG_BLC_WINDOW_Y1, (u8 *) &sensor->ctrls.blc_window_y1->cur.val);
	if (ret < 0) return ret;

	ret = gs_ar0234_read_reg8(sensor, GS_REG_BLC_RATIO, (u8 *) &sensor->ctrls.blc_ratio->cur.val);
	if (ret < 0) return ret;

	ret = gs_ar0234_read_reg8(sensor, GS_REG_BLC_FACE_LEVEL, (u8 *) &sensor->ctrls.blc_face_level->cur.val);
	if (ret < 0) return ret;

	ret = gs_ar0234_read_reg8(sensor, GS_REG_BLC_FACE_WEIGHT, (u8 *) &sensor->ctrls.blc_face_weight->cur.val);
	if (ret < 0) return ret;

	ret = gs_ar0234_read_reg8(sensor, GS_REG_BLC_ROI_LEVEL, (u8 *) &sensor->ctrls.blc_roi_level->cur.val);
	if (ret < 0) return ret;

	ret = gs_ar0234_read_reg8(sensor, GS_REG_FACE_DETECT, &uval8);
	if (ret < 0) return ret;
	sensor->ctrls.face_detect_0->cur.val = ((uval8>>0) & 0x01);
	sensor->ctrls.face_detect_4->cur.val = ((uval8>>4) & 0x01);
	sensor->ctrls.face_detect_5->cur.val = ((uval8>>5) & 0x01);

	ret = gs_ar0234_read_reg8(sensor, GS_REG_FACE_DETECT_SPEED, (u8 *) &sensor->ctrls.face_detect_speed->cur.val);
	if (ret < 0) return ret;

	ret = gs_ar0234_read_reg8(sensor, GS_REG_FACE_DETECT_THRESHOLD, (u8 *) &sensor->ctrls.face_detect_threshold->cur.val);
	if (ret < 0) return ret;

	ret = gs_ar0234_read_reg8(sensor, GS_REG_FACE_CHROMA_THRESHOLD, (u8 *) &sensor->ctrls.face_chroma_threshold->cur.val);
	if (ret < 0) return ret;

	ret = gs_ar0234_read_reg16(sensor, GS_REG_FACE_MIN_SIZE, (u16 *) &sensor->ctrls.face_min_size->cur.val);
	if (ret < 0) return ret;

	ret = gs_ar0234_read_reg16(sensor, GS_REG_FACE_MAX_SIZE, (u16 *) &sensor->ctrls.face_max_size->cur.val);
	if (ret < 0) return ret;

	ret = gs_ar0234_read_reg8(sensor, GS_REG_WHITEBALANCE, &uval8);
	if (ret < 0) return ret;
	sensor->ctrls.auto_wb->cur.val = (uval8 & 0x0F) == 0x0F ? 1 : 0;

	ret = gs_ar0234_read_reg16(sensor, GS_REG_WB_TEMPERATURE, &uval);
	if (ret < 0) return ret;
	if( uval == 0 )  sensor->ctrls.wb_preset->cur.val = V4L2_WHITE_BALANCE_MANUAL;
	else if(( uval > 2500) && (uval < 3500)) sensor->ctrls.wb_preset->cur.val = V4L2_WHITE_BALANCE_INCANDESCENT;
	else if(( uval > 3500) && (uval < 4500)) sensor->ctrls.wb_preset->cur.val = V4L2_WHITE_BALANCE_FLUORESCENT;
	else if(( uval > 4500) && (uval < 5000)) sensor->ctrls.wb_preset->cur.val = V4L2_WHITE_BALANCE_FLUORESCENT_H;
	else if(( uval > 5000) && (uval < 6000)) sensor->ctrls.wb_preset->cur.val = V4L2_WHITE_BALANCE_HORIZON;
	else if(( uval > 6000) && (uval < 7000)) sensor->ctrls.wb_preset->cur.val = V4L2_WHITE_BALANCE_DAYLIGHT;
	//else if(( uval> 5400) && (uval < 5600)) sensor->ctrls.wb_preset->cur.val = V4L2_WHITE_BALANCE_FLASH;
	else if(( uval > 7000) && (uval < 8000)) sensor->ctrls.wb_preset->cur.val = V4L2_WHITE_BALANCE_CLOUDY;
	else if(( uval > 8000) && (uval < 10000)) sensor->ctrls.wb_preset->cur.val = V4L2_WHITE_BALANCE_SHADE;

	ret = gs_ar0234_read_reg16(sensor, GS_REG_AWB_MAN_X, (u16 *) &sensor->ctrls.awb_man_x->cur.val);
	if (ret < 0) return ret;

	ret = gs_ar0234_read_reg16(sensor, GS_REG_AWB_MAN_Y, (u16 *) &sensor->ctrls.awb_man_y->cur.val);
	if (ret < 0) return ret;

	ret = gs_ar0234_read_reg8(sensor, GS_REG_MIRROR_FLIP, &uval8);
	if (ret < 0) return ret;
	sensor->ctrls.hflip->cur.val = uval8 & 0x01;
	sensor->ctrls.vflip->cur.val = (uval8>>1) & 0x01;

	ret = gs_ar0234_read_reg8(sensor, GS_REG_ANTIFLICKER_MODE, &uval8);
	if (ret < 0) return ret;
	if((uval8&0x3) == 0)	sensor->ctrls.powerline->cur.val = V4L2_CID_POWER_LINE_FREQUENCY_DISABLED;
	else if((uval8&0x3) == 2)	sensor->ctrls.powerline->cur.val = V4L2_CID_POWER_LINE_FREQUENCY_AUTO;
	else {
		ret = gs_ar0234_read_reg8(sensor, GS_REG_ANTIFLICKER_FREQ, &uval8);
		if (ret < 0) return ret;
		if(uval8 <= 55) sensor->ctrls.powerline->cur.val = V4L2_CID_POWER_LINE_FREQUENCY_50HZ;
		else if(uval8 >= 55) sensor->ctrls.powerline->cur.val = V4L2_CID_POWER_LINE_FREQUENCY_60HZ;
	}

	ret = gs_ar0234_read_reg8(sensor, GS_REG_COLORFX, &uval8);
	if (ret < 0) return ret;
	switch(uval8) {
		case 0x00:	sensor->ctrls.colorfx->cur.val = V4L2_COLORFX_NONE; break;
		case 0x03:	sensor->ctrls.colorfx->cur.val = V4L2_COLORFX_BW; break;
		case 0x0D:	sensor->ctrls.colorfx->cur.val = V4L2_COLORFX_SEPIA; break;
		case 0x07:	sensor->ctrls.colorfx->cur.val = V4L2_COLORFX_NEGATIVE; break;
		case 0x05:	sensor->ctrls.colorfx->cur.val = V4L2_COLORFX_EMBOSS; break;
		case 0x0F:	sensor->ctrls.colorfx->cur.val = V4L2_COLORFX_SKETCH; break;
		case 0x08:	sensor->ctrls.colorfx->cur.val = V4L2_COLORFX_SKY_BLUE; break;
		case 0x09:	sensor->ctrls.colorfx->cur.val = V4L2_COLORFX_GRASS_GREEN; break;
		case 0x11:	sensor->ctrls.colorfx->cur.val = V4L2_COLORFX_ART_FREEZE; break;
		case 0x04:	sensor->ctrls.colorfx->cur.val = V4L2_COLORFX_SILHOUETTE; break;
		case 0x10:	sensor->ctrls.colorfx->cur.val = V4L2_COLORFX_SOLARIZATION;	break;
		case 0x02:	sensor->ctrls.colorfx->cur.val = V4L2_COLORFX_ANTIQUE; break;
		default: sensor->ctrls.colorfx->cur.val = V4L2_COLORFX_NONE; break;
	}
	return ret;
}

static const struct v4l2_ctrl_ops gs_ar0234_ctrl_ops = {
	.g_volatile_ctrl = gs_ar0234_g_volatile_ctrl,
	.s_ctrl = gs_ar0234_s_ctrl,
};

static const char * const gs_ar0234_test_pattern_menu[] = {
	"Disabled",
	"Enabled"
};

static const struct v4l2_ctrl_config zoom_speed = {
        .ops = &gs_ar0234_ctrl_ops,
        .id = V4L2_CID_ZOOM_SPEED,
        .name = "Zoom speed",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = V4L2_CTRL_FLAG_SLIDER,
		.min = -128,
        .max = 127,
        .step = 1,
		.def = -128,
};

static const struct v4l2_ctrl_config noise_red = {
        .ops = &gs_ar0234_ctrl_ops,
        .id = V4L2_CID_NOISE_RED,
        .name = "Noise reduction",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = V4L2_CTRL_FLAG_SLIDER,
		.min = -32768,
        .max = 32767,
        .step = 1,
		.def = 0,
};

static const struct v4l2_ctrl_config roi_mode_0 = {
	    .ops = &gs_ar0234_ctrl_ops,
        .id = V4L2_CID_ROI_MODE_0,
        .name = "Region of Interest (Bound)",
        .type = V4L2_CTRL_TYPE_BOOLEAN,
		.min = 0,
        .max = 1,
        .step = 1,
		.def = 0,
};
static const struct v4l2_ctrl_config roi_mode_1 = {
	    .ops = &gs_ar0234_ctrl_ops,
        .id = V4L2_CID_ROI_MODE_1,
        .name = "Region of Interest (Lock)",
        .type = V4L2_CTRL_TYPE_BOOLEAN,
		.min = 0,
        .max = 1,
        .step = 1,
		.def = 0,
};
static const struct v4l2_ctrl_config roi_mode_2 = {
	    .ops = &gs_ar0234_ctrl_ops,
        .id = V4L2_CID_ROI_MODE_2,
        .name = "Region of Interest (Face)",
        .type = V4L2_CTRL_TYPE_BOOLEAN,
		.min = 0,
        .max = 1,
        .step = 1,
		.def = 1,
};

static const struct v4l2_ctrl_config exposure_upper = {
	    .ops = &gs_ar0234_ctrl_ops,
        .id = V4L2_CID_EXPOSURE_UPPER,
        .name = "Exposure Upper (x 100us)",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = V4L2_CTRL_FLAG_SLIDER,
		.min = 0,
        .max = 0xFFFF,
        .step = 1,
		.def = 333,
};
static const struct v4l2_ctrl_config exposure_max = {
	    .ops = &gs_ar0234_ctrl_ops,
        .id = V4L2_CID_EXPOSURE_MAX,
        .name = "Exposure Max (x 100us)",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = V4L2_CTRL_FLAG_SLIDER,
		.min = 0,
        .max = 0xFFFF,
        .step = 1,
		.def = 333,
};
static const struct v4l2_ctrl_config gain_upper = {
	    .ops = &gs_ar0234_ctrl_ops,
        .id = V4L2_CID_GAIN_UPPER,
        .name = "Gain Upper (value = u8.8)",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = V4L2_CTRL_FLAG_SLIDER,
		.min = 0,
        .max = 0xFFFF,
        .step = 1,
		.def = 0x0800,
};
static const struct v4l2_ctrl_config gain_max = {
		.ops = &gs_ar0234_ctrl_ops,
        .id = V4L2_CID_GAIN_MAX,
        .name = "Gain Max (max = 2^value, s7.8)",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = V4L2_CTRL_FLAG_SLIDER,
		.min = 0,
        .max = 0xFFFF,
        .step = 1,
		.def = 0x0580,
};

static const struct v4l2_ctrl_config blc_window_x0 = {
		.ops = &gs_ar0234_ctrl_ops,
        .id = V4L2_CID_BLC_WINDOW_X0,
        .name = "BLC window X0 (0~128)",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = V4L2_CTRL_FLAG_SLIDER,
		.min = 0,
        .max = 0x80,
        .step = 1,
		.def = 0x00,
};
static const struct v4l2_ctrl_config blc_window_y0 = {
		.ops = &gs_ar0234_ctrl_ops,
        .id = V4L2_CID_BLC_WINDOW_Y0,
        .name = "BLC window Y0 (0~128)",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = V4L2_CTRL_FLAG_SLIDER,
		.min = 0,
        .max = 0x80,
        .step = 1,
		.def = 0x00,
};
static const struct v4l2_ctrl_config blc_window_x1 = {
		.ops = &gs_ar0234_ctrl_ops,
        .id = V4L2_CID_BLC_WINDOW_X1,
        .name = "BLC window X1 (0~128)",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = V4L2_CTRL_FLAG_SLIDER,
		.min = 0,
        .max = 0x80,
        .step = 1,
		.def = 0x80,
};
static const struct v4l2_ctrl_config blc_window_y1 = {
		.ops = &gs_ar0234_ctrl_ops,
        .id = V4L2_CID_BLC_WINDOW_Y1,
        .name = "BLC window Y1 (0~128)",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = V4L2_CTRL_FLAG_SLIDER,
		.min = 0,
        .max = 0x80,
        .step = 1,
		.def = 0x80,
};
static const struct v4l2_ctrl_config blc_ratio = {
		.ops = &gs_ar0234_ctrl_ops,
        .id = V4L2_CID_BLC_RATIO,
        .name = "BLC ratio (0~128)",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = V4L2_CTRL_FLAG_SLIDER,
		.min = 0,
        .max = 0x80,
        .step = 1,
		.def = 0x80,
};
static const struct v4l2_ctrl_config blc_face_level = {
		.ops = &gs_ar0234_ctrl_ops,
        .id = V4L2_CID_BLC_FACE_LEVEL,
        .name = "BLC face Level (0~128)",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = V4L2_CTRL_FLAG_SLIDER,
		.min = 0,
        .max = 0x80,
        .step = 1,
		.def = 0x80,
};
static const struct v4l2_ctrl_config blc_face_weight = {
		.ops = &gs_ar0234_ctrl_ops,
        .id = V4L2_CID_BLC_FACE_WEIGHT,
        .name = "BLC face weight (0~128)",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = V4L2_CTRL_FLAG_SLIDER,
		.min = 0,
        .max = 0x80,
        .step = 1,
		.def = 0x80,
};
static const struct v4l2_ctrl_config blc_roi_level = {
		.ops = &gs_ar0234_ctrl_ops,
        .id = V4L2_CID_BLC_ROI_LEVEL,
        .name = "BLC roi level (0~128)",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = V4L2_CTRL_FLAG_SLIDER,
		.min = 0,
        .max = 0x80,
        .step = 1,
		.def = 0x00,
};

static const struct v4l2_ctrl_config face_detect_0 = {
		.ops = &gs_ar0234_ctrl_ops,
        .id = V4L2_CID_FACE_DETECT_0,
        .name = "Face detection (enable)",
        .type = V4L2_CTRL_TYPE_BOOLEAN,
		.min = 0,
        .max = 1,
        .step = 1,
		.def = 0,
};
static const struct v4l2_ctrl_config face_detect_4 = {
		.ops = &gs_ar0234_ctrl_ops,
        .id = V4L2_CID_FACE_DETECT_4,
        .name = "Face detection (rectangles)",
        .type = V4L2_CTRL_TYPE_BOOLEAN,
		.min = 0,
        .max = 1,
        .step = 1,
		.def = 0,
};
static const struct v4l2_ctrl_config face_detect_5 = {
		.ops = &gs_ar0234_ctrl_ops,
        .id = V4L2_CID_FACE_DETECT_5,
        .name = "Face detection (saturated)",
        .type = V4L2_CTRL_TYPE_BOOLEAN,
		.min = 0,
        .max = 1,
        .step = 1,
		.def = 0,
};

static const struct v4l2_ctrl_config face_detect_speed = {
		.ops = &gs_ar0234_ctrl_ops,
        .id = V4L2_CID_FACE_DETECT_SPEED,
        .name = "Face detection speed (x 0.1s)",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = V4L2_CTRL_FLAG_SLIDER,
		.min = 0,
        .max = 0xFF,
        .step = 1,
		.def = 0x00,
};
static const struct v4l2_ctrl_config face_detect_threshold = {
		.ops = &gs_ar0234_ctrl_ops,
        .id = V4L2_CID_FACE_DETECT_THRESHOLD,
        .name = "Face detection theshold ",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = V4L2_CTRL_FLAG_SLIDER,
		.min = 0,
        .max = 0xFF,
        .step = 1,
		.def = 0x80,
};
static const struct v4l2_ctrl_config face_chroma_threshold = {
		.ops = &gs_ar0234_ctrl_ops,
        .id = V4L2_CID_FACE_CHROMA_THRESHOLD,
        .name = "Face chroma theshold ",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = V4L2_CTRL_FLAG_SLIDER,
		.min = 0,
        .max = 0x1F,
        .step = 1,
		.def = 0x0C,
};
static const struct v4l2_ctrl_config face_min_size = {
		.ops = &gs_ar0234_ctrl_ops,
        .id = V4L2_CID_FACE_MIN_SIZE,
        .name = "Face minimum size",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = V4L2_CTRL_FLAG_SLIDER,
		.min = 0,
        .max = 0x4000,
        .step = 1,
		.def = 0x0400,
};
static const struct v4l2_ctrl_config face_max_size = {
		.ops = &gs_ar0234_ctrl_ops,
        .id = V4L2_CID_FACE_MAX_SIZE,
        .name = "Face maximum size",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = V4L2_CTRL_FLAG_SLIDER,
		.min = 0,
        .max = 0x4000,
        .step = 1,
		.def = 0x4000,
};

static const struct v4l2_ctrl_config awb_man_x = {
		.ops = &gs_ar0234_ctrl_ops,
        .id = V4L2_CID_AWB_MAN_X,
        .name = "AWB manual X",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = V4L2_CTRL_FLAG_SLIDER,
		.min = -32768,
        .max = 32767,
        .step = 1,
		.def = 0x0000,
};
static const struct v4l2_ctrl_config awb_man_y = {
		.ops = &gs_ar0234_ctrl_ops,
        .id = V4L2_CID_AWB_MAN_Y,
        .name = "AWB manual Y",
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags = V4L2_CTRL_FLAG_SLIDER,
		.min = -32768,
        .max = 32767,
        .step = 1,
		.def = 0x0000,
};

static const struct v4l2_ctrl_config store_registers = {
		.ops = &gs_ar0234_ctrl_ops,
        .id = V4L2_CID_STORE_REGISTERS,
        .name = "Store Registers to NVM",
        .type = V4L2_CTRL_TYPE_BUTTON,
		.min = 0,
        .max = 0,
        .step = 0,
		.def = 0,
};
static const struct v4l2_ctrl_config restore_registers = {
		.ops = &gs_ar0234_ctrl_ops,
        .id = V4L2_CID_RESTORE_REGISTERS,
        .name = "Restore Registers from NVM",
        .type = V4L2_CTRL_TYPE_BUTTON,
		.min = 0,
        .max = 0,
        .step = 0,
		.def = 0,
};
static const struct v4l2_ctrl_config restore_factory = {
		.ops = &gs_ar0234_ctrl_ops,
        .id = V4L2_CID_RESTORE_FACTORY,
        .name = "Restore to Factory Settings",
        .type = V4L2_CTRL_TYPE_BUTTON,
		.min = 0,
        .max = 0,
        .step = 0,
		.def = 0,
};
static const struct v4l2_ctrl_config reboot = {
		.ops = &gs_ar0234_ctrl_ops,
        .id = V4L2_CID_REBOOT,
        .name = "Reboot Camera",
        .type = V4L2_CTRL_TYPE_BUTTON,
		.min = 0,
        .max = 0,
        .step = 0,
		.def = 0,
};


static int gs_ar0234_init_controls(struct gs_ar0234_dev *sensor)
{
	const struct v4l2_ctrl_ops *ops = &gs_ar0234_ctrl_ops;
	struct gs_ar0234_ctrls *ctrls = &sensor->ctrls;
	struct v4l2_ctrl_handler *hdl = &ctrls->handler;
	int ret;

	v4l2_ctrl_handler_init(hdl, 32);

	dev_dbg_ratelimited(sensor->dev, "%s: \n", __func__);

	/* we can use our own mutex for the ctrl lock */
	hdl->lock = &sensor->lock;

	/* Clock related controls */
	// ctrls->pixel_rate = v4l2_ctrl_new_std(hdl, ops, V4L2_CID_PIXEL_RATE, 0, INT_MAX, 1, gs_ar0234_calc_pixel_rate(sensor));

	// Camera control
	ctrls->store_registers = v4l2_ctrl_new_custom(hdl, &store_registers, NULL);
	ctrls->restore_registers = v4l2_ctrl_new_custom(hdl, &restore_registers, NULL);
	ctrls->restore_factory = v4l2_ctrl_new_custom(hdl, &restore_factory, NULL);
	ctrls->reboot = v4l2_ctrl_new_custom(hdl, &reboot, NULL);

	/* Auto/manual white balance */
	ctrls->auto_wb = v4l2_ctrl_new_std(hdl, ops, V4L2_CID_AUTO_WHITE_BALANCE, 0, 1, 1, 1);
	ctrls->push_to_white = v4l2_ctrl_new_std(hdl, ops, V4L2_CID_DO_WHITE_BALANCE, 0, 0, 0, 0);
	ctrls->wb_temp = v4l2_ctrl_new_std(hdl, ops, V4L2_CID_WHITE_BALANCE_TEMPERATURE , 0, 0xFFFF, 1, 6500);
	ctrls->wb_preset = v4l2_ctrl_new_std_menu(hdl, ops, V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE, V4L2_WHITE_BALANCE_SHADE, 0x2, V4L2_WHITE_BALANCE_DAYLIGHT);
	ctrls->awb_man_x = v4l2_ctrl_new_custom(hdl, &awb_man_x, NULL);
	ctrls->awb_man_y = v4l2_ctrl_new_custom(hdl, &awb_man_y, NULL);

	/* auto/manual exposure*/
	ctrls->auto_exp = v4l2_ctrl_new_std_menu(hdl, ops, V4L2_CID_EXPOSURE_AUTO, V4L2_EXPOSURE_APERTURE_PRIORITY, 0, V4L2_EXPOSURE_AUTO);
	ctrls->brightness = v4l2_ctrl_new_std(hdl, ops, V4L2_CID_BRIGHTNESS, -32768, 32767, 1, 0);
	//ctrls->brightness->flags |= V4L2_CTRL_FLAG_VOLATILE;
	ctrls->exposure_absolute = v4l2_ctrl_new_std(hdl, ops, V4L2_CID_EXPOSURE_ABSOLUTE , 0, 0xFFFF, 1, 333);
	ctrls->exposure = v4l2_ctrl_new_std(hdl, ops, V4L2_CID_EXPOSURE , -32768,  32767, 1, -2200);
	ctrls->gain = v4l2_ctrl_new_std(hdl, ops, V4L2_CID_GAIN , 0x100,  32767, 1, 0x100);
	ctrls->exposure_metering = v4l2_ctrl_new_std_menu(hdl, ops, V4L2_CID_EXPOSURE_METERING, V4L2_EXPOSURE_METERING_MATRIX, 0, V4L2_EXPOSURE_METERING_CENTER_WEIGHTED);
	ctrls->blc_level = v4l2_ctrl_new_std(hdl, ops, V4L2_CID_BACKLIGHT_COMPENSATION , 0,  0x80, 1, 0);
	ctrls->roi_mode_0 = v4l2_ctrl_new_custom(hdl, &roi_mode_0, NULL);
	ctrls->roi_mode_1 = v4l2_ctrl_new_custom(hdl, &roi_mode_1, NULL);
	ctrls->roi_mode_2 = v4l2_ctrl_new_custom(hdl, &roi_mode_2, NULL);
	ctrls->exposure_upper = v4l2_ctrl_new_custom(hdl, &exposure_upper, NULL);
	ctrls->exposure_max= v4l2_ctrl_new_custom(hdl, &exposure_max, NULL);
	ctrls->gain_upper= v4l2_ctrl_new_custom(hdl, &gain_upper, NULL);
	ctrls->gain_max = v4l2_ctrl_new_custom(hdl, &gain_max, NULL);
	ctrls->blc_window_x0 = v4l2_ctrl_new_custom(hdl, &blc_window_x0, NULL);
	ctrls->blc_window_y0 = v4l2_ctrl_new_custom(hdl, &blc_window_y0, NULL);
	ctrls->blc_window_x1 = v4l2_ctrl_new_custom(hdl, &blc_window_x1, NULL);
	ctrls->blc_window_y1 = v4l2_ctrl_new_custom(hdl, &blc_window_y1, NULL);
	ctrls->blc_ratio = v4l2_ctrl_new_custom(hdl, &blc_ratio, NULL);
	ctrls->blc_face_level = v4l2_ctrl_new_custom(hdl, &blc_face_level, NULL);
	ctrls->blc_face_weight = v4l2_ctrl_new_custom(hdl, &blc_face_weight, NULL);
	ctrls->blc_roi_level = v4l2_ctrl_new_custom(hdl, &blc_roi_level, NULL);

	/* face detection*/
	ctrls->face_detect_0 = v4l2_ctrl_new_custom(hdl, &face_detect_0, NULL);
	ctrls->face_detect_4 = v4l2_ctrl_new_custom(hdl, &face_detect_4, NULL);
	ctrls->face_detect_5 = v4l2_ctrl_new_custom(hdl, &face_detect_5, NULL);
	ctrls->face_detect_speed = v4l2_ctrl_new_custom(hdl, &face_detect_speed, NULL);
	ctrls->face_detect_threshold = v4l2_ctrl_new_custom(hdl, &face_detect_threshold, NULL);
	ctrls->face_chroma_threshold = v4l2_ctrl_new_custom(hdl, &face_chroma_threshold, NULL);
	ctrls->face_min_size = v4l2_ctrl_new_custom(hdl, &face_min_size, NULL);
	ctrls->face_max_size = v4l2_ctrl_new_custom(hdl, &face_max_size, NULL);

	/* Auto/manual gain */
	// ctrls->auto_gain = v4l2_ctrl_new_std(hdl, ops, V4L2_CID_AUTOGAIN, 0, 1, 1, 1);

	/* basic */
	ctrls->saturation = v4l2_ctrl_new_std(hdl, ops, V4L2_CID_SATURATION, 0, 0x2000, 1, 0x1000);
	ctrls->contrast = v4l2_ctrl_new_std(hdl, ops, V4L2_CID_CONTRAST, -32768, 32767, 1, 0);
	ctrls->hflip = v4l2_ctrl_new_std(hdl, ops, V4L2_CID_HFLIP, 0, 1, 1, 0);
	ctrls->vflip = v4l2_ctrl_new_std(hdl, ops, V4L2_CID_VFLIP, 0, 1, 1, 0);
	ctrls->sharpness = v4l2_ctrl_new_std(hdl, ops, V4L2_CID_SHARPNESS, -32768, 32767, 1, 0);
	ctrls->gamma = v4l2_ctrl_new_std(hdl, ops, V4L2_CID_GAMMA, 0, 0x7FFF, 1, 0);
	ctrls->noise_red = v4l2_ctrl_new_custom(hdl, &noise_red, NULL);

	/* zoom/pan/tilt */
	ctrls->zoom = v4l2_ctrl_new_std(hdl, ops, V4L2_CID_ZOOM_ABSOLUTE, 0, 0x7FFF, 1, 0x0100);
	ctrls->pan = v4l2_ctrl_new_std(hdl, ops, V4L2_CID_PAN_ABSOLUTE, 0, 0x80, 1, 0x40);
	ctrls->tilt = v4l2_ctrl_new_std(hdl, ops, V4L2_CID_TILT_ABSOLUTE, 0, 0x80, 1, 0x40);
	ctrls->zoom_speed = v4l2_ctrl_new_custom(hdl, &zoom_speed, NULL);

	/* anti flicker */
	ctrls->powerline = v4l2_ctrl_new_std_menu(hdl, ops, V4L2_CID_POWER_LINE_FREQUENCY, V4L2_CID_POWER_LINE_FREQUENCY_AUTO, 0, V4L2_CID_POWER_LINE_FREQUENCY_AUTO);

	/* test pattern */
	ctrls->testpattern = v4l2_ctrl_new_std_menu_items(hdl, ops, V4L2_CID_TEST_PATTERN, ARRAY_SIZE(gs_ar0234_test_pattern_menu) - 1, 0, 0, gs_ar0234_test_pattern_menu);

	/* effects */
	ctrls->colorfx = v4l2_ctrl_new_std_menu(hdl, ops, V4L2_CID_COLORFX, V4L2_COLORFX_SET_CBCR, 0, V4L2_COLORFX_NONE);

	if (hdl->error) {
		ret = hdl->error;
		dev_err(sensor->dev, "%s: error: %d\n", __func__, ret);
		goto free_ctrls;
	}

	// ctrls->pixel_rate->flags |= V4L2_CTRL_FLAG_READ_ONLY;
	// ctrls->gain->flags |= V4L2_CTRL_FLAG_VOLATILE;
	// ctrls->exposure->flags |= V4L2_CTRL_FLAG_VOLATILE;

	// v4l2_ctrl_auto_cluster(3, &ctrls->auto_wb, 0, false);
	// v4l2_ctrl_auto_cluster(2, &ctrls->auto_gain, 0, true);
	// v4l2_ctrl_auto_cluster(2, &ctrls->auto_exp, 1, true);

	sensor->sd.ctrl_handler = hdl;
	return 0;

free_ctrls:
	v4l2_ctrl_handler_free(hdl);
	return ret;
}

static int ops_enum_frame_size(struct v4l2_subdev *sub_dev, struct v4l2_subdev_state *sd_state, struct v4l2_subdev_frame_size_enum *fse)
{
	struct gs_ar0234_dev *sensor = to_gs_ar0234_dev(sub_dev);
	struct resolution res = {.width=U16_MAX, .height=U16_MAX};
	int i, unique=0;

	if (fse->pad >= NUM_PADS)
		return -EINVAL;
	if (fse->code != sensor->fmt.code) {
		dev_dbg_ratelimited(sub_dev->dev, "%s unsupported fmt.code: 0x%04x.\n", __func__, fse->code);
		return -EINVAL;
	}

	// return the unique resoluitons.
	for(i = 0 ; i < ARRAY_SIZE(sensor_res_list) ; i++) {
		if(res.width != sensor_res_list[i].width || res.height != sensor_res_list[i].height) {
			res = sensor_res_list[i];
			if (fse->index == unique++) {
				fse->min_width  = fse->max_width  = res.width;
				fse->min_height = fse->max_height = res.height;
				dev_dbg_ratelimited(sub_dev->dev, "%s: offer size WxH@mode=%dx%d for mode 0x%04x.\n", __func__, res.width, res.height, fse->code);
				return 0;
			}
		}
	}
	return -EINVAL;
}

static int ops_enum_frame_interval(struct v4l2_subdev *sub_dev, struct v4l2_subdev_state *sd_state, struct v4l2_subdev_frame_interval_enum *fie)
{
	struct gs_ar0234_dev *sensor = to_gs_ar0234_dev(sub_dev);
	int i, count=0;

	if (fie->pad >= NUM_PADS)
		return -EINVAL;
	// only suports MEDIA_BUS_FMT_YUYV8_1X16
	if (fie->code != sensor->fmt.code) {
		dev_dbg_ratelimited(sub_dev->dev, "%s unsupported fmt.code: 0x%04x.\n", __func__, fie->code);
		return -EINVAL;
	}

	if (fie->width == 0 || fie->height == 0 || fie->code == 0) {
		dev_err_ratelimited(sub_dev->dev, "%s Please assign pix-fmt & resolution before enum framerates.\n", __func__);
		return -EINVAL;
	}

	fie->interval.numerator = 1;

	// find the n-th entry that matches the given resolution.
	for(i=0; i < ARRAY_SIZE(sensor_res_list); i++) {
		if(fie->width == sensor_res_list[i].width && fie->height == sensor_res_list[i].height) {
			if (fie->index == count++) {
				fie->interval.denominator = sensor_res_list[i].framerate;
				dev_dbg_ratelimited(sub_dev->dev, "%s: offer framerate %dfps for WxH@mode=%dx%d@0x%04x.\n", __func__, fie->interval.denominator, fie->width, fie->height, fie->code);
				return 0;
			}
		}
	}
	return -EINVAL;
}

static int ops_get_frame_interval(struct v4l2_subdev *sub_dev, struct v4l2_subdev_frame_interval *fi)
{
	struct gs_ar0234_dev *sensor = to_gs_ar0234_dev(sub_dev);

	dev_dbg(sub_dev->dev, "%s: \n", __func__);

	if (fi->pad >= NUM_PADS)
		return -EINVAL;

	fi->interval.numerator = 1;
	fi->interval.denominator = sensor->mode->framerate;

	return 0;
}

static int ops_set_frame_interval(struct v4l2_subdev *sub_dev, struct v4l2_subdev_frame_interval *fi)
{
	int i;
	struct gs_ar0234_dev *sensor = to_gs_ar0234_dev(sub_dev);

	dev_dbg(sub_dev->dev, "%s(setting interval = %d)\n", __func__, fi->interval.denominator);
	for(i=0; i < ARRAY_SIZE(sensor_res_list); i++) {
		if(sensor_res_list[i].framerate == fi->interval.denominator) {
			sensor->framerate = fi->interval.denominator;
			return 0;
		}
	}

	dev_err(sensor->dev, "unsupported framerate: %d\n", fi->interval.denominator);
	return -EINVAL;
}

static int gs_ar0234_enum_mbus_code(struct v4l2_subdev *sub_dev, struct v4l2_subdev_state *sd_state, struct v4l2_subdev_mbus_code_enum *code)
{
	int ret = 0;
	dev_dbg_ratelimited(sub_dev->dev, "%s: \n", __func__);

	if ((code->pad >= NUM_PADS))
		return -EINVAL;

	switch (code->index) {
		case 	GS_CF_YUV422:
		case 	GS_CF_YUV420:
		case 	GS_CF_YUV400:
		case 	GS_CF_YUV422BT:
		case 	GS_CF_YUV420BT:
		case 	GS_CF_YUV400BT:
			code->code = MEDIA_BUS_FMT_YUYV8_1X16;
			break;
		case 	GS_CF_RGB_888:
			code->code = MEDIA_BUS_FMT_RGB888_3X8;
			break;
		case 	GS_CF_RGB_565:
			code->code = MEDIA_BUS_FMT_RGB565_2X8_LE;
			break;
		case 	GS_CF_RGB_555:
			code->code = MEDIA_BUS_FMT_RGB555_2X8_PADHI_LE;
			break;
		case 	GS_CF_JPEG422:
		case 	GS_CF_JPEG420:
		case 	GS_CF_JPEG422EX:
		case 	GS_CF_JPEG420EX:
			code->code = MEDIA_BUS_FMT_JPEG_1X8;
			break;
		case 	GS_CF_BAYER_16:
			code->code = MEDIA_BUS_FMT_SRGGB16_1X16;
			break;
		case 	GS_CF_BAYER_12:
			code->code = MEDIA_BUS_FMT_SRGGB12_1X12;
			break;
		case 	GS_CF_BAYER_10:
			code->code = MEDIA_BUS_FMT_SRGGB10_1X10;
			break;
		case 	GS_CF_BAYER_8:
			code->code = MEDIA_BUS_FMT_SRGGB8_1X8;
			break;
		default:
			ret = -EINVAL;
			break;
	}
	dev_dbg_ratelimited(sub_dev->dev, "%s: code->code = 0x%08x\n", __func__, code->code);
	return ret;
}

static int gs_ar0234_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct gs_ar0234_dev *sensor = to_gs_ar0234_dev(sd);
	int ret = 0;

	if (sensor->ep.bus_type != V4L2_MBUS_CSI2_DPHY){
		dev_err(sensor->dev, "endpoint bus_type not supported: %d\n", sensor->ep.bus_type);
		return -EINVAL;
	}
	
	if (enable)
	{
		mutex_lock(&sensor->lock);

		// turn off mipi?
		gs_ar0234_write_reg8(sensor, 0xE1, 0x02); // format change state
		// set rres, fixed formats reg 0x10,
		gs_ar0234_write_reg8(sensor, 0x10, sensor->mode->frame_format_code);
		// set fr reg- 0x16 (16b = 8b,8b [fraction)]) = 60,50,30,25 or any int
		gs_ar0234_write_reg16(sensor, 0x16, ((u16)(sensor->mode->framerate) << 8));
		//sensor->fmt.
		//  set format type
		//turn on mipi
		gs_ar0234_write_reg8(sensor, 0xE1, 0x03); // format change state

		// ret = gs_ar0234_upcall(sensor, "start_stream");

		mutex_unlock(&sensor->lock);

		pr_debug("%s: Starting stream at WxH@fps=%dx%d@%d\n", __func__, sensor->mode->width, sensor->mode->height, sensor->mode->framerate);
	}
	else
		pr_debug("%s: Stopping stream \n", __func__);

	return ret;
}


int gs_ar0234_init_cfg(struct v4l2_subdev *sd, struct v4l2_subdev_state *state) 
{
	struct gs_ar0234_dev *sensor = to_gs_ar0234_dev(sd);
	dev_info(sensor->dev, "%s \n",__func__);
	//TODO: DEBUG
#if 0
	struct gs_ar0234_dev *sensor = to_gs_ar0234_dev(sd);
	pr_debug("%s: BLa\n", __func__);
	mutex_lock(&sensor->lock);
	pr_debug("%s: BLa\n", __func__);
	mutex_unlock(&sensor->lock);
	pr_debug("%s: BLa\n", __func__);
#endif
	return 0;
}


static const struct v4l2_subdev_core_ops gs_ar0234_core_ops = {
	.s_power = gs_ar0234_s_power,
	.log_status = v4l2_ctrl_subdev_log_status,
	.subscribe_event = v4l2_ctrl_subdev_subscribe_event,
	.unsubscribe_event = v4l2_event_subdev_unsubscribe,
};

static const struct v4l2_subdev_video_ops gs_ar0234_video_ops = {
	.g_frame_interval = ops_get_frame_interval,
	.s_frame_interval = ops_set_frame_interval,
	.s_stream = gs_ar0234_s_stream,
};

static const struct v4l2_subdev_pad_ops gs_ar0234_pad_ops = {
	.init_cfg = gs_ar0234_init_cfg,
	.enum_mbus_code = gs_ar0234_enum_mbus_code,
	.get_fmt = ops_get_fmt,
	.set_fmt = gs_ar0234_set_fmt,
	.enum_frame_size = ops_enum_frame_size,
	.enum_frame_interval = ops_enum_frame_interval,
};

static const struct v4l2_subdev_ops gs_ar0234_subdev_ops = {
	.core = &gs_ar0234_core_ops,
	.video = &gs_ar0234_video_ops,
	.pad = &gs_ar0234_pad_ops,
};

static int gs_ar0234_link_setup(struct media_entity *entity, const struct media_pad *local, const struct media_pad *remote, u32 flags)
{
	return 0;
}

static const struct media_entity_operations gs_ar0234_sd_media_ops = {
	.link_setup = gs_ar0234_link_setup,
};

static const struct regmap_config sensor_regmap_config = {
	.reg_bits = 8,
	.val_bits = 8,
	.cache_type = REGCACHE_NONE,
};


static void gs_ar0234_remove(struct i2c_client *client);


/**
 * @brief update firmware function
 * 
 * @param fw 
 * @param context 
 */
static void gs_ar0234_fw_update(const struct firmware *fw, void *context)
{
	int ret;
	struct gs_ar0234_dev *sensor = (struct gs_ar0234_dev *)context;

	if (!fw)
		return;

	mutex_lock(&sensor->lock);

	dev_info(sensor->dev, "--------> Firmware update in progress . . .\n");

	switch(sensor->update_type)
	{
		case MCU:
		case NVM:
		case MCUNVM:
		case BOOT:
			ret = flashapp(sensor, (char *) fw->data, fw->size);
			break;
		case ISP:
			ret = flashisp(sensor, (char *) fw->data, fw->size);
			break;
		default:
			break;
	}	
	if(ret < 0)
		goto exit;

	//print_hex_dump(KERN_ALERT, "", DUMP_PREFIX_OFFSET, 16, 1, fw->data, 100, 1);
	//write_lines((char *) fw->data, fw->size);

	sensor->firmware_loaded = 1;
exit:
	//release_firmware(fw);
	mutex_unlock(&sensor->lock);
	if (ret < 0) {
		dev_err(sensor->dev, "Failed to load firmware: %d\n", ret);
		sensor->firmware_loaded = 0;
		//gs_ar0234_remove(sensor->i2c_client);
	}
	else
		dev_info(sensor->dev, "<-------- Firmware update finised\n");
}

/**
 * @brief fw update handler
 * 
 * @param fw 
 * @param context 
 */
static void gs_ar0234_fw_handler(const struct firmware *fw, void *context)
{
	int ret;
	struct gs_ar0234_dev *sensor = (struct gs_ar0234_dev *)context;
	const struct firmware *fw_local;
	u16 isp_code;
	char * isp_name;

	mutex_lock(&sensor->probe_lock);

	if(sensor->update_type == BOOT) // always update both MCU firmware and NVM
	{
		dev_info(sensor->dev, "Boot: Loading MCU Firmware: %s (%04x)\n", MCU_FIRMWARE_NAME, MCU_FIRMWARE_VERSION);
		gs_ar0234_fw_update(fw, sensor);
		release_firmware(fw); 
		sensor->mcu_version = MCU_FIRMWARE_VERSION;
		sensor->nvm_version = NVM_FIRMWARE_VERSION;
	}
	else 
	{
		if(sensor->mcu_version != MCU_FIRMWARE_VERSION) 
		{
			sensor->update_type = MCU; // update MCU firmware only
			dev_info(sensor->dev, "Loading MCU Firmware: %s (%04x)\n", MCU_FIRMWARE_NAME, MCU_FIRMWARE_VERSION);
			gs_ar0234_fw_update(fw, sensor);
			release_firmware(fw); 
			sensor->mcu_version = MCU_FIRMWARE_VERSION;
		}
		else 
			release_firmware(fw); 

		// update NVM if version does not match and version number in the conf five is not zero
		if ( (sensor->nvm_version != nvm_firmware_versions[sensor->csi_id]) && (nvm_firmware_versions[sensor->csi_id] != 0)) 
		{
			if(nvm_firmware_names[sensor->csi_id] != NULL) // firmware name must exist
			{
				sensor->update_type = NVM;
				dev_info(sensor->dev, "Loading NVM Firmware: %s (%04x)\n", nvm_firmware_names[sensor->csi_id], nvm_firmware_versions[sensor->csi_id]);
				if(request_firmware_direct(&fw_local, nvm_firmware_names[sensor->csi_id], sensor->dev) == 0) 
				{
					gs_ar0234_fw_update(fw_local,sensor);
					release_firmware(fw_local); 
					sensor->nvm_version = nvm_firmware_versions[sensor->csi_id];
				}
				else	
					dev_err(sensor->dev, "request_firmware_direct failed\n");
			}
			else
				dev_err(sensor->dev, "Loading NVM Firmware failed\n");
		}
	}

	ret = gs_ar0234_version(sensor, ISP, &isp_code);
	if(ret)
		dev_err(sensor->dev, "gs_ar0234_version failed\n");

	pr_debug("-->%s: ISP version: %04x\n",__func__, isp_code);
	if(isp_code != ISP_FIRMWARE_VERSION) 
	{
		sensor->update_type = ISP;	
		ret = gs_get_camera_type(sensor, &sensor->sensor_type);
		if(ret)
			dev_err(sensor->dev, "gs_get_camera_type failed\n");
		pr_debug("---%s: cameratype: %d\n",__func__, sensor->sensor_type);	

		if (sensor->sensor_type == COLOR) isp_name=ISP_COLOR_FIRMWARE_NAME;
		else if (sensor->sensor_type == MONOCHROME) isp_name=ISP_MONO_FIRMWARE_NAME;

		if(sensor->sensor_type != UNKNOWN) // dont update, leave previous ISP image intact
		{
			dev_info(sensor->dev, "Loading ISP Firmware: %s (%04x)\n", isp_name, ISP_FIRMWARE_VERSION);
			if(request_firmware_direct(&fw_local, ISP_COLOR_FIRMWARE_NAME, sensor->dev) == 0) {
				gs_ar0234_fw_update(fw_local, sensor);
				release_firmware(fw_local); 
				sensor->isp_version = ISP_FIRMWARE_VERSION;
			}
			else	
				dev_err(sensor->dev, "request_firmware_direct failed\n");
		}
		else 
			dev_info(sensor->dev, "Loading ISP Firmware skipped\n");
	}

	// read register values from Sensor
	ret = gs_ar0234_i_cntrl(sensor);
	if (ret)
		sensor->firmware_loaded = -1;

	// Power down
	ret = gs_ar0234_s_power(&sensor->sd, GS_POWER_DOWN);
	if (ret)
		sensor->firmware_loaded = -1;
	pr_debug("---%s: Power down\n",__func__);	
	mutex_unlock(&sensor->probe_lock);
}




#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 3, 0)
static int gs_ar0234_probe(struct i2c_client *client)
#else
static int gs_ar0234_probe(struct i2c_client *client, const struct i2c_device_id *id)
#endif
{
	struct device *dev = &client->dev;
	struct fwnode_handle *endpoint;
	struct gs_ar0234_dev *sensor;
	struct v4l2_mbus_framefmt *fmt;
	int ret;
	u16 mcu_code, nvm_code, isp_code;
	bool update = false;

	pr_debug("-->%s: gs_ar0234 Probe start\n",__func__);

	sensor = devm_kzalloc(dev, sizeof(*sensor), GFP_KERNEL);
	if (!sensor)
		return -ENOMEM;

	// default init sequence initialize sensor to 1080p30 YUV422 UYVY
	fmt = &sensor->fmt;
	fmt->code = MEDIA_BUS_FMT_UYVY8_1X16; // MEDIA_BUS_FMT_UYVY8_2X8
	fmt->colorspace = V4L2_COLORSPACE_SRGB;
	fmt->ycbcr_enc = V4L2_MAP_YCBCR_ENC_DEFAULT(fmt->colorspace);
	fmt->quantization = V4L2_QUANTIZATION_FULL_RANGE;
	fmt->xfer_func = V4L2_MAP_XFER_FUNC_DEFAULT(fmt->colorspace);
	fmt->width = 1280;
	fmt->height = 720;
	fmt->field = V4L2_FIELD_NONE;
	sensor->framerate = 30;
	sensor->mode = &sensor_res_list[0];
	sensor->mbus_num = GS_CF_YUV422;
	sensor->update_type = NONE;
	sensor->dev = dev;

	/* request reset pin */
	sensor->reset_gpio = devm_gpiod_get_optional(dev, "reset", GPIOD_ASIS);
	if (IS_ERR(sensor->reset_gpio)) {
		ret = PTR_ERR(sensor->reset_gpio);
		if (ret != -EPROBE_DEFER)
			dev_err(dev, "Cannot get reset GPIO (%d)", ret);
		return ret;
	}

	endpoint = fwnode_graph_get_next_endpoint(dev_fwnode(&client->dev), NULL);
	if (!endpoint) {
		dev_err(dev, "endpoint node not found\n");
		return -EINVAL;
	}

	ret = v4l2_fwnode_endpoint_parse(endpoint, &sensor->ep);
	fwnode_handle_put(endpoint);
	if (ret) {
		dev_err(dev, "Could not parse endpoint\n");
		return ret;
	}

	pr_debug("---%s: 1 sensor->ep.bus_type=%d\n", __func__, sensor->ep.bus_type);

	if (sensor->ep.bus_type != V4L2_MBUS_CSI2_DPHY) {
		dev_err(dev, "Unsupported bus type %d\n", sensor->ep.bus_type);
		return -EINVAL;
	}

	sensor->regmap = devm_regmap_init_i2c(client, &sensor_regmap_config);
	if (IS_ERR(sensor->regmap)) {
		dev_err(dev, "regmap init failed\n");
		return PTR_ERR(sensor->regmap);
	}
	sensor->i2c_client = client;

	mutex_init(&sensor->lock);
	mutex_init(&sensor->probe_lock);

	// Power Up
	ret = gs_ar0234_s_power(&sensor->sd, GS_POWER_UP);
	if (ret) return -EIO;
	pr_debug("---%s: Power up\n",__func__);	

#ifdef DEBUG
	gs_print_params();
#endif

	ret = of_property_read_u32(dev->of_node, "csi_id", &sensor->csi_id); // get the csi port id
	if (ret) {
		dev_err(dev, "csi id missing or invalid\n");
		return ret;
	}
	pr_debug("---%s: CSI ID = %d\n",__func__,sensor->csi_id);	

	mutex_lock(&sensor->probe_lock);

	 // check for bootloader
	ret = gs_boot_id(sensor, &mcu_code);
	if (ret) return -EIO;
	if(mcu_code == BOOTID) //in case camera is in bootloader, program the default MCU firmware.
	{
		update = true;
		sensor->update_type = BOOT;

		// call update handler
		ret = request_firmware_nowait(THIS_MODULE, FW_ACTION_UEVENT, MCU_FIRMWARE_NAME, dev, GFP_KERNEL, sensor, gs_ar0234_fw_handler);
		if (ret) {
			dev_err(dev, "Failed request_firmware_nowait err %d\n", ret);
			if (ret) return -EIO;
		}
	}
	else  // else if one of the firmware versions need updating, then update
	{
		// get firmware versions
		ret = gs_ar0234_version(sensor, MCU, &mcu_code);
		if (ret) return -EIO;

		ret = gs_ar0234_version(sensor, NVM, &nvm_code);
		if (ret) return -EIO;

		ret = gs_ar0234_version(sensor, ISP, &isp_code);
		if (ret) return -EIO;

		sensor->mcu_version = mcu_code;
		sensor->nvm_version = nvm_code;
		sensor->isp_version = isp_code;

		pr_debug("---%s: Firmware versions: %04x %04x %04x\n", __func__, mcu_code, nvm_code, isp_code);
		if((mcu_code != MCU_FIRMWARE_VERSION) || (nvm_code != nvm_firmware_versions[sensor->csi_id]) || (isp_code != ISP_FIRMWARE_VERSION))
		{
			update = true;
			// call update handler
			//dev_info(dev, "Normal: Loading  MCU Firmware: %04x\n", MCU_FIRMWARE_VERSION);
			ret = request_firmware_nowait(THIS_MODULE, FW_ACTION_UEVENT, MCU_FIRMWARE_NAME, dev, GFP_KERNEL, sensor, gs_ar0234_fw_handler);
			if (ret) {
				dev_err(dev, "Failed request_firmware_nowait err %d\n", ret);
				if (ret) return -EIO;
			}
		}
	}

	ret = gs_get_camera_type(sensor, &sensor->sensor_type);
	if (ret) return -EINVAL;

	v4l2_i2c_subdev_init(&sensor->sd, client, &gs_ar0234_subdev_ops);

	sensor->sd.flags |= V4L2_SUBDEV_FL_HAS_EVENTS | V4L2_SUBDEV_FL_HAS_DEVNODE;
	sensor->sd.dev = &client->dev;
	sensor->pad.flags = MEDIA_PAD_FL_SOURCE;
	sensor->sd.entity.ops = &gs_ar0234_sd_media_ops;
	sensor->sd.entity.function = MEDIA_ENT_F_CAM_SENSOR;

	ret = media_entity_pads_init(&sensor->sd.entity, 1, &sensor->pad);
	if (ret) return -EINVAL;

	ret = gs_ar0234_init_controls(sensor);
	if (ret)
		goto entity_cleanup;

	ret = v4l2_async_register_subdev_sensor(&sensor->sd);
	if (ret)
		goto free_ctrls;

	if(update == false) // if firmware update was performed dont do this
	{
		// read register values from Sensor
		ret = gs_ar0234_i_cntrl(sensor);
		if (ret)
			goto free_ctrls;

		// Power down
		ret = gs_ar0234_s_power(&sensor->sd, GS_POWER_DOWN);
		if (ret)
			goto free_ctrls;
		pr_debug("---%s: Power down\n",__func__);	
	} // if firmware update was performed untill here

	pr_debug("<--%s: gs_ar0234 Probe end successful, return\n",__func__);
	mutex_unlock(&sensor->probe_lock);
	mutex_destroy(&sensor->probe_lock);

	return 0;

free_ctrls:
	v4l2_ctrl_handler_free(&sensor->ctrls.handler);
entity_cleanup:
	pr_debug("<--%s: gs_ar0234 ERR entity_cleanup\n",__func__);
	media_entity_cleanup(&sensor->sd.entity);
	mutex_destroy(&sensor->lock);
	return ret;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
static int gs_ar0234_remove(struct i2c_client *client)
#else
static void gs_ar0234_remove(struct i2c_client *client)
#endif
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct gs_ar0234_dev *sensor = to_gs_ar0234_dev(sd);

	v4l2_async_unregister_subdev(&sensor->sd);
	media_entity_cleanup(&sensor->sd.entity);
	mutex_destroy(&sensor->lock);
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
	return 0;
#endif
}

static const struct i2c_device_id gs_ar0234_id[] = {
	{"gs_ar0234", 0},
	{},
};
MODULE_DEVICE_TABLE(i2c, gs_ar0234_id);

static const struct of_device_id gs_ar0234_dt_ids[] = {
	{ .compatible = "scailx,gs_AR0234" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, gs_ar0234_dt_ids);

static struct i2c_driver gs_ar0234_i2c_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name  = "gs_ar0234",
		.of_match_table	= gs_ar0234_dt_ids,
	},
	.id_table = gs_ar0234_id,
	.probe = gs_ar0234_probe,
	.remove = gs_ar0234_remove,
};

module_i2c_driver(gs_ar0234_i2c_driver);

MODULE_DESCRIPTION("gs_ar0234 MIPI Camera Subdev Driver");
MODULE_LICENSE("GPL");
