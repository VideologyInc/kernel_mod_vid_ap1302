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

#define GS_POWER_UP 		1
#define GS_POWER_DOWN 		0


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


enum commands {
	GS_COMD_8BIT_REG_W =            0x30,
	GS_COMD_8BIT_REG_R =            0x31,
	GS_COMD_16BIT_REG_W =           0x32,
	GS_COMD_16BIT_REG_R =           0x33,
	GS_COMD_32BIT_REG_W =           0x34,
	GS_COMD_32BIT_REG_R =           0x35,
	GS_COMD_ISP_FLASH_W =           0x40,
	GS_COMD_ISP_FLASH_R =           0x41,
	GS_COMD_ISP_FLASH_ERASE =       0x42,
	// Password protected commands
	GS_COMD_ISP_FLASH_GET_ID =      0x43,
	GS_COMD_ISP_FLASH_BLOCK_ERASE = 0x44,
	GS_COMD_ISP_FLASH_GET_STAT =    0x45,
	GS_COMD_ISP_FLASH_GET_CRC =     0x47,
	GS_COMD_NVM_W =                 0x50,
	GS_COMD_NVM_R =                 0x51,
	GS_COMD_NVM_ERASE =             0x52,
	GS_COMD_R_SERIAL =              0x61,
};

enum regs {
	GS_REG_DUMMY					= 0x00,
	GS_REG_BRIGHTNESS     			= 0x02,
	GS_REG_CONTRAST       			= 0x04,
	GS_REG_SATURATION     			= 0x06,
	GS_REG_SHARPNESS      			= 0x0A,
	GS_REG_NOISE_RED      			= 0x0C,
	GS_REG_GAMMA		  			= 0x0E,
	GS_REG_ZOOM						= 0x18,
	GS_REG_ZOOM_SPEED				= 0x1C,
	GS_REG_PAN						= 0x1D,
	GS_REG_TILT						= 0x1E,
	GS_REG_MIRROR_FLIP    			= 0x1F,
	GS_REG_EXPOSURE_MODE  			= 0x20,
	GS_REG_ROI_MODE     			= 0x21,
	GS_REG_EXPOSURE_UPPER   		= 0x24,
	GS_REG_EXPOSURE_MAX     		= 0x28,
	GS_REG_GAIN_UPPER       		= 0x2C,
	GS_REG_GAIN_MAX         		= 0x2E,
	GS_REG_GAIN						= 0x30,
	GS_REG_EXPOSURE_ABS    			= 0x34,
	GS_REG_AE_TARGET      			= 0x3C,
	GS_REG_BLC_MODE					= 0x40,
	GS_REG_BLC_LEVEL				= 0x41,
	GS_REG_BLC_WINDOW_X0    		= 0x42,
	GS_REG_BLC_WINDOW_Y0    		= 0x43,
	GS_REG_BLC_WINDOW_X1    		= 0x44,
	GS_REG_BLC_WINDOW_Y1    		= 0x45,
	GS_REG_BLC_RATIO        		= 0x4A,
	GS_REG_BLC_FACE_LEVEL   		= 0x4B,
	GS_REG_BLC_FACE_WEIGHT  		= 0x4C,
	GS_REG_BLC_ROI_LEVEL    		= 0x4F,
	GS_REG_WHITEBALANCE   			= 0x50,
	GS_REG_WB_TEMPERATURE 			= 0x52,
	GS_REG_FACE_DETECT 				= 0x55,
	GS_REG_ANTIFLICKER_MODE 		= 0x56,
	GS_REG_ANTIFLICKER_FREQ 		= 0x57,
	GS_REG_COLORFX					= 0x76,
	GS_REG_FACE_DETECT_SPEED 		= 0x82,
	GS_REG_FACE_DETECT_THRESHOLD 	= 0x83,
	GS_REG_FACE_CHROMA_THRESHOLD 	= 0x84,
	GS_REG_FACE_MIN_SIZE  			= 0x86,
	GS_REG_FACE_MAX_SIZE    		= 0x88,
	GS_REG_AWB_MAN_X				= 0x8A,
	GS_REG_AWB_MAN_Y				= 0x8C,
	GS_REG_TESTPATTERN 				= 0xE0,
	GS_REG_POWER                    = 0xE7,
	GS_REG_SAVE_RESTART				= 0xF0,
};

enum colorformat {
	GS_CF_YUV422 = 0,
	GS_CF_YUV420,
	GS_CF_YUV400,
	GS_CF_YUV422BT,
	GS_CF_YUV420BT,
	GS_CF_YUV400BT,
	GS_CF_RGB_888,
	GS_CF_RGB_565,
	GS_CF_RGB_555,
	GS_CF_JPEG422,
	GS_CF_JPEG420,
	GS_CF_JPEG422EX,
	GS_CF_JPEG420EX,
	GS_CF_BAYER_16,
	GS_CF_BAYER_12,
	GS_CF_BAYER_10,
	GS_CF_BAYER_8,
};

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
	struct v4l2_mbus_framefmt fmt;
	struct gs_ar0234_ctrls ctrls;
	const struct resolution *mode;
	int mbus_num;
	int framerate;
};





#endif // INCLUDES_CAM_AR0234_H_