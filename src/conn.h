/*
 * Written by ZhongYao Luo <luozhongyao@gmail.com>
 *
 * Copyright 2020 ZhongYao Luo
 */

#ifndef __CONN_H__
#define __CONN_H__

#include <stdint.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>

typedef enum {
	CONN_STATE_IDLE,
	CONN_STATE_ADVERTISING,
	CONN_STATE_LOCAL_CONNECTING,
	CONN_STATE_REMOTE_CONNECTING,
	CONN_STATE_CONNECTED,
	CONN_STATE_TOP,
} conn_state_t;

struct conn {
	unsigned used:1;
	conn_state_t state;
	struct bt_conn bt_conn;
};

void conn_free(struct conn *conn);
struct conn *conn_get_free(const bdaddr_t *peer_addr);
struct conn *conn_lookup_by_peer_addr(const bdaddr_t *peer_addr);
struct conn *conn_lookup_by_handle(uint16_t handle);

conn_state_t conn_get_state(struct conn *conn);
void conn_set_state(struct conn *conn, conn_state_t state);

#endif /* __CONN_H__*/

