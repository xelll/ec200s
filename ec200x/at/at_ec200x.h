#ifndef __AT_EC200X_H
#define __AT_EC200X_H

#include <stdint.h>
#include <stddef.h>

struct at_ec200x_t
{

};

struct at_urc_t
{
    const char *prefix;
    const char *suffix;

    int (*func)(char *data, uint32_t len);
};

int at_ec200x_wait_connect(uint32_t time_out_ms);
int at_ec200x_set_cmd_echo_mode(uint8_t mode);

int at_ec200x_send_cmd_wait_ack(const char *cmd, size_t cmd_len, char *ack, uint32_t timeout_ms);

int at_ec200x_parse_msg(void);

int at_ec200x_parse_unk(char *data, uint32_t len);

struct at_urc_t *at_ec200x_get_urc_obj(char *data, uint32_t len);

int at_ec200x_reg_urc_table(struct at_urc_t *tab, const uint32_t size);

#endif /* __AT_EC200X_H */
