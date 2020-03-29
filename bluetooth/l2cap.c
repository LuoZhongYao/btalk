#include "debug.h"
#include "pktf.h"
#include "sbc_codec.h"
#include <bluetooth/hci_lib.h>
#include <bluetooth/l2cap.h>
#include <string.h>

void l2cap_writev(struct bt_conn *con, uint16_t cid, const struct iovec iovec[], int iovcnt)
{
	unsigned size = iov_size(iovec, iovcnt);
	uint8_t hdr[4] = {
		size & 0xFF, (size >> 8) & 0xFF,
		cid & 0xff, (cid >> 8) & 0xff,
	};
	struct iovec iov[iovcnt + 1];
	iov[0].iov_base = hdr;
	iov[1].iov_len = 4;
	memcpy(iov + 1, iovec, sizeof(struct iovec) * iovcnt);

	acl_writev(con, iov, iovcnt + 1);
}

void l2cap_write(struct bt_conn *con, uint16_t cid, const void *buf, unsigned size)
{
	uint8_t hdr[4] = {
		size & 0xFF, (size >> 8) & 0xFF,
		cid & 0xff, (cid >> 8) & 0xff,
	};

	struct iovec iovec[2] = {
		{.iov_base = hdr, .iov_len = 4},
		{.iov_base = (void*)buf, .iov_len = size},
	};

	acl_writev(con, iovec, 2);
}

void l2cap_recv_frame(struct bt_conn *conn, struct l2cap_hdr *hdr)
{
	switch (hdr->cid) {

	default:
		BLOGE("Unhandle l2acp channel %04x\n", hdr->cid);
	break;

	case 1:
	break;

	case L2CAP_APP_CHANNEL:
		//app_l2cap_handler((app_command_hdr*)hdr->paylod, hdr->len);
	break;

	case L2CAP_SBC_CHANNEL:
		sbc_codec_dec(hdr->paylod, hdr->len);
	break;

#if defined(M480)
	case L2CAP_LOG_CHANNEL:
		BLOGD("Dongle[%d]: %.*s", hdr->paylod[0], hdr->len - 1, hdr->paylod + 1);
	break;
#endif
	}
}

