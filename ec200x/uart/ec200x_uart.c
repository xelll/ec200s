#include "ec200x_uart.h"

#include <string.h>
#include <stdlib.h>

#include "atomic.h"
#include "printf.h"
#include "sleep.h"
#include "sysctl.h"
#include "fpioa.h"
#include "gpiohs.h"

#include "cQueue.h"
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct ec200x_uart_hw_info_t ec200x_uart_hw_info;
volatile int ec200x_uart_line_num = 0;

///////////////////////////////////////////////////////////////////////////////
extern volatile uart_t *const uart[3];

static uart_device_number_t uart_ch = UART_DEVICE_1;

static int uart_channel_putc(char c)
{
    while(uart[uart_ch]->LSR & (1u << 5))
        continue;
    uart[uart_ch]->THR = c;
    return c & 0xff;
}

static int uart_channel_getc(uint8_t *data)
{
    /* If received empty */
    if(!(uart[uart_ch]->LSR & 1))
        return -1;

    *data = (uint8_t)(uart[uart_ch]->RBR & 0xff);

    return 1;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static inline void ec200x_uart_push_msg(uint8_t *data, uint32_t len);

/**
 * 中断里进行接收，
 * 当收到'\r\n' + $data + '\r\n'之后，
 * 将之前缓存的数据，放进队列中
 * 然后清空缓存，继续接收
 */

#define RECV_BUF_MAX_LEN        (1024 * 3)
static uint8_t recv_buf[RECV_BUF_MAX_LEN];

static uint8_t recv_last_ch = 0;
static volatile uint32_t recv_buf_len = 0;

static int uart_irq_cb(void *ctx)
{
    uint8_t ch = 0, push = 0;

    do {
        if(uart_channel_getc(&ch) < 0)
            return -1;

        if(RECV_BUF_MAX_LEN <= recv_buf_len) {
            printk("full");
            recv_buf_len = 0;
        }

        recv_buf[recv_buf_len++] = ch;

        if(('\r' == recv_last_ch) && ('\n' == ch) && (2 < recv_buf_len)){
            push = 1;
        } else if(('>' == recv_last_ch) && (' ' == ch)) {
            push = 1;
        }

        recv_last_ch = ch;

        if(push) {
            push = 0;

            ec200x_uart_push_msg(recv_buf, recv_buf_len);

            recv_buf_len = 0;
            recv_last_ch = 0;
        }
    } while (1);

    return 0;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#define UART_MSG_QUEUE_SIZE         (10)
static Queue_t uart_msg_q;

static volatile uint8_t q_wr_flag = 0;
static volatile uint8_t q_rd_flag = 0;

static void ec200x_uart_init_msg_list(void)
{
    q_init(&uart_msg_q, sizeof(struct ec200x_uart_msg_t), UART_MSG_QUEUE_SIZE, FIFO, false);

    if(NULL == uart_msg_q.queue) {
        printk("OOM %s\r\n", __func__);
        return;
    }
    memset(uart_msg_q.queue, 0x00, uart_msg_q.queue_sz);
}

static inline void ec200x_uart_push_msg(uint8_t *data, uint32_t len)
{
    uint8_t *tmp = NULL;

    if(UART_MSG_SM_BUF_SIZE < len) {
        tmp = malloc(len);
        if(NULL == tmp) {
            printk("OOM %s\r\n", __func__);

            return;
        }
        memcpy(tmp, data, len);
    }

    struct ec200x_uart_msg_t msg;

    msg.len = len;
    if(NULL != tmp) {
        msg.bg_buf = tmp;
    } else {
        msg.bg_buf = NULL;
        memcpy(msg.sm_buf, data, len);
    }

    volatile uint32_t wait_cnt = 0;

    uint64_t t = sysctl_get_time_us();

    while (1) {
        mb();
        if(0x00 == q_rd_flag) { break; }

        asm volatile("nop");
        asm volatile("nop");

        wait_cnt++;
        if((10 * 1000) < wait_cnt) { /* 37x us */
            t = sysctl_get_time_us() - t;

            if(NULL != tmp) { free(tmp); }
            printk("TMO @push %ld\r\n", t);

            return;
        }
    }

    q_wr_flag = 1;
    if(true != q_push(&uart_msg_q, (void *)&msg)) {
        q_wr_flag = 0;

        if(NULL != tmp) { free(tmp); }
        printk("push fail\r\n");

        return;
    }
    q_wr_flag = 0;

    ec200x_uart_line_num++;

    // print_hex_array("push", (uint8_t *)data, len > 16 ? 16 : len);

    // printk("push num %d\r\n", ec200x_uart_line_num);
}

int ec200x_uart_pop_msg(struct ec200x_uart_msg_t *msg)
{
    volatile uint32_t wait_cnt = 0;

    while (1) {
        mb();
        if(0x00 == q_wr_flag) { break; }

        asm volatile("nop");
        asm volatile("nop");

        wait_cnt++;
        if((10 * 1000) < wait_cnt) {
            printk("TMO @pop\r\n");
            return -2;
        }
    }

    q_rd_flag = 1;
    if(true != q_pop(&uart_msg_q, (void *)msg)) {
        q_rd_flag = 0;

        printk("pop fail\r\n");
        return -1;
    }
    q_rd_flag = 0;

    ec200x_uart_line_num--;

    // printk("pop num %d\r\n", ec200x_uart_line_num);

    return 0;
}

void ec200x_uart_free_msg(struct ec200x_uart_msg_t *msg)
{
    if(msg) {
        if(NULL != msg->bg_buf) {
            free(msg->bg_buf);
        }
    }
}

int ec200x_uart_send_cmd(const char *cmd, size_t cmd_len)
{
    for(size_t i = 0; i < cmd_len; i++)
        uart_channel_putc(cmd[i]);

    return 0;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void ec200x_uart_init(uart_device_number_t ch, uint32_t baud)
{
    uart_ch = ch;
    if(UART_DEVICE_MAX <= uart_ch) {
        printk("error uart dev %d\r\n", uart_ch);

        return;
    }

    recv_buf_len = 0;
    memset(recv_buf, 0, RECV_BUF_MAX_LEN);

    ec200x_uart_init_msg_list();

    uart_init(ch);
    uart_configure(ch, baud, UART_BITWIDTH_8BIT, UART_STOP_1, UART_PARITY_NONE);
    uart_set_receive_trigger(ch, UART_RECEIVE_FIFO_1);
}

int ec200x_device_init(struct ec200x_uart_hw_info_t *hw_info)
{
    if(-1 == hw_info->io_pwr) {
        return -1;
    }

    fpioa_set_function(hw_info->io_pwr, FUNC_GPIOHS0 + hw_info->gpiohs_pwr);
    gpiohs_set_drive_mode(hw_info->gpiohs_pwr, GPIO_DM_OUTPUT);

    if(-1 != hw_info->io_sta) {
        fpioa_set_function(hw_info->io_sta, FUNC_GPIOHS0 + hw_info->gpiohs_sta);
        gpiohs_set_drive_mode(hw_info->gpiohs_sta, GPIO_DM_INPUT_PULL_DOWN);
    }

    gpiohs_set_pin(hw_info->gpiohs_pwr, 1);
    msleep(500);
    gpiohs_set_pin(hw_info->gpiohs_pwr, 0);

    volatile uint32_t wait_cnt = 0;

    if(-1 != hw_info->io_sta) {
        while(1) {
            msleep(10);
            wait_cnt += 10;

            if((10 * 1000) < wait_cnt) {
                printk("TMO wait ec200x boot\r\n");
                return -2;
            }

            if(0x00 == gpiohs_get_pin(hw_info->gpiohs_sta)) {
                printk("ec200x power on\r\n");
                break;
            }
        }
    } else {
        /* 强制等待6s，模块应该可以启动完成了。 */
        sleep(6);
    }

    /* 跳过模块发送 '\r\nRDY\r\n' */
    msleep(1500);

    fpioa_set_function(hw_info->io_tx, FUNC_UART1_TX + hw_info->uart_dev * 2);
    fpioa_set_function(hw_info->io_rx, FUNC_UART1_RX + hw_info->uart_dev * 2);

    ec200x_uart_init(hw_info->uart_dev, hw_info->uart_baud);
    ec200x_en_uart_irq();

    return 0;
}

void ec200x_en_uart_irq(void)
{
    uart_irq_register(uart_ch, UART_RECEIVE, uart_irq_cb, NULL, 4);
}

void ec200x_dis_uart_irq(void)
{
    uart_irq_unregister(uart_ch, UART_RECEIVE);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#define PRINT_ASCII() \
            printk("| "); \
            for(;j <= i; j++) { \
                if(('\r' == data[j]) || ('\n' == data[j])) { \
                    data[j] = ' '; \
                } \
                printk("%c", data[j]); \
            } \
            printk("\r\n");

void print_hex_array(char *tag, uint8_t *data, uint32_t len)
{
	if(tag)
		printk("%s %d\r\n", tag, len);

    int i = 0, j = 0;

#define LINE_DATA_NUM	(8)
	for(i = 0; i < len; i++) {
		printk("%02X ", data[i]);
		if((LINE_DATA_NUM - 1) == (i % LINE_DATA_NUM)) {
            PRINT_ASCII();
        }

        if(i == (len - 1) && ((LINE_DATA_NUM - 1) != (i % LINE_DATA_NUM))) {
            for(int k = 0; k < LINE_DATA_NUM - (len % LINE_DATA_NUM); k++) {
                printk("   ");
            }
            PRINT_ASCII();
        }
	}
	printk("\r\n");
#undef LINE_DATA_NUM
}
