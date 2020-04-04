#include <config.h>

#include "hci_private.h"
#include "bt_uart.h"
#include "debug.h"
#include "time.h"

#if defined(ENABLE_HCI_H4)

static const struct h4_pkt_match {
    uint8_t type;
    uint8_t hlen;
    uint8_t loff;
    uint8_t lsize;
    uint16_t maxlen;
} h4_pkts [] = {
	{0, 0, 0, 0, 0 },   /* 0 */
	{
		.type = HCI_COMMAND_PKT,
		.hlen = HCI_COMMAND_HDR_SIZE,
		.loff = 2,
		.lsize = 1,
		.maxlen = HCI_MAX_COMMAND_SIZE,
	},   /* hci command */

	{
		.type = HCI_ACLDATA_PKT, 
		.hlen = HCI_ACL_HDR_SIZE, 
		.loff = 2, 
		.lsize = 2, 
		.maxlen = HCI_MAX_FRAME_SIZE 
	},  /* acl data*/

	{
		.type = HCI_SCODATA_PKT, 
		.hlen = HCI_SCO_HDR_SIZE, 
		.loff = 2, 
		.lsize = 1, 
		.maxlen = HCI_MAX_SCO_SIZE
	},  /* sco data */

	{
		.type = HCI_EVENT_PKT, 
		.hlen = HCI_EVENT_HDR_SIZE, 
		.loff = 1, 
		.lsize = 1, 
		.maxlen = HCI_MAX_EVENT_SIZE
	}  /* hci event */
};


#define READ_BYTE(c) \
    do {\
        PT_WAIT_UNTIL(&c->rx_irq, (kfifo_len(f) >= (c->readn - c->curn)));\
		c->curn += kfifo_out(f, c->rx_buf + c->curn, c->readn - c->curn); \
    } while(c->curn < c->readn)

static unsigned short h4_pkt_len(const unsigned char *pkt, const struct h4_pkt_match *match)
{
    switch(match->lsize) {
    case 0: return 0;
    case 1: return pkt[match->loff + 1];
    case 2: return pkt[match->loff + 1] | pkt[match->loff + 2] << 8;
    }
    return 0xFFFF;
}

PT_THREAD(hci_h4_rx_irq(struct hci_conn *c))
{
	unsigned dlen;
	struct kfifo *f = uart_rx_fifo();

	PT_BEGIN(&c->rx_irq);

	while (1) {
		c->readn = 1;
		c->curn = 0;
		READ_BYTE(c);

		if (c->rx_buf[0] > HCI_EVENT_PKT || c->rx_buf[0] < HCI_ACLDATA_PKT) {
			BLOGE("Invalid package type: %02x\n", c->rx_buf[0]);
			continue;
		}

		c->readn += h4_pkts[c->rx_buf[0]].hlen;
		READ_BYTE(c);
		dlen = h4_pkt_len(c->rx_buf, h4_pkts + c->rx_buf[0]);
        if (dlen > HCI_MAX_ACL_SIZE) {
			BLOGE("Invalid packet\n");
            continue;
		}

        c->readn += dlen;
        READ_BYTE(c);

		if (!pktf_in(&c->rx_pktf.f, c->rx_buf, c->readn)) {
#ifdef ENABLE_DEBUG
			BLOGL("pktf(%s) no space, request %d, avail = %d, pktf avail = %d\n",
				c->rx_pktf.f.name, c->readn, kfifo_avail(&c->rx_pktf.f.fifo), pktf_avail(&c->rx_pktf.f));
#endif
		}
	}

	PT_END(&c->rx_irq);
}

PT_THREAD(hci_h4_tx_work(struct hci_conn *c))
{
	unsigned size;
	uint8_t *buf;
	struct pktf *f;
	PT_BEGIN(&c->tx_work);

	while(1) {

		PT_WAIT_UNTIL(&c->tx_work, (acl_can_send(c) || cmd_can_send(c)) && !uart_tx_busy());

		if (acl_can_send(c)) {
			if (!c->acl_connected) {
				pktf_reset(&c->tx_pktf.f);
				continue;
			}
			f = &c->tx_pktf.f;
			c->acl_cnt--;
		} else if (cmd_can_send(c)) {
			f = &c->cmd_pktf.f;
		} else {
			continue;
		}

		buf = uart_dma_buffer();

		size = pktf_out(f, buf);
		uart_dma_tx_transfer(size);
	};

	PT_END(&c->tx_work);
}

void hci_h4_work(struct hci_conn *c)
{
	hci_h4_rx_irq(c);
	if ((c->inited & HCI_CONN_INIT_STEP_1)
		&& (!c->timeout
			|| TIMEOUT(c->timeout)))
	{
		TIMER_SETUP(c->timeout, D_MILLISECOND(100));
		hci_send_cmd(OGF_HOST_CTL, OCF_RESET, NULL, 0);
	}

	if (c->timeout && TIMEOUT(c->timeout) && c->reset_uart) {
		c->reset_uart = 0;
		TIMER_RESET(c->timeout);
		bt_uart_set_baudrate(HCI_BAUDRATE);
		bt_uart_set_flowctrl();
	}
}

void hci_h4_init(struct hci_conn *c)
{
	TIMER_SETUP(c->timeout, D_MILLISECOND(100));
}

#endif
