/*
 * Copyright (C) 2024 Videology Inc, Inc. All Rights Reserved.
 */
#include <linux/i2c.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/device.h>

#include "cam_ar0234.h"
#include "gs_ap1302.h"
#include "gs_image_update.h"


/**
 * @brief  get address, data and data_count from array of chars ending with "\n"
 * @param line 
 * @param paddress 
 * @param pdata 
 * @return * size of pdata
 */
int getdata(char * line, u32 * paddress, u8 * pdata)
{
    int icount = 0;
    char * endptr = strchr(line, (int)' '); // find 1st space ptr.
    *paddress = strtoul(line, &endptr, 16);
    for(int i = 0; i < MAX_LINE; i++) // get data
    {
        if (line[i] == '\n') {
            return (icount-1);
        }
        if (line[i] == ' ')
        {
            endptr = &line[i+2];
            pdata[icount]  = (unsigned char)strtoul(&line[i+1], &endptr, 16);
            icount++;
        }
    }
    return (icount-1);
}


/**
 * @brief print address and one line of bytes 
 * 
 * @param address 
 * @param pdata 
 * @param size 
 * @return int 
 */
int print_data(unsigned int address, u8 * pdata, int size)
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


/**
 * @brief reads data from buffer and write data to flash line by line
 * 
 * @param sensor 
 * @param buffer 
 * @param size 
 * @param bwriteapp 
 * @param bwritenvm 
 * @return int 
 */
int write_buffer(struct gs_ar0234_dev *sensor, char * buffer, int size, bool bwriteapp, bool bwritenvm) 
{
    int numvalues;
    u32 address, fixaddress;
    u8 values[64];
    char * nextptr;
    int ret;
    struct i2c_client *client = sensor->i2c_client;

    //print_hex_dump(KERN_ALERT, "", DUMP_PREFIX_OFFSET, 16, 1, buffer, 100, 1); 

    nextptr = buffer;
    while ((nextptr - buffer) < size) 
    {
        if (strstarts(nextptr, "//")) {;}
        else if (strstarts(nextptr, "[TOTALSIZE")) {;}
        else if (strstarts(nextptr, "[BLOCKSIZE")) {;}
        else
        {
            numvalues = getdata(nextptr, &address, values);
            if((bwriteapp == true) && (bwritenvm == true))
            {
                // write all
                if((address >= FLASH_APP_START) && (address <= FLASH_NVM_MAX))
                {
                    ret = gs_flashwrite(sensor, address, (u8)numvalues, values);
                }
            }
            else if ((bwriteapp == true) && (bwritenvm == false)) 
            {
                // write mainapp
                if((address >= FLASH_APP_START) && (address <= FLASH_APP_MAX))
                {
                    ret = gs_flashwrite(sensor, address, (u8)numvalues, values);
                }
            }
            else if((bwriteapp == false) && (bwritenvm == true)) 
            {
                // write nvm
                if ((address >= FLASH_NVM_START) && (address <= FLASH_NVM_MAX))
                {
                    ret = gs_flashwrite(sensor, address, (u8)numvalues, values);
                }
                else if (address < FLASH_NVM_SIZE) 
                {
                    //swap nvm page address
                    fixaddress = address & 0xFCFF;
                    if ((address & 0x300) == 0x300)         fixaddress |= 0x000; //swap
                    else if ((address & 0x300) == 0x200)    fixaddress |= 0x100;
                    else if ((address & 0x300) == 0x100)    fixaddress |= 0x200;
                    else if ((address & 0x300) == 0x000)    fixaddress |= 0x300;
                    ret = gs_flashwrite(sensor, FLASH_NVM_START + fixaddress, (u8)numvalues, values);
                }
            }
            if (ret < 0) {
                dev_err(&client->dev, "%s: error: flashwrite err=%d\n", __func__, ret);
                return ret;
            }
        }
        nextptr = strchr(nextptr+1, (int)'\n') + 1; // find next  pointer to line
    }
    return 0;
}



/**
 * @brief update the mainapp and/or the nvm
 * 
 * @param sensor 
 * @param buffer 
 * @param size 
 * @param type 
 * @return int 
 */
int flashapp(struct gs_ar0234_dev *sensor, char * buffer, int size)
{
    int ret;
    bool bwriteapp, bwritenvm;
    u16 readcrc, calccrc;
    struct i2c_client *client = sensor->i2c_client;

    // check id i2C bus is free
    pr_debug("-->%s: check %d\n", __func__, sensor->csi_id);
    ret = gs_check_wait(sensor, 50, 3000); //check i2c for max 3 seconds
    if (ret < 0) {
		dev_err(&client->dev, "%s: error: timeout err=%d\n", __func__, ret);
		return ret;
	}

    //print_hex_dump(KERN_ALERT, "", DUMP_PREFIX_OFFSET, 16, 1, buffer, 100, 1);

    if(sensor->update_type != BOOT)
    {
        // set pass
        //pr_debug("-->%s: ppp\n", __func__);
        ret = gs_set_password(sensor, PPP);
        if (ret < 0) {
            dev_err(&client->dev, "%s: error: password err=%d\n", __func__, ret);
            return ret;
        }

        // start bootloader
        pr_debug("---%s: bootloader %d\n", __func__, sensor->csi_id);
        ret = gs_start_bootloader(sensor);
        if (ret < 0) {
            dev_err(&client->dev, "%s: error: bootloader err=%d\n", __func__, ret);
            return ret;
        }
        msleep(100); // wait for bootloader
        ret = gs_check_wait(sensor, 100, 3000); //check bootloader for max 3 seconds
        if (ret < 0) {
            dev_err(&client->dev, "%s: error: check_wait err=%d\n", __func__, ret);
            return ret;
        }
    }

    // erase flash
    pr_debug("---%s: erase %d\n", __func__, sensor->csi_id);
    switch(sensor->update_type)
    {
        case MCUNVM:
        case BOOT:
            bwriteapp = true;
            bwritenvm = true;
            ret = gs_erase_all(sensor);
            break;
        case MCU:
            bwriteapp = true;
            bwritenvm = false;
            ret = gs_erase_app(sensor);
            break;
        case NVM:
            bwriteapp = false;
            bwritenvm = true;
            ret = gs_erase_nvm(sensor);
            break;
        default:
            ret = gs_reboot(sensor); // wrong type, do reboot
            return -1;
            break;
    }
    if (ret < 0) {
		dev_err(&client->dev, "%s: error: erase err=%d\n", __func__, ret);
		return ret;
	}

    // wait for erase
    pr_debug("---%s: check %d\n", __func__, sensor->csi_id);
    ret = gs_check_wait(sensor, 10, 3000); //check bootloader for max 3 seconds
    if (ret < 0) {
        dev_err(&client->dev, "%s: error: check_wait err=%d\n", __func__, ret);
        return ret;
    }

    // write flash
    pr_debug("---%s: write %d\n", __func__, sensor->csi_id);
    ret = write_buffer(sensor, buffer, size, bwriteapp, bwritenvm);
    if (ret < 0) {
        dev_err(&client->dev, "%s: error: write_buffer err=%d\n", __func__, ret);
        // what to do when write fails?
        // erase app+nvm, app, nvm -> then reboot into bootloader? 
    }

    // check crc's when programming app
    if(bwriteapp)
    {
        // get the CRC's
        pr_debug("---%s: crc %d\n", __func__, sensor->csi_id);
        ret = gs_app_read_crc(sensor, &readcrc);
        if(ret) {
            dev_err(&client->dev, "%s: error: read_crc err=%d\n", __func__, ret);
            // what to do when crc fails?
            // erase app+nvm, app, nvm -> then reboot into bootloader? 
        }
        pr_debug("---%s: calc crc %d\n", __func__, sensor->csi_id);
        ret = gs_app_calc_crc(sensor, &calccrc);
        if(ret) {
            dev_err(&client->dev, "%s: error: calc_crc err=%d\n", __func__, ret);
            // what to do when crc fails?
            // erase app+nvm, app, nvm -> then reboot into bootloader? 
        }

        // check the CRC's
        if(readcrc != calccrc) {
            dev_err(&client->dev, "%s: error: crc check failed err=%d\n", __func__, ret);
            return -1;
        }
    }

    pr_debug("---%s: reboot %d\n", __func__, sensor->csi_id);
    ret = gs_reboot(sensor);
    if(ret) {
        dev_err(&client->dev, "%s: error: reboot err=%d\n", __func__, ret);
    }

    // check id i2C bus is free
    pr_debug("---%s: check %d\n", __func__, sensor->csi_id);
    ret = gs_check_wait(sensor, 50, 5000); //check reboot for max 5 seconds
    if (ret < 0) {
		dev_err(&client->dev, "%s: error: timeout err=%d\n", __func__, ret);
		return ret;
	}

    pr_debug("<--%s: done %d\n", __func__, sensor->csi_id);

    // done
    return 0;
}


/**
 * @brief reads data from buffer and write data to flash line by line
 * 
 * @param sensor 
 * @param buffer 
 * @param size 
 * @param binsize 
 * @param crc 
 * @return int 
 */
int write_isp_buffer(struct gs_ar0234_dev *sensor, char * buffer, int size, u32 * binsize, u16 * crc ) 
{
    int numvalues;
    u32 address;
    u8 values[64];
    char * nextptr;
    char * findptr;

    int ret;
    struct i2c_client *client = sensor->i2c_client;

    nextptr = buffer;
    while ((nextptr - buffer) < size) 
    {
        if (strstarts(nextptr, "// CRC")) {
            findptr = strchr(nextptr, (int)'x'); // find 1st 'x' ptr.
            *crc = strtoul(findptr+1, NULL, 16);
        }
        else if (strstarts(nextptr, "// Size")) {
            findptr = strchr(nextptr, (int)'x'); // find 1st 'x' ptr.
            *binsize = strtoul(findptr+1, NULL, 16);
        }
        else if (strstarts(nextptr, "//")) {;}
        else if (strstarts(nextptr, "[TOTALSIZE")) {;}
        else if (strstarts(nextptr, "[BLOCKSIZE")) {;}
        else
        {
            numvalues = getdata(nextptr, &address, values);
            ret = gs_isp_write(sensor, address, (u8) numvalues, values);
            if (ret < 0) {
                dev_err(&client->dev, "%s: error: flashwrite err=%d\n", __func__, ret);
                return ret;
            }
        }
        nextptr = strchr(nextptr+1, (int)'\n') + 1; // find next  pointer to line
    }
    return 0;
}



/**
 * @brief update the isp
 * 
 * @param sensor 
 * @param buffer 
 * @param size 
 * @param type 
 * @return int 
 */
int flashisp(struct gs_ar0234_dev *sensor, char * buffer, int size)
{
    int ret;
    u16 status;
    u16 readcrc, calccrc;
    u32 binsize;
    struct i2c_client *client = sensor->i2c_client;

    // check id i2C bus is free
    pr_debug("-->%s: check %d\n", __func__, sensor->csi_id);
    ret = gs_check_wait(sensor, 50, 3000); //check i2c for max 3 seconds
    if (ret < 0) {
		dev_err(&client->dev, "%s: error: timeout err=%d\n", __func__, ret);
		return ret;
	}

    // set pass
    ret = gs_set_password(sensor, PPP);
    if (ret < 0) {
        dev_err(&client->dev, "%s: error: password err=%d\n", __func__, ret);
        return ret;
    }

    // set camera in upgrader mode
    pr_debug("---%s: upgrader mode %d\n", __func__, sensor->csi_id);
    ret = gs_upgrader_mode(sensor);
    if (ret < 0) {
		dev_err(&client->dev, "%s: error: upgrader-mode err=%d\n", __func__, ret);
		return ret;
	}

    //ret = gs_get_spi_id(struct gs_ar0234_dev *sensor, 0x9F, u8 * mf, u16 * id); // cmd:  (0x9F = JEDEC) or (0x90 =ID)

    // Check the SPI status
    ret = gs_get_spistatus(sensor, &status);
    if (ret < 0) {
		dev_err(&client->dev, "%s: error: flash status %04X err=%d\n", __func__, status, ret);
		return ret;
	}

    // erase flash
    pr_debug("---%s: erase %d\n", __func__, sensor->csi_id);
    ret = gs_isp_erase_all(sensor);
    if (ret < 0) {
		dev_err(&client->dev, "%s: error: erase err=%d\n", __func__, ret);
		return ret;
	}

    // write flash
    pr_debug("---%s: write %d\n", __func__, sensor->csi_id);
    ret = write_isp_buffer(sensor, buffer, size, &binsize, &readcrc);
    if (ret < 0) {
        dev_err(&client->dev, "%s: error: write_isp_buffer err=%d\n", __func__, ret);
        // TODO: what to do when write fails?
        // erase app+nvm, app, nvm -> then reboot into bootloader? 
    }

    // get the crc from camera
    pr_debug("---%s: get crc %d\n", __func__, sensor->csi_id);
    ret = gs_isp_calc_crc(sensor, 0, binsize-1, &calccrc);
    if(ret < 0) {
        dev_err(&client->dev, "%s: error: calc crc err=%d\n", __func__, ret);
        return -1;
    }   
    // check the CRC's
    pr_debug("---%s: CRC check %d\n", __func__, sensor->csi_id);
    if(readcrc != calccrc) {
        dev_err(&client->dev, "%s: error: crc check failed %x != %x err=%d\n", __func__, readcrc, calccrc, ret);
        return -1;
    }

    // reboot camera
    pr_debug("---%s: restart %d\n", __func__, sensor->csi_id);
    ret = gs_restart(sensor);
    if(ret) {
        dev_err(&client->dev, "%s: error: restart err=%d\n", __func__, ret);
    }

    // check if i2C bus is free
    pr_debug("---%s: check %d\n", __func__, sensor->csi_id);
    ret = gs_check_wait(sensor, 50, 5000); //check restart for max 5 seconds
    if (ret < 0) {
		dev_err(&client->dev, "%s: error: timeout err=%d\n", __func__, ret);
		return ret;
	}

    pr_debug("<--%s: done %d\n", __func__, sensor->csi_id);

    // done
    return 0;
}




