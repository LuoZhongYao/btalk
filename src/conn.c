/*
 * Written by ZhongYao Luo <luozhongyao@gmail.com>
 *
 * Copyright 2020 ZhongYao Luo
 */

#include "conn.h"
#include <config.h>
#include <defs.h>
#include <stdbool.h>
#include <rate_display.h>
#include <bluetooth/hci_lib.h>

#define MAX_CONN	2
static struct conn conn_list[MAX_CONN];

#ifdef ENABLE_DEBUG
static const char *const state_strings[] = {
	"Idle", "Local connecting", "Remote connecting", "Connected"
};
#endif

static void conn_exit_state(struct conn *conn, conn_state_t state)
{
	switch (conn->state) {
	default: break;
	case CONN_STATE_IDLE:
	break;

	case CONN_STATE_LOCAL_CONNECTING:
	break;

	case CONN_STATE_REMOTE_CONNECTING:
	break;

	case CONN_STATE_CONNECTED:
	break;
	}
}

static void conn_enter_state(struct conn *conn, conn_state_t old_state)
{
	switch (conn->state) {
	default: break;
	case CONN_STATE_IDLE:
	break;

	case CONN_STATE_LOCAL_CONNECTING:
	break;

	case CONN_STATE_REMOTE_CONNECTING:
	break;

	case CONN_STATE_CONNECTED:
	break;
	}
}

void conn_set_state(struct conn *conn, conn_state_t state)
{
	if (conn == NULL)
		return ;

	conn_state_t old_state = conn->state;
	if (state < CONN_STATE_TOP && state != old_state) {
		conn_exit_state(conn, state);
		conn->state = state;
		conn_enter_state(conn, old_state);

#ifdef ENABLE_DEBUG
		BLOGD("Conn[%d] state change: %s -> %s\n",
			conn_id(conn), state_strings[old_state], state_strings[state]);
#endif
	}
}

conn_state_t conn_get_state(struct conn *conn)
{
	return conn->state;
}

int conn_id(struct conn *conn)
{
	if (conn >= conn_list && conn < &conn_list[MAX_CONN])
		return conn - conn_list;

	return -1;
}

struct conn *conn_get_free(const bdaddr_t *peer_addr)
{
	unsigned i;
	struct conn *conn;

	for (i = 0; i < ARRAY_SIZE(conn_list); i++) {
		conn = conn_list + i;
		if (conn->used == false) {
			conn->used = 1;
			bacpy(&conn->bt_conn.peer_addr, peer_addr);
			return conn;
		}
	}

	return NULL;
}

void conn_free(struct conn *conn)
{
	memset(conn, 0, sizeof(*conn));
}

struct conn *conn_lookup_by_peer_addr(const bdaddr_t *peer_addr)
{
	unsigned i;
	struct conn *conn;

	for (i = 0; i < ARRAY_SIZE(conn_list); i++) {
		if (conn->used && bacmp(&conn->bt_conn.peer_addr, peer_addr)) {
			return conn;
		}
	}

	return NULL;
}

struct conn *conn_lookup_by_handle(uint16_t handle)
{
	unsigned i;
	struct conn *conn;

	for (i = 0; i < ARRAY_SIZE(conn_list); i++) {
		if (conn->used && conn->bt_conn.handle == handle) {
			return conn;
		}
	}

	return NULL;
}

struct conn *conn_lookup_by_id(int id)
{
	if (id >= 0 && id < MAX_CONN)
		return conn_list + id;

	return NULL;
}

static void sbc_enc_handler(const uint8_t *buf, unsigned size)
{
	//if (app.status == APP_STATUS_CONNECTED)
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

	//if (kfifo_avail(&app.dec_fifo) < size) {
	//	BLOGE("Decode fifo overflow, avail = %d, size = %d, len = %d, req = %d\n",
	//		kfifo_avail(&app.dec_fifo), kfifo_size(&app.dec_fifo),
	//		kfifo_len(&app.dec_fifo), size);
	//}

	//kfifo_in(&app.dec_fifo, buf, size);

	////void AudioPlayStart(void);
	//if ((kfifo_len(&app.dec_fifo) >= 128 * 4 * 4))
	//{
	//	//AudioPlayStart();
	//}
}

void conn_init(void)
{
	unsigned i;
	struct conn *conn;

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

	for (i = 0; i < ARRAY_SIZE(conn_list); i++) {
		conn = &conn_list[i];
		sbc_codec_init(&conn->codec, &a2dp, 672, sbc_enc_handler, sbc_dec_handler);
	}
}

void conn_workloop(void)
{
	unsigned i;
	struct conn *conn;

	for (i = 0; i < ARRAY_SIZE(conn_list); i++) {
		conn = &conn_list[i];
		sbc_codec_workloop(&conn->codec);
	}
}
