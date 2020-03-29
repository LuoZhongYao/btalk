/*************************************************
 * Anthor  : LuoZhongYao@gmail.com
 * Modified: 2019/09/02
 ************************************************/
#ifndef __RDABT_H__
#define __RDABT_H__

#include <stdint.h>
#include <bluetooth/hci.h>

void rdabt_init(void);
void rdabt_reset_complete(bdaddr_t *ba);
void rdabt_vendor_cmd_complete(evt_cmd_complete *ev);
void rdabt_vendor_evt(const uint8_t *ev, unsigned size);

#endif /* __RDABT_H__*/

