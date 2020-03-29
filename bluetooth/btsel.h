/*************************************************
 * Anthor  : LuoZhongYao@gmail.com
 * Modified: 2019/08/26
 ************************************************/
#ifndef __BTSEL_H__
#define __BTSEL_H__
#include <config.h>

#if defined(CSR8811)

# include "csr.h"
# define BTHW_INIT csr_init
# define BTRESET_COMPLETE csr_reset_complete
# define BTVENDOR_EVENT	csr_vendor_evt
# define BTVENDOR_CMD_COMPLETE(...)

#elif defined(RTL8761)

# include "rtlbt.h"
# define BTHW_INIT rtlbt_init
# define BTRESET_COMPLETE rtlbt_reset_complete
# define BTVENDOR_EVENT rtlbt_vendor_evt
# define BTVENDOR_CMD_COMPLETE rtlbt_vendor_cmd_complete

#elif defined(RDA5876)

# include "rdabt.h"
# define BTHW_INIT rdabt_init
# define BTRESET_COMPLETE rdabt_reset_complete
# define BTVENDOR_EVENT rdabt_vendor_evt
# define BTVENDOR_CMD_COMPLETE rdabt_vendor_cmd_complete

#endif /* RTL8761 */

#endif /* __BTSEL_H__*/

