/*
 * Written by ZhongYao Luo <luozhongyao@gmail.com>
 *
 * Copyright 2020 ZhongYao Luo
 */

#include "conn.h"
#include <defs.h>
#include <stdbool.h>
#include <bluetooth/hci_lib.h>

#define MAX_CONN	2
static struct conn conn_list[MAX_CONN];

#ifdef ENABLE_DEBUG
static const char *const state_strings[] = {
	"Idle", "Advertising", "Local connecting", "Remote connecting", "Connected"
};
#endif

static void conn_exit_state(struct conn *conn, conn_state_t state)
{
	switch (conn->state) {
	default: break;
	case CONN_STATE_IDLE:
	break;

	case CONN_STATE_ADVERTISING:
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

	case CONN_STATE_ADVERTISING:
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
	conn_state_t old_state = conn->state;
	if (state < CONN_STATE_TOP && state != old_state) {
		conn_exit_state(conn, state);
		conn->state = state;
		conn_enter_state(conn, old_state);

#ifdef ENABLE_DEBUG
		BLOGD("Conn state change: %s -> %s\n",
			state_string[old_state], state_string[state]);
#endif
	}
}

conn_state_t conn_get_state(struct conn *conn)
{
	return conn->state;
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
