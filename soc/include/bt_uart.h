/*************************************************
 * Anthor  : LuoZhongYao@gmail.com
 * Modified: 2019/07/17
 ************************************************/
#ifndef __BT_UART_H__
#define __BT_UART_H__
#include <stdint.h>
#include <stdbool.h>
#include <fifo.h>

void UART0_Init(uint32_t baudrate);
void bt_uart_set_flowctrl(void);
void bt_uart_set_baudrate(uint32_t baudrate);
unsigned uart_write(const uint8_t *buf, unsigned size);
bool uart_tx_busy(void);
unsigned uart_rx_len(void);
unsigned uart_rx_out(uint8_t *buf, unsigned size);
unsigned uart_rx_get(uint8_t *ch);
struct kfifo *uart_rx_fifo(void);
void *uart_dma_buffer(void);

void uart0_pdma_irqhandler(void);
void uart_dma_tx_transfer(unsigned size);

#endif /* __BT_UART_H__*/

