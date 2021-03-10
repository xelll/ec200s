#include "ec200x_mqtt.h"

#include <stdio.h>
#include <stdlib.h>

#include "sleep.h"
#include "printf.h"

#include "ec200x_uart.h"

#include "str2hex.h"
#include "str_hex.h"
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static volatile int qmtopen_ret = 0;
static volatile int qmtconn_ret = 0;
static volatile int qmt_sub_ret = 0;
static volatile int qmt_pubex_ret = 0;

static ec200x_mqtt_recv_cb mqtt_cb = NULL;

/* "\r\n+QMTOPEN: 0,0\r\n" */
static int cmd_qmtopen_ret(char *data, uint32_t len)
{
    char *p = data + 12;
    
    int tmp = 0;

    print_hex_array("qmtopen", (uint8_t *)data, len);

    tmp = atoi(p);

    if(tmp != ec200x_mqtt.client_idx) {
        qmtopen_ret = -2;
        printk("error client idx %d\r\n", tmp);
        return -1;
    }

    p = skip_chr(',', p);
    tmp = atoi(p);

    ec200x_mqtt.status.qmtopen.result = tmp;
    qmtopen_ret = tmp;

    return 0;
}

/* "\r\n+QMTCONN: 0,0,0\r\n " */
static int cmd_qmtconn_ret(char *data, uint32_t len)
{
    print_hex_array("qmtconn", (uint8_t *)data, len);

    int tmp = 0;
    char *p = data + 12;

    tmp = atoi(p);
    if(tmp != ec200x_mqtt.client_idx) {
        qmtconn_ret = -2;
        printk("error client idx %d\r\n", tmp);
        return -1;
    }

    p = skip_chr(',', p);
    tmp = atoi(p);

    qmtconn_ret = tmp;
    ec200x_mqtt.status.qmtconn.result = tmp;

    p = skip_chr(',', p);
    tmp = atoi(p);
    ec200x_mqtt.status.qmtconn.ret_code = tmp;

    return 0;
}

/* "\r\n+QMTSUB: 0,1,0,0\r\n " */
static int cmd_qmtsub_ret(char *data, uint32_t len)
{
    print_hex_array("qmtsub", (uint8_t *)data, len);

    int tmp = 0;
    char *p = data + 11;

    tmp = atoi(p);
    if(tmp != ec200x_mqtt.client_idx) {
        qmt_sub_ret = -2;
        printk("error client idx %d\r\n", tmp);
        return -1;
    }

    p = skip_chr(',', p);
    tmp = atoi(p);
    ec200x_mqtt.status.qmt_sub.msgid = tmp;

    p = skip_chr(',', p);
    tmp = atoi(p);
    ec200x_mqtt.status.qmt_sub.result = tmp;
    qmt_sub_ret = tmp;

    if(tmp < 2) {
        p = skip_chr(',', p);
        tmp = atoi(p);
        ec200x_mqtt.status.qmt_sub.value = tmp;
    }

    return 0;
}

/* "\r\n+QMTSUB: 0,1,0,0\r\n " */
static int cmd_qmtpubex_ret(char *data, uint32_t len)
{
    // print_hex_array("qmtpubex", (uint8_t *)data, len);

    int tmp = 0;
    char *p = data + 11;

    tmp = atoi(p);
    if(tmp != ec200x_mqtt.client_idx) {
        qmt_pubex_ret = -2;
        printk("error client idx %d\r\n", tmp);
        return -1;
    }

    p = skip_chr(',', p);
    ec200x_mqtt.status.qmt_pubex.msgid = atoi(p);

    p = skip_chr(',', p);
    ec200x_mqtt.status.qmt_pubex.result = atoi(p);

    if(0x01 == ec200x_mqtt.status.qmt_pubex.result) {
        p = skip_chr(',', p);
        ec200x_mqtt.status.qmt_pubex.value = atoi(p);
    }

    qmt_pubex_ret = 0;

    return 0;
}

/* "\r\n+QMTRECV: 0,0,"test",10,"1234567890"\r\n" */
static int cmd_qmtrecv_ret(char *data, uint32_t len)
{
    // print_hex_array("qmtrecv", (uint8_t *)data, len);

    char *p = data + 12, *pp = NULL;
    int tmp = 0;

    struct ec200x_mqtt_msg_t msg;

    tmp = atoi(p);
    if(tmp != ec200x_mqtt.client_idx) {
        printk("error client idx %d\r\n", tmp);
        return -1;
    }

    p = skip_chr(',', p);
    msg.msgid = atoi(p);

    p = skip_chr(',', p);

    pp = p++;
    p = skip_chr('\"', p);
    if(32 <= (p - pp)) {
        printk("topic len err %ld\r\n", (p - pp));
        return -3;
    }
    memcpy(msg.topic, pp, (p - pp));
    msg.topic[(p - pp)] = 0;

    p = skip_chr(',', p);
    msg.payload_len = atoi(p);

    p = skip_chr(',', p);
    p = skip_chr('\"', p);
    pp = p;
    p = skip_chr('\"', p);
    p--;

    if(msg.payload_len != (p - pp)) {
        printk("get error payload len %ld %d \r\n", (p - pp), msg.payload_len);
        return -3;
    }

    msg.paylod = pp;
    msg.paylod[msg.payload_len] = 0;

    // printk("topic:   %s\r\n", msg.topic);
    // printk("msg_id:  %d\r\n", msg.msgid);
    // printk("msg_len: %d\r\n", msg.payload_len);
    // printk("msg:    \'%s\'\r\n", msg.paylod);

#if 0
    static int cnt = 0;
    char temp[32];
    size_t llen = snprintf(temp, sizeof(temp), "%d", cnt++);

    temp[llen] = 0xAA;

    ec200x_mqtt_pub(&ec200x_mqtt, "xel", temp, llen + 1);
#else
    if(mqtt_cb) { return mqtt_cb(&msg); }
#endif
    return -1;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct ec200x_mqtt_t ec200x_mqtt;

struct at_urc_t mqtt_cmds[] = {
    {"\r\n+QMTOPEN", "\r\n", cmd_qmtopen_ret},
    {"\r\n+QMTCONN", "\r\n", cmd_qmtconn_ret},
    {"\r\n+QMTSUB", "\r\n", cmd_qmtsub_ret},
    {"\r\n+QMTPUBEX", "\r\n", cmd_qmtpubex_ret},
    {"\r\n+QMTRECV", "\r\n", cmd_qmtrecv_ret},
};

const uint32_t mqtt_cmd_tabsz = sizeof(mqtt_cmds) / sizeof(struct at_urc_t);
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static char mqtt_temp_buf[1024 * 4];

/* 
1. keepalive
2. recv/mode
3. dataformat
*/
int ec200x_mqtt_cfg(struct ec200x_mqtt_t *mqtt)
{
    size_t len = 0;
    int ret = 0;

    struct ec200x_mqtt_cfg_t *cfg = &mqtt->cfg;

    /* 发送与接收模式需要一致 */
    if(cfg->dataformat.send_mode != cfg->dataformat.recv_mode) {
        printk("dataformat: send_mode should equal recv_mode\r\n");
        return -CMD_ERR_UNKNOWN;
    }

    if(cfg->dataformat.flag) {
        len = snprintf(mqtt_temp_buf, sizeof(mqtt_temp_buf), \
                "AT+QMTCFG=\"dataformat\",%d,%d,%d\r\n", mqtt->client_idx, cfg->dataformat.send_mode, cfg->dataformat.recv_mode);

        printk("\'%s\'\r\n", mqtt_temp_buf);

        ret = at_ec200x_send_cmd_wait_ack(mqtt_temp_buf, len, "\r\nOK\r\n", 500);
        if(0x00 != ret) {
            printk("cfg dataformat fail %d\r\n", ret);
            return ret;
        }

        /* send/mode */
        len = snprintf(mqtt_temp_buf, sizeof(mqtt_temp_buf), \
                "AT+QMTCFG=\"send/mode\",%d,%d\r\n", mqtt->client_idx, cfg->dataformat.send_mode);

        printk("\'%s\'\r\n", mqtt_temp_buf);

        ret = at_ec200x_send_cmd_wait_ack(mqtt_temp_buf, len, "\r\nOK\r\n", 500);
        if(0x00 != ret) {
            printk("cfg send/mode fail %d\r\n", ret);
            return ret;
        }
    }

    if(0x00 != cfg->keep_alive_time) {
        len = snprintf(mqtt_temp_buf, sizeof(mqtt_temp_buf), \
                "AT+QMTCFG=\"keepalive\",%d,%d\r\n", mqtt->client_idx, cfg->keep_alive_time);

        printk("\'%s\'\r\n", mqtt_temp_buf);

        ret = at_ec200x_send_cmd_wait_ack(mqtt_temp_buf, len, "\r\nOK\r\n", 500);
        if(0x00 != ret) {
            printk("cfg keepalive fail %d\r\n", ret);
            return ret;
        }
    }

    if(cfg->recv_mode.flag) {
        len = snprintf(mqtt_temp_buf, sizeof(mqtt_temp_buf), \
                "AT+QMTCFG=\"recv/mode\",%d,%d,%d\r\n", mqtt->client_idx, cfg->recv_mode.msg_recv_mode, cfg->recv_mode.msg_len_enable);

        printk("\'%s\'\r\n", mqtt_temp_buf);

        ret = at_ec200x_send_cmd_wait_ack(mqtt_temp_buf, len, "\r\nOK\r\n", 500);
        if(0x00 != ret) {
            printk("cfg recv/mode fail %d\r\n", ret);
            return ret;
        }
    }

    return CMD_ERR_NONE;
}

int ec200x_mqtt_open(struct ec200x_mqtt_t *mqtt)
{
    size_t len = 0;
    int ret = 0;

    struct ec200x_mqtt_cfg_t *cfg = &mqtt->cfg;

    len = snprintf(mqtt_temp_buf, sizeof(mqtt_temp_buf), \
        "AT+QMTOPEN=%d,\"%s\",%d\r\n", mqtt->client_idx, cfg->qmtopen.hostname, cfg->qmtopen.port);

    printk("\'%s\'\r\n", mqtt_temp_buf);

    qmtopen_ret = 0x1000;

    /* 等到了OK */
    ret = at_ec200x_send_cmd_wait_ack(mqtt_temp_buf, len, "\r\nOK\r\n", 1000 * 120);
    if(0x00 != ret) {
        printk("qmtopen fail %d\r\n", ret);
        return ret;
    }

    volatile uint32_t wait_cnt = 0;

    /* 再等待返回"QMTOPEN: 0,0" */
    while(1) {
        at_ec200x_parse_msg();

        if(0x1000 != qmtopen_ret) {
            if(0x00 != ec200x_mqtt.status.qmtopen.result) { return -1; }
            return CMD_ERR_NONE;
        }

        wait_cnt ++;
        if((120 * 1000) > wait_cnt) {
            msleep(1);
        } else { return -CMD_ERR_WAIT_TIMEOUT; }
    }

    return CMD_ERR_NONE;
}

int ec200x_mqtt_conn(struct ec200x_mqtt_t *mqtt)
{
    size_t len = 0;
    int ret = 0;

    struct ec200x_mqtt_cfg_t *cfg = &mqtt->cfg;

    if((0x00 == cfg->qmtconn.username[0]) || (0x00 == cfg->qmtconn.password[0])) {
        len = snprintf(mqtt_temp_buf, sizeof(mqtt_temp_buf), \
            "AT+QMTCONN=%d,\"%s\"\r\n", mqtt->client_idx, cfg->qmtconn.client_id);
    } else {
        len = snprintf(mqtt_temp_buf, sizeof(mqtt_temp_buf), \
            "AT+QMTCONN=%d,\"%s\",\"%s\",\"%s\"\r\n", mqtt->client_idx, \
                cfg->qmtconn.client_id, cfg->qmtconn.username, cfg->qmtconn.password);
    }

    printk("\'%s\'\r\n", mqtt_temp_buf);

    qmtconn_ret = 0x1000;

    /* 等到了OK */
    ret = at_ec200x_send_cmd_wait_ack(mqtt_temp_buf, len, "\r\nOK\r\n", 1000 * 15);
    if(0x00 != ret) {
        printk("qmtconn fail %d\r\n", ret);
        return ret;
    }

    volatile uint32_t wait_cnt = 0;

    /* 再等待QMTCONN */
    while(1) {
        at_ec200x_parse_msg();

        if(0x1000 != qmtconn_ret) {
            if(0x00 != ec200x_mqtt.status.qmtconn.result) { return -1; }
            return 0;
        }

        wait_cnt ++;
        if((15 * 1000) > wait_cnt) {
            msleep(1);
        } else { return -CMD_ERR_WAIT_TIMEOUT; }
    }

    return CMD_ERR_NONE;
}

int ec200x_mqtt_sub(struct ec200x_mqtt_t *mqtt)
{
    size_t len = 0;
    int ret = 0;

    struct ec200x_mqtt_cfg_t *cfg = &mqtt->cfg;

    len = snprintf(mqtt_temp_buf, sizeof(mqtt_temp_buf), \
            "AT+QMTSUB=%d,%d,\"%s\",%d\r\n", mqtt->client_idx, cfg->qmtsub.msgid, cfg->qmtsub.topic, cfg->qmtsub.qos);

    printk("\'%s\'\r\n", mqtt_temp_buf);

    qmt_sub_ret = 0x1000;

    /* 等到了OK */
    ret = at_ec200x_send_cmd_wait_ack(mqtt_temp_buf, len, "\r\nOK\r\n", 1000 * 20);
    if(0x00 != ret) {
        printk("qmtsub fail %d\r\n", ret);
        return ret;
    }

    volatile uint32_t wait_cnt = 0;

    /* 再等待QMTSUB */
    while(1) {
        at_ec200x_parse_msg();

        if(0x1000 != qmt_sub_ret) {
            if(0x00 != ec200x_mqtt.status.qmt_sub.result) { return -1; }
            return CMD_ERR_NONE;
        }

        wait_cnt ++;
        if((20 * 1000) > wait_cnt) {
            msleep(1);
        } else { return -CMD_ERR_WAIT_TIMEOUT; }
    }

    return CMD_ERR_NONE;
}

int ec200x_mqtt_pub(struct ec200x_mqtt_t *mqtt, char *topic, char *msg, uint32_t msg_len)
{
    int ret = 0;
    size_t len = 0;
    volatile uint32_t wait_cnt = 0;

    struct ec200x_mqtt_cfg_t *cfg = &mqtt->cfg;

    len = snprintf(mqtt_temp_buf, sizeof(mqtt_temp_buf), \
            "AT+QMTPUBEX=%d,%d,%d,%d,\"%s\",%d\r\n", mqtt->client_idx, 0, 0, 0, topic, msg_len);

    printk("\'%s\'", mqtt_temp_buf);

    ret = at_ec200x_send_cmd_wait_ack(mqtt_temp_buf, len, "\r\n> ", 1000 * 120);
    if(0x00 != ret) {
        printk("qmtpub fail %d\r\n", ret);
        return ret;
    }

    qmt_pubex_ret = 0x1000;

    /* 等到了OK */
    ret = at_ec200x_send_cmd_wait_ack(msg, msg_len, "\r\nOK\r\n", 1000 * 15);
    if(0x00 != ret) {
        printk("qmtpub msg fail %d\r\n", ret);
        return ret;
    }

    /* 再等待QMTPUBEX */
    wait_cnt = 0;
    while(1) {
        at_ec200x_parse_msg();

        if(0x1000 != qmt_pubex_ret) {
            if(0x00 != ec200x_mqtt.status.qmt_pubex.result) { return -1; }

            printk("%s->%d\r\n", __func__, __LINE__);

            return CMD_ERR_NONE;
        }

        wait_cnt ++;
        if((15 * 1000) > wait_cnt) {
            msleep(1);
        } else { return -CMD_ERR_WAIT_TIMEOUT; }
    }

    printk("%s->%d\r\n", __func__, __LINE__);

    return -CMD_ERR_UNKNOWN;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ec200x_mqtt_reg_cb(ec200x_mqtt_recv_cb cb)
{
    if(NULL == cb) { return; }
    mqtt_cb = cb;
}
