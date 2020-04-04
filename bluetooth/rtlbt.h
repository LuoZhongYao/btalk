/*************************************************
 * Anthor  : LuoZhongYao@gmail.com
 * Modified: 2019/08/26
 ************************************************/
#ifndef __RTLBT_H__
#define __RTLBT_H__

#include <bluetooth/hci.h>
#include <stdint.h>

void rtlbt_init(void);
void rtlbt_reset_complete(bdaddr_t *ba);
void rtlbt_vendor_cmd_complete(evt_cmd_complete *ev);
void rtlbt_vendor_evt(const uint8_t *ev, unsigned size);

#endif /* __RTLBT_H__*/

