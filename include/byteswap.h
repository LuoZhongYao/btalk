/*************************************************
 * Anthor  : LuoZhongYao@gmail.com
 * Modified: 2019/09/02
 ************************************************/
#ifndef __BYTESWAP_H__
#define __BYTESWAP_H__

#define bswap_16(value) \
	((((value) & 0xff) << 8) | ((value) >> 8))

#define bswap_32(value) \
	(((uint32_t)bswap_16((uint16_t)((value) & 0xffff)) << 16) | \
	 (uint32_t)bswap_16((uint16_t)((value) >> 16)))

#define bswap_64(value) \
	(((uint64_t)bswap_32((uint32_t)((value) & 0xffffffff)) \
	  << 32) | \
	  (uint64_t)bswap_32((uint32_t)((value) >> 32)))


#endif /* __BYTESWAP_H__*/

