/*
 * Written by ZhongYao Luo <luozhongyao@gmail.com>
 *
 * Copyright 2020 ZhongYao Luo
 */

#include "flash.h"
#include "chipsel.h"

void flash_erase(unsigned addr)
{
	FMC_Erase(addr);
}

void flash_write(unsigned addr, uint32_t *data, unsigned size)
{
	unsigned i = 0;
	for (i = 0; i < (size >> 2); i++) {
		FMC_Write(addr + (i << 2), data[i]);
	}
}

void flash_read(unsigned addr, uint32_t *data, unsigned size)
{
	unsigned i = 0;
	for (i = 0; i < (size >> 2); i++) {
		data[i] = FMC_Read(addr + (i << 2));
	}
}
