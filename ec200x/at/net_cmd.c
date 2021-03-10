#include "at_ec200x.h"

#include <stdlib.h>
#include <string.h>

#include "printf.h"

#include "cmds.h"

#include "ec200x_uart.h"
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static int cmd_qiact_ret_val = 0;

/* '\r\n+QIACT: 1,1,1,"10.36.118.23"\r\n' */
static int cmd_qiact_ret(char *data, uint32_t len)
{
    // print_hex_array("qiact", (uint8_t *)data, len);

    char *p = data + 10;

    ec200x_net_info.context_ip.contextID = atoi(p);

    p = skip_chr(',', p);
    ec200x_net_info.context_ip.contextState = atoi(p);

    p = skip_chr(',', p);
    ec200x_net_info.context_ip.contextType = atoi(p);

    p = skip_chr(',', p);
    p = skip_chr('\"', p);

    char *pp = p;
    p = skip_chr('\"', p);

    memcpy(ec200x_net_info.context_ip.ip, pp, (p - pp - 2));

    ec200x_net_info.context_ip.ip[p - pp - 2] = 0;

    printk("ip:\'%s\'\r\n", ec200x_net_info.context_ip.ip);

    /* 0.0.0.0 */
    if(7 >= (p - pp - 2)) {
        printk("ip error len\r\n");
        return CMD_ERR_LEN_ERR;
    }

    cmd_qiact_ret_val = 0;

    return CMD_ERR_NONE;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

struct ec200x_net_info_t ec200x_net_info;

struct at_urc_t net_cmds[] = {
    {"\r\n+QIACT", "\r\n", cmd_qiact_ret},
};

const uint32_t net_cmd_tabsz = sizeof(net_cmds) / sizeof(struct at_urc_t);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct ec200x_context_ip_t * ec200x_net_get_ip(void)
{
#if 1
    struct ec200x_exec_cmd_wait_flag_t cmd;

    cmd.cmd = "AT+QIACT?\r\n";
    cmd.cmd_len = 11;
    cmd.cmd_ack = "\r\nOK\r\n";
    cmd.cmd_tmo_ms = 1000 * 150;

    cmd_qiact_ret_val = 0x1000;

    cmd.wait_val = &cmd_qiact_ret_val;
    cmd.wait_val_invaild = cmd_qiact_ret_val;
    cmd.wait_val_vaild = 0x00;
    cmd.wait_retry_cnt = 2;

    if(0x00 != at_ec200x_exec_cmd_wait_flag(&cmd)) {
        return NULL;
    }

#else
    int ret = at_ec200x_send_cmd_wait_ack("AT+QIACT?\r\n", 11, "\r\nOK\r\n", 1000 * 150);

    if(0x00 != ret) {
        printk("exec AT+QIACT? fail %d\r\n", ret);

        return NULL;
    }
#endif

    return &ec200x_net_info.context_ip;
}

int ec200x_net_enable(void)
{
    int ret = at_ec200x_send_cmd_wait_ack("AT+QIACT=1\r\n", 12, "\r\nOK\r\n", 1000 * 150);

    if(0x00 != ret) {
        printk("exec AT+QIACT=1 fail %d\r\n", ret);

        return ret;
    }

    return CMD_ERR_NONE;
}
