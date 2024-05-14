/*
 * Copyright (C) 2023 Videology Inc, Inc. All Rights Reserved.
 */
#ifndef INCLUDES_CAM_AR0234_H_
#define INCLUDES_CAM_AR0234_H_

#include <linux/delay.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-fwnode.h>
#include <media/v4l2-subdev.h>

#define MIN_HEIGHT			720
#define MIN_WIDTH			1280
#define MAX_HEIGHT			1080
#define MAX_WIDTH			1920

#define PAD_SINK			0
#define PAD_SOURCE			1
#define NUM_PADS			1

#define MAX_CAMERA_DEVICES 10

#define V4L2_CID_CAMERA_CAM_AR0234 	(V4L2_CID_CAMERA_CLASS_BASE+50) 		//camera controls for CAM_AR0234
#define V4L2_CID_USER_CAM_AR0234 	(V4L2_CID_USER_BASE+2000) 				//user controls for CAM_AR0234
#define V4L2_CID_DETECT_CAM_AR0234 	(V4L2_CID_DETECT_CLASS_BASE+50) 		//detect controls for CAM_AR0234
#define V4L2_CID_PROC_CAM_AR0234 	(V4L2_CID_IMAGE_PROC_CLASS_BASE+50) 	//Image processing controls for CAM_AR0234

// Camera controls
#define V4L2_CID_ZOOM_SPEED			(V4L2_CID_CAMERA_CAM_AR0234+0)
#define V4L2_CID_ROI_MODE_0      	(V4L2_CID_CAMERA_CAM_AR0234+1)
#define V4L2_CID_ROI_MODE_1      	(V4L2_CID_CAMERA_CAM_AR0234+2)
#define V4L2_CID_ROI_MODE_2      	(V4L2_CID_CAMERA_CAM_AR0234+3)
#define V4L2_CID_EXPOSURE_UPPER		(V4L2_CID_CAMERA_CAM_AR0234+4)
#define V4L2_CID_EXPOSURE_MAX		(V4L2_CID_CAMERA_CAM_AR0234+5)
#define V4L2_CID_GAIN_UPPER			(V4L2_CID_CAMERA_CAM_AR0234+6)
#define V4L2_CID_GAIN_MAX			(V4L2_CID_CAMERA_CAM_AR0234+7)
#define V4L2_CID_BLC_WINDOW_X0		(V4L2_CID_CAMERA_CAM_AR0234+8)
#define V4L2_CID_BLC_WINDOW_Y0		(V4L2_CID_CAMERA_CAM_AR0234+9)
#define V4L2_CID_BLC_WINDOW_X1		(V4L2_CID_CAMERA_CAM_AR0234+10)
#define V4L2_CID_BLC_WINDOW_Y1		(V4L2_CID_CAMERA_CAM_AR0234+11)
#define V4L2_CID_BLC_RATIO		    (V4L2_CID_CAMERA_CAM_AR0234+12)
#define V4L2_CID_BLC_FACE_LEVEL	    (V4L2_CID_CAMERA_CAM_AR0234+13)
#define V4L2_CID_BLC_FACE_WEIGHT    (V4L2_CID_CAMERA_CAM_AR0234+14)
#define V4L2_CID_BLC_ROI_LEVEL      (V4L2_CID_CAMERA_CAM_AR0234+15)
#define V4L2_CID_AWB_MAN_X			(V4L2_CID_CAMERA_CAM_AR0234+16)
#define V4L2_CID_AWB_MAN_Y			(V4L2_CID_CAMERA_CAM_AR0234+17)
#define V4L2_CID_STORE_REGISTERS    (V4L2_CID_CAMERA_CAM_AR0234+20)
#define V4L2_CID_RESTORE_REGISTERS  (V4L2_CID_CAMERA_CAM_AR0234+21)
#define V4L2_CID_RESTORE_FACTORY 	(V4L2_CID_CAMERA_CAM_AR0234+22) // restores both registers and calibration parameters
#define V4L2_CID_REBOOT         	(V4L2_CID_CAMERA_CAM_AR0234+23) 

// User controls
#define V4L2_CID_NOISE_RED      	(V4L2_CID_USER_CAM_AR0234+0)

// Detect controls
#define V4L2_CID_FACE_DETECT_0			(V4L2_CID_DETECT_CAM_AR0234+0)
#define V4L2_CID_FACE_DETECT_4			(V4L2_CID_DETECT_CAM_AR0234+1)
#define V4L2_CID_FACE_DETECT_5			(V4L2_CID_DETECT_CAM_AR0234+2)
#define V4L2_CID_FACE_DETECT_SPEED  	(V4L2_CID_DETECT_CAM_AR0234+3)	
#define V4L2_CID_FACE_DETECT_THRESHOLD	(V4L2_CID_DETECT_CAM_AR0234+4)	
#define V4L2_CID_FACE_CHROMA_THRESHOLD	(V4L2_CID_DETECT_CAM_AR0234+5)	
#define V4L2_CID_FACE_MIN_SIZE			(V4L2_CID_DETECT_CAM_AR0234+6)	
#define V4L2_CID_FACE_MAX_SIZE			(V4L2_CID_DETECT_CAM_AR0234+7)	

// Image Processing  controls
#define V4L2_CID_PROC_BLA			(V4L2_CID_PROC_CAM_AR0234+0)
#define V4L2_CID_PROC_BLA1			(V4L2_CID_PROC_CAM_AR0234+1)

struct resolution {
	u16 width;
	u16 height;
	u16 framerate;
	u16 frame_format_code;
	char *name;
};

struct gs_ar0234_ctrls {
	struct v4l2_ctrl_handler handler;
	// struct v4l2_ctrl *pixel_rate;
	struct v4l2_ctrl *auto_exp;
	struct v4l2_ctrl *exposure;
	struct v4l2_ctrl *exposure_absolute;
	struct v4l2_ctrl *exposure_metering; // =blc mode
	struct v4l2_ctrl *exposure_upper;
	struct v4l2_ctrl *exposure_max;
	struct v4l2_ctrl *gain_upper;
	struct v4l2_ctrl *gain_max;
	struct v4l2_ctrl *blc_level;
	struct v4l2_ctrl *blc_window_x0;
	struct v4l2_ctrl *blc_window_y0;
	struct v4l2_ctrl *blc_window_x1;
	struct v4l2_ctrl *blc_window_y1;
	struct v4l2_ctrl *blc_ratio;
	struct v4l2_ctrl *blc_face_level;
	struct v4l2_ctrl *blc_face_weight;
	struct v4l2_ctrl *blc_roi_level;
	struct v4l2_ctrl *face_detect_0;
	struct v4l2_ctrl *face_detect_4;
	struct v4l2_ctrl *face_detect_5;
	struct v4l2_ctrl *face_detect_speed;	
	struct v4l2_ctrl *face_detect_threshold;
	struct v4l2_ctrl *face_chroma_threshold;
	struct v4l2_ctrl *face_min_size;
	struct v4l2_ctrl *face_max_size;
	struct v4l2_ctrl *auto_wb;
	struct v4l2_ctrl *push_to_white;
	struct v4l2_ctrl *wb_temp;
	struct v4l2_ctrl *wb_preset;
	// struct v4l2_ctrl *blue_balance;
	// struct v4l2_ctrl *red_balance;
	struct v4l2_ctrl *awb_man_x;
	struct v4l2_ctrl *awb_man_y;
	// struct v4l2_ctrl *auto_gain;
	struct v4l2_ctrl *gain;
	struct v4l2_ctrl *brightness;
	// struct v4l2_ctrl *light_freq;
	struct v4l2_ctrl *saturation;
	struct v4l2_ctrl *contrast;
	struct v4l2_ctrl *sharpness;
	struct v4l2_ctrl *noise_red;
	struct v4l2_ctrl *roi_mode_0;
	struct v4l2_ctrl *roi_mode_1;
	struct v4l2_ctrl *roi_mode_2;
	struct v4l2_ctrl *gamma;
	// struct v4l2_ctrl *hue;
	struct v4l2_ctrl *hflip;
	struct v4l2_ctrl *vflip;
	struct v4l2_ctrl *powerline;
	struct v4l2_ctrl *testpattern;
	struct v4l2_ctrl *colorfx;
	struct v4l2_ctrl *zoom;
	struct v4l2_ctrl *zoom_speed;
	struct v4l2_ctrl *pan;
	struct v4l2_ctrl *tilt;
	struct v4l2_ctrl *store_registers;
	struct v4l2_ctrl *restore_registers;
	struct v4l2_ctrl *restore_factory;
	struct v4l2_ctrl *reboot;
};

struct gs_ar0234_dev {
	struct device *dev;
	struct regmap *regmap;
	struct i2c_client *i2c_client;
	struct v4l2_subdev sd;
	struct media_pad pad;
	struct v4l2_fwnode_endpoint ep; /* the parsed DT endpoint info */
	struct gpio_desc *reset_gpio;
	struct mutex lock;
	struct mutex probe_lock;
	struct v4l2_mbus_framefmt fmt;
	struct gs_ar0234_ctrls ctrls;
	//const struct resolution *mode;
	int mbus_num;
	int framerate;
	int firmware_loaded;
	int update_type;
	u16 mcu_version;
	u16 nvm_version;
	u16 isp_version;
	u8  sensor_type;
	int csi_id;
};


#endif // INCLUDES_CAM_AR0234_H_