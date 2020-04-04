/*
 * Written by ZhongYao Luo <luozhongyao@gmail.com>
 *
 * Copyright 2020 ZhongYao Luo
 */
#include <soc.h>
#include <crc.h>
#include "chipsel.h"

uint32_t crc32(const uint32_t *buf, unsigned size)
{
	unsigned i;
	CRC_Open(CRC_32, (CRC_CHECKSUM_RVS | CRC_CHECKSUM_COM), 0xFFFFFFFF, CRC_CPU_WDATA_32);
	for (i = 0;i < size / 4;i++)
		CRC_WRITE_DATA(buf[i]);
	return CRC_GetChecksum();
}

uint16_t crc16(const uint8_t *data, unsigned size)
{
	unsigned i;

	CRC_Open(CRC_CCITT, CRC_WDATA_RVS, 0xFFFF, CRC_CPU_WDATA_8);
	for (i = 0; i < size; i ++) {
		CRC_WRITE_DATA(data[i]);
	}

	return CRC_GetChecksum();
}

uint16_t crc16_array(const uint8_t **datas, unsigned *size)
{
	unsigned i;
	const uint8_t **p = datas;

	CRC_Open(CRC_CCITT, CRC_WDATA_RVS, 0xFFFF, CRC_CPU_WDATA_8);

	while (*p) {
		for ( i = 0; i < *size; i++) {
			CRC_WRITE_DATA((*p)[i]);
		}
		p++;
		size++;
	}

	return CRC_GetChecksum();
}
