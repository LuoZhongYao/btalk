/*
 * Written by ZhongYao Luo <luozhongyao@gmail.com>
 *
 * Copyright 2020 ZhongYao Luo
 */

#ifndef __USER_CB_H__
#define __USER_CB_H__

#include "hci_private.h"

static struct bt_conn *bt_conn_lookup_by_handle(struct hci_conn *c, uint16_t handle)
{
	const struct btcb *btcb = c->btcb;
	if (btcb && btcb->lookup_conn_by_handle)
		return btcb->lookup_conn_by_handle(handle);

	return NULL;
}

static bool bt_get_bdaddr(struct hci_conn *c, bdaddr_t *ba)
{
	const struct btcb *btcb = c->btcb;
	if (btcb && btcb->get_bdaddr)
		return btcb->get_bdaddr(ba);

	return false;
}

static void bt_init_complete(struct hci_conn *c)
{
	const struct btcb *btcb = c->btcb;
	if (btcb && btcb->init_complete)
		btcb->init_complete();
}

#endif /* __USER_CB_H__*/

