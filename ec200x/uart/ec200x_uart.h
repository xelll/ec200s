#ifndef __EC200X_UART_H
#define __EC200X_UART_H

#include <stdint.h>

#include "uart.h"

struct ec200x_uart_hw_info_t
{
    uart_device_number_t uart_dev;
    uint32_t uart_baud;

    uint8_t io_tx;
    uint8_t io_rx;

    int8_t io_pwr;
    int8_t io_sta;

    uint8_t gpiohs_pwr;
    uint8_t gpiohs_sta;
};

#define UART_MSG_SM_BUF_SIZE    (128)

struct ec200x_uart_msg_t
{
    uint32_t len;

    uint8_t sm_buf[UART_MSG_SM_BUF_SIZE];
    uint8_t *bg_buf;
};

extern volatile int ec200x_uart_line_num;
extern struct ec200x_uart_hw_info_t ec200x_uart_hw_info;

int ec200x_device_init(struct ec200x_uart_hw_info_t *hw_info);

void ec200x_en_uart_irq(void);
void ec200x_dis_uart_irq(void);

int ec200x_uart_pop_msg(struct ec200x_uart_msg_t *msg);
void ec200x_uart_free_msg(struct ec200x_uart_msg_t *msg);

int ec200x_uart_send_cmd(const char *cmd, size_t cmd_len);

void print_hex_array(char *tag, uint8_t *data, uint32_t len);

#endif /* __EC200X_UART_H */
