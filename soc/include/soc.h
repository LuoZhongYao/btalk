/*
 * Written by ZhongYao Luo <luozhongyao@gmail.com>
 *
 * Copyright 2020 ZhongYao Luo
 */

#ifndef __SOC_H__
#define __SOC_H__

#include <stdint.h>

void soc_init(void);

uint32_t crc32(const uint32_t *buf, unsigned size);
uint16_t crc16(const uint8_t *data, unsigned size);
uint16_t crc16_array(const uint8_t **datas, unsigned *size);

void flash_erase(unsigned addr);
void flash_write(unsigned addr, uint32_t *data, unsigned size);
void flash_read(unsigned addr, uint32_t *data, unsigned size);


#endif /* __SOC_H__*/

