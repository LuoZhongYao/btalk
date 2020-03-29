/*************************************************
 * Anthor  : LuoZhongYao@gmail.com
 * Modified: 2019/07/04
 ************************************************/
#ifndef __L2CAP_H__
#define __L2CAP_H__

#include "defs.h"
#include <stdint.h>
#include <bluetooth/conn.h>

struct l2cap_hdr {
	uint16_t     len;
	uint16_t     cid;
	uint8_t paylod[0];
} __attribute__((packed));
#define L2CAP_HDR_SIZE		4
#define L2CAP_ENH_HDR_SIZE	6
#define L2CAP_EXT_HDR_SIZE	8

#define L2CAP_SBC_CHANNEL	0xFFFD
#define L2CAP_APP_CHANNEL	0xFFFC
#define L2CAP_LOG_CHANNEL	0xFFFE

void sbc_write(const uint8_t *buf, unsigned size);
void l2cap_write(struct bt_conn *conn, uint16_t cid, const void *buf, unsigned size);
void l2cap_writev(struct bt_conn *conn, uint16_t cid, const struct iovec *iovec, int iovcnt);
void l2cap_write_short(struct bt_conn *conn, uint16_t cid, const void *buf, unsigned size);
void l2cap_recv_frame(struct bt_conn *conn, struct l2cap_hdr *hdr);


#endif /* __L2CAP_H__*/

