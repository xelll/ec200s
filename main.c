#include <stdio.h>
#include <stdint.h>

#include "fpioa.h"
#include "printf.h"
#include "sleep.h"
#include "sysctl.h"
#include "gpiohs.h"

#include "cmds.h"
#include "at_ec200x.h"
#include "ec200x_uart.h"
#include "ec200x_mqtt.h"

static int mqtt_cb(struct ec200x_mqtt_msg_t *msg)
{
#if 1
    printk("topic:   %s\r\n", msg->topic);
    printk("msg_id:  %d\r\n", msg->msgid);
    printk("msg_len: %d\r\n", msg->payload_len);
    printk("msg:    \'%s\'\r\n", msg->paylod);
#endif

    return 0;
}

static void init_ec200x_mqtt_cfg(void)
{
    memset((uint8_t *)&ec200x_mqtt, 0, sizeof(struct ec200x_mqtt_t));

    ec200x_mqtt.client_idx = 0;

    struct ec200x_mqtt_cfg_t *cfg = &ec200x_mqtt.cfg;

    cfg->keep_alive_time = 0;

    cfg->dataformat.flag = 1;
    cfg->dataformat.recv_mode = MQTT_RECV_SEND_MODE_HEX;
    cfg->dataformat.send_mode = MQTT_RECV_SEND_MODE_HEX;

    cfg->recv_mode.flag = 1;
    cfg->recv_mode.msg_recv_mode = 0;
    cfg->recv_mode.msg_len_enable = 1;

    cfg->qmtopen.port = 1883;
    strncpy(cfg->qmtopen.hostname, "203.195.245.13", sizeof(cfg->qmtopen.hostname));

    strncpy(cfg->qmtconn.client_id, "ec200x", sizeof(cfg->qmtconn.client_id));
    memset(cfg->qmtconn.username, 0, sizeof(cfg->qmtconn.username));
    memset(cfg->qmtconn.password, 0, sizeof(cfg->qmtconn.password));

    cfg->qmtsub.qos = 0;
    cfg->qmtsub.msgid = 1;
    strncpy(cfg->qmtsub.topic, "test", sizeof(cfg->qmtsub.topic));
}

static void init_ec200x_cmd_tab(void)
{
    at_ec200x_reg_urc_table(basic_cmds, basic_cmd_tabsz);
    at_ec200x_reg_urc_table(gps_cmds, gps_cmd_tabsz);
    at_ec200x_reg_urc_table(net_cmds, net_cmd_tabsz);
    at_ec200x_reg_urc_table(mqtt_cmds, mqtt_cmd_tabsz);
}

static void init_ec200x_dev_cfg(void)
{
    memset((uint8_t *)&ec200x_uart_hw_info, 0, sizeof(struct ec200x_uart_hw_info_t));

    ec200x_uart_hw_info.uart_dev = UART_DEVICE_1;
    ec200x_uart_hw_info.uart_baud = 115200;

    ec200x_uart_hw_info.io_tx = 24;
    ec200x_uart_hw_info.io_rx = 25;

    ec200x_uart_hw_info.io_pwr = 23;
    ec200x_uart_hw_info.gpiohs_pwr = 0;

    ec200x_uart_hw_info.io_sta = 22;
    ec200x_uart_hw_info.gpiohs_sta = 1;
}

int main(int argc, char **argv)
{
    uart_config(UART_DEVICE_3, 1500000, UART_BITWIDTH_8BIT, UART_STOP_1, UART_PARITY_NONE);

    printk("start\r\n");

    plic_init();
    sysctl_enable_irq();

    init_ec200x_cmd_tab();

    init_ec200x_dev_cfg();
    ec200x_device_init(&ec200x_uart_hw_info);

    if(0 > at_ec200x_wait_connect(5 * 1000)) {
        printk("wait module fail\r\n");
    }

    char *iccid = ec200x_get_ccid();
    if(NULL != iccid) {
        printk("iccid: %s\r\n", iccid);
    }

    char *imei = ec200x_get_imei();
    if(NULL != imei) {
        printk("imei: %s\r\n", imei);
    }

    ec200x_net_enable();
    ec200x_net_get_ip();

    /* start configure mqtt. */
    init_ec200x_mqtt_cfg();
    ec200x_mqtt_reg_cb(mqtt_cb);

    ec200x_mqtt_cfg(&ec200x_mqtt);
    ec200x_mqtt_open(&ec200x_mqtt);
    ec200x_mqtt_conn(&ec200x_mqtt);
    ec200x_mqtt_sub(&ec200x_mqtt);

    // uint8_t flag = 0;
    while (1) {
        msleep(600);

        // if(flag) {
        //     flag = 0;
            // int csq = ex200x_get_csq();
        //     ec200x_net_get_ip();
            // printk("get csq: %d\r\n", csq);
        // } else {
        //     flag = 1;
        //     struct ec200x_gps_info_t *info = ec200x_gps_get_loc();
        //     if(NULL != info) {
        //         ec200x_gps_print(info);
        //     } else {
        //         printk("get gps info fail\r\n");
        //     }
        // }
        at_ec200x_parse_msg();
    }

    return 0;
}
