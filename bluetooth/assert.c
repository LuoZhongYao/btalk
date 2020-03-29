/*
 * Written by ZhongYao Luo <luozhongyao@gmail.com>
 *
 * Copyright 2020 ZhongYao Luo
 */

#include <debug.h>

void __aeabi_assert(const char *expr, const char *file, const char *line)
{
	BLOGE("%s %s:%s\n", expr, file, line);
	while (1);
}
