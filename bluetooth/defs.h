/*************************************************
 * Anthor  : LuoZhongYao@gmail.com
 * Modified: 2019/07/18
 ************************************************/
#ifndef __DEFS_H__
#define __DEFS_H__

#include <stddef.h>

#define MIN(a, b) ((a) > (b) ? (b) : (a))
#define MAX(a, b) ((a) < (b) ? (b) : (a))

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#ifndef container_of
# define container_of(ptr, type, member) ({ \
	const typeof( ((type *)0)->member ) *__mptr = (ptr); \
	(type *)( (char *)__mptr - offsetof(type,member) );})
#endif

#ifndef __packed
#define __packed __attribute__((packed))
#endif

#define HTOLE16(n) (n)
#define HTOLE32(n) (n)
#define HTOLE64(n) (n)

struct iovec
{
	void *iov_base;
	unsigned iov_len;
};

static inline unsigned iov_size(const struct iovec iovec[], int iovcnt)
{
	int i;
	unsigned size = 0;
	for (i = 0;i < iovcnt;i++)
		size +=iovec[i].iov_len;
	return size;
}

#endif /* __DEFS_H__*/

