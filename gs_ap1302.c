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


