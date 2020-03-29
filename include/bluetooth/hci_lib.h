/*************************************************
 * Anthor  : LuoZhongYao@gmail.com
 * Modified: 2019/07/05
 ************************************************/
#ifndef __HCI_LIB_H__
#define __HCI_LIB_H__

#include <defs.h>
#include <byteswap.h>
#include <stdbool.h>
#include <string.h>
#include <bluetooth/conn.h>
#include <bluetooth/hci.h>

void acl_write(struct bt_conn *conn, const void *buf, uint16_t size);
void acl_writev(struct bt_conn *conn, const struct iovec iovec[], int iovcnt);

#define BDADDR_ANY	&((bdaddr_t) {{0, 0, 0, 0, 0, 0}})
#define BDADDR_ALL	&((bdaddr_t) {{0xff, 0xff, 0xff, 0xff, 0xff, 0xff}})
#define BDADDR_INVAL &((bdaddr_t) {{0x00, 0xff, 0xff, 0xff, 0xff, 0xff}})
static inline int bacmp(const bdaddr_t *ba, const bdaddr_t *bb)
{
	return memcmp(ba, bb, sizeof(*ba));
}

static inline void bacpy(bdaddr_t *dst, const bdaddr_t *src)
{
	memcpy(dst, src, sizeof(*dst));
}

static inline void bazero(bdaddr_t *ba)
{
	memset(ba, 0, sizeof(*ba));
}

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define htobs(d)  (d)
#define htobl(d)  (d)
#define htobll(d) (d)
#define btohs(d)  (d)
#define btohl(d)  (d)
#define btohll(d) (d)
#elif __BYTE_ORDER == __BIG_ENDIAN
#define htobs(d)  bswap_16(d)
#define htobl(d)  bswap_32(d)
#define htobll(d) bswap_64(d)
#define btohs(d)  bswap_16(d)
#define btohl(d)  bswap_32(d)
#define btohll(d) bswap_64(d)
#else
#error "Unknown byte order"
#endif

/* Bluetooth unaligned access */
#define bt_get_unaligned(ptr)			\
__extension__ ({				\
	struct __attribute__((packed)) {	\
		__typeof__(*(ptr)) __v;		\
	} *__p = (__typeof__(__p)) (ptr);	\
	__p->__v;				\
})

#define bt_put_unaligned(val, ptr)		\
do {						\
	struct __attribute__((packed)) {	\
		__typeof__(*(ptr)) __v;		\
	} *__p = (__typeof__(__p)) (ptr);	\
	__p->__v = (val);			\
} while(0)

#if __BYTE_ORDER == __LITTLE_ENDIAN
static inline uint64_t bt_get_le64(const void *ptr)
{
	return bt_get_unaligned((const uint64_t *) ptr);
}

static inline uint64_t bt_get_be64(const void *ptr)
{
	return bswap_64(bt_get_unaligned((const uint64_t *) ptr));
}

static inline uint32_t bt_get_le32(const void *ptr)
{
	return bt_get_unaligned((const uint32_t *) ptr);
}

static inline uint32_t bt_get_be32(const void *ptr)
{
	return bswap_32(bt_get_unaligned((const uint32_t *) ptr));
}

static inline uint16_t bt_get_le16(const void *ptr)
{
	return bt_get_unaligned((const uint16_t *) ptr);
}

static inline uint16_t bt_get_be16(const void *ptr)
{
	return bswap_16(bt_get_unaligned((const uint16_t *) ptr));
}

static inline void bt_put_le64(uint64_t val, const void *ptr)
{
	bt_put_unaligned(val, (uint64_t *) ptr);
}

static inline void bt_put_be64(uint64_t val, const void *ptr)
{
	bt_put_unaligned(bswap_64(val), (uint64_t *) ptr);
}

static inline void bt_put_le32(uint32_t val, const void *ptr)
{
	bt_put_unaligned(val, (uint32_t *) ptr);
}

static inline void bt_put_be32(uint32_t val, const void *ptr)
{
	bt_put_unaligned(bswap_32(val), (uint32_t *) ptr);
}

static inline void bt_put_le16(uint16_t val, const void *ptr)
{
	bt_put_unaligned(val, (uint16_t *) ptr);
}

static inline void bt_put_be16(uint16_t val, const void *ptr)
{
	bt_put_unaligned(bswap_16(val), (uint16_t *) ptr);
}

#elif __BYTE_ORDER == __BIG_ENDIAN
static inline uint64_t bt_get_le64(const void *ptr)
{
	return bswap_64(bt_get_unaligned((const uint64_t *) ptr));
}

static inline uint64_t bt_get_be64(const void *ptr)
{
	return bt_get_unaligned((const uint64_t *) ptr);
}

static inline uint32_t bt_get_le32(const void *ptr)
{
	return bswap_32(bt_get_unaligned((const uint32_t *) ptr));
}

static inline uint32_t bt_get_be32(const void *ptr)
{
	return bt_get_unaligned((const uint32_t *) ptr);
}

static inline uint16_t bt_get_le16(const void *ptr)
{
	return bswap_16(bt_get_unaligned((const uint16_t *) ptr));
}

static inline uint16_t bt_get_be16(const void *ptr)
{
	return bt_get_unaligned((const uint16_t *) ptr);
}

static inline void bt_put_le64(uint64_t val, const void *ptr)
{
	bt_put_unaligned(bswap_64(val), (uint64_t *) ptr);
}

static inline void bt_put_be64(uint64_t val, const void *ptr)
{
	bt_put_unaligned(val, (uint64_t *) ptr);
}

static inline void bt_put_le32(uint32_t val, const void *ptr)
{
	bt_put_unaligned(bswap_32(val), (uint32_t *) ptr);
}

static inline void bt_put_be32(uint32_t val, const void *ptr)
{
	bt_put_unaligned(val, (uint32_t *) ptr);
}

static inline void bt_put_le16(uint16_t val, const void *ptr)
{
	bt_put_unaligned(bswap_16(val), (uint16_t *) ptr);
}

static inline void bt_put_be16(uint16_t val, const void *ptr)
{
	bt_put_unaligned(val, (uint16_t *) ptr);
}
#else
#error "Unknown byte order"
#endif

#define GIAC	(uint8_t[]){0x33, 0x8b, 0x9e}
#define LIAC	(uint8_t[]){0x00, 0x8b, 0x9e}

struct btcb {
	void (*inquiry_result)(inquiry_info_with_rssi *info);
	void (*inquiry_complete)(void);
	void (*conn_request)(evt_conn_request *req);
	void (*conn_complete)(evt_conn_complete *com);
	void (*disconn_complete)(evt_disconn_complete *disconn);
	void (*io_capability_request)(evt_io_capability_request *req);
	void (*simple_pairing_complete)(evt_simple_pairing_complete *com);
	void (*link_key_notify)(evt_link_key_notify *notify);
	void (*link_key_request)(evt_link_key_req *req);
	void (*user_confirm_request)(evt_user_confirm_request *confirm);

	struct bt_conn* (*lookup_conn_by_peer_addr)(const bdaddr_t *peer_addr);
	struct bt_conn* (*lookup_conn_by_handle)(uint16_t handle);

	bool (*get_bdaddr)(bdaddr_t *ba);
	void (*init_complete)(void);
};

void btstack_run(void);
void hci_runtime_init(const struct btcb *btcb);
void hci_conn_write_scan_enable(uint8_t scan);
void hci_conn_write_dev_class(const uint8_t dev_class[3]);
void hci_conn_inquiry(const uint8_t lap[3], uint8_t length, uint8_t num_rsp);
void hci_conn_write_supervision_timeout(uint16_t timeout);
void hci_conn_link_key_neg_reply(bdaddr_t *ba);
void hci_conn_link_key_reply(bdaddr_t *ba, uint8_t link_key[16]);
void hci_conn_user_confirm_reply(bdaddr_t *ba);
void hci_conn_user_confirm_neg_reply(bdaddr_t *ba);
void hci_conn_disconnect_request(uint8_t reason);
void hci_conn_connection_cancel(bdaddr_t *ba);
void hci_conn_read_local_version(void);
void hci_conn_le_advertising_param(uint16_t min_interval, uint16_t max_interval,
	uint8_t advtype, uint8_t own_bdaddr_type, uint8_t direct_bdaddr_type,
	bdaddr_t *direct_bdaddr, uint8_t chan_map, uint8_t filter);
void hci_conn_le_set_advertising_data(uint8_t *data, uint8_t length);
void hci_conn_le_set_advertising_enable(uint8_t enable);
void hci_conn_le_set_scan_param(uint8_t type, uint16_t interval, uint16_t window,
	uint8_t own_bdaddr_type, uint8_t filter);
void hci_conn_le_set_scan_enable(uint8_t enable, uint8_t filter_dup);


int hci_send_cmd(uint16_t ogf, uint16_t ocf, const void *params, uint8_t plen);

#endif /* __HCI_LIB_H__*/

