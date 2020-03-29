/*************************************************
 * Anthor  : LuoZhongYao@gmail.com
 * Modified: 2019/07/05
 ************************************************/
#ifndef __HCI_PRIVATE_H__
#define __HCI_PRIVATE_H__
#include "hci_3wire.h"
#include "hci_h4.h"
#include "pktf.h"
#include "pt-1.4/pt.h"
#include "bluetooth/hci_lib.h"
#include <stdint.h>

#define RX_PKTF_NR	32
#define CMD_PKTF_NR	32

#define HCI_CONN_INIT_STEP_1	0x1
#define HCI_CONN_INIT_STEP_2	0x2

struct hci_conn
{
	uint16_t curn;
	uint16_t readn;
	uint16_t handle;

	uint8_t inited:2;
	uint8_t acl_connected:1;
	uint8_t shared_lock:1;
	uint8_t reset_uart:1;
	uint8_t hw_used:3;
	uint64_t timeout;

	uint16_t acl_pkts;
	uint16_t acl_mtu;
	uint16_t acl_cnt;

#if defined(ENABLE_HCI_H5)
	struct hci_3wire h5;
#endif

#if defined(ENABLE_HCI_H4)
#endif

	struct pt rx_work;
	struct pt rx_irq;
	struct pt tx_work;
	uint8_t  rx_buf[HCI_MAX_FRAME_SIZE * 2 + 14];

	const struct btcb *btcb;
	//Beware: shared buf can't be nested, can't be used inside interrupt, no need to use multiple cycles
	uint8_t shared_buff[HCI_MAX_FRAME_SIZE + 1];

	uint8_t cmd_pktf_buf[512];
	uint8_t rx_pktf_buf[4 * 1024];
	uint8_t tx_pktf_buf[4 * 1024];
	__TYPE_PKTF(CMD_PKTF_NR) cmd_pktf;
	__TYPE_PKTF(RX_PKTF_NR) rx_pktf;
	__TYPE_PKTF(RX_PKTF_NR) tx_pktf;
};

extern struct hci_conn hci_conn;

void hci_conn_reset_complete(void);
void hci_conn_bthw_connected(void);
void hci_conn_init(void);
void hci_conn_init_complete(void);

static inline bool acl_can_send(struct hci_conn *c)
{
	return (pktf_len(&c->tx_pktf.f) != 0) && (c->acl_cnt > 0);
}

static inline bool cmd_can_send(struct hci_conn *c)
{
	return pktf_len(&c->cmd_pktf.f) != 0;
}

#define SHARED_LOCK(c) do { \
	if ((c)->shared_lock) {\
		BLOGE("shared lock %s: %d\n", __FILE__, __LINE__);\
		while((c)->shared_lock);\
	} \
	(c)->shared_lock = 1; } while(0)

#define SHARED_UNLOCK(c) do{ (c)->shared_lock = 0; } while(0)

#endif /* __HCI_PRIVATE_H__*/

