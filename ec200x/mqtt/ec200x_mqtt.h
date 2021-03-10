#ifndef __EC200X_MQTT_H
#define __EC200X_MQTT_H

#include "cmds.h"

#define MQTT_RECV_SEND_MODE_CHR (0)
#define MQTT_RECV_SEND_MODE_HEX (1)

struct ec200x_mqtt_cfg_t
{
    int keep_alive_time; /* 0~3600, Default 120 */

    struct {
        struct {
            int flag; /* 1:en; 0:dis */
            int msg_recv_mode; /* 0:urc; 1: not urc */
            int msg_len_enable; /* 1: add len in msg; 0: no len in msg */
        } recv_mode;

        struct {
            int flag; /* 1:en; 0:dis */
            int send_mode; /* 0:chr; 1:hex */
            int recv_mode; /* 0:chr; 1:hex */
        } dataformat;
    } qmtcfg;

    struct {
        uint16_t port; /* 1~65535 */
        char hostname[100]; /* domain or ip */
    } qmtopen;

    struct {
        char client_id[32];
        char username[32];
        char password[32];
    } qmtconn;

    struct {
        uint16_t msgid;
        uint16_t qos;

        char topic[64];
    } qmtsub;
};

struct ec200x_status_t
{
    struct {
        int result;
    } qmtopen;

    struct {
        int result;
        int ret_code;
    } qmtconn;

    struct {
        int msgid;
        int result;
        int value;
    } qmt_sub;

    struct {
        int msgid;
        int result;
        int value;
    } qmt_pubex;
};

struct ec200x_mqtt_msg_t
{
    int msgid;
    char topic[32];
    int payload_len;
    char *paylod;
};

struct ec200x_mqtt_t
{
    int client_idx;

    struct ec200x_mqtt_cfg_t cfg;
    struct ec200x_status_t status;
};

typedef int (*ec200x_mqtt_recv_cb)(struct ec200x_mqtt_msg_t *msg);

extern struct ec200x_mqtt_t ec200x_mqtt;

extern struct at_urc_t mqtt_cmds[];
extern const uint32_t mqtt_cmd_tabsz;

void ec200x_mqtt_reg_cb(ec200x_mqtt_recv_cb cb);

int ec200x_mqtt_cfg(struct ec200x_mqtt_t *mqtt);

int ec200x_mqtt_open(struct ec200x_mqtt_t *mqtt);

int ec200x_mqtt_conn(struct ec200x_mqtt_t *mqtt);

int ec200x_mqtt_sub(struct ec200x_mqtt_t *mqtt);

int ec200x_mqtt_pub(struct ec200x_mqtt_t *mqtt, char *topic, char *msg, uint32_t msg_len);

#endif /* __EC200X_MQTT_H */
