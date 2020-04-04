#include "debug.h"
#include "time.h"
#include "pt-1.4/pt.h"
#include "sbc_codec.h"
#include "app_private.h"
#include "rate_display.h"
#include <config.h>
#include <string.h>

#define APP_FLASH_MAGIC	0x48534144

#ifdef M480

#define APP_FLASH_BASE	0x3f000

#elif defined(I94100)

#define APP_FLASH_BASE  0x3e000

#endif

#ifdef ENABLE_DEBUG
static const char *const status_string[] = {
	"Base",
	"Init",
	"Idle",
	"Discovery",
	"Inquiry",
	"Remote connect",
	"Local connect",
	"Connected",
};
#endif

struct app app;

__attribute__((weak))
void power_off(void)
{
	BLOGE("Not Impl %s\n", __func__);
}

const uint8_t dev_class[] = {0x08, 0x25, 0x01};
static void sbc_enc_handler(const uint8_t *buf, unsigned size)
{
	if (app.status == APP_STATUS_CONNECTED)
	{
		/* sbc_write(buf, size); */
		/* Sacrifice logic, get a little optimization */
		//acl_write(buf, size);
	}
}

static void sbc_dec_handler(const uint8_t *buf, unsigned size)
{
	static struct rate_display ra;

	rate_display(&ra, size, "Decode", "B", 5);

	if (kfifo_avail(&app.dec_fifo) < size) {
		BLOGE("Decode fifo overflow, avail = %d, size = %d, len = %d, req = %d\n",
			kfifo_avail(&app.dec_fifo), kfifo_size(&app.dec_fifo),
			kfifo_len(&app.dec_fifo), size);
	}

	kfifo_in(&app.dec_fifo, buf, size);

	//void AudioPlayStart(void);
	if ((kfifo_len(&app.dec_fifo) >= 128 * 4 * 4))
	{
		//AudioPlayStart();
	}
}

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
	hci_conn_write_dev_class(dev_class);
	hci_conn_write_scan_enable(SCAN_PAGE);
}

static void exit_state_idle(app_status_t status)
{
}

static void exit_state_discovery(app_status_t status)
{
	hci_conn_write_scan_enable(SCAN_PAGE);
}

static void exit_state_inquiring(app_status_t status)
{
	if (status != APP_STATUS_INQUIRYING
		&& app.status == APP_STATUS_INQUIRYING
		&& app.inquirying)
	{
		hci_send_cmd(OGF_LINK_CTL, OCF_INQUIRY_CANCEL, NULL, 0);
	}
	app.inquirying = false;
}

static void exit_state_local_connect(app_status_t status)
{
	if ((status != APP_STATUS_CONNECTED
		|| status != APP_STATUS_IDLE)
		&& app.connecting)
	{
		hci_conn_connection_cancel(&app.flash.remote_address);
	}
	app.connecting = false;
}

static void exit_state_remote_connect(app_status_t status)
{
}

static void exit_state_connected(app_status_t status)
{
	hci_conn_write_scan_enable(SCAN_PAGE);

#if defined(M480)
	app_media_status_change(0);
#endif
	sbc_codec_reset();
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

static void enter_state_discovery(app_status_t status)
{
	hci_conn_write_scan_enable(SCAN_PAGE | SCAN_INQUIRY);
}

static void enter_state_inquiring(app_status_t old_status)
{
	if (old_status != APP_STATUS_INQUIRYING)
	{
		app.inquirying = true;
		app.rssi = -127;
		memset(&app.flash.remote_address, 0, sizeof(app.flash.remote_address));
		hci_conn_inquiry(GIAC, 4, 8);
	}
}

static void enter_state_local_connect(app_status_t status)
{
	create_conn_cp cp = {
		.bdaddr = app.flash.remote_address,
		.pkt_type = ACL_PTYPE_MASK,
		.pscan_rep_mode = 0,
		.pscan_mode = 0,
		.clock_offset = 0,
		.role_switch = 1
	};

	BLOGD("Create connection rssi = %d " BD_STR "\n",
		app.rssi, BD_FMT(&app.flash.remote_address));

	app.connecting = true;
	hci_send_cmd(OGF_LINK_CTL, OCF_CREATE_CONN, &cp, sizeof(cp));
}

static void enter_state_remote_connect(app_status_t old_status)
{
}

static void enter_state_connected(app_status_t old_status)
{
	if (old_status == APP_STATUS_LOCAL_CONNECT) {
		hci_conn_write_supervision_timeout(0xc80);
	}
	hci_conn_write_scan_enable(SCAN_DISABLED);
	app_flash_store();
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

	case APP_STATUS_DISCOVERY:
		exit_state_discovery(status);
	break;

	case APP_STATUS_INQUIRYING:
		exit_state_inquiring(status);
	break;

	case APP_STATUS_REMOTE_CONNECT:
		exit_state_remote_connect(status);
	break;

	case APP_STATUS_LOCAL_CONNECT:
		exit_state_local_connect(status);
	break;

	case APP_STATUS_CONNECTED:
		exit_state_connected(status);
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

	case APP_STATUS_DISCOVERY:
		enter_state_discovery(old_status);
	break;

	case APP_STATUS_INQUIRYING:
		enter_state_inquiring(old_status);
	break;

	case APP_STATUS_LOCAL_CONNECT:
		enter_state_local_connect(old_status);
	break;

	case APP_STATUS_REMOTE_CONNECT:
		enter_state_remote_connect(old_status);
	break;

	case APP_STATUS_CONNECTED:
		enter_state_connected(old_status);
	break;
	}
}

void app_set_status(app_status_t status)
{
	app_status_t old_status = app.status;
	if (status < APP_STATUS_MAX && status != app.status) {

		if (status >= APP_STATUS_INQUIRYING)
			TIMER_RESET(app.reconnect_timeout);

		app_exit_status(status);
		app.status = status;
		app_enter_status(old_status);

		if (status == APP_STATUS_IDLE && app.reconnect_timeout != 0)
			app_led_set_status(APP_STATUS_LOCAL_CONNECT);
		else
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

void app_start_inquiry(void)
{
	if (app.status <= APP_STATUS_INIT) {
		app.init_cmpl_status = APP_STATUS_INQUIRYING;
	} else if (app.status != APP_STATUS_CONNECTED) {
		app_set_status(APP_STATUS_INQUIRYING);
	}
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

void app_start_discovery(void)
{
	if (app.status > APP_STATUS_INIT) {
		app_set_status(APP_STATUS_DISCOVERY);
	} else {
		app.init_cmpl_status = APP_STATUS_DISCOVERY;
	}
}

void app_power_off(void)
{
	app.power_off = 1;
	if (app.status == APP_STATUS_IDLE) {
		power_off();
	} else if (app.status != APP_STATUS_CONNECTED) {
		app_set_status(APP_STATUS_IDLE);
	} else {
		hci_conn_disconnect_request(0x13);
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
	app_status_t status = APP_STATUS_IDLE;

#if defined(M480)
	status = APP_STATUS_DISCOVERY;
	if (bacmp(&app.flash.remote_address, BDADDR_ANY)
		&& bacmp(&app.flash.remote_address, BDADDR_ALL))
	{
		status = APP_STATUS_LOCAL_CONNECT;
	}
#endif

	if (app.init_cmpl_status != APP_STATUS_BASE)
		status = app.init_cmpl_status;

	app_set_status(status);
}

void app_init(void)
{
#define __(a, b) a ## b
#define _(a, b) __(a, b)
	struct a2dp_sbc a2dp = {
		.channel_mode = A2DP_CHANNEL_MODE_JOINT_STEREO,
		.frequency = _(A2DP_SAMPLING_FREQ_, APP_AUDIO_RATE),
		.allocation_method = A2DP_ALLOCATION_SNR,
		.subbands = A2DP_SUBBANDS_8,
		.block_length = A2DP_BLOCK_LENGTH_16,
		.min_bitpool = 2,
		.max_bitpool = 53,
	};
#undef _
#undef __

	memset(&app, 0, sizeof(app));

	app_flash_load();

	kfifo_init(&app.dec_fifo, app.dec_buf, sizeof(app.dec_buf));
	sbc_codec_init(&a2dp, 672, sbc_enc_handler, sbc_dec_handler);

	app_set_status(APP_STATUS_INIT);
}
