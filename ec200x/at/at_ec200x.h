#ifndef __AT_EC200X_H
#define __AT_EC200X_H

#include <stdint.h>
#include <stddef.h>

struct at_ec200x_t
{

};

struct ec200x_exec_cmd_wait_flag_t
{
    const char *cmd;
    size_t cmd_len;
    char *cmd_ack;
    uint32_t cmd_tmo_ms;

    int *wait_val;
    int wait_val_vaild;
    int wait_val_invaild;

    uint32_t wait_retry_cnt;
};

struct at_urc_t
{
    const char *prefix;
    const char *suffix;

    int (*func)(char *data, uint32_t len);
};

int at_ec200x_wait_connect(uint32_t time_out_ms);
int at_ec200x_set_cmd_echo_mode(uint8_t mode);

struct at_urc_t *at_ec200x_get_urc_obj(char *data, uint32_t len);
int at_ec200x_reg_urc_table(struct at_urc_t *tab, const uint32_t size);

int at_ec200x_parse_msg(void);
int at_ec200x_parse_unk(char *data, uint32_t len);

int at_ec200x_send_cmd_wait_ack(const char *cmd, size_t cmd_len, char *ack, uint32_t timeout_ms);
int at_ec200x_exec_cmd_wait_flag(struct ec200x_exec_cmd_wait_flag_t *cmd);

#endif /* __AT_EC200X_H */
