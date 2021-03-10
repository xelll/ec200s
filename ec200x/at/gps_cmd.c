#include "at_ec200x.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "printf.h"

#include "ec200x_uart.h"

#include "cmds.h"

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static uint8_t two_chr_to_dec(char a, char b)
{
    uint8_t aa, bb;

    aa = chr_to_u8(a);
    bb = chr_to_u8(b);

    // return (uint8_t)(((aa << 4) & 0xF0) | (bb & 0x0F));
    return (uint8_t)(aa * 10 + bb);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/* '\r\n+QGPSLOC: 073539.00,22.56502,113.87605,1.9,73.8,0,1.36,0.0,0.0,050321,13\r\n' */
static int cmd_qgps_loc_ret(char *data, uint32_t len)
{
    if(74 > len) {
        print_hex_array("loc len err", (uint8_t *)data, len);
        return CMD_ERR_LEN_ERR;
    }

    char *p = data + 12;

    char utc[10], date[7];

    memcpy(utc, p, 6);
    utc[6] = 0;

    p += 9;
    p = skip_chr(',',p);
    ec200x_gps_info.latitude = atof(p);

    p = skip_chr(',',p);
    ec200x_gps_info.longitude = atof(p);

    p = skip_chr(',',p);
    ec200x_gps_info.hdop = atof(p);

    p = skip_chr(',',p);
    ec200x_gps_info.altitude = atof(p);

    p = skip_chr(',',p);
    ec200x_gps_info.fix = atoi(p);

    p = skip_chr(',',p);
    ec200x_gps_info.cog = atof(p);

    p = skip_chr(',',p);
    ec200x_gps_info.spkm = atof(p);

    p = skip_chr(',',p);
    ec200x_gps_info.spkn = atof(p);

    p = skip_chr(',',p);
    memcpy(date, p, 6);
    date[6] = 0;

    p += 6;
    p = skip_chr(',',p);
    ec200x_gps_info.nsta = atoi(p);

    if(NULL != strchr(date, '-')) {
        printk("illgal date\r\n");

        ec200x_gps_info.flag = 0;
        ec200x_gps_info.time.u64 = 0;
    } else {
        ec200x_gps_info.flag = 1;

        ec200x_gps_info.time.hour = two_chr_to_dec(utc[0], utc[1]);
        ec200x_gps_info.time.min = two_chr_to_dec(utc[2], utc[3]);
        ec200x_gps_info.time.sec = two_chr_to_dec(utc[4], utc[5]);

        ec200x_gps_info.time.day = two_chr_to_dec(date[0], date[1]);
        ec200x_gps_info.time.mon = two_chr_to_dec(date[2], date[3]);
        ec200x_gps_info.time.year = (uint16_t)(2000 + two_chr_to_dec(date[4], date[5]));
    }

    return CMD_ERR_NONE;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct ec200x_gps_info_t ec200x_gps_info;

struct at_urc_t gps_cmds[] = {
    {"\r\n+QGPSLOC", "\r\n", cmd_qgps_loc_ret},
};

const uint32_t gps_cmd_tabsz = sizeof(gps_cmds) / sizeof(struct at_urc_t);
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int ec200x_gps_en(void)
{
    int ret = at_ec200x_send_cmd_wait_ack("AT+QGPS=1\r\n", 11, "\r\nOK\r\n", 300);

    if(0x00 != ret) {
        printk("exec AT+QGPS=1 fail %d\r\n", ret);
        return ret;
    }

    return CMD_ERR_NONE;
}

struct ec200x_gps_info_t * ec200x_gps_get_loc(void)
{
    int ret = at_ec200x_send_cmd_wait_ack("AT+QGPSLOC=2\r\n", 14, "\r\nOK\r\n", 300);

    if(0x00 != ret) {
        printk("exec AT+QGPSLOC=2 fail %d\r\n", ret);
        return NULL;
    }

    return &ec200x_gps_info;
}

void ec200x_gps_print(struct ec200x_gps_info_t *info)
{
    printf("latitude:  %f\r\n", info->latitude);
    printf("longitude: %f\r\n", info->longitude);
    printf("hdop:      %f\r\n", info->hdop);
    printf("altitude:  %f\r\n", info->altitude);
    printf("cog:       %f\r\n", info->cog);
    printf("spkm:      %f\r\n", info->spkm);
    printf("spkn:      %f\r\n", info->spkn);
    printk("fix:       %d\r\n", info->fix);
    printk("nsta:      %d\r\n", info->nsta);
    printk("time:      %04d-%02d-%02d %02d:%02d:%02d\r\n", \
        info->time.year, info->time.mon, info->time.day, \
        info->time.hour, info->time.min, info->time.sec    
        );
}
