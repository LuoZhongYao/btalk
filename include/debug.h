/*************************************************
 * Anthor  : LuoZhongYao@gmail.com
 * Modified: 2019/07/01
 ************************************************/
#ifndef __DEBUG_H__
#define __DEBUG_H__

#define ENABLE_DEBUG

#ifdef ENABLE_DEBUG
#include <stdio.h>

# define LOGD(fmt, ...)	printf(fmt, ##__VA_ARGS__)
# define LOGW(fmt, ...)	printf(fmt, ##__VA_ARGS__)
# define LOGE(fmt, ...)	printf(fmt, ##__VA_ARGS__)

#define BLOGL(fmt, ...) do {\
	static uint64_t _limit = 0; \
	if (_limit == 0 || TIMEOUT(_limit)) { \
		BLOGD(fmt, ##__VA_ARGS__); \
		TIMER_SETUP(_limit, D_SEC(1));\
	} } while (0)

#define BLOGT(expr, fmt, ...) do {							\
	uint64_t _old = jiffies;								\
	{ expr; }												\
	BLOGD("%lld " fmt "\n", jiffies - _old, ##__VA_ARGS__);	\
} while (0)

#else /* ENABLE_DEBUG */

# define DebugInit()
# define LOGD(fmt, ...)
# define LOGW(fmt, ...)
# define LOGE(fmt, ...)
# define BLOGL(fmt, ...)
# define BLOGT(expr, fmt, ...) expr

#endif /* ENABLE_DEBUG */

#define BLOGI	LOGD
#define BLOGV	LOGD
#define BLOGD	LOGD
#define BLOGW	LOGW
#define BLOGE	LOGE
#define BD_STR	"%02x:%02x:%02x:%02x:%02x:%02x"
#define BD_FMT(bd) (bd)->b[5], (bd)->b[4], (bd)->b[3], (bd)->b[2], (bd)->b[1], (bd)->b[0]

#endif /* __DEBUG_H__*/

