/*
 * Copyright (C) 2024 Videology Inc, Inc. All Rights Reserved.
 */
#ifndef INCLUDES_G_IMAGE_UPDATE_H_
#define INCLUDES_G_IMAGE_UPDATE_H_

#define MAX_LINE 255
#define MAX_VALUES 100

// bootloader flash defines
#define FLASH_APP_START         0x1A00
#define FLASH_APP_MAX           0xF3FF
#define FLASH_APP_SIZE          FLASH_APP_MAX - FLASH_APP_START + 1
#define FLASH_NVM_START         0xF400
#define FLASH_NVM_MAX           0xF7FF
#define FLASH_NVM_SIZE          FLASH_NVM_MAX - FLASH_NVM_START + 1
#define FLASH_CRC_ADDRESS       0xF3F0
#define FLASH_APP_SIZE_ADDRESS  0xF3F4
#define FLASH_APP_MAX_ADDRESS   0xF3E0
#define FLASH_APP_START_ADDRESS 0x1A00
#define FLASH_MAX               0xF9FF
#define FLASH_PAGE_SIZE         512


int flashapp(struct gs_ar0234_dev *sensor, char * buffer, int size);
int flashisp(struct gs_ar0234_dev *sensor, char * buffer, int size);


#endif //INCLUDES_G_IMAGE_UPDATE_H_
