/*
 * Written by ZhongYao Luo <luozhongyao@gmail.com>
 *
 * Copyright 2020 ZhongYao Luo
 */

#ifndef __SOC_CRC_H__
#define __SOC_CRC_H__

#include <stdint.h>

uint32_t crc32(const uint32_t *buf, unsigned size);
uint16_t crc16(const uint8_t *data, unsigned size);
uint16_t crc16_array(const uint8_t **datas, unsigned *size);

#endif /* __SOC_CRC_H__*/

