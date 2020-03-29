/*************************************************
 * Anthor  : LuoZhongYao@gmail.com
 * Modified: 2019/08/12
 ************************************************/
#ifndef __HCI_3WIRE_H__
#define __HCI_3WIRE_H__

#include <bluetooth/hci.h>
#include <pt-1.4/pt.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <pktf.h>

#if defined(ENABLE_HCI_H5)

#define H5_MAX_TX_WIN 1

struct hci_3wire {
	enum {
		H5_UNINITIALIZED,
		H5_INITIALIZED,
		H5_ACTIVE,
	} state;

	enum {
		H5_AWAKE,
		H5_SLEEPING,
		H5_WAKING_UP,
	} sleep;

	uint64_t timeout;
	uint8_t flags;

	uint8_t tx_seq;
	uint8_t tx_ack;
	uint8_t rx_ack;

	uint16_t tx_queue_size;
	uint8_t tx_queue_buf[HCI_MAX_FRAME_SIZE];
};

struct hci_conn;
void hci_3wire_work(struct hci_conn *c);
void hci_3wire_init(struct hci_conn *c);
void hci_3wire_send_frame(struct hci_3wire *c, uint8_t pkt_type, const uint8_t *data, unsigned len);
bool hci_3wire_can_send_reliable_frame(struct hci_3wire *c);
PT_THREAD(hci_3wire_rx_irq(struct hci_conn *c));
PT_THREAD(hci_3wire_tx_work(struct hci_conn *c));

#endif

#endif /* __HCI_3WIRE_H__*/
