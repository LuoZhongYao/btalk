/*************************************************
 * Anthor  : LuoZhongYao@gmail.com
 * Modified: 2019/08/12
 ************************************************/
#ifndef __HCI_H4_H__
#define __HCI_H4_H__

#include <pt-1.4/pt.h>

#if defined(ENABLE_HCI_H4)

struct hci_conn;
void hci_h4_init(struct hci_conn *c);
void hci_h4_work(struct hci_conn *c);
PT_THREAD(hci_h4_rx_irq(struct hci_conn *c));
PT_THREAD(hci_h4_tx_work(struct hci_conn *c));

#endif

#endif /* __HCI_H4_H__*/

