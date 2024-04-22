/*
 * Copyright (C) 2023 Videology Inc, Inc. All Rights Reserved.
 */

#include <linux/i2c.h>
#include <linux/types.h>


#include "cam_ar0234.h"
#include "gs_ap1302.h"



/**
 * generic
 */

void mymsleep(int wait)
{
	msleep(wait);
}


int gs_ar0234_i2c_trx_retry(struct i2c_adapter *adap, struct i2c_msg *msgs, int num)
{
	int i, ret;
	for (i = 0; i < I2C_RETRIES; i++) {
		ret = i2c_transfer(adap, msgs, num);
		if (ret > 0)
			break;
	}
	return ret;
}

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

	ret = i2c_transfer(client->adapter, msg, 1);
	if (ret < 0) {
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
		ret = gs_check(sensor);
		if (ret == 0) return 0;
		mymsleep(wait);
		to += wait;
		if(to > timeout) 
			return -1;
	}
	return 0;
}



/*
 * register - mainapp
 */

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

/**
 * functions - mainapp
 */

int gs_ar0234_power(struct gs_ar0234_dev *sensor, int on)
{
	int ret;
	if(on) 
	{
		ret = gs_ar0234_write_reg8(sensor, GS_REG_POWER, 0); //power up
		if(ret) return ret;
		// wait for camera to boot up and accept i2c commands
		mymsleep(100);

		ret = gs_check_wait(sensor, 100, 5000);
	}
	else 
	{
		ret = gs_ar0234_write_reg8(sensor, GS_REG_POWER, 1); // power down
	}
	return ret;
}

int gs_ar0234_version(struct gs_ar0234_dev *sensor, int type, u16 * version)
{
	int ret;
	u8 major = 0;
	u8 minor = 0;
	u8 ver[2];

	switch(type) 
	{
		case MCU:
		case MCUNVM:
			major = GS_REG_MCU_MAJOR_VERSION;
			minor = GS_REG_MCU_MINOR_VERSION;
			break;
		case NVM:
			major = GS_REG_NVM_MAJOR_VERSION;
			minor = GS_REG_NVM_MINOR_VERSION;
			break;
		case ISP:
			major = GS_REG_ISP_MAJOR_VERSION;
			minor = GS_REG_ISP_MINOR_VERSION;
			break;
		default: 
			*version = 0;
			return 0;
			break;
	}
	ret = gs_ar0234_read_reg8(sensor, major, &ver[1]);
	if(ret) return ret;
	ret = gs_ar0234_read_reg8(sensor, minor, &ver[0]);
	if(ret) return ret;
	*version = (((u16)ver[1])<<8) | ver[0];
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
	int ret = gs_ar0234_write_reg8(sensor, 0xFC, password & 0xFF);
	if (ret) return ret;
	ret = gs_ar0234_write_reg8(sensor, 0xFD, (password>>8) & 0xFF);
	if (ret) return ret;
	return 0;
}

int gs_restart(struct gs_ar0234_dev *sensor)
{
	int ret = gs_ar0234_write_reg8(sensor, 0xF0, 0x99);
	if (ret) return ret;
	return 0;
}

int gs_start_bootloader(struct gs_ar0234_dev *sensor)
{
	int ret = gs_ar0234_write_reg8(sensor, 0xF0, 0xA5);
	if (ret) return ret;
	return 0;
}

int gs_upgrader_mode(struct gs_ar0234_dev *sensor)
{
	return gs_ar0234_write_reg8(sensor, 0xEB, 0x82);
}

int gs_get_camera_type(struct gs_ar0234_dev *sensor, u8 * type)
{
	int ret = 0;
	ret = gs_ar0234_read_reg8(sensor, GS_REG_CAMERA_TYPE, type);
	return ret;
}


//#define MYDEBUG
#ifdef MYDEBUG 
int pd(unsigned int address, u8 * pdata, int size)
{
    char str[255];
    char * endptr;

    sprintf(str,"%08X ",address);
    endptr = strchr(str, (int)'\0');
    for (int i=0; i < size; i++)
    {
        sprintf(endptr,"%02X ", pdata[i]); // data
        endptr = strchr(endptr, (int)'\0');
    }
    sprintf(endptr,"\n");
    printk("%s",str);
    return 0;
}
#endif


/**
 *  NVM - mainapp
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
		dev_err(&client->dev, "%s: error: addr=%x err=%d\n", __func__, addr, ret);
		return ret;
	}
	return 0;
}

int gs_ar0234_write_nvm(struct gs_ar0234_dev *sensor, u8 page, u8 addr, u8 size, u8 * buf)
{
	struct i2c_client *client = sensor->i2c_client;
	struct i2c_msg msg;
	int ret;
	u8 writebuf[68];

	if (size == 0) return 0;

	writebuf[0] = 0x50;
	writebuf[1] = page;
	writebuf[2] = addr;

	for (int i=0; i<size; i++)
	{
		writebuf[i+3] = buf[i];
	}

	msg.addr = client->addr;
	msg.flags = client->flags;
	msg.buf = writebuf;
	msg.len = size+3;

	ret = gs_ar0234_i2c_trx_retry(client->adapter, &msg, 1);
	if (ret < 0) {
		dev_err(&client->dev, "%s: error: addr=%x, err=%d\n", __func__, addr, ret);
	}
	return 0;
}

/** 
 * ISP - SPI Flash - mainapp
 */

int gs_isp_write(struct gs_ar0234_dev *sensor, u32 addr, u8 size, u8 * buf)
{
	struct i2c_client *client = sensor->i2c_client;
	struct i2c_msg msg;
	int ret;
	u16 status;
    int timeout;
	u8 writebuf[68];

	if (size == 0) return 0;
	if (size > 64) return -1; //max size  is 64

#ifdef MYDEBUG
	pd(addr, buf, size);
#endif

	writebuf[0] = 0x40;
	writebuf[1] = (u8) (addr & 0xFF);
	writebuf[2] = (u8) ((addr >> 8) & 0xFF);
	writebuf[3] = (u8) ((addr >> 16) & 0xFF);

	for (int i=0; i<size; i++)
	{
		writebuf[i+4] = buf[i];
	}

	msg.addr = client->addr;
	msg.flags = client->flags;
	msg.buf = writebuf;
	msg.len = size+4;

	ret = gs_ar0234_i2c_trx_retry(client->adapter, &msg, 1);
	if (ret < 0) {
		dev_err(&client->dev, "%s: error: addr=%x, err=%d\n", __func__, addr, ret);
	}

	timeout = 0;
	status = 0xFFFF;
	while (status != 0x0000) {
		ret = gs_get_spistatus(sensor, &status);
		if (ret < 0) {
			dev_err(&client->dev, "%s: error: flash status %04X err=%d\n", __func__, status, ret);
			return ret;
		}
		timeout ++;
		if (timeout >= 1000) {
			dev_err(&client->dev, "%s: error: flash write timeout %04X err=%d\n", __func__, status, ret);
			return -1;
		}
	}
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

	ret = gs_ar0234_i2c_trx_retry(client->adapter, &msg, 1);
	if (ret < 0) {
		dev_err(&client->dev, "%s: error: err=%d\n", __func__, ret);
		return ret;
	}

	ret = gs_check_wait(sensor, 10, 1000); //check in steps of 10ms, with timeout of 100ms
	if(ret < 0) {
		dev_err(&client->dev, "%s: error: crc timeout, err=%d\n", __func__, ret);
		return ret;
	}

	msg.addr = client->addr;
	msg.flags = client->flags | I2C_M_RD;
	msg.buf = mybuf;
	msg.len = 2;

	ret = gs_ar0234_i2c_trx_retry(client->adapter, &msg, 1);
	if (ret < 0) {
		dev_err(&client->dev, "%s: error: err=%d\n", __func__, ret);
		return ret;
	}

	*crc = ((u16) mybuf[0]) | (((u16) mybuf[1]) << 8);
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

	ret = gs_check_wait(sensor, 10, 1000); //check in steps of 10ms, with timeout of 1000ms
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
	u16 status;
    int timeout;	

	buf[0] = 0x42;
	buf[1] = 0x01;

	msg.addr = client->addr;
	msg.flags = client->flags;
	msg.buf = buf;
	msg.len = sizeof(buf);

	ret = gs_ar0234_i2c_trx_retry(client->adapter, &msg, 1);
	if (ret < 0) {
		dev_err(&client->dev, "%s: error: err=%d\n", __func__, ret);
		return ret;
	}
	mymsleep(10);

	timeout = 0;
	status = 0xFFFF;
	while (status != 0x0000) {
		ret = gs_get_spistatus(sensor, &status);
		if (ret < 0) {
			dev_err(&client->dev, "%s: error: flash status %04X err=%d\n", __func__, status, ret);
			return ret;
		}
		timeout ++;
		mymsleep(10);
		if (timeout >= 1000) {
			dev_err(&client->dev, "%s: error: flash write timeout %04X err=%d\n", __func__, status, ret);
			return -1;
		}
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
		*mf = mybuf[0];
		*id = ((u16) mybuf[1] << 8) | mybuf[2];
	}
	else 
	{
		*mf = mybuf[2];
		*id = ((u16) mybuf[3]) & 0xFF;	
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
	*status = ((u16)mybuf[1] << 8) | mybuf[0];
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
	
	if (size > 64) return -1; 

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
	int ret;
	u8 writebuf[68];

#ifdef MYDEBUG
	pd(addr, buf, size);
#endif

	if (size > 64) return -1;

	writebuf[0] = 0x38;
	writebuf[1] = (u8) (addr & 0xFF);
	writebuf[2] = (u8) ((addr >> 8) & 0xFF);

	for (int i=0; i<size; i++)
	{
		writebuf[i+3] = buf[i];
	}

	msg.addr = client->addr;
	msg.flags = client->flags;
	msg.buf = writebuf;
	msg.len = size+3;

	ret = gs_ar0234_i2c_trx_retry(client->adapter, &msg, 1);
	if (ret < 0) {
		dev_err(&client->dev, "%s: error: addr=%x, err=%d\n", __func__, addr, ret);
	}
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
	buf[4] = (u8) ((size >> 8) & 0xFF);

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
	struct i2c_msg msg[2];
	u8 mybuf[2];
	int ret;
	
	mybuf[0] = 0x41;
	mybuf[1] = 0x00;

	msg[0].addr = client->addr;
	msg[0].flags = client->flags;
	msg[0].buf = mybuf;
	msg[0].len = sizeof(mybuf);

	ret = gs_ar0234_i2c_trx_retry(client->adapter, &msg[0], 1);
	if (ret < 0) {
		dev_err(&client->dev, "%s: error: err=%d\n", __func__, ret);
		return ret;
	}

	ret = gs_check_wait(sensor, 10, 100); //check in steps of 10ms, with timeout of 100ms
	if(ret < 0) {
		dev_err(&client->dev, "%s: error: crc timeout, err=%d\n", __func__, ret);
		return ret;
	}

	msg[1].addr = client->addr;
	msg[1].flags = client->flags | I2C_M_RD;
	msg[1].buf = mybuf;
	msg[1].len = 2;

	ret = gs_ar0234_i2c_trx_retry(client->adapter, &msg[1], 1);
	if (ret < 0) {
		dev_err(&client->dev, "%s: error: err=%d\n", __func__, ret);
		return ret;
	}

	*pcrc = ((u16) mybuf[0]) | (((u16) mybuf[1]) << 8);
	return 0;
}

int gs_app_read_crc(struct gs_ar0234_dev *sensor, u16 * pcrc)
{
	int ret;
	u8 dat[2];

	ret = gs_flashread(sensor,FLASH_APP_CRC_ADDRESS, 2, dat);
	*pcrc = (((u16)dat[1])<<8) | dat[0];
#ifdef MYDEBUG
	printk("DEBUG: crc = %04x",*pcrc);
#endif

	return ret;
}

int gs_app_read_size(struct gs_ar0234_dev *sensor, u16 * psize)
{
	int ret;
	u8 dat[2];

	ret = gs_flashread(sensor,FLASH_APP_SIZE_ADDRESS, 2, dat);
	*psize = (((u16)dat[1])<<8) | dat[0];

	return ret;	
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
	gs_page_erase(sensor,FLASH_APP_CRC_ADDRESS,2);
	return 0;
}

int gs_app_erase_size(struct gs_ar0234_dev *sensor)
{
	gs_page_erase(sensor,FLASH_APP_SIZE_ADDRESS,2);
	return 0;
}

int gs_erase_app(struct gs_ar0234_dev *sensor)
{
	for (int n = FLASH_APP_START_ADDRESS; n < FLASH_APP_MAX; n += FLASH_PAGE_SIZE)
	{
        gs_page_erase(sensor, n, FLASH_PAGE_SIZE);
	}
	return 0;
}

int gs_erase_nvm(struct gs_ar0234_dev *sensor)
{
	for (int n = FLASH_NVM_START_ADDRESS; n < FLASH_NVM_MAX; n += FLASH_PAGE_SIZE)
	{
        gs_page_erase(sensor, n, FLASH_PAGE_SIZE);
	}
	return 0;
}

int gs_erase_all(struct gs_ar0234_dev *sensor)

{
	struct i2c_client *client = sensor->i2c_client;
	struct i2c_msg msg;
	u8 buf[2];
	int ret;

	buf[0] = 0x44;
	buf[1] = 0x01;

	msg.addr = client->addr;
	msg.flags = client->flags;
	msg.buf = buf;
	msg.len = sizeof(buf);

	ret = gs_ar0234_i2c_trx_retry(client->adapter, &msg, 1);
	if (ret < 0) {
		dev_err(&client->dev, "%s: error: err=%d\n", __func__, ret);
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

	buf[0] = 0x46;
	buf[1] = 0x01;

	msg.addr = client->addr;
	msg.flags = client->flags;
	msg.buf = buf;
	msg.len = sizeof(buf);

	ret = gs_ar0234_i2c_trx_retry(client->adapter, &msg, 1);
	if (ret < 0) {
		dev_err(&client->dev, "%s: error: err=%d\n", __func__, ret);
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

int gs_boot_cameratype(struct gs_ar0234_dev *sensor, u8 * type)
{
	struct i2c_client *client = sensor->i2c_client;
	struct i2c_msg msg[2];
	u8 mybuf[0];
	int ret;

	mybuf[0] = 0xF1;
	mybuf[1] = 0x00;

	msg[0].addr = client->addr;
	msg[0].flags = client->flags;
	msg[0].buf = mybuf;
	msg[0].len = 1;

	msg[1].addr = client->addr;
	msg[1].flags = client->flags | I2C_M_RD;
	msg[1].buf = mybuf;
	msg[1].len = 1;

	ret = gs_ar0234_i2c_trx_retry(client->adapter, msg, 2);
	if (ret < 0) {
		dev_err(&client->dev, "%s: error: err=%d\n", __func__, ret);
		return ret;
	}

	*type = mybuf[0];
	return 0;
}


#define POLY 0x1021
/**
 * @brief 16-bit CRC
 * 
 * @param CRC_acc 
 * @param CRC_input 
 * @return unsigned short 
 */
u16 UpdateCRC (u16 CRC_acc, u8 CRC_input)
{
	unsigned char i; // loop counter
	// Create the CRC "dividend" for polynomial arithmetic (binary arithmetic
	// with no carries)
	CRC_acc = CRC_acc ^ (CRC_input << 8);
	// "Divide" the poly into the dividend using CRC XOR subtraction
	// CRC_acc holds the "remainder" of each divide
	//
	// Only complete this division for 8 bits since input is 1 byte
	for (i = 0; i < 8; i++)
	{
		// Check if the MSB is set (if MSB is 1, then the POLY can "divide"
		// into the "dividend")
		if ((CRC_acc & 0x8000) == 0x8000)
		{
			// if so, shift the CRC value, and XOR "subtract" the poly
			CRC_acc = CRC_acc << 1;
			CRC_acc ^= POLY;
		}
		else
		{
			// if not, just shift the CRC value
			CRC_acc = CRC_acc << 1;
		}
	}
	// Return the final remainder (CRC value)
	return CRC_acc;
}

/**
 * @brief calculate crc over buffer of bytes
 * 
 * @param seed 
 * @param count 
 * @param value 
 * @return u16 
 */
u16 gs_crc(u16 seed, u8 count, u8 * value)
{
	int n;
	u16 newcrc = seed;
	for(n=0; n < count; n++)
	{
		newcrc = UpdateCRC (newcrc, value[n]);
	}
	return newcrc;
}
