/*
 * Copyright (C) 2023 Videology Inc, Inc. All Rights Reserved.
 */
#ifndef INCLUDES_GS_AP1302_H_
#define INCLUDES_GS_AP1302_H_

#define I2C_RETRIES 300

#define GS_POWER_UP 		1
#define GS_POWER_DOWN 		0

/**
 * bootloader flash defines 
 */
#define FLASH_APP_CRC_ADDRESS       0xF3F0
#define FLASH_APP_SIZE_ADDRESS      0xF3F4
#define FLASH_PAGE_SIZE             512
#define FLASH_APP_START_ADDRESS     0x1A00
#define FLASH_APP_MAX               0xF3FF
#define FLASH_NVM_START_ADDRESS     0xF400
#define FLASH_NVM_MAX               0xF7FF
#define FLASH_APP_MAX_ADDRESS       0xF3E0
#define FLASH_MAX                   0xF9FF
#define FLASH_APP_SIZE  FLASH_APP_MAX - FLASH_APP_START + 1
#define FLASH_NVM_SIZE  FLASH_NVM_MAX - FLASH_NVM_START + 1
#define PPP 0x0554
#define BOOTID 0x5AA5
#define INITIAL_CRC 0xFFFF


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
	GS_REG_CAMERA_TYPE              = 0xF1,
	GS_REG_ISP_MINOR_VERSION		= 0xEC,
	GS_REG_ISP_MAJOR_VERSION		= 0xED,
	GS_REG_NVM_MINOR_VERSION		= 0xEE,
	GS_REG_NVM_MAJOR_VERSION		= 0xEF,
	GS_REG_MCU_MINOR_VERSION		= 0xFE,
	GS_REG_MCU_MAJOR_VERSION		= 0xFF,
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

enum versiontype {
	NONE = 0,
	MCU,
	MCUNVM,
	NVM,
	ISP,
	BOOT
};


enum cameratype {
	UNKNOWN = 0,
	COLOR,
	MONOCHROME
};

/**
 * Function prototypes
 */

// Register - mainapp
int gs_ar0234_read_reg8(struct gs_ar0234_dev *sensor, u8 addr, u8 *val);
int gs_ar0234_read_reg16(struct gs_ar0234_dev *sensor, u8 addr, u16 *val);
int gs_ar0234_read_reg32(struct gs_ar0234_dev *sensor, u8 addr, u32 *val);
int gs_ar0234_write_reg8(struct gs_ar0234_dev *sensor, u8 addr, u8 val);
int gs_ar0234_write_reg16(struct gs_ar0234_dev *sensor, u8 addr, u16 val);
int gs_ar0234_write_reg32(struct gs_ar0234_dev *sensor, u8 addr, u32 val);

// funtions - mainapp
int gs_ar0234_power(struct gs_ar0234_dev *sensor, int on);
int gs_ar0234_version(struct gs_ar0234_dev *sensor, int type, u16 * version);
int gs_read_serial(struct gs_ar0234_dev *sensor, u8 * buf);
int gs_set_password(struct gs_ar0234_dev *sensor, u16 password);
int gs_restart(struct gs_ar0234_dev *sensor);
int gs_start_bootloader(struct gs_ar0234_dev *sensor);

// generic
int gs_check(struct gs_ar0234_dev *sensor);
int gs_check_wait(struct gs_ar0234_dev *sensor, u16 wait, u16 timeout);

// mainapp
int gs_upgrader_mode(struct gs_ar0234_dev *sensor);
int gs_get_camera_type(struct gs_ar0234_dev *sensor, u8 * type);


// NVM - mainapp
int gs_read_nvm(struct gs_ar0234_dev *sensor, u8 page, u8 addr, u8 size, u8 * buf);
int gs_ar0234_write_nvm(struct gs_ar0234_dev *sensor, u8 page, u8 addr, u8 size, u8 * buf);

// ISP - SPI Flash - mainapp
int gs_isp_write(struct gs_ar0234_dev *sensor, u32 addr, u8 size, u8 * buf);
int gs_isp_calc_crc(struct gs_ar0234_dev *sensor, u32 addr1, u32 addr2, u16 * crc);
int gs_isp_erase_page(struct gs_ar0234_dev *sensor, u32 addr);
int gs_isp_erase_all(struct gs_ar0234_dev *sensor);
int gs_get_spi_id(struct gs_ar0234_dev *sensor, u8 cmd, u8 * mf, u16 * id);
int gs_get_spistatus(struct gs_ar0234_dev *sensor, u16 * status);

// MainApp - bootloader
int gs_flashread(struct gs_ar0234_dev *sensor, u16 addr, u8 size, u8 * buf);
int gs_flashwrite(struct gs_ar0234_dev *sensor, u16 addr, u8 size, u8 * buf);
int gs_page_erase(struct gs_ar0234_dev *sensor, u16 addr, u16 size);
int gs_app_calc_crc(struct gs_ar0234_dev *sensor, u16 * pcrc);
int gs_app_read_crc(struct gs_ar0234_dev *sensor, u16 * pcrc);
int gs_app_read_size(struct gs_ar0234_dev *sensor, u16 * psize);
int gs_app_write_size(struct gs_ar0234_dev *sensor, u16 size);
int gs_app_write_crc(struct gs_ar0234_dev *sensor, u16 crc);
int gs_app_erase_crc(struct gs_ar0234_dev *sensor);
int gs_app_erase_size(struct gs_ar0234_dev *sensor);
int gs_erase_app(struct gs_ar0234_dev *sensor);
int gs_erase_nvm(struct gs_ar0234_dev *sensor);
int gs_erase_all(struct gs_ar0234_dev *sensor);
int gs_reboot(struct gs_ar0234_dev *sensor);
int gs_boot_id(struct gs_ar0234_dev *sensor, u16 * id);
int gs_boot_cameratype(struct gs_ar0234_dev *sensor, u8 * type);

u16 gs_crc(u16 seed, u8 count, u8 * value);

#endif // INCLUDES_GS_AP1302_H_