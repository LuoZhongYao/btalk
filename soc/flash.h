/*
 * Written by ZhongYao Luo <luozhongyao@gmail.com>
 *
 * Copyright 2020 ZhongYao Luo
 */

#ifndef __FLASH_H__
#define __FLASH_H__

#include <stdint.h>

void flash_erase(unsigned addr);
void flash_write(unsigned addr, uint32_t *data, unsigned size);
void flash_read(unsigned addr, uint32_t *data, unsigned size);

#endif /* __FLASH_H__*/

