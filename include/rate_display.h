#ifndef __RATE_DISPLAY_H__
#define __RATE_DISPLAY_H__

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include "time.h"
#include "debug.h"

struct rate_display
{
    uint64_t ra_size;
    uint64_t ra_time;
};

static __inline void rate_display(struct rate_display *dis, unsigned long size, const char *prompt, const char *uint, unsigned interval)
{
	uint64_t now = jiffies;
    dis->ra_size += size;
    if(dis->ra_time == 0) {
        dis->ra_time = now;
    } else if((now - dis->ra_time) >= D_SEC(interval)) {
        BLOGI("%s %" PRIu64 " %s/s\n", prompt, (dis->ra_size * D_SEC(1)) / (now - dis->ra_time), uint);
        dis->ra_time = now;
        dis->ra_size = 0;
    }
}

#endif
