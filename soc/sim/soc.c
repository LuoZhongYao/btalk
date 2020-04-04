/*
 * Written by ZhongYao Luo <luozhongyao@gmail.com>
 *
 * Copyright 2020 ZhongYao Luo
 */

#include "fifo.h"
#include <debug.h>
#include <stdbool.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <stddef.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/timerfd.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>

void flash_init(void);
volatile uint64_t jiffies = 0;

struct uart
{
	int fd;
	int timerfd;
	unsigned tx_size;
	volatile bool	tx_busy;
	uint8_t tx_buff[2048];
	uint8_t rx_buff[720];
	uint8_t	rx_fifo_buf[4 * 1024];
	struct kfifo rx_fifo;
};

static struct uart uart;
struct sockaddr_hci {                                                            
	sa_family_t hci_family;                                                      
	unsigned short  hci_dev;                                                     
	unsigned short  hci_channel;                                                 
};                                                                               
#define HCI_DEV_NONE    0xffff                                                   

#define HCI_CHANNEL_RAW     0                                                    
#define HCI_CHANNEL_USER    1                                                    
#define HCI_CHANNEL_MONITOR 2                                                    
#define HCI_CHANNEL_CONTROL 3                                                    
#define HCI_CHANNEL_LOGGING 4 

static int open_channel(uint16_t index)
{
	int fd;
	int on = 1;
	struct sockaddr_hci addr;

	BLOGD("Opening user channel for hci%u\n", index);

	fd = socket(PF_BLUETOOTH, SOCK_RAW | SOCK_CLOEXEC, BTPROTO_HCI);
	if (fd < 0) {
		perror("Failed to open Bluetooth socket");
		return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.hci_family = AF_BLUETOOTH;
	addr.hci_dev = index;
	addr.hci_channel = HCI_CHANNEL_USER;

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
		close(fd);
		perror("setsockopt");
		return -1;
	}

	if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		close(fd);
		perror("Failed to bind Bluetooth socket");
		return -1;
	}

	return fd;
}


static void *soc_backend_handler(void *arg)
{
	uint64_t exp;
	fd_set rdfs;
	struct timeval tmo = {0, 40};
	while (1) {
		FD_ZERO(&rdfs);
		FD_SET(uart.fd, &rdfs);
		FD_SET(uart.timerfd, &rdfs);
		select(uart.timerfd + 1, &rdfs, NULL, NULL, NULL);

		if (FD_ISSET(uart.timerfd, &rdfs)) {
			jiffies++;
			read(uart.timerfd, &exp, sizeof(exp));
			if (uart.tx_busy == true) {
				write(uart.fd, uart.tx_buff, uart.tx_size);
				uart.tx_busy = false;
			}
		}

		if (FD_ISSET(uart.fd, &rdfs)) {
			ssize_t rn = read(uart.fd, uart.rx_buff, sizeof(uart.rx_buff));
			if (kfifo_avail(&uart.rx_fifo) < rn) {
				BLOGE("UART FIFO overrun req = %d, avail = %d\n", rn, kfifo_avail(&uart.rx_fifo));
			}

			kfifo_in(&uart.rx_fifo, uart.rx_buff, rn);
		}
	}
}

unsigned uart_rx_len(void)
{
	return kfifo_len(&uart.rx_fifo);
}

unsigned uart_rx_out(uint8_t *buf, unsigned size)
{
	return kfifo_out(&uart.rx_fifo, buf, size);
}

unsigned uart_rx_get(uint8_t *ch)
{
	return kfifo_get(&uart.rx_fifo, ch);
}

bool uart_tx_busy(void)
{
	return uart.tx_busy;
}

void *uart_dma_buffer(void)
{
	return uart.tx_busy ? NULL : uart.tx_buff;
}

void uart_dma_tx_transfer(unsigned size)
{
	if (!uart.tx_busy) {
		uart.tx_size = size;
		uart.tx_busy = true;
	}
}

struct kfifo *uart_rx_fifo(void)
{
	return &uart.rx_fifo;
}

void UART_Reset(unsigned baudrate)
{
}

static uint16_t calc_crc (uint8_t ch, uint16_t crc)
{
	/* Calculate the CRC using the above 16 entry lookup table */

	static const uint16_t crc_table[] =
		{
			0x0000, 0x1081, 0x2102, 0x3183,
			0x4204, 0x5285, 0x6306, 0x7387,
			0x8408, 0x9489, 0xa50a, 0xb58b,
			0xc60c, 0xd68d, 0xe70e, 0xf78f
		};

	/* Do this four bits at a time - more code, less space */

    crc = (crc >> 4) ^ crc_table[(crc ^ ch) & 0x000f];
    crc = (crc >> 4) ^ crc_table[(crc ^ (ch >> 4)) & 0x000f];

	return crc;
}

uint16_t crc16_array(const uint8_t **datas, unsigned *size)
{
	unsigned i;
	uint16_t crc = 0;
	const uint8_t **p = datas;
	while (*p) {
		for (i = 0; i < *size; i++) {
			crc = calc_crc((*p)[i], crc);
		}
		p++;
		size++;
	}

	return crc;
}

void soc_init(void)
{
	pthread_t pid;
	struct itimerspec its = {
		.it_value = {.tv_sec = 0, .tv_nsec = 40 * 1000},
		.it_interval = {.tv_sec = 0, .tv_nsec = 40 * 1000}
	};

	flash_init();
	uart.fd = open_channel(1);
	uart.timerfd = timerfd_create(0, 0);
	timerfd_settime(uart.timerfd, 0, &its, NULL);
	kfifo_init(&uart.rx_fifo, uart.rx_fifo_buf, sizeof(uart.rx_fifo_buf));
	pthread_create(&pid, NULL, soc_backend_handler, NULL);
}
