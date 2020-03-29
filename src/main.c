/*
 * Written by ZhongYao Luo <luozhongyao@gmail.com>
 *
 * Copyright 2020 ZhongYao Luo
 */
#include <time.h>
#include <debug.h>
#include <sbc_codec.h>
#include <bluetooth/hci_lib.h>

#include "conn.h"
#include "app_private.h"

static void app_handle_inquiry_result(inquiry_info_with_rssi *result)
{
	if (app.status != APP_STATUS_INQUIRYING)
		return ;

	if (!memcmp(result->dev_class, dev_class, sizeof(dev_class)) && result->rssi > app.rssi) {
		BLOGI("Update found device " BD_STR " old rssi = %d, new rssi = %d\n",
			BD_FMT(&result->bdaddr), app.rssi, result->rssi);
		bacpy(&app.flash.remote_address, &result->bdaddr);
		app.rssi = result->rssi;
	}
}

static void app_handle_inquiry_complete(void)
{
	if (bacmp(&app.flash.remote_address, BDADDR_ANY)) {
		app.inquirying = false;
		app_set_status(APP_STATUS_LOCAL_CONNECT);
	} else {
		hci_conn_inquiry(GIAC, 4, 8);
	}
}

static void app_handle_disconn_complete(uint8_t status, uint8_t reason)
{
#if defined(M480)
	if (reason == 0x08)
		TIMER_SETUP(app.reconnect_timeout, D_SEC(3));
#endif

	app_set_status(APP_STATUS_IDLE);
}

static void app_handle_link_key_notify(bdaddr_t *ba, uint8_t key_type, uint8_t link_key[16])
{
	if (!bacmp(&app.flash.remote_address, ba)) {
		app.flash.key_type = key_type;
		memcpy(app.flash.link_key, link_key, sizeof(app.flash.link_key));
		app_flash_store();
	}
}

static void app_handle_link_key_req(bdaddr_t *ba)
{
	if (!bacmp(&app.flash.remote_address, ba)) {
		hci_conn_link_key_reply(ba, app.flash.link_key);
	} else {
		hci_conn_link_key_neg_reply(ba);
	}
}

static void app_handle_user_confirm_request(bdaddr_t *ba)
{
	if (!bacmp(&app.flash.remote_address, ba)) {
		hci_conn_user_confirm_reply(ba);
	} else {
		hci_conn_user_confirm_neg_reply(ba);
	}
}

static void hci_inquiry_result_evt(inquiry_info_with_rssi *info)
{
	BLOGI("Found " BD_STR ", rssi = %d dev_class = %02x%02x%02x\n",
		BD_FMT(&info->bdaddr), info->rssi, info->dev_class[0], info->dev_class[1], info->dev_class[2]);

	app_handle_inquiry_result(info);
}

static void hci_inquiry_complete_evt(void)
{
	app_handle_inquiry_complete();
}

static void hci_conn_complete_evt(evt_conn_complete *con)
{
	struct conn *conn;
	conn_state_t st;

	BLOGD("Connection complete status = %x " BD_STR "\n",
		con->status, BD_FMT(&con->bdaddr));

	conn = conn_lookup_by_peer_addr(&con->bdaddr);
	if (!conn) {
	}

	st = conn_get_state(conn);
	if (con->status == 0 && (st == CONN_STATE_LOCAL_CONNECTING ||
		st == CONN_STATE_REMOTE_CONNECTING)) {
		conn->bt_conn.handle = con->handle;
		conn_set_state(conn, CONN_STATE_CONNECTED);
	}
}

static void hci_conn_reqeust_evt(evt_conn_request *req)
{
	struct conn *conn;
	union {
		accept_conn_req_cp accept;
		reject_conn_req_cp reject;
	} cp;

	BLOGD("Connection request " BD_STR "\n", BD_FMT(&req->bdaddr));
	conn = conn_get_free(&req->bdaddr);
	if (conn) {
		conn_set_state(conn, CONN_STATE_REMOTE_CONNECTING);
		cp.accept.role = 0x01;
		bacpy(&cp.accept.bdaddr, &req->bdaddr);
		hci_send_cmd(OGF_LINK_CTL, OCF_ACCEPT_CONN_REQ, &cp, sizeof(cp.accept));
	} else {
		cp.reject.reason = 0x1f;
		bacpy(&cp.reject.bdaddr, &req->bdaddr);
		hci_send_cmd(OGF_LINK_CTL, OCF_REJECT_CONN_REQ, &cp, sizeof(cp.reject));
	}
}

static void hci_disconn_complete_evt(evt_disconn_complete *evt)
{
	struct conn *conn;

	BLOGD("Disconnect complete handle = %04x, status = %d, reason = %x\n",
		evt->handle, evt->status, evt->reason);

	conn = conn_lookup_by_handle(evt->handle);
	if (conn) {
		conn_set_state(conn, CONN_STATE_ADVERTISING);
	}
}

static void hci_io_capability_request_evt(evt_io_capability_request *req)
{
	io_capability_reply_cp cp = {
		.bdaddr = req->bdaddr,
		.capability = 0x03,
		.oob_data = 0,
		.authentication = 0x04,
	};
	hci_send_cmd(OGF_LINK_CTL, OCF_IO_CAPABILITY_REPLY, &cp, sizeof(cp));
}

static void hci_simple_pairing_complete_evt(evt_simple_pairing_complete *com)
{
	BLOGD("Simple pairing complete status = %d " BD_STR "\n", com->status, BD_FMT(&com->bdaddr));
}

static void hci_link_key_notify_evt(evt_link_key_notify *notify)
{
	BLOGD("Link key notify: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x " BD_STR "\n",
		notify->link_key[0], notify->link_key[1], notify->link_key[2], notify->link_key[3],
		notify->link_key[4], notify->link_key[5], notify->link_key[6], notify->link_key[7],
		notify->link_key[8], notify->link_key[9], notify->link_key[10], notify->link_key[11],
		notify->link_key[12], notify->link_key[13], notify->link_key[14], notify->link_key[15],
		BD_FMT(&notify->bdaddr));

	app_handle_link_key_notify(&notify->bdaddr, notify->key_type, notify->link_key);
}

static struct bt_conn *lookup_conn_by_peer_addr(const bdaddr_t *ba)
{
	struct conn *conn;

	conn = conn_lookup_by_peer_addr(ba);
	if (conn) {
		return &conn->bt_conn;
	}

	return NULL;
} 

static struct bt_conn *lookup_conn_by_handle(uint16_t handle)
{
	struct conn *conn;

	conn = conn_lookup_by_handle(handle);
	if (conn) {
		return &conn->bt_conn;
	}

	return NULL;
} 

static const struct btcb btcb = {
	.inquiry_result = hci_inquiry_result_evt,
	.inquiry_complete = hci_inquiry_complete_evt,
	.conn_request = hci_conn_reqeust_evt,
	.conn_complete = hci_conn_complete_evt,
	.disconn_complete = hci_disconn_complete_evt,
	.io_capability_request = hci_io_capability_request_evt,
	.simple_pairing_complete = hci_simple_pairing_complete_evt,
	.link_key_notify = hci_link_key_notify_evt,

	.lookup_conn_by_handle = lookup_conn_by_handle,
	.lookup_conn_by_peer_addr = lookup_conn_by_peer_addr,

	.get_bdaddr = app_get_local_address,
	.init_complete = app_handle_init_complete,
};

int main(void)
{
	app_init();
	hci_runtime_init(&btcb);

	while (1) {
		btstack_run();
		sbc_codec_run();
	}

	return 0;
}
