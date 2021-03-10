#include "at_ec200x.h"

#include <stdlib.h>
#include <string.h>

#include "printf.h"
#include "sleep.h"

#include "cmds.h"

#include "ec200x_uart.h"
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static int cmd_qiact_ret_val = 0;
static int cmd_cops_ret_val = 0;

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

/* '\r\n+COPS: 0,0,"CHN-UNICOM",7\r\n' */
static int cmd_cops_ret(char *data, uint32_t len)
{
    char *pp = NULL, *p = data + 9;

    p = skip_chr('\"', p);
    pp = p;
    p = skip_chr('\"', p);
    p --;

    if(0x00 == (p - pp)) { return -CMD_ERR_UNKNOWN; }

    cmd_cops_ret_val = 0;

    return CMD_ERR_NONE;
}

static int cmd_lbs_loc_ret(char *data, uint32_t len)
{
    data[len] = 0;

    printk("\'%s\'\r\n", data);

    return CMD_ERR_NONE;
}

static int cmd_lbs_loc_http_ret(char *data, uint32_t len)
{
    printk("http result: \'%s\'\r\n", data);

    return CMD_ERR_NONE;
}

static int cmd_lbs_loc_fail_ret(char *data, uint32_t len)
{
    printk("got fail: \'%s\'\r\n", data);

    return CMD_ERR_NONE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

struct ec200x_net_info_t ec200x_net_info;

struct at_urc_t net_cmds[] = {
    {"\r\n+QIACT", "\r\n", cmd_qiact_ret},
    {"\r\n+COPS", "\r\n", cmd_cops_ret},
    {"+GETREFLOC", "\r\n", cmd_lbs_loc_ret},
    {"\r\nhttps", "\r\n", cmd_lbs_loc_http_ret},
    {"https", "\r\n", cmd_lbs_loc_http_ret},
    {"\r\n\r\n", "", cmd_lbs_loc_fail_ret},
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
    cmd.cmd_tmo_ms = 150 * 1000;

    cmd_qiact_ret_val = 0x1000;

    cmd.wait_val = &cmd_qiact_ret_val;
    cmd.wait_val_invaild = cmd_qiact_ret_val;
    cmd.wait_val_vaild = 0x00;
    cmd.retry_cnt = 10;

    if(0x00 != at_ec200x_exec_cmd_wait_flag(&cmd)) {
        printk("wait net fail\r\n");

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
    int ret = -CMD_ERR_UNKNOWN;

    /* 查询是否连接到基站 */
    struct ec200x_exec_cmd_wait_flag_t cmd;

    cmd.cmd = "AT+COPS?\r\n";
    cmd.cmd_len = 10;
    cmd.cmd_ack = "\r\nOK\r\n";
    cmd.cmd_tmo_ms = 10 * 1000;

    cmd_cops_ret_val = 0x1000;

    cmd.wait_val = &cmd_cops_ret_val;
    cmd.wait_val_invaild = cmd_cops_ret_val;
    cmd.wait_val_vaild = 0x00;
    cmd.retry_cnt = 10;

    if(0x00 != (ret = at_ec200x_exec_cmd_wait_flag(&cmd))) {
        printk("wait net fail\r\n");

        return ret;
    }

    ret = at_ec200x_send_cmd_wait_ack("AT+QIACT=1\r\n", 12, "\r\nOK\r\n", 1000 * 150);

    if(0x00 != ret) {
        printk("exec AT+QIACT=1 fail %d\r\n", ret);

        return ret;
    }

    return CMD_ERR_NONE;
}

int at_ec200x_exec_cmd_wait_flag(struct ec200x_exec_cmd_wait_flag_t *cmd)
{
    int ret = -1;
    uint32_t retry = 0;

    while(1) {
        ret = at_ec200x_send_cmd_wait_ack(cmd->cmd, cmd->cmd_len, cmd->cmd_ack, cmd->cmd_tmo_ms);

        if(0x00 != ret) {
            printk("exec \'%s\' fail %d\r\n", cmd->cmd, ret);
            return ret;
        } else {
            if(*cmd->wait_val != cmd->wait_val_invaild) {
                if(*cmd->wait_val == cmd->wait_val_vaild) { return 0; }
            }
        }

        retry ++;
        if(retry > cmd->retry_cnt) {
            return -CMD_ERR_WAIT_TIMEOUT;
        }
        msleep(100);
    }

    return CMD_ERR_NONE;
}

int ec200x_get_lbs_loc(void)
{
    int ret = -CMD_ERR_UNKNOWN;


printk("%s->%d\r\n", __func__, __LINE__);

    ret = at_ec200x_send_cmd_wait_ack("AT+LBS=\"RFLOC\"\r\n", 16, "\r\nOK\r\n", 1000 * 150);

printk("%s->%d %d\r\n", __func__, __LINE__, ret);

    if(0x00 != ret) {
        printk("exec AT+LBS fail %d\r\n", ret);
    }

    return CMD_ERR_NONE;
}
