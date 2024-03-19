/*
 * Copyright (C) 2023 Videology Inc, Inc. All Rights Reserved.
 */

#include <linux/i2c.h>
#include <linux/types.h>

#include "cam_ar0234.h"
#include "gs_ap1302.h"

int gs_ar0234_i2c_trx_retry(struct i2c_adapter *adap, struct i2c_msg *msgs, int num)
{
	int i, ret;
	for (i = 0; i < I2C_RETRIES; i++) {
		ret = i2c_transfer(adap, msgs, num);
		if (ret >= 0)
			break;
		msleep(1);
	}
	return ret;
	// return 0;
}

int gs_ar0234_check8(struct gs_ar0234_dev *sensor)
{
	struct i2c_client *client = sensor->i2c_client;
	struct i2c_msg msg[2];
	u8 buf[2];
	int ret;

	buf[0] = 0x31;
	buf[1] = 0x00;

	msg[0].addr = client->addr;
	msg[0].flags = client->flags;
	msg[0].buf = buf;
	msg[0].len = sizeof(buf);

	msg[1].addr = client->addr;
	msg[1].flags = client->flags | I2C_M_RD;
	msg[1].buf = buf;
	msg[1].len = 1;

	ret = i2c_transfer(client->adapter, msg, 2);

	if (ret < 0) {
		//dev_err(&client->dev, "%s: poll8 \n", __func__);
		return -1;
	}
	return 0;
}

int gs_ar0234_read_reg8(struct gs_ar0234_dev *sensor, u8 addr, u8 *val)
{
	struct i2c_client *client = sensor->i2c_client;
	struct i2c_msg msg[2];
	u8 buf[2];
	int ret;

	buf[0] = 0x31;
	buf[1] = addr;

	msg[0].addr = client->addr;
	msg[0].flags = client->flags;
	msg[0].buf = buf;
	msg[0].len = sizeof(buf);

	msg[1].addr = client->addr;
	msg[1].flags = client->flags | I2C_M_RD;
	msg[1].buf = buf;
	msg[1].len = 1;
	ret = gs_ar0234_i2c_trx_retry(client->adapter, msg, 2);
	if (ret < 0) {
		dev_err(&client->dev, "%s: error: addr=%x, err=%d\n", __func__, addr, ret);
		return ret;
	}

	*val = buf[0];
	return 0;
}

int gs_ar0234_read_reg16(struct gs_ar0234_dev *sensor, u8 addr, u16 *val)
{
	struct i2c_client *client = sensor->i2c_client;
	struct i2c_msg msg[2];
	u8 buf[2];
	int ret;

	buf[0] = 0x33;
	buf[1] = addr;

	msg[0].addr = client->addr;
	msg[0].flags = client->flags;
	msg[0].buf = buf;
	msg[0].len = sizeof(buf);

	msg[1].addr = client->addr;
	msg[1].flags = client->flags | I2C_M_RD;
	msg[1].buf = buf;
	msg[1].len = 2;

	ret = gs_ar0234_i2c_trx_retry(client->adapter, msg, 2);
	if (ret < 0) {
		dev_err(&client->dev, "%s: error: addr=%x, err=%d\n", __func__, addr, ret);
		return ret;
	}

	*val = ((u16)buf[1] << 8) | (u16)buf[0];
	return 0;
}

int gs_ar0234_read_reg32(struct gs_ar0234_dev *sensor, u8 addr, u32 *val)
{
	struct i2c_client *client = sensor->i2c_client;
	struct i2c_msg msg[2];
	u8 buf[2];
	u8 bufo[4];
	int ret;

	buf[0] = 0x35;
	buf[1] = addr;

	msg[0].addr = client->addr;
	msg[0].flags = client->flags;
	msg[0].buf = buf;
	msg[0].len = sizeof(buf);

	msg[1].addr = client->addr;
	msg[1].flags = client->flags | I2C_M_RD;
	msg[1].buf = bufo;
	msg[1].len = 4;

	ret = gs_ar0234_i2c_trx_retry(client->adapter, msg, 2);
	if (ret < 0) {
		dev_err(&client->dev, "%s: error: addr=%x, err=%d\n", __func__, addr, ret);
		return ret;
	}

	*val = ((u32)bufo[3] << 24) | ((u32)bufo[2] << 16) | ((u32)bufo[1] << 8) | (u32)bufo[0];
	return 0;
}

int gs_ar0234_write_reg8(struct gs_ar0234_dev *sensor, u8 addr, u8 val)
{
	struct i2c_client *client = sensor->i2c_client;
	struct i2c_msg msg;
	u8 buf[3];
	int ret;

	buf[0] = 0x30;
	buf[1] = addr;
	buf[2] = val;

	msg.addr = client->addr;
	msg.flags = client->flags;
	msg.buf = buf;
	msg.len = sizeof(buf);

	ret = gs_ar0234_i2c_trx_retry(client->adapter, &msg, 1);
	if (ret < 0) {
		dev_err(&client->dev, "%s: error: addr=%x, err=%d\n", __func__, addr, ret);
		return ret;
	}

	return 0;
}

int gs_ar0234_write_reg16(struct gs_ar0234_dev *sensor, u8 addr, u16 val)
{
	struct i2c_client *client = sensor->i2c_client;
	struct i2c_msg msg;
	u8 buf[4];
	int ret;

	buf[0] = 0x32;
	buf[1] = addr;
	buf[2] = val & 0xff;
	buf[3] = (val >> 8) & 0xFF;

	msg.addr = client->addr;
	msg.flags = client->flags;
	msg.buf = buf;
	msg.len = sizeof(buf);

	ret = gs_ar0234_i2c_trx_retry(client->adapter, &msg, 1);
	if (ret < 0) {
		dev_err(&client->dev, "%s: error: addr=%x, err=%d\n", __func__, addr, ret);
		return ret;
	}

	return 0;
}

int gs_ar0234_write_reg32(struct gs_ar0234_dev *sensor, u8 addr, u32 val)
{
	struct i2c_client *client = sensor->i2c_client;
	struct i2c_msg msg;
	u8 buf[6];
	int ret;

	buf[0] = 0x34;
	buf[1] = addr;
	buf[2] = val & 0xff;
	buf[3] = (val >> 8) & 0xff;
	buf[4] = (val >> 16) & 0xff;
	buf[5] = (val >> 24) & 0xff;

	msg.addr = client->addr;
	msg.flags = client->flags;
	msg.buf = buf;
	msg.len = sizeof(buf);

	ret = gs_ar0234_i2c_trx_retry(client->adapter, &msg, 1);
	if (ret < 0) {
		dev_err(&client->dev, "%s: error: addr=%x, err=%d\n", __func__, addr, ret);
		return ret;
	}

	return 0;
}

int gs_ar0234_power(struct gs_ar0234_dev *sensor, int on)
{
	int ret;
	int timeout = 0; 
	if(on) 
	{
		ret = gs_ar0234_write_reg8(sensor, GS_REG_POWER, 0); //power up
		if(ret) return ret;
	
		// wait for camera to boot up and accept i2c commands
		msleep(100); // wait a bit for camera to start
		//msleep(2000);
		while(timeout < 5000) // 5 seconds 
		{
			if (gs_ar0234_check8(sensor) < 0  ) // check if I2C is fail 
			{
				timeout += 250; // in steps of 250 ms
				msleep(250);
			}
			else break; //exit
		}
	}
	else 
	{
		ret = gs_ar0234_write_reg8(sensor, GS_REG_POWER, 1); // power down
	}
	// pr_debug("<--%s:  timeout  = %d,  %d\n",__func__, timeout, on);
	return ret;
}

#if 0

int gs_check(struct gs_ar0234_dev *sensor)
{
	struct i2c_client *client = sensor->i2c_client;
	struct i2c_msg msg[2];
	u8 buf[1];
	int ret;

	buf[0] = 0x00;

	msg[0].addr = client->addr;
	msg[0].flags = client->flags;
	msg[0].buf = buf;
	msg[0].len = 0;

	ret = gs_ar0234_i2c_trx_retry(client->adapter, msg, 1);
	if (ret < 0) {
		dev_err(&client->dev, "%s: error: addr=%x, err=%d\n", __func__, addr, ret);
		return ret;
	}
	return 0;
}

int gs_check_wait(struct gs_ar0234_dev *sensor, u16 wait, u16 timeout)
{
	int ret;
	u16 to=0;

	ret = gs_check(sensor);
	while(ret != 0) 
	{
		to += wait;
		if(to > timeout) 
		{
			return -1;
		}
		mssleep(wait);
		ret = gs_check(sensor);
	}
	return 0;
}

int gs_read_serial(struct gs_ar0234_dev *sensor, u8 * buf)
{
	struct i2c_client *client = sensor->i2c_client;
	struct i2c_msg msg[2];
	u8 mybuf[1];
	int ret;
	
	if (sizeof(buf) < (sizeof(u8) * 16)) return -1;

	mybuf[0] = 0x61;

	msg[0].addr = client->addr;
	msg[0].flags = client->flags;
	msg[0].buf = mybuf;
	msg[0].len = sizeof(mybuf);

	msg[1].addr = client->addr;
	msg[1].flags = client->flags | I2C_M_RD;
	msg[1].buf = buf;
	msg[1].len = 16;
	ret = gs_ar0234_i2c_trx_retry(client->adapter, msg, 2);
	if (ret < 0) {
		dev_err(&client->dev, "%s: error: err=%d\n", __func__, ret);
		return ret;
	}
	return 0;
}

int gs_set_password(struct gs_ar0234_dev *sensor, u16 password)
{
	ret = gs_ar0234_write_reg8(sensor, 0xFC, password & 0xFF);
	if (ret) return ret;
	ret = gs_ar0234_write_reg8(sensor, 0xFD, (password>>8) & 0xFF);
	if (ret) return ret;
	return 0;
}

int restart(struct gs_ar0234_dev *sensor)
{
	ret = gs_ar0234_write_reg8(sensor, 0xF0, 0x99);
	if (ret) return ret;
	return 0;
}

/**
 *  NVM
 */

int gs_read_nvm(struct gs_ar0234_dev *sensor, u8 page, u8 addr, u8 size, u8 * buf)
{
	struct i2c_client *client = sensor->i2c_client;
	struct i2c_msg msg[2];
	u8 mybuf[4];
	int ret;
	
	if (page > 3) return -1;

	mybuf[0] = 0x51;
	mybuf[1] = page;
	mybuf[2] = addr;
	mybuf[3] = size;

	msg[0].addr = client->addr;
	msg[0].flags = client->flags;
	msg[0].buf = mybuf;
	msg[0].len = sizeof(mybuf);

	msg[1].addr = client->addr;
	msg[1].flags = client->flags | I2C_M_RD;
	msg[1].buf = buf;
	msg[1].len = size;
	ret = gs_ar0234_i2c_trx_retry(client->adapter, msg, 2);
	if (ret < 0) {
		dev_err(&client->dev, "%s: error: addr=%x, err=%d\n", __func__, addr, ret);
		return ret;
	}
	return 0;
}

int gs_ar0234_write_nvm(struct gs_ar0234_dev *sensor, u8 page, u8 addr, u8 size, u8 * buf)
{
	struct i2c_client *client = sensor->i2c_client;
	struct i2c_msg msg;
	u8 * mybuf;
	int ret;

	if (size == 0) return 0;

	mybuf = (u8 *) malloc(sizeof(u8) * (size+3));
	mybuf[0] = 0x50;
	mybuf[1] = page;
	mybuf[2] = addr;

	for (int i=0; i<size; i++)
	{
		mybuf[i+3] = buf[i];
	}

	msg.addr = client->addr;
	msg.flags = client->flags;
	msg.buf = mybuf;
	msg.len = sizeof(mybuf);

	ret = gs_ar0234_i2c_trx_retry(client->adapter, &msg, 1);
	if (ret < 0) {
		dev_err(&client->dev, "%s: error: addr=%x, err=%d\n", __func__, addr, ret);
		free(mybuf);
		return ret;
	}
	free(mybuf);
	return 0;
}

/** 
 * ISP - SPI Flash
 */

int gs_isp_write(struct gs_ar0234_dev *sensor, u32 addr, u8 size, u8 * buf)
{
	struct i2c_client *client = sensor->i2c_client;
	struct i2c_msg msg;
	u8 * mybuf;
	int ret;

	if (size == 0) return 0;
	if (size > 64) return -1; //max size  is 64

	mybuf = (u8 *) malloc(sizeof(u8) * (size+4));
	mybuf[0] = 0x40;
	mybuf[1] = (u8) (addr & 0xFF);
	mybuf[2] = (u8) ((addr >> 8) & 0xFF);
	mybuf[3] = (u8) ((addr >> 16) & 0xFF);

	for (int i=0; i<size; i++)
	{
		mybuf[i+4] = buf[i];
	}

	msg.addr = client->addr;
	msg.flags = client->flags;
	msg.buf = mybuf;
	msg.len = sizeof(mybuf);

	ret = gs_ar0234_i2c_trx_retry(client->adapter, &msg, 1);
	if (ret < 0) {
		dev_err(&client->dev, "%s: error: addr=%x, err=%d\n", __func__, addr, ret);
		free(mybuf);
		return ret;
	}
	free(mybuf);
	return 0;
}

int gs_isp_calc_crc(struct gs_ar0234_dev *sensor, u32 addr1, u32 addr2, u16 * crc)
{
	struct i2c_client *client = sensor->i2c_client;
	struct i2c_msg msg;
	u8 mybuf[7];
	int ret;
	
	mybuf[0] = 0x47;
	mybuf[1] = (u8) (addr1 & 0xFF);
	mybuf[2] = (u8) ((addr1 >> 8) & 0xFF);
	mybuf[3] = (u8) ((addr1 >> 16) & 0xFF);
	mybuf[4] = (u8) (addr2 & 0xFF);
	mybuf[5] = (u8) ((addr2 >> 8) & 0xFF);
	mybuf[6] = (u8) ((addr2 >> 16) & 0xFF);

	msg.addr = client->addr;
	msg.flags = client->flags;
	msg.buf = mybuf;
	msg.len = sizeof(mybuf);

	ret = gs_ar0234_i2c_trx_retry(client->adapter, msg, 1);
	if (ret < 0) {
		dev_err(&client->dev, "%s: error: err=%d\n", __func__, ret);
		return ret;
	}

	ret = gs_check_wait(sensor, 10, 100) //check in steps of 10ms, with timeout of 100ms
	if(ret < 0) {
		dev_err(&client->dev, "%s: error: crc timeout, err=%d\n", __func__, ret);
		return ret;
	}

	msg.addr = client->addr;
	msg.flags = client->flags | I2C_M_RD;
	msg.buf = mybuf;
	msg.len = 2;

	ret = gs_ar0234_i2c_trx_retry(client->adapter, msg, 1);
	if (ret < 0) {
		dev_err(&client->dev, "%s: error: err=%d\n", __func__, ret);
		return ret;
	}

	crc = ((u16) mybuf[0]) | (((u16) mybuf[1]) << 8);
	return 0;
}

int gs_isp_erase_page(struct gs_ar0234_dev *sensor, u32 addr)
{
	struct i2c_client *client = sensor->i2c_client;
	struct i2c_msg msg;
	u8 buf[4];
	int ret;

	buf[0] = 0x44;
	buf[1] = (u8) (addr & 0xFF);
	buf[2] = (u8) ((addr >> 8) & 0xFF);
	buf[3] = (u8) ((addr >> 16) & 0xFF);

	msg.addr = client->addr;
	msg.flags = client->flags;
	msg.buf = buf;
	msg.len = sizeof(buf);

	ret = gs_ar0234_i2c_trx_retry(client->adapter, &msg, 1);
	if (ret < 0) {
		dev_err(&client->dev, "%s: error: addr=%x, err=%d\n", __func__, addr, ret);
		return ret;
	}

	ret = gs_check_wait(sensor, 10, 1000) //check in steps of 10ms, with timeout of 1000ms
	if(ret < 0) {
		dev_err(&client->dev, "%s: error: crc timeout, err=%d\n", __func__, ret);
		return ret;
	}

	return 0;
}

int gs_isp_erase_all(struct gs_ar0234_dev *sensor)
{
	struct i2c_client *client = sensor->i2c_client;
	struct i2c_msg msg;
	u8 buf[2];
	int ret;

	buf[0] = 0x42;
	buf[1] = 0x01;

	msg.addr = client->addr;
	msg.flags = client->flags;
	msg.buf = buf;
	msg.len = sizeof(buf);

	ret = gs_ar0234_i2c_trx_retry(client->adapter, &msg, 1);
	if (ret < 0) {
		dev_err(&client->dev, "%s: error: addr=%x, err=%d\n", __func__, addr, ret);
		return ret;
	}

	ret = gs_ar0234_check_wait(sensor, 10, 6000) //check in steps of 10ms, with timeout of 6000ms
	if(ret < 0) {
		dev_err(&client->dev, "%s: error: crc timeout, err=%d\n", __func__, ret);
		return ret;
	}

	return 0;
}

//cmd = 0x9F (= JEDEC) or 0x90 (=ID)
int gs_get_spi_id(struct gs_ar0234_dev *sensor, u8 cmd, u8 * mf, u16 * id)
{
	struct i2c_client *client = sensor->i2c_client;
	struct i2c_msg msg[2];
	u8 mybuf[4];
	int ret;

	if((cmd != 0x9F) || (cmd != 0x90)) return -1;
	
	mybuf[0] = 0x43;
	mybuf[1] = cmd;
	mybuf[2] = 0;
	mybuf[3] = 0;

	msg[0].addr = client->addr;
	msg[0].flags = client->flags;
	msg[0].buf = mybuf;
	msg[0].len = 2;

	msg[1].addr = client->addr;
	msg[1].flags = client->flags | I2C_M_RD;
	msg[1].buf = mybuf;
	msg[1].len = 4;

	ret = gs_ar0234_i2c_trx_retry(client->adapter, msg, 2);
	if (ret < 0) {
		dev_err(&client->dev, "%s: error: err=%d\n", __func__, ret);
		return ret;
	}

	if (cmd == 0x9F)
	{
		mf = mybuf[0];
		id = ((u16) mybuf[1] << 8) | mybuf[2];
	}
	else 
	{
		mf = mybuf[2];
		id = ((u16) mybuf[3]) & 0xFF;	
	}
	return 0;
}

int gs_get_spistatus(struct gs_ar0234_dev *sensor, u16 * status)
{
	struct i2c_client *client = sensor->i2c_client;
	struct i2c_msg msg[2];
	u8 mybuf[0];
	int ret;

	mybuf[0] = 0x45;
	mybuf[1] = 0;

	msg[0].addr = client->addr;
	msg[0].flags = client->flags;
	msg[0].buf = mybuf;
	msg[0].len = 1;

	msg[1].addr = client->addr;
	msg[1].flags = client->flags | I2C_M_RD;
	msg[1].buf = mybuf;
	msg[1].len = 2;

	ret = gs_ar0234_i2c_trx_retry(client->adapter, msg, 2);
	if (ret < 0) {
		dev_err(&client->dev, "%s: error: err=%d\n", __func__, ret);
		return ret;
	}

	status = ((u16)mybuf[1] << 8) | mybuf[0];
	return 0;
}

/**
 * MainApp - bootloader
 */
 
 int gs_flashread(struct gs_ar0234_dev *sensor, u16 addr, u8 size, u8 * buf)
 {
	struct i2c_client *client = sensor->i2c_client;
	struct i2c_msg msg[2];
	u8 mybuf[4];
	int ret;
	
	if ((size != 16) && (size != 32) || (size != 64)) return -1;

	mybuf[0] = 0x39;
	mybuf[1] = (u8) (addr & 0xFF);
	mybuf[2] = (u8) ((addr >> 8) & 0xFF);
	mybuf[3] = size;

	msg[0].addr = client->addr;
	msg[0].flags = client->flags;
	msg[0].buf = mybuf;
	msg[0].len = sizeof(mybuf);

	msg[1].addr = client->addr;
	msg[1].flags = client->flags | I2C_M_RD;
	msg[1].buf = buf;
	msg[1].len = size;
	ret = gs_ar0234_i2c_trx_retry(client->adapter, msg, 2);
	if (ret < 0) {
		dev_err(&client->dev, "%s: error: addr=%x, err=%d\n", __func__, addr, ret);
		return ret;
	}
	return 0;
 }

int gs_flashwrite(struct gs_ar0234_dev *sensor, u16 addr, u8 size, u8 * buf)
 {
	struct i2c_client *client = sensor->i2c_client;
	struct i2c_msg msg;
	u8 * mybuf;
	int ret;

	if ((size != 16) && (size != 32) || (size != 64)) return -1;

	mybuf = (u8 *) malloc(sizeof(u8) * (size+4));
	mybuf[0] = 0x38;
	mybuf[1] = (u8) (addr & 0xFF);
	mybuf[2] = (u8) ((addr >> 8) & 0xFF);
	mybuf[3] = size;

	for (int i=0; i<size; i++)
	{
		mybuf[i+4] = buf[i];
	}

	msg.addr = client->addr;
	msg.flags = client->flags;
	msg.buf = mybuf;
	msg.len = sizeof(mybuf);

	ret = gs_ar0234_i2c_trx_retry(client->adapter, &msg, 1);
	if (ret < 0) {
		dev_err(&client->dev, "%s: error: addr=%x, err=%d\n", __func__, addr, ret);
		free(mybuf);
		return ret;
	}
	free(mybuf);
	return 0;

 }

int gs_page_erase(struct gs_ar0234_dev *sensor, u16 addr, u16 size)
{
	struct i2c_client *client = sensor->i2c_client;
	struct i2c_msg msg;
	u8 buf[5];
	int ret;

	if(size > FLASH_PAGE_SIZE) return -1; //bootloader bug!

	buf[0] = 0x44;
	buf[1] = (u8) (addr & 0xFF);
	buf[2] = (u8) ((addr >> 8) & 0xFF);
	buf[3] = (u8) (size & 0xFF);
	buf[3] = (u8) ((size >> 8) & 0xFF);

	msg.addr = client->addr;
	msg.flags = client->flags;
	msg.buf = buf;
	msg.len = sizeof(buf);

	ret = gs_ar0234_i2c_trx_retry(client->adapter, &msg, 1);
	if (ret < 0) {
		dev_err(&client->dev, "%s: error: addr=%x, err=%d\n", __func__, addr, ret);
		return ret;
	}

	return 0;
}

int gs_app_calc_crc(struct gs_ar0234_dev *sensor, u16 * pcrc)
{
	struct i2c_client *client = sensor->i2c_client;
	struct i2c_msg msg;
	u8 mybuf[0];
	int ret;
	
	mybuf[0] = 0x41;
	mybuf[1] = 0x00;

	msg.addr = client->addr;
	msg.flags = client->flags;
	msg.buf = mybuf;
	msg.len = sizeof(mybuf);

	ret = gs_ar0234_i2c_trx_retry(client->adapter, msg, 1);
	if (ret < 0) {
		dev_err(&client->dev, "%s: error: err=%d\n", __func__, ret);
		return ret;
	}

	ret = gs_check_wait(sensor, 10, 100) //check in steps of 10ms, with timeout of 100ms
	if(ret < 0) {
		dev_err(&client->dev, "%s: error: crc timeout, err=%d\n", __func__, ret);
		return ret;
	}

	msg.addr = client->addr;
	msg.flags = client->flags | I2C_M_RD;
	msg.buf = mybuf;
	msg.len = 2;

	ret = gs_ar0234_i2c_trx_retry(client->adapter, msg, 1);
	if (ret < 0) {
		dev_err(&client->dev, "%s: error: err=%d\n", __func__, ret);
		return ret;
	}

	*pcrc = ((u16) mybuf[0]) | (((u16) mybuf[1]) << 8);
	return 0;
}

int gs_app_read_crc(struct gs_ar0234_dev *sensor, u16 * pcrc)
{
	return gs_flashread(sensor,FLASH_APP_CRC_ADDRESS, 2, &0pcrc);
}

int gs_app_read_size(struct gs_ar0234_dev *sensor, u16 * psize)
{
	return gs_flashread(sensor,FLASH_APP_SIZE_ADDRESS, 2, psize);
}

int gs_app_write_size(struct gs_ar0234_dev *sensor, u16 size)
{
	u8 buf[2];

	buf[0] = (u8) (size & 0xFF);
	buf[1] = (u8) ((size >> 8) & 0xFF);
	return gs_flashwrite(sensor, FLASH_APP_SIZE_ADDRESS, 2, buf);
}

int gs_app_write_crc(struct gs_ar0234_dev *sensor, u16 crc)
{
	u8 buf[2];

	buf[0] = (u8) (crc & 0xFF);
	buf[1] = (u8) ((crc >> 8) & 0xFF);
	return gs_flashwrite(sensor, FLASH_APP_CRC_ADDRESS, 2, buf);
}

int gs_app_erase_crc(struct gs_ar0234_dev *sensor)
{
	gs_erase_page(sensor,FLASH_APP_CRC_ADDRESS,2);
}

int gs_app_erase_size(struct gs_ar0234_dev *sensor)
{
	gs_erase_page(sensor,FLASH_APP_SIZE_ADDRESS,2);
}

int gs_erase_app(struct gs_ar0234_dev *sensor)
{
	for (int n = FLASH_APP_START_ADDRESS, n < FLASH_APP_MAX; n += FLASH_PAGE_SIZE)
	{
        gs_erase_page(sensor, n, FLASH_PAGE_SIZE);
	}
}

int gs_erase_nvm(struct gs_ar0234_dev *sensor)
{
	for (int n = FLASH_NVM_START_ADDRESS, n < FLASH_NVM_MAX; n += FLASH_PAGE_SIZE)
	{
        gs_erase_page(sensor, n, FLASH_PAGE_SIZE);
	}
}

int gs_erase_all(struct gs_ar0234_dev *sensor)

{
	struct i2c_client *client = sensor->i2c_client;
	struct i2c_msg msg;
	u8 buf[2];
	int ret;

	if(size > FLASH_PAGE_SIZE) return -1; //bootloader bug!

	buf[0] = 0x44;
	buf[1] = 0x01;

	msg.addr = client->addr;
	msg.flags = client->flags;
	msg.buf = buf;
	msg.len = sizeof(buf);

	ret = gs_ar0234_i2c_trx_retry(client->adapter, &msg, 1);
	if (ret < 0) {
		dev_err(&client->dev, "%s: error: addr=%x, err=%d\n", __func__, addr, ret);
		return ret;
	}

	return 0;
}

int gs_reboot(struct gs_ar0234_dev *sensor)
{
	struct i2c_client *client = sensor->i2c_client;
	struct i2c_msg msg;
	u8 buf[2];
	int ret;

	if(size > FLASH_PAGE_SIZE) return -1; //bootloader bug!

	buf[0] = 0x46;
	buf[1] = 0x01;

	msg.addr = client->addr;
	msg.flags = client->flags;
	msg.buf = buf;
	msg.len = sizeof(buf);

	ret = gs_ar0234_i2c_trx_retry(client->adapter, &msg, 1);
	if (ret < 0) {
		dev_err(&client->dev, "%s: error: addr=%x, err=%d\n", __func__, addr, ret);
		return ret;
	}

	return 0;
}

int gs_boot_id(struct gs_ar0234_dev *sensor, u16 * id)
{
	struct i2c_client *client = sensor->i2c_client;
	struct i2c_msg msg[2];
	u8 mybuf[0];
	int ret;

	mybuf[0] = 0x47;
	mybuf[1] = 0x00;

	msg[0].addr = client->addr;
	msg[0].flags = client->flags;
	msg[0].buf = mybuf;
	msg[0].len = 1;

	msg[1].addr = client->addr;
	msg[1].flags = client->flags | I2C_M_RD;
	msg[1].buf = mybuf;
	msg[1].len = 2;

	ret = gs_ar0234_i2c_trx_retry(client->adapter, msg, 2);
	if (ret < 0) {
		dev_err(&client->dev, "%s: error: err=%d\n", __func__, ret);
		return ret;
	}

	*id = ((u16)mybuf[1] << 8) | mybuf[0];
	return 0;
}

#endif
