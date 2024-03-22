/*
 * Copyright (C) 2024 Videology Inc, Inc. All Rights Reserved.
 */
#include <linux/i2c.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/delay.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <stdbool.h>
#include "cam_ar0234.h"
#include "gs_ap1302.h"
#include "gs_image_update.h"




#if 0
/**
 * @brief  strstarts - does @str start with @prefix?
 * @param str: string to examine
 * @param prefix: prefix to look for.
 * @return bool
 */
bool strstarts(const char *str, const char *prefix)
{
     return strncmp(str, prefix, strlen(prefix)) == 0;
}
#endif



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
                if((address > FLASH_APP_START) && (address <= FLASH_NVM_MAX))
                {
                    ret = gs_flashwrite(sensor, address, (u8)numvalues, values);
                }
            }
            else if ((bwriteapp == true) && (bwritenvm == false)) 
            {
                // write mainapp
                if((address > FLASH_APP_START) && (address <= FLASH_APP_MAX))
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
int flashapp(struct gs_ar0234_dev *sensor, char * buffer, int size, int type)
{
    int ret;
    bool bwriteapp, bwritenvm;
    u16 readcrc, calccrc;
    //u16 appsize;
    struct i2c_client *client = sensor->i2c_client;


    // check id i2C bus is free
    printk("DEBUG: check");
    ret = gs_check_wait(sensor, 50, 3000);
    if (ret < 0) {
		dev_err(&client->dev, "%s: error: timeout err=%d\n", __func__, ret);
		return ret;
	}


    // set pass
    printk("DEBUG: password");
    ret = gs_set_password(sensor, PPP);
    if (ret < 0) {
		dev_err(&client->dev, "%s: error: password err=%d\n", __func__, ret);
		return ret;
	}

    // start bootloader
    printk("DEBUG: bootloader");
    ret = gs_start_bootloader(sensor);
    if (ret < 0) {
		dev_err(&client->dev, "%s: error: bootloader err=%d\n", __func__, ret);
		return ret;
	}
    msleep(500); // wait for bootloader
    ret = gs_check_wait(sensor, 10, 3000); //check bootloader for max 3 seconds
    if (ret < 0) {
		dev_err(&client->dev, "%s: error: check_wait err=%d\n", __func__, ret);
		return ret;
	}

    // erase flash
    printk("DEBUG: erase %d %d",bwriteapp,bwritenvm);
    switch(type)
    {
        case MCUNVM:
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

    // write flash
    printk("DEBUG: write");
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
        printk("DEBUG: CRC");
        ret = gs_app_read_crc(sensor, &readcrc);
        if(ret) {
            dev_err(&client->dev, "%s: error: read_crc err=%d\n", __func__, ret);
            // what to do when crc fails?
            // erase app+nvm, app, nvm -> then reboot into bootloader? 
        }
        ret = gs_app_calc_crc(sensor, &calccrc);
        if(ret) {
            dev_err(&client->dev, "%s: error: calc_crc err=%d\n", __func__, ret);
            // what to do when crc fails?
            // erase app+nvm, app, nvm -> then reboot into bootloader? 
        }
        /*
        ret = gs_app_read_size(sensor, &appsize);
        if(ret) {
            dev_err(&client->dev, "%s: error: calc_crc err=%d\n", __func__, ret);
            // what to do when crc fails?
            // erase app+nvm, app, nvm -> then reboot into bootloader? 
        }
        */

        // check the CRC's
        if(readcrc != calccrc) {
            dev_err(&client->dev, "%s: error: crc check failed err=%d\n", __func__, ret);
            return -1;
        }
        printk("DEBUG: Done");
    }

    printk("DEBUG: reboot");
    ret = gs_reboot(sensor);
    if(ret) {
        dev_err(&client->dev, "%s: error: reboot err=%d\n", __func__, ret);

    }

    // check id i2C bus is free
    printk("DEBUG: check");
    ret = gs_check_wait(sensor, 50, 3000);
    if (ret < 0) {
		dev_err(&client->dev, "%s: error: timeout err=%d\n", __func__, ret);
		return ret;
	}

    // done
    return 0;
}






/**
 * @brief write_lines brom buffer and convert to data bytes
 * 
 * @param buffer 
 * @return int 
 */
int write_lines(char * buffer, int size) 
{
    int numvalues;
    u32 address;
    u8 values[64];
    char * nextptr;

    nextptr = buffer;
    while ((nextptr - buffer) < size) 
    {
        if (strstarts(nextptr, "//")) {;}
        else if (strstarts(nextptr, "[TOTALSIZE")) {;}
        else if (strstarts(nextptr, "[BLOCKSIZE")) {;}
        else
        {
            numvalues = getdata(nextptr, &address, values);
            
            print_data(address, values, numvalues);
        }
        nextptr = strchr(nextptr+1, (int)'\n') + 1; // find next  pointer to line
    }
    return 0;
}




