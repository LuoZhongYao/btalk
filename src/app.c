#include "debug.h"
#include "time.h"
#include "pt-1.4/pt.h"
#include "sbc_codec.h"
#include "app_private.h"
#include "rate_display.h"
#include <soc.h>
#include <config.h>
#include <string.h>

#define APP_FLASH_MAGIC	0x48534144
#define APP_FLASH_BASE  0x3e000

#ifdef ENABLE_DEBUG
static const char *const status_string[] = {
	"Base",
	"Init",
	"Idle",
	"Advertising",
};
#endif

struct app app;

__attribute__((weak))
void power_off(void)
{
	BLOGE("Not Impl %s\n", __func__);
}

const uint8_t dev_class[] = {0x08, 0x25, 0x01};

void app_flash_store(void)
{
	unsigned i = 0;
	uint8_t buf[(sizeof(struct app_flash) + 3) & ~3];
	uint32_t *p = (uint32_t*)buf;

	app.flash.magic = APP_FLASH_MAGIC;
	memcpy(buf, &app.flash, sizeof(app.flash));

	flash_erase(APP_FLASH_BASE);

	flash_write(APP_FLASH_BASE, p, sizeof(buf));
	BLOGD("FMC Store local magic = %.*s, " BD_STR " remote " BD_STR "\n",
		4, (char*)&app.flash.magic, BD_FMT(&app.flash.local_address), BD_FMT(&app.flash.remote_address));
}

void app_flash_load(void)
{
	unsigned i = 0;
	uint8_t buf[(sizeof(struct app_flash) + 3) & ~3];
	uint32_t *p = (uint32_t*)buf;

	memset(buf, 0, sizeof(buf));

	flash_read(APP_FLASH_BASE, p, sizeof(buf));

	memcpy(&app.flash, buf, sizeof(app.flash));
	BLOGD("FMC Load local magic = %.*s, " BD_STR " remote " BD_STR "\n",
		4, (char*)&app.flash.magic, BD_FMT(&app.flash.local_address), BD_FMT(&app.flash.remote_address));

	if (app.flash.magic != APP_FLASH_MAGIC)
	{
		app_flash_store();
	}
}

static void exit_state_init(app_status_t status)
{
	uint8_t adv_data[31] = {
		0x06, 0x09,
		'b', 't', 'a', 'l', 'k'
	};
	hci_conn_write_dev_class(dev_class);
	hci_conn_write_scan_enable(SCAN_PAGE);
	hci_conn_le_set_scan_param(0x00, 0x0010, 0x0010, 0x00, 0x00);
	hci_conn_le_set_advertising_data(adv_data, sizeof(adv_data));
}

static void exit_state_idle(app_status_t status)
{
}

static void exit_state_advertising(app_status_t status)
{
	hci_conn_le_set_advertising_enable(0x00);
}

static void enter_state_init(app_status_t status)
{
}

static void enter_state_idle(app_status_t status)
{
	if (app.power_off) {
		BLOGE("Power Off\n");
		power_off();
	}
}

static void enter_state_advertising(app_status_t status)
{
	DELAY(D_MILLISECOND(100));
	hci_conn_le_set_scan_enable(0x01, 0x00);
	hci_conn_le_set_advertising_enable(0x01);
}

static void app_exit_status(app_status_t status)
{
	switch (app.status) {
	default: break;
	case APP_STATUS_INIT:
		exit_state_init(status);
	break;

	case APP_STATUS_IDLE:
		exit_state_idle(status);
	break;

	case APP_STATUS_ADVERTISING:
		exit_state_advertising(status);
	break;

	}
}

static void app_enter_status(app_status_t old_status)
{
	switch (app.status) {
	default: break;
	case APP_STATUS_INIT:
		enter_state_init(old_status);
	break;

	case APP_STATUS_IDLE:
		enter_state_idle(old_status);
	break;

	case APP_STATUS_ADVERTISING:
		enter_state_advertising(old_status);
	break;

	}
}

void app_set_status(app_status_t status)
{
	app_status_t old_status = app.status;
	if (status < APP_STATUS_MAX && status != app.status) {

		app_exit_status(status);
		app.status = status;
		app_enter_status(old_status);

		app_led_set_status(status);

		BLOGE("App status change: %s -> %s\n", status_string[old_status], status_string[status]);
	}
}

app_status_t app_get_status(void)
{
	return app.status;
}

void app_srand(uint32_t seed)
{
	//app.srand = seed << 23 | TIMER0->CNT | TIMER1->CNT << 15;
	BLOGD("Random seed: %x\n", app.srand);
}

int app_rand(void)
{
	app.srand = ((app.srand * 1103515245U) + 12345U) & 0x7fffffff;
	return app.srand;
}

bool app_get_local_address(bdaddr_t *ba)
{
	if (!bacmp(&app.flash.local_address, BDADDR_ANY) ||
		!bacmp(&app.flash.local_address, BDADDR_ALL) ||
		!bacmp(&app.flash.local_address, BDADDR_INVAL)) {
		uint32_t b[2];
		b[0] = app_rand();
		b[1] = app_rand();
		bacpy(&app.flash.local_address, (bdaddr_t*)b);
		app_flash_store();
	}

	bacpy(ba, &app.flash.local_address);

	return true;
}

void app_power_off(void)
{
	app.power_off = 1;
	if (app.status == APP_STATUS_IDLE) {
		power_off();
	}
}

struct kfifo *app_dec_fifo(void)
{
	return &app.dec_fifo;
}

void app_dec_fifo_reset(void)
{
	kfifo_reset(&app.dec_fifo);
}

void app_handle_init_complete(void)
{
	app_status_t status = APP_STATUS_ADVERTISING;

	if (app.init_cmpl_status != APP_STATUS_BASE)
		status = app.init_cmpl_status;

	app_set_status(status);
}

void app_init(void)
{

	memset(&app, 0, sizeof(app));

	app_flash_load();

	kfifo_init(&app.dec_fifo, app.dec_buf, sizeof(app.dec_buf));

	app_set_status(APP_STATUS_INIT);
}
