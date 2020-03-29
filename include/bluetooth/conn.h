/*
 * Written by ZhongYao Luo <luozhongyao@gmail.com>
 *
 * Copyright 2020 ZhongYao Luo
 */

#ifndef __BT_CONN_H__
#define __BT_CONN_H__
#include <bluetooth/hci.h>

struct bt_conn {
	uint16_t handle;
	bdaddr_t peer_addr;

	uint16_t rx_len;
	uint8_t  *rx_pos;
	uint8_t  l2cap[1050];
};

#endif /* __BT_CONN_H__*/

