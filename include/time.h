/*************************************************
 * Anthor  : LuoZhongYao@gmail.com
 * Modified: 2019/07/02
 ************************************************/
#ifndef __TIME_H__
#define __TIME_H__
#include <stdint.h>

extern volatile uint64_t jiffies;

#define D_MILLISECOND(n) ((n) * 25)
#define D_SEC(n)	((n) * D_MILLISECOND(1000))

#define TIMEOUT(n) ((n) && ((n) <= jiffies))
#define TIMER_VALID(n)	(n)
#define TIMER_SETUP(n, m) do { n = jiffies + m; } while(0)
#define TIMER_RESET(n)	do { n = 0; } while(0)
#define DELAY(hz) do { uint64_t _t; TIMER_SETUP(_t, hz); while(!TIMEOUT(_t));} while(0)

#endif /* __TIME_H__*/

