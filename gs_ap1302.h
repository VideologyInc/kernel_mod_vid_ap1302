/*
 * Copyright (C) 2023 Videology Inc, Inc. All Rights Reserved.
 */
#ifndef INCLUDES_GS_AP1302_H_
#define INCLUDES_GS_AP1302_H_

#define I2C_RETRIES 50



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


/**
 * Function prototypes
 */
int gs_ar0234_read_reg8(struct gs_ar0234_dev *sensor, u8 addr, u8 *val);
int gs_ar0234_read_reg16(struct gs_ar0234_dev *sensor, u8 addr, u16 *val);
int gs_ar0234_read_reg32(struct gs_ar0234_dev *sensor, u8 addr, u32 *val);
int gs_ar0234_write_reg8(struct gs_ar0234_dev *sensor, u8 addr, u8 val);
int gs_ar0234_write_reg16(struct gs_ar0234_dev *sensor, u8 addr, u16 val);
int gs_ar0234_write_reg32(struct gs_ar0234_dev *sensor, u8 addr, u32 val);
int gs_ar0234_power(struct gs_ar0234_dev *sensor, int on);

#endif // INCLUDES_GS_AP1302_H_