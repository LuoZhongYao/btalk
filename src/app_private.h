/*************************************************
 * Anthor  : LuoZhongYao@gmail.com
 * Modified: 2019/07/12
 ************************************************/
#ifndef __APP_PRIVATE_H__
#define __APP_PRIVATE_H__

#include "app.h"
#include "app_led.h"
#include "app_l2cap.h"
#include "fifo.h"
#include <stdint.h>

struct app_flash
{
	uint32_t magic;
	bdaddr_t local_address;
	bdaddr_t remote_address;
	uint8_t key_type;
	uint8_t link_key[16];
} __attribute__((aligned(4)));

struct app
{
	unsigned power_off:1;
	unsigned inquirying:1;
	unsigned connecting:1;
	app_status_t init_cmpl_status:4;
	app_status_t status:4;
	uint32_t srand;
	uint64_t reconnect_timeout;
	int8_t	 rssi;
	uint8_t  media_status;
	struct   kfifo dec_fifo;
	uint8_t	 dec_buf[8 * 1024];

	struct app_flash flash;
};

extern struct app app;
extern const uint8_t dev_class[3];

#endif /* __APP_PRIVATE_H__*/

