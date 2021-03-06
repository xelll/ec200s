#include "at_ec200x.h"

#include <stdlib.h>

#include "atomic.h"
#include "sleep.h"
#include "printf.h"

#include "cmds.h"
#include "ec200x_uart.h"

static struct at_urc_t *at_urc_table = NULL;
static uint32_t at_urc_table_size = 0;

struct at_urc_t *at_ec200x_get_urc_obj(char *data, uint32_t len)
{
    if((0x00 == at_urc_table_size) || (NULL == at_urc_table)) {
        return NULL;
    }

    size_t prefix_len, suffix_len;

    for(uint32_t i = 0; i < at_urc_table_size; i++) {
        prefix_len = strlen(at_urc_table[i].prefix);
        suffix_len = strlen(at_urc_table[i].suffix);

        if((prefix_len ? !strncmp(data, at_urc_table[i].prefix, prefix_len) : 1) && \
            (suffix_len ? !strncmp(data + len - suffix_len, at_urc_table[i].suffix, suffix_len) : 1))
        {
            return &at_urc_table[i];
        }
    }

    return NULL;
}

int at_ec200x_reg_urc_table(struct at_urc_t *tab, const uint32_t size)
{
    if((0x00 == size) || (NULL == tab)) {
        return -3;
    }

    if(0x00 == at_urc_table_size) {
        at_urc_table = (struct at_urc_t *)malloc(sizeof(struct at_urc_t) * size);
        if(NULL == at_urc_table) {
            printk("OOM %s\r\n", __func__);
            return -1;
        }
        at_urc_table_size = size;
        memcpy((uint8_t *)at_urc_table, (uint8_t *)tab, sizeof(struct at_urc_t) * size);
    } else {
        if(NULL == at_urc_table) {
            printk("unk err %s\r\n", __func__);
            return -2;
        }
        struct at_urc_t *new = (struct at_urc_t *)malloc(sizeof(struct at_urc_t) * (size + at_urc_table_size));
        if(NULL == new) {
            printk("OOM %s\r\n", __func__);
            return -1;
        }
        memcpy((uint8_t *)new, (uint8_t *)tab, sizeof(struct at_urc_t) * size);
        memcpy((uint8_t *)(new + size), (uint8_t *)at_urc_table, sizeof(struct at_urc_t) * at_urc_table_size);

        free(at_urc_table);
        at_urc_table = new;
        at_urc_table_size += size;
    }

    return 0;
}

int at_ec200x_set_cmd_echo_mode(uint8_t mode)
{
    const char *mode_str[2] = {"ATE0\r\n", "ATE1\r\n"};

    int ret = 0;

    ret = at_ec200x_send_cmd_wait_ack(mode_str[mode & 0x01], 6, "OK\r\n", 300);

    if(0x00 != ret) {
        printk("exec %s fail\r\n", mode_str[mode & 0x01]);
    }

    return ret;
}

int at_ec200x_wait_connect(uint32_t time_out_ms)
{
    volatile uint32_t wait_cnt = 0;

    /* ????????????RDY */
    if(0x00 == at_ec200x_send_cmd_wait_ack("", 0, "\r\nRDY\r\n", 1000 * 10)) {
        time_out_ms = 1000; /* ?????????RDY???????????????????????????2??????????????? */
    }

    while(1) {
        if(0x00 == at_ec200x_send_cmd_wait_ack("AT\r\n", 4, "OK\r\n", 500)) {
            break;
        }

        wait_cnt += 500;
        if(wait_cnt > time_out_ms) { return -CMD_ERR_WAIT_TIMEOUT; }
    }

    return at_ec200x_set_cmd_echo_mode(0);
}

int at_ec200x_parse_unk(char *data, uint32_t len)
{
    /* ?????????????????? */
    extern uint8_t query_imei_flag;
    extern int cmd_imei_ret(char *data, uint32_t len);

    if(query_imei_flag && (19 == len)) {
        query_imei_flag = cmd_imei_ret(data, len);
    } else {
        print_hex_array("UNK", (uint8_t *)data, len);
    }

    return 0;
}

int at_ec200x_parse_msg(void)
{
    int ret = -CMD_ERR_UNKNOWN;

    struct ec200x_uart_msg_t msg;

    while(1) {
        mb();
        if(ec200x_uart_line_num) {
            if(0x00 == ec200x_uart_pop_msg(&msg)) {
                char *buf = (char *)(msg.bg_buf ? msg.bg_buf : msg.sm_buf);

                struct at_urc_t * urc = at_ec200x_get_urc_obj(buf, msg.len);

                if(NULL != urc) {
                    ret = urc->func(buf, msg.len);
                } else {
                    // print_hex_array("UNK", (uint8_t *)buf, msg.len);
                    at_ec200x_parse_unk(buf, msg.len);
                }
                ec200x_uart_free_msg(&msg);
            }
        } else { break; }
    }

    return ret;
}

int at_ec200x_send_cmd_wait_ack(const char *cmd, size_t cmd_len, char *ack, uint32_t timeout_ms)
{
    if(NULL == ack) { return -CMD_ERR_INVAILD_ARG; }

    /* ?????????????????????????????????????????????????????? */
    at_ec200x_parse_msg();

    ec200x_uart_send_cmd(cmd, cmd_len);

    volatile size_t wait_cnt = 0;

    struct ec200x_uart_msg_t msg;

    int urc_ret = -CMD_ERR_WAIT_TIMEOUT;

    size_t ack_len = strlen(ack);

    timeout_ms += 50; /* ?????????????????????????????? */

    while(1) {
        msleep(5);
        wait_cnt += 5;
        if(wait_cnt > timeout_ms) {
            printk("TMO %s\r\n", __func__);

            return (urc_ret == CMD_ERR_NONE) ? (-CMD_ERR_WAIT_TIMEOUT) : urc_ret;
        }

        mb();
        if(ec200x_uart_line_num) {
            /* ???????????????1ms??????????????????2????????? */
            msleep(1);
            wait_cnt += 1;

            if(0x00 == ec200x_uart_pop_msg(&msg)) {
                /* ?????????????????????ack??????????????????urc?????? */
                char *buf = (char *)(msg.bg_buf ? msg.bg_buf : msg.sm_buf);

                if(strncmp(buf, ack, ack_len) == 0) {
                    ec200x_uart_free_msg(&msg);
                    return CMD_ERR_NONE;
                }else {
                    struct at_urc_t * urc = at_ec200x_get_urc_obj(buf, msg.len);
                    if(NULL != urc) {
                        /* TODO: ??????????????????????????????????????????????????? */
                        urc_ret = urc->func(buf, msg.len);

                        /* ????????????CME/ERROR */
                        if((-CMD_ERR_CME_0) > urc_ret) {
                            return urc_ret;
                        }
                    } else {
                        at_ec200x_parse_unk(buf, msg.len);
                    }
                }
                ec200x_uart_free_msg(&msg);
            }
        }
    }

    return (urc_ret == CMD_ERR_NONE) ? (-CMD_ERR_UNKNOWN) : urc_ret;
}

int at_ec200x_exec_cmd_wait_flag(struct ec200x_exec_cmd_wait_flag_t *cmd)
{
    int ret = -1;
    volatile uint32_t retry = 0;

    while(1) {
        ret = at_ec200x_send_cmd_wait_ack(cmd->cmd, cmd->cmd_len, cmd->cmd_ack, cmd->cmd_tmo_ms);

        if(0x00 != ret) {
            printk("exec \'%s\' fail %d\r\n", cmd->cmd, ret);
            return ret;
        } else {
            msleep(100);

            at_ec200x_parse_msg();
            mb();

            if(*cmd->wait_val != cmd->wait_val_invaild) {
                if(*cmd->wait_val == cmd->wait_val_vaild) { return CMD_ERR_NONE; }
            }
        }

        retry ++;
        if(retry > cmd->wait_retry_cnt) {
            printk("TMO exec \'%s\'\r\n", cmd->cmd);
            return -CMD_ERR_WAIT_TIMEOUT;
        }
    }

    return CMD_ERR_NONE;
}
