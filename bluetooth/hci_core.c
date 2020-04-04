#include <config.h>

#include "btsel.h"
#include <debug.h>
#include <time.h>
#include <defs.h>
#include <string.h>
#include <bluetooth/l2cap.h>

#include "user_cb.h"
#include "hci_private.h"

#define DEV_NAME "Revolution Pro Controller 3"

#if defined(ENABLE_HCI_H4)
# define HCI_TX_WORK	hci_h4_tx_work
//# define HCI_RX_IRQ		hci_h4_rx_irq
# define HCI_RX_IRQ(...)
# define HCI_WORK		hci_h4_work
# define HCI_INIT		hci_h4_init
#elif defined(ENABLE_HCI_H5)
# define HCI_TX_WORK	hci_3wire_tx_work
//# define HCI_RX_IRQ		hci_3wire_rx_irq
# define HCI_RX_IRQ(...)
# define HCI_INIT		hci_3wire_init
# define HCI_WORK		hci_3wire_work
#else
# error "No HCI interface available"
#endif

static void l2cap_recv_acldata(struct bt_conn *c, const uint8_t *buf, unsigned size, uint8_t flags)
{
	uint16_t len;
	struct l2cap_hdr *hdr;

	switch (flags) {
	case ACL_START:
	case ACL_START_NO_FLUSH:
	case ACL_COMPLETE:
		if (c->rx_len) {
			BLOGE("Unexpected start frame (len %d)\n", c->rx_len);
			c->rx_len = 0;
		}

		if (size < L2CAP_HDR_SIZE) {
			BLOGE("Frame is too short (len %d)\n", size);
			return ;
		}

		hdr = (struct l2cap_hdr *) buf;
		len = hdr->len + L2CAP_HDR_SIZE;

		memcpy(c->l2cap, buf, size);
		c->rx_pos = c->l2cap + size;
		c->rx_len = len - size;
		if (len == size) {
			l2cap_recv_frame(c, (struct l2cap_hdr*)c->l2cap);
		}
	break;

	case ACL_CONT:
		if (!c->rx_len) {
			BLOGE("Unexpected continuation frame (len %d)\n", size);
			return ;
		}

		if (size > c->rx_len) {
			BLOGE("Fragment is too long (len %d, expected %d)\n", size, c->rx_len);
			c->rx_len = 0;
			c->rx_pos = NULL;
			return ;
		}

		memcpy(c->rx_pos, buf, size);
		c->rx_len -= size;
		c->rx_pos += size;

		if (!c->rx_len) {
			l2cap_recv_frame(c, (struct l2cap_hdr*)c->l2cap);
		}
	break;
	}
}

extern void hci_recv_event(struct event_hdr *hdr);
static inline void hci_rx_handler(struct hci_conn *c)
{
	//uint8_t rx_buf[517];
	SHARED_LOCK(c);

	pktf_out(&c->rx_pktf.f, c->shared_buff);
	switch (c->shared_buff[0]) {
	case HCI_ACLDATA_PKT: {
		struct bt_conn *conn;
		struct acl_hdr *hdr = (struct acl_hdr*)c->shared_buff;
		conn = bt_conn_lookup_by_handle(c, hdr->handle & 0x3FF);
		if (conn) {
			l2cap_recv_acldata(conn, hdr->payload, hdr->dlen, (hdr->handle >> 12 & 0xF));
		}
	} break;

	case HCI_EVENT_PKT: {
		struct event_hdr *hdr = (struct event_hdr*)c->shared_buff;
		hci_recv_event(hdr);
	} break;
	}

	SHARED_UNLOCK(c);
}

static PT_THREAD(hci_rx_work(struct hci_conn *c))
{
	PT_BEGIN(&c->rx_work);

	while (1) {
		PT_WAIT_UNTIL(&c->rx_work, pktf_len(&c->rx_pktf.f) != 0);
		hci_rx_handler(c);
	}

	PT_END(&c->rx_work);
}


struct hci_conn hci_conn;
struct iovr
{
	int iovcnt;
	int  rpos;
	void *rptr;
	unsigned rsize;
	const struct iovec *iovec;
};

bool iovr_empty(struct iovr *iovr)
{
	return (iovr->iovcnt <= iovr->rpos)
		|| ((iovr->iovcnt == (iovr->rpos + 1)) && !iovr->rsize);
}

static inline void iovr_copy(struct iovr *iovr, void *buf, unsigned size)
{
	memcpy(buf, iovr->rptr, size);
	if (size == iovr->rsize) {
		iovr->rpos++;
		if (iovr->rpos < iovr->iovcnt) {
			iovr->rptr = iovr->iovec[iovr->rpos].iov_base;
			iovr->rsize = iovr->iovec[iovr->rpos].iov_len;
		}
	} else {
		iovr->rsize -= size;
		iovr->rptr += size;
	}
}

unsigned iov_out(struct iovr *iovr, void *buf, unsigned size)
{
	unsigned rs = 0;
	unsigned rqs = size;
	while (rqs && !iovr_empty(iovr)) {
		unsigned cnt = MIN(rqs, iovr->rsize);
		iovr_copy(iovr, buf + rs, cnt);
		rs += cnt;
		rqs -= cnt;
	}
	return rs;
}

void  iov_init(struct iovr *iovr, const struct iovec iovec[], int iovcnt)
{
	iovr->iovec = iovec;
	iovr->iovcnt = iovcnt;
	iovr->rpos = 0;
	if (iovr->rpos < iovr->iovcnt) {
		iovr->rsize = iovec[0].iov_len;
		iovr->rptr = iovec[0].iov_base;
	}
}

void acl_writev(struct bt_conn *conn, const struct iovec iovec[], int iovcnt)
{
	uint16_t sent = 0;
	uint16_t len = iov_size(iovec, iovcnt);
	uint16_t acl_nr = (len + hci_conn.acl_mtu - 1) / hci_conn.acl_mtu;
	struct iovr iovr;
	struct acl_hdr *hdr = (struct acl_hdr*)hci_conn.shared_buff;

	if (!hci_conn.acl_connected || acl_nr > pktf_avail(&hci_conn.tx_pktf.f))
		return ;

	SHARED_LOCK(&hci_conn);

	hdr->packet = 0x02;
	hdr->handle = 0x2000;
	iov_init(&iovr, iovec, iovcnt);
	while (len) {
		uint16_t cnt = MIN(len, hci_conn.acl_mtu);
		hdr->handle |= conn->handle;

		cnt = iov_out(&iovr, hdr->payload, cnt);
		hdr->dlen = cnt;
		pktf_in(&hci_conn.tx_pktf.f, hdr, sizeof(*hdr) + cnt);

		hdr->handle = 0x1000;
		len -= cnt;
		sent += cnt;
	}

	SHARED_UNLOCK(&hci_conn);
}

void acl_write(struct bt_conn *conn, const void *buf, uint16_t size)
{
	uint16_t sent = 0;
	uint16_t len = size;
	uint16_t acl_nr = (size + hci_conn.acl_mtu - 1) / hci_conn.acl_mtu;
	struct acl_hdr *hdr = (struct acl_hdr*)hci_conn.shared_buff;

	if (!hci_conn.acl_connected || acl_nr > pktf_avail(&hci_conn.tx_pktf.f))
		return ;

	SHARED_LOCK(&hci_conn);

	hdr->packet = 0x02;
	hdr->handle = 0x2000;
	while (len) {
		uint16_t cnt = MIN(len, hci_conn.acl_mtu);
		hdr->dlen = cnt;
		hdr->handle |= conn->handle;
		memcpy(hdr->payload, buf + sent, cnt);
		pktf_in(&hci_conn.tx_pktf.f, hdr, sizeof(*hdr) + cnt);

		hdr->handle = 0x1000;
		len -= cnt;
		sent += cnt;
	}

	SHARED_UNLOCK(&hci_conn);
}

void l2cap_write_short(struct bt_conn *conn, uint16_t cid, const void *buf, unsigned size)
{
	struct {
		struct acl_hdr acl;
		struct l2cap_hdr l2c;
	} __attribute__((packed)) hdr;

	if (!hci_conn.acl_connected
		|| !pktf_avail(&hci_conn.tx_pktf.f)
		|| (size + 4) > hci_conn.acl_mtu)
		return ;

	hdr.acl.packet = 0x02;
	hdr.acl.handle = 0x2000 | conn->handle;
	hdr.acl.dlen = 4 + size;
	hdr.l2c.cid = cid;
	hdr.l2c.len = size;
	pktf_in2(&hci_conn.tx_pktf.f, &hdr, sizeof(hdr), buf, size);
}


int hci_send_cmd(uint16_t ogf, uint16_t ocf, const void *params, uint8_t plen)
{
	uint8_t hdr[4];
	uint16_t opcode = cmd_opcode_pack(ogf, ocf);

	hdr[0] = 0x01;
	hdr[1] = opcode & 0xFF;
	hdr[2] = opcode >> 8;
	hdr[3] = plen;

	return pktf_in2(&hci_conn.cmd_pktf.f, hdr, 4, params, plen) ? 0 : -1;
}

void hci_conn_reset_complete(void)
{
	if (hci_conn.inited & HCI_CONN_INIT_STEP_1) {
		hci_send_cmd(OGF_LE_CTL, OCF_LE_RAND, NULL, 0);
	}
}

void hci_conn_init(void)
{
	bdaddr_t ba;
	if (hci_conn.inited & HCI_CONN_INIT_STEP_1) {
		hci_conn.inited &= ~HCI_CONN_INIT_STEP_1;
		bt_get_bdaddr(&hci_conn, &ba);
		BTRESET_COMPLETE(&ba);
	}
}

void hci_conn_bthw_connected(void)
{
	union {
		uint8_t value;
		set_event_mask_cp evt_mask;
		change_local_name_cp	change_local_name;
		write_simple_pairing_mode_cp simple_pairing_mode;
		delete_stored_link_key_cp del_stored_link_key;
		write_page_activity_cp	page_activity;
		write_inq_activity_cp	inq_activity;
		write_page_timeout_cp	page_timeout;
		write_inquiry_mode_cp	inquiry_mode;
		host_buffer_size_cp		host_buff_size;
	} cp;

	if (hci_conn.inited & HCI_CONN_INIT_STEP_1) {
		hci_send_cmd(OGF_HOST_CTL, OCF_RESET, NULL, 0);
	} else if (!(hci_conn.inited & HCI_CONN_INIT_STEP_1)
		&& (hci_conn.inited & HCI_CONN_INIT_STEP_2)) {

		hci_send_cmd(OGF_INFO_PARAM, OCF_READ_LOCAL_FEATURES, NULL, 0);
		hci_send_cmd(OGF_INFO_PARAM, OCF_READ_BUFFER_SIZE, NULL, 0);

		memset(&cp.evt_mask.mask, 0xff, sizeof(cp.evt_mask.mask));
		cp.evt_mask.mask[6] = 0xbf;
		cp.evt_mask.mask[7] = 0x3d;
		hci_send_cmd(OGF_HOST_CTL, OCF_SET_EVENT_MASK, &cp, sizeof(cp.evt_mask));

		hci_conn_le_write_host_supported(0x01, 0x01);
		hci_conn_le_set_advertising_param(0x800, 0x800,
			0x03, 0x00, 0x00, BDADDR_ANY, 0x07, 0x00);

		memset(&cp, 0, sizeof(cp));
		cp.del_stored_link_key.delete_all = 1;
		hci_send_cmd(OGF_HOST_CTL, OCF_DELETE_STORED_LINK_KEY, &cp, sizeof(cp.del_stored_link_key));

		cp.host_buff_size.acl_mtu = 1040;
		cp.host_buff_size.sco_mtu = 64;
		cp.host_buff_size.acl_max_pkt = 100;
		cp.host_buff_size.sco_max_pkt = 10;
		hci_send_cmd(OGF_HOST_CTL, OCF_HOST_BUFFER_SIZE, &cp, sizeof(cp.host_buff_size));

		memset(&cp, 0, sizeof(cp));
		memcpy(cp.change_local_name.name, DEV_NAME, MIN(sizeof(DEV_NAME), HCI_MAX_NAME_LENGTH));
		hci_send_cmd(OGF_HOST_CTL, OCF_CHANGE_LOCAL_NAME, &cp, sizeof(cp.change_local_name));

		cp.inquiry_mode.mode = 1;
		hci_send_cmd(OGF_HOST_CTL, OCF_WRITE_INQUIRY_MODE, &cp, sizeof(cp.inquiry_mode));

		cp.value = SCAN_PAGE;
		hci_send_cmd(OGF_HOST_CTL, OCF_WRITE_SCAN_ENABLE, &cp, sizeof(cp.value));

		cp.page_activity.interval = 0x0400;
		cp.page_activity.window = 0x0024;
		hci_send_cmd(OGF_HOST_CTL, OCF_WRITE_PAGE_ACTIVITY, &cp, sizeof(cp.page_activity));

		cp.inq_activity.interval = 0x0400;
		cp.inq_activity.window = 0x0024;
		hci_send_cmd(OGF_HOST_CTL, OCF_WRITE_INQ_ACTIVITY, &cp, sizeof(cp.inq_activity));

		cp.simple_pairing_mode.mode = 1;
		hci_send_cmd(OGF_HOST_CTL, OCF_WRITE_SIMPLE_PAIRING_MODE, &cp, sizeof(cp.simple_pairing_mode));
	}
}

void hci_conn_init_complete(void)
{
	hci_conn.inited = 0;
	BLOGI("HCI connection init complete\n");
	bt_init_complete(&hci_conn);
}

void hci_conn_write_dev_class(const uint8_t dev_class[3])
{
	write_class_of_dev_cp cp;
	memcpy(&cp, dev_class, 3);
	hci_send_cmd(OGF_HOST_CTL, OCF_WRITE_CLASS_OF_DEV, &cp, sizeof(cp));
}

void hci_conn_inquiry(const uint8_t lap[3], uint8_t length, uint8_t num_rsp)
{
	inquiry_cp cp = {
		.length = length,
		.num_rsp = num_rsp,
	};
	memcpy(cp.lap, lap, 3);
	hci_send_cmd(OGF_LINK_CTL, OCF_INQUIRY, &cp, sizeof(cp));
}

void hci_conn_write_supervision_timeout(uint16_t timeout)
{
	write_link_supervision_timeout_cp cp = {
		.handle = hci_conn.handle,
		.timeout = timeout
	};
	hci_send_cmd(OGF_HOST_CTL, OCF_WRITE_LINK_SUPERVISION_TIMEOUT, &cp, sizeof(cp));
}

void hci_conn_write_scan_enable(uint8_t scan)
{
	hci_send_cmd(OGF_HOST_CTL, OCF_WRITE_SCAN_ENABLE, &scan, sizeof(scan));
}

void hci_conn_link_key_reply(bdaddr_t *ba, uint8_t link_key[16])
{
	link_key_reply_cp cp;
	memcpy(&cp.bdaddr, ba, sizeof(*ba));
	memcpy(cp.link_key, link_key, 16);
	hci_send_cmd(OGF_LINK_CTL, OCF_LINK_KEY_REPLY, &cp, sizeof(cp));
}

void hci_conn_link_key_neg_reply(bdaddr_t *ba)
{
	hci_send_cmd(OGF_LINK_CTL, OCF_LINK_KEY_NEG_REPLY, ba, sizeof(*ba));
}

void hci_conn_user_confirm_reply(bdaddr_t *ba)
{
	user_confirm_reply_cp cp = {
		.bdaddr = *ba,
	};
	hci_send_cmd(OGF_LINK_CTL, OCF_USER_CONFIRM_REPLY, &cp, sizeof(cp));
}

void hci_conn_user_confirm_neg_reply(bdaddr_t *ba)
{
	hci_send_cmd(OGF_LINK_CTL, OCF_USER_CONFIRM_NEG_REPLY, ba, sizeof(*ba));
}

void hci_conn_disconnect_request(uint8_t reason)
{
	disconnect_cp cp = {
		.handle = hci_conn.handle,
		.reason = reason,
	};

	if (hci_conn.acl_connected) {
		hci_send_cmd(OGF_LINK_CTL, OCF_DISCONNECT, &cp, sizeof(cp));
	}
}

void hci_conn_connection_cancel(bdaddr_t *ba)
{
	create_conn_cancel_cp cp = {
		.bdaddr = *ba,
	};
	hci_send_cmd(OGF_LINK_CTL, OCF_CREATE_CONN_CANCEL, &cp, sizeof(cp));
}

void hci_conn_read_local_version(void)
{
	hci_send_cmd(OGF_INFO_PARAM, OCF_READ_LOCAL_VERSION, NULL, 0);
}

void hci_conn_le_set_advertising_param(uint16_t min_interval, uint16_t max_interval,
	uint8_t advtype, uint8_t own_bdaddr_type, uint8_t direct_bdaddr_type,
	bdaddr_t *direct_bdaddr, uint8_t chan_map, uint8_t filter)
{
	le_set_advertising_parameters_cp cp = {
		.min_interval = htobs(min_interval),
		.max_interval = htobs(max_interval),
		.advtype = advtype,
		.own_bdaddr_type = own_bdaddr_type,
		.direct_bdaddr_type = direct_bdaddr_type,
		.direct_bdaddr = *direct_bdaddr,
		.chan_map = chan_map,
		.filter = filter,
	};

	hci_send_cmd(OGF_LE_CTL, OCF_LE_SET_ADVERTISING_PARAMETERS, &cp, sizeof(cp));
}

void hci_conn_le_set_advertising_data(uint8_t *data, uint8_t length)
{
	le_set_advertising_data_cp cp;

	cp.length = MIN(length, 31);
	memcpy(cp.data, data, cp.length);
	hci_send_cmd(OGF_LE_CTL, OCF_LE_SET_ADVERTISING_DATA, &cp, sizeof(cp));
}

void hci_conn_le_set_advertising_enable(uint8_t enable)
{
	hci_send_cmd(OGF_LE_CTL, OCF_LE_SET_ADVERTISE_ENABLE, &enable, sizeof(enable));
}

void hci_conn_le_set_scan_param(uint8_t type, uint16_t interval, uint16_t window,
	uint8_t own_bdaddr_type, uint8_t filter)
{
	le_set_scan_parameters_cp cp = {
		.type = type,
		.interval = htobs(interval),
		.window = htobs(window),
		.own_bdaddr_type = own_bdaddr_type,
		.filter = filter,
	};

	hci_send_cmd(OGF_LE_CTL, OCF_LE_SET_SCAN_PARAMETERS, &cp, sizeof(cp));
}

void hci_conn_le_set_scan_enable(uint8_t enable, uint8_t filter_dup)
{
	le_set_scan_enable_cp cp = {enable, filter_dup};
	hci_send_cmd(OGF_LE_CTL, OCF_LE_SET_SCAN_ENABLE, &cp, sizeof(cp));
}

void hci_conn_le_write_host_supported(uint8_t le_supp_host, uint8_t sim_le_host)
{
	write_le_host_supported_cp cp = {le_supp_host, sim_le_host};
	hci_send_cmd(OGF_HOST_CTL, OCF_WRITE_LE_HOST_SUPPORTED, &cp, sizeof(cp));
}

void btstack_run(void)
{
	hci_rx_work(&hci_conn);
	HCI_TX_WORK(&hci_conn);
	HCI_WORK(&hci_conn);
}

void hci_runtime_init(const struct btcb *btcb)
{
	memset(&hci_conn, 0, sizeof(hci_conn));
	PT_INIT(&hci_conn.rx_work);
	PT_INIT(&hci_conn.rx_irq);
	PT_INIT(&hci_conn.tx_work);
	pktf_init(&hci_conn.rx_pktf.f, hci_conn.rx_pktf_buf, sizeof(hci_conn.rx_pktf_buf), RX_PKTF_NR);
	pktf_init(&hci_conn.tx_pktf.f, hci_conn.tx_pktf_buf, sizeof(hci_conn.tx_pktf_buf), RX_PKTF_NR);
	pktf_init(&hci_conn.cmd_pktf.f, hci_conn.cmd_pktf_buf, sizeof(hci_conn.cmd_pktf_buf), CMD_PKTF_NR);
	hci_conn.inited = 3;
	hci_conn.btcb = btcb;

#ifdef ENABLE_DEBUG
	hci_conn.tx_pktf.f.name = "TX";
	hci_conn.rx_pktf.f.name = "RX";
#endif

	BTHW_INIT();
	HCI_INIT(&hci_conn);
}
