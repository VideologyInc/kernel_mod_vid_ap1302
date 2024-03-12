// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 Videology Inc, Inc. All Rights Reserved.
 */
#ifndef INCLUDES_GS_AP1302_H_
#define INCLUDES_GS_AP1302_H_

#define I2C_RETRIES 50

int gs_ar0234_read_reg8(struct gs_ar0234_dev *sensor, u8 addr, u8 *val);
int gs_ar0234_read_reg16(struct gs_ar0234_dev *sensor, u8 addr, u16 *val);
int gs_ar0234_read_reg32(struct gs_ar0234_dev *sensor, u8 addr, u32 *val);
int gs_ar0234_write_reg8(struct gs_ar0234_dev *sensor, u8 addr, u8 val);
int gs_ar0234_write_reg16(struct gs_ar0234_dev *sensor, u8 addr, u16 val);
int gs_ar0234_write_reg32(struct gs_ar0234_dev *sensor, u8 addr, u32 val);
int gs_ar0234_power(struct gs_ar0234_dev *sensor, int on);

#endif // INCLUDES_GS_AP1302_H_