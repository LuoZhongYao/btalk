/*
 * Written by ZhongYao Luo <luozhongyao@gmail.com>
 *
 * Copyright 2020 ZhongYao Luo
 */

#ifndef __OS_SUPPORT_CUSTOM_H__
#define __OS_SUPPORT_CUSTOM_H__

#include <debug.h>

#define OVERRIDE_SPEEX_FATAL
static inline void _speex_fatal(const char *str, const char *file, int line)
{
	BLOGE("Fatal (internal) error in %s, line %d: %s\n", file, line, str);
	while (1);
}

#define OVERRIDE_SPEEX_WARNING
static inline void speex_warning(const char *str)
{
	BLOGW("%s\n", str);
}

#define OVERRIDE_SPEEX_WARNING_INT
static inline void speex_warning_int(const char *str, int val)
{
	BLOGW("%s %d\n", str, val);
}

#endif /* __OS_SUPPORT_CUSTOM_H__*/

