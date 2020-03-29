#include "debug.h"
#include "btsel.h"
#include "defs.h"
#include "hci_private.h"

#define get_ecc_payload(cc) (((uint8_t*)cc) + 3)

static void hci_cmd_complete_evt(evt_cmd_complete *ecc)
{
	if (cmd_opcode_ogf(ecc->opcode) == OGF_VENDOR_CMD) {
		BTVENDOR_CMD_COMPLETE(ecc);
	} else switch (ecc->opcode) {

	case cmd_opcode_pack(OGF_HOST_CTL, OCF_RESET):
		hci_conn_reset_complete();
	break;

	case cmd_opcode_pack(OGF_LE_CTL, OCF_LE_RAND): {
		le_rand_rp *rp = (le_rand_rp*)get_ecc_payload(ecc);
		//if (rp->status == 0)
		//	app_srand(rp->random & 0xFFFFFFFF);
		hci_conn_init();
	} break;

	case cmd_opcode_pack(OGF_INFO_PARAM, OCF_READ_BUFFER_SIZE): {
		read_buffer_size_rp *rp = (read_buffer_size_rp*)get_ecc_payload(ecc);
		hci_conn.acl_mtu = rp->acl_mtu;
		hci_conn.acl_pkts = rp->acl_max_pkt;
		hci_conn.acl_cnt = rp->acl_max_pkt;
	} break;

	case cmd_opcode_pack(OGF_HOST_CTL, OCF_WRITE_SIMPLE_PAIRING_MODE):
		hci_conn_init_complete();
	break;

	}
}

static void hci_cmd_status_evt(evt_cmd_status *rp)
{
	switch (rp->opcode) {
	case 0x0000:
#if defined(CSR8811)
		hci_conn_bthw_connected();
#endif
	break;
	}
}

static void hci_num_comp_pkts_evt(evt_num_comp_pkts *ev)
{
	struct {
		uint16_t hndl;
		uint16_t nr_of_hndl;
	} __attribute__((packed)) *p = (void*)(((uint8_t*)&ev->num_hndl) + 1);

	for (uint8_t i = 0;i < ev->num_hndl;i++, p++) {
		hci_conn.acl_cnt += p->nr_of_hndl;
	}

	hci_conn.acl_cnt = MIN(hci_conn.acl_cnt, hci_conn.acl_pkts);
}

static void hci_handle_le_mete_evt(evt_le_meta_event *ev)
{
	switch (ev->subevent) {
	case EVT_LE_ADVERTISING_REPORT:
	break;
	}
}

#define EVT_CB(_cb, _type) do {						\
	if (btcb && btcb->_cb) {	\
		btcb->_cb((_type *)hdr->payload);	\
	}												\
} while(0)

void hci_recv_event(struct event_hdr *hdr)
{
	const struct btcb *btcb = hci_conn.btcb;
	switch (hdr->evt) {
	default:
		BLOGE("unhandle hci event: %x\n", hdr->evt);
	break;

	case EVT_INQUIRY_RESULT:
	case EVT_EXTENDED_INQUIRY_RESULT:
	break;
	case EVT_INQUIRY_RESULT_WITH_RSSI: {
		if (btcb == NULL)
			break;

		for (uint8_t i = 0; i < hdr->payload[0];i++) {
			inquiry_info_with_rssi *info = (inquiry_info_with_rssi*)(hdr->payload + 1 + i * sizeof(inquiry_info_with_rssi));
			btcb->inquiry_result(info);
		}
	} break;

	case EVT_INQUIRY_COMPLETE:
		if (btcb && btcb->inquiry_complete) {
			btcb->inquiry_complete();
		}
	break;

	case EVT_CMD_STATUS:
		hci_cmd_status_evt((evt_cmd_status*)hdr->payload);
	break;

	case EVT_CMD_COMPLETE:
		hci_cmd_complete_evt((evt_cmd_complete*)hdr->payload);
	break;

	case EVT_CONN_COMPLETE:
		EVT_CB(conn_complete, evt_conn_complete);
	break;

	case EVT_CONN_REQUEST:
		EVT_CB(conn_request, evt_conn_request);
	break;

	case EVT_DISCONN_COMPLETE:
		EVT_CB(disconn_complete, evt_disconn_complete);
	break;

	case EVT_IO_CAPABILITY_REQUEST:
		EVT_CB(io_capability_request, evt_io_capability_request);
	break;

	case EVT_USER_CONFIRM_REQUEST:
		EVT_CB(user_confirm_request, evt_user_confirm_request);
	break;

	case EVT_SIMPLE_PAIRING_COMPLETE:
		EVT_CB(simple_pairing_complete, evt_simple_pairing_complete);
	break;

	case EVT_LINK_KEY_NOTIFY:
		EVT_CB(link_key_notify, evt_link_key_notify);
	break;

	case EVT_LINK_KEY_REQ:
		EVT_CB(link_key_request, evt_link_key_req);
	break;

	case EVT_NUM_COMP_PKTS:
		hci_num_comp_pkts_evt((evt_num_comp_pkts*)hdr->payload);
	break;

	case EVT_LE_META_EVENT:
		hci_handle_le_mete_evt((evt_le_meta_event *)hdr->payload);
	break;

	case EVT_VENDOR:
		BTVENDOR_EVENT(hdr->payload, hdr->plen);
	break;

	}
}
