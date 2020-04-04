/*************************************************
 * Anthor  : LuoZhongYao@gmail.com
 * Modified: 2019/07/05
 ************************************************/
#ifndef __APP_H__
#define __APP_H__

#include <stdbool.h>
#include <bluetooth/hci_lib.h>

typedef enum
{
	APP_STATUS_BASE,
	APP_STATUS_INIT,
	APP_STATUS_IDLE,

	APP_STATUS_ADVERTISING,

	APP_STATUS_MAX,
} app_status_t;

void app_init(void);
void app_run(void);
void app_power_off(void);
void app_srand(uint32_t seed);
struct kfifo *app_dec_fifo(void);
void app_dec_fifo_reset(void);
int app_rand(void);

app_status_t app_get_status(void);
void app_set_status(app_status_t status);

void app_flash_store(void);
void app_start_discovery(void);
void app_start_inquiry(void);

void app_handle_init_complete(void);
bool app_get_local_address(bdaddr_t *ba);

#endif /* __APP_H__*/

