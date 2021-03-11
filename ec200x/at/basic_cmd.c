#include "at_ec200x.h"

#include <stdlib.h>
#include <string.h>

#include "printf.h"

#include "cmds.h"

#include "ec200x_uart.h"

/* '\r\n+CME ERROR: 10\r\n' */
static int cmd_cme_err(char *data, uint32_t len)
{
    char *p = data + 14;

    // print_hex_array("cme", (uint8_t *)data, len);

    int cme = atoi(p);

    printk("got cme %d\r\n", cme);

    return -(CMD_ERR_CME_0 + cme);
}

/* '\r\nERROR\r\n' */
static int cmd_error_ret(char *data, uint32_t len)
{
    if(9 != len) { return CMD_ERR_LEN_ERR; }

    return -CMD_ERR_ERROR;
}

/* '\r\n+CSQ: 31,99\r\n' */
static int cmd_csq_ret(char *data, uint32_t len)
{
    char *p = data + 8;

    ec200x_basic_info.csq = atoi(p);

    // printk("got csq: %d\r\n", ec200x_basic_info.csq);

    return CMD_ERR_NONE;
}

/* '\r\n+QCCID: 89860117851125305885\r\n' */
static int cmd_qccid_ret(char *data, uint32_t len)
{
    // print_hex_array("qccid", (uint8_t *)data, len);
    char *p = data + 10;

    if(32 != len) { return CMD_ERR_LEN_ERR; }

    memcpy(ec200x_basic_info.iccid, p, len - 12);
    ec200x_basic_info.iccid[len - 12] = 0;

    return CMD_ERR_NONE;
}

/* '\r\nRDY\r\n' */
static int cmd_rdy_ret(char *data, uint32_t len)
{
    if(7 != len) {
        return -CMD_ERR_LEN_ERR;
    }

    printk("ec200x maybe reboot\r\n");

    return CMD_ERR_NONE;
}

static int cmd_at_ate_ret(char *data, uint32_t len)
{
    return CMD_ERR_NONE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

struct ec200x_basic_info_t ec200x_basic_info;

struct at_urc_t basic_cmds[] = {
    {"\r\n+CME ERROR", "\r\n", cmd_cme_err},
    {"\r\nERROR", "\r\n", cmd_error_ret},

    {"\r\nRDY", "\r\n", cmd_rdy_ret},
    {"AT\r", "\r\n", cmd_at_ate_ret},
    {"ATE0\r", "\r\n", cmd_at_ate_ret},

    {"\r\n+CSQ", "\r\n", cmd_csq_ret},
    {"\r\n+QCCID", "\r\n", cmd_qccid_ret},
};

const uint32_t basic_cmd_tabsz = sizeof(basic_cmds) / sizeof(struct at_urc_t);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
uint8_t query_imei_flag = 0;

int cmd_imei_ret(char *data, uint32_t len)
{
    char *p = data + 2;

    if(len != 19) {
        return CMD_ERR_LEN_ERR;
    }

    memcpy(ec200x_basic_info.imei, p, 15);
    ec200x_basic_info.imei[15] = 0;

    return CMD_ERR_NONE;
}

char * ec200x_get_imei(void)
{
    query_imei_flag = 1;

    int ret = at_ec200x_send_cmd_wait_ack("AT+GSN\r\n", 8, "\r\nOK\r\n", 300);

    query_imei_flag = 0;

    if(0x00 != ret) {
        printk("exec AT+GSN fail %d\r\n", ret);

        return NULL;
    }

    return &ec200x_basic_info.imei[0];
}
///////////////////////////////////////////////////////////////////////////////

int ex200x_get_csq(void)
{
    int ret = at_ec200x_send_cmd_wait_ack("AT+CSQ\r\n", 8, "\r\nOK\r\n", 300);

    if(0x00 != ret) {
        printk("exec AT+CSQ fail %d\r\n", ret);

        return ret;
    }

    return ec200x_basic_info.csq;
}

char * ec200x_get_ccid(void)
{
    int ret = at_ec200x_send_cmd_wait_ack("AT+QCCID\r\n", 10, "\r\nOK\r\n", 300);

    if(0x00 != ret) {
        printk("exec AT+QCCID fail %d\r\n", ret);

        return NULL;
    }

    return &ec200x_basic_info.iccid[0];
}
