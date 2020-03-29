#include <config.h>

#include <crc.h>
#include <time.h>
#include <bt_uart.h>
#include <debug.h>

#include "hci_private.h"

#if defined(ENABLE_HCI_H5)

#define H5_HDR_SEQ(hdr)		((hdr)[0] & 0x07)
#define H5_HDR_ACK(hdr)		(((hdr)[0] >> 3) & 0x07)
#define H5_HDR_CRC(hdr)		(((hdr)[0] >> 6) & 0x01)
#define H5_HDR_RELIABLE(hdr)	(((hdr)[0] >> 7) & 0x01)
#define H5_HDR_PKT_TYPE(hdr)	((hdr)[1] & 0x0f)
#define H5_HDR_LEN(hdr)		((((hdr)[1] >> 4) & 0x0f) + ((hdr)[2] << 4))

#define HCI_3WIRE_ACK_PKT   0
#define HCI_3WIRE_LINK_PKT 15

#define SLIP_DELIMITER	0xc0
#define SLIP_ESC		0xdb
#define SLIP_ESC_DELIM	0xdc
#define SLIP_ESC_ESC	0xdd

#define H5_RX_ESC		0x01
#define H5_TX_ACK_SEQ	0x02
#define H5_CRC			0x04
#define H5_TX_WAIT_ACK	0x08

#define H5_LINK_TIMEOUT	D_MILLISECOND(100)
#define H5_TX_ACK_TIMEOUT D_MILLISECOND(100)

static void hci_3wire_reset(struct hci_conn *c)
{
	c->h5.state = H5_UNINITIALIZED;
	c->h5.tx_seq = 0;
	c->h5.tx_ack = 0;
	c->h5.flags &= ~H5_TX_WAIT_ACK;
	c->h5.tx_queue_size = 0;
}

static inline int unslip_one_byte(struct hci_conn *c, uint8_t ch)
{
	uint8_t byte = ch;
	if (!(c->h5.flags & H5_RX_ESC) && ch == SLIP_ESC) {
		c->h5.flags |= H5_RX_ESC;
		return 0;
	}

	if (ch == SLIP_DELIMITER)
		return 1;

	if (c->h5.flags & H5_RX_ESC) {
		c->h5.flags &= ~H5_RX_ESC;
		switch (ch) {
		case SLIP_ESC_DELIM:
			byte = SLIP_DELIMITER;
		break;

		case SLIP_ESC_ESC:
			byte = SLIP_ESC;
		break;

		default:
			return 1;
		break;
		}
	}
	c->rx_buf[c->curn++] = byte;
	return 0;
}

static inline void hci_3wire_recv_frame(struct hci_conn *c)
{
	const uint8_t *data;
	const uint8_t sync_rsp[] = {0x02, 0x7d};
	const uint8_t conf_rsp[] = {0x04, 0x7b};
	const uint8_t conf_req[3] = {0x03, 0xfc, 0x01};

	switch (H5_HDR_PKT_TYPE(c->rx_buf)) {
	case HCI_EVENT_PKT:
	case HCI_ACLDATA_PKT:
	case HCI_SCODATA_PKT:
		c->rx_buf[3] = H5_HDR_PKT_TYPE(c->rx_buf);
		if (!pktf_in(&c->rx_pktf.f, c->rx_buf + 3, c->readn - 3)) {
#ifdef ENABLE_DEBUG
			BLOGL("pktf(%s) no space, request %d, avail = %d, pktf avail = %d\n",
				c->rx_pktf.f.name, c->readn, kfifo_avail(&c->rx_pktf.f.fifo), pktf_avail(&c->rx_pktf.f));
#endif
		}

	break;

	case HCI_3WIRE_LINK_PKT:
		data = c->rx_buf + 4;
		if (data[0] == 0x01 && data[1] == 0x7e) { /* sync req */
			BLOGD("3wire sync req\n");
			if (c->h5.state == H5_ACTIVE)
				hci_3wire_reset(c);
			TIMER_SETUP(c->h5.timeout, H5_LINK_TIMEOUT);
			hci_3wire_send_frame(&c->h5, HCI_3WIRE_LINK_PKT, sync_rsp, 2);
		} else if (data[0] == 0x02 && data[1] == 0x7d) {	/* sync rsp */
			BLOGD("3wire sync rsp\n");
			if (c->h5.state == H5_ACTIVE)
				hci_3wire_reset(c);
			c->h5.state = H5_INITIALIZED;
			hci_3wire_send_frame(&c->h5, HCI_3WIRE_LINK_PKT, conf_req, 3);
		} else if (data[0] == 0x03 && data[1] == 0xfc) {	/* conf req */
			BLOGD("3wire conf req\n");
			if (c->h5.state != H5_UNINITIALIZED)
				hci_3wire_send_frame(&c->h5, HCI_3WIRE_LINK_PKT, conf_rsp, 2);
		} else if (data[0] == 0x04 && data[1] == 0x7b) {	/* conf rsp */
			BLOGD("3wire conf rsp\n");
			if (H5_HDR_LEN(c->rx_buf) > 5 && data[3] & 0x10)
				c->h5.flags |= H5_CRC;
			c->h5.state = H5_ACTIVE;
			TIMER_RESET(c->h5.timeout);
		} else if (data[0] == 0x05 && data[1] == 0xfa) {	/* sleep req */
			c->h5.sleep = H5_SLEEPING;
		} else if (data[0] == 0x06 && data[1] == 0xf9) {	/* woken req */
			c->h5.sleep = H5_AWAKE;
		} else if (data[0] == 0x07 && data[1] == 0x78) {	/* wakeup req */
		}
	break;
	}
}

#define READ_BYTE(c) \
		do {  while(c->curn < c->readn) { \
			PT_WAIT_UNTIL(&c->rx_irq, kfifo_len(f) > 0); \
			kfifo_get(f, &ch); \
			if (unslip_one_byte(c, ch) == 1) \
				goto again; \
		} } while(0)

static void hci_3wire_cull(struct hci_3wire *c)
{
	if (c->tx_seq == c->rx_ack
		&& c->state == H5_ACTIVE
		&& c->flags & H5_TX_WAIT_ACK)
	{
		c->tx_queue_size = 0;
		c->flags &= ~H5_TX_WAIT_ACK;
		TIMER_RESET(c->timeout);
	}
}

PT_THREAD(hci_3wire_rx_irq)(struct hci_conn *c)
{
	uint8_t ch;
	struct kfifo *f = uart_rx_fifo();
	PT_BEGIN(&c->rx_irq);

	while (1) {
again:
		do {
			PT_WAIT_UNTIL(&c->rx_irq, kfifo_len(f) > 0);
			kfifo_get(f, &ch);
		} while (ch != SLIP_DELIMITER);

		do {
			PT_WAIT_UNTIL(&c->rx_irq, kfifo_len(f) > 0);
			kfifo_get(f, &ch);
		} while (ch == SLIP_DELIMITER);

		c->h5.flags &= ~H5_RX_ESC;

		c->readn = 4;
		c->curn = 0;

		if (unslip_one_byte(c, ch) == 1)
			goto again;

		READ_BYTE(c);

		if (((c->rx_buf[0] + c->rx_buf[1] + c->rx_buf[2] + c->rx_buf[3]) & 0xff) != 0xff) {
			BLOGE("3wire header crc error\n");
			goto again;
		}

		if (H5_HDR_RELIABLE(c->rx_buf) && H5_HDR_SEQ(c->rx_buf) != c->h5.tx_ack) {
			if (((H5_HDR_SEQ(c->rx_buf) + 1) & 0x7) == c->h5.tx_ack)
				c->h5.flags |= H5_TX_ACK_SEQ;

			BLOGE("3wire SEQ(%d) != ACK(%d), TX SEQ = %d, RX ACK = %d, uart_tx_busy = %d\n",
				H5_HDR_SEQ(c->rx_buf), c->h5.tx_ack, c->h5.tx_seq, c->h5.rx_ack, uart_tx_busy());
			goto again;
		}

		if (c->h5.state != H5_ACTIVE && H5_HDR_PKT_TYPE(c->rx_buf) != HCI_3WIRE_LINK_PKT) {
			BLOGE("h5 state = %d, pkt type = %d\n", c->h5.state, H5_HDR_PKT_TYPE(c->rx_buf));
			goto again;
		}

		c->readn += H5_HDR_LEN(c->rx_buf);
		READ_BYTE(c);
		if (H5_HDR_CRC(c->rx_buf)) {
			c->readn += 2;
			READ_BYTE(c);
			c->readn -= 2;
		}

		if (H5_HDR_RELIABLE(c->rx_buf)) {
			c->h5.tx_ack = (c->h5.tx_ack + 1) & 0x7;
			c->h5.flags |= H5_TX_ACK_SEQ;
		}

		c->h5.rx_ack = H5_HDR_ACK(c->rx_buf);
		hci_3wire_cull(&c->h5);

		hci_3wire_recv_frame(c);
	}

	PT_END(&c->rx_irq);
}

static inline uint8_t *slip_delim(uint8_t *ptr)
{
	*ptr++ = SLIP_DELIMITER;
	return ptr;
}

static inline uint8_t *slip_one_byte(uint8_t *ptr, uint8_t c)
{
	switch (c) {
	case SLIP_DELIMITER:
		*ptr++ = SLIP_ESC;
		*ptr++ = SLIP_ESC_DELIM;
	break;

	case SLIP_ESC:
		*ptr++ = SLIP_ESC;
		*ptr++ = SLIP_ESC_ESC;
	break;

	default:
		*ptr++ = c;
	break;
	}

	return ptr;
}

PT_THREAD(hci_3wire_tx_work(struct hci_conn *c))
{
	PT_BEGIN(&c->tx_work);
	struct pktf *f;

	while(1) {
		PT_WAIT_UNTIL(&c->tx_work, (acl_can_send(c) || cmd_can_send(c))
				&& (c->h5.tx_queue_size == 0));

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

		c->h5.tx_queue_size = (uint16_t)pktf_out(f, c->h5.tx_queue_buf);
	}

	PT_END(&c->tx_work);
}

void hci_3wire_send_frame(struct hci_3wire *c, uint8_t pkt_type,
	const uint8_t *data, unsigned len)
{
	unsigned i;
	uint8_t *buf;
	uint8_t *ptr;
	uint8_t hdr[4];
	uint16_t crc;

	buf = uart_dma_buffer();
	if (buf == NULL)
		return ;

	ptr = buf;

	ptr = slip_delim(ptr);

	hdr[0] = c->tx_ack << 3;

	if ((c->flags & H5_CRC) && pkt_type != HCI_3WIRE_LINK_PKT)
		hdr[0] |= 1 << 6;

	if (pkt_type == HCI_ACLDATA_PKT || pkt_type == HCI_COMMAND_PKT) {
		hdr[0] |= 1 << 7;
		hdr[0] |= c->tx_seq;
		c->tx_seq = (c->tx_seq + 1) & 0x7;
	}

	hdr[1] = pkt_type | ((len & 0x0f) << 4);
	hdr[2] = len >> 4;
	hdr[3] = ~((hdr[0] + hdr[1] + hdr[2]) & 0xff);

	for (i = 0; i < sizeof(hdr); i++)
		ptr = slip_one_byte(ptr, hdr[i]);

	for (i = 0; i < len; i++)
		ptr = slip_one_byte(ptr, data[i]);

	if (H5_HDR_CRC(hdr)) {
		crc = crc16_array((const uint8_t *[]){hdr, data, NULL},
			(unsigned []){4, len, 0});
		ptr = slip_one_byte(ptr, (crc >> 8) & 0xFF);
		ptr = slip_one_byte(ptr, crc & 0xFF);
	}

	ptr = slip_delim(ptr);
	uart_dma_tx_transfer(ptr - buf);
}

void hci_3wire_work(struct hci_conn *c)
{
	const uint8_t sync_req[] = {0x01, 0x7e};
	const uint8_t conf_req[3] = {0x03, 0xfc, 0x01};
	unsigned size, flags;

	hci_3wire_rx_irq(c);

	if (TIMEOUT(c->timeout) && c->reset_uart) {
		c->reset_uart = 0;
		UART_Reset(HCI_BAUDRATE);
		hci_3wire_reset(c);
		TIMER_RESET(c->timeout);
		TIMER_SETUP(c->h5.timeout, H5_LINK_TIMEOUT);
	}

	if (TIMEOUT(c->h5.timeout)) {
		if (c->h5.state != H5_ACTIVE)
			TIMER_SETUP(c->h5.timeout, H5_LINK_TIMEOUT);

		if (c->h5.state == H5_UNINITIALIZED) {
			hci_3wire_send_frame(&c->h5, HCI_3WIRE_LINK_PKT, sync_req, sizeof(sync_req));
			return ;
		}

		if (c->h5.state == H5_INITIALIZED) {
			hci_3wire_send_frame(&c->h5, HCI_3WIRE_LINK_PKT, conf_req, sizeof(conf_req));
			return ;
		}

		if (c->h5.state == H5_ACTIVE && c->h5.flags & H5_TX_WAIT_ACK)
		{
			BLOGD("retransmit SEQ %d, ACK %d, RX ACK %d\n", c->h5.tx_seq, c->h5.tx_ack, c->h5.rx_ack);
			c->h5.flags &= ~H5_TX_WAIT_ACK;
			c->h5.tx_seq = (c->h5.tx_seq - 1) & 0x7;
		}

		TIMER_RESET(c->h5.timeout);
	}

	if (uart_tx_busy() || c->h5.state != H5_ACTIVE)
		return ;

	size = c->h5.tx_queue_size;
	flags = c->h5.flags;
	if(size && !(flags & H5_TX_WAIT_ACK)) {
		c->h5.flags &= ~H5_TX_ACK_SEQ;
		c->h5.flags |= H5_TX_WAIT_ACK;

		hci_3wire_send_frame(&c->h5, c->h5.tx_queue_buf[0], c->h5.tx_queue_buf + 1, size - 1);
		TIMER_SETUP(c->h5.timeout, H5_TX_ACK_TIMEOUT);
	}

	if ((c->h5.flags & H5_TX_ACK_SEQ))
	{
		c->h5.flags &= ~H5_TX_ACK_SEQ;
		hci_3wire_send_frame(&c->h5, HCI_3WIRE_ACK_PKT, NULL, 0);
	}
}

void hci_3wire_init(struct hci_conn *c)
{
	TIMER_SETUP(c->h5.timeout, H5_LINK_TIMEOUT);
}

#endif
