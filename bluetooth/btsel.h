/*************************************************
 * Anthor  : LuoZhongYao@gmail.com
 * Modified: 2019/08/26
 ************************************************/
#ifndef __BTSEL_H__
#define __BTSEL_H__
#include <config.h>

#define CSR8811		1
#define RTL8761		2
#define RDA5876		3
#define BLUEZ		4

#if CONFIG_BT_MODULE == CSR8811

# include "csr.h"
# define BTHW_INIT csr_init
# define BTRESET_COMPLETE csr_reset_complete
# define BTVENDOR_EVENT	csr_vendor_evt
# define BTVENDOR_CMD_COMPLETE(...)

#elif CONFIG_BT_MODULE == RTL8761

# include "rtlbt.h"
# define BTHW_INIT rtlbt_init
# define BTRESET_COMPLETE rtlbt_reset_complete
# define BTVENDOR_EVENT rtlbt_vendor_evt
# define BTVENDOR_CMD_COMPLETE rtlbt_vendor_cmd_complete

#elif CONFIG_BT_MODULE == RDA5876

# include "rdabt.h"
# define BTHW_INIT rdabt_init
# define BTRESET_COMPLETE rdabt_reset_complete
# define BTVENDOR_EVENT rdabt_vendor_evt
# define BTVENDOR_CMD_COMPLETE rdabt_vendor_cmd_complete

#elif CONFIG_BT_MODULE == BLUEZ

# define BTHW_INIT(...) 
# define BTRESET_COMPLETE(...) hci_conn_bthw_connected()
# define BTVENDOR_EVENT(...)
# define BTVENDOR_CMD_COMPLETE(...)

#endif /* RTL8761 */

#endif /* __BTSEL_H__*/

