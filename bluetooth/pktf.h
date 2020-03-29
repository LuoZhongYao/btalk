/*************************************************
 * Anthor  : LuoZhongYao@gmail.com
 * Modified: 2019/07/04
 ************************************************/
#ifndef __PKTF_H__
#define __PKTF_H__

#include "fifo.h"
#include "debug.h"

struct pktf
{
	unsigned in;
	unsigned out;
	unsigned mask;
	struct kfifo fifo;
#ifdef ENABLE_DEBUG
	const char *name;
#endif
	unsigned short pkt_size[0];
};

#define __TYPE_PKTF(n) union {struct pktf f; unsigned char _[sizeof(struct pktf) + sizeof(unsigned short) * n];}
int pktf_init(struct pktf *pktf, void *buf, unsigned bsize, unsigned pkt_num);
unsigned pktf_out_peek(struct pktf *pktf, void *buf);
unsigned pktf_out(struct pktf *pktf, void *buf);
unsigned pktf_in(struct pktf *pktf, const void *buf, unsigned size);
unsigned pktf_in2(struct pktf *pktf, const void *buf0, unsigned size0, const void *buf1, unsigned size1);
unsigned pktf_discard(struct pktf *pktf);
void pktf_reset(struct pktf *pktf);

static inline unsigned pktf_len(struct pktf *pktf)
{
	return pktf->in - pktf->out;
}

static inline unsigned pktf_size(struct pktf *pktf)
{
	return pktf->mask + 1;
}

static inline unsigned pktf_avail(struct pktf *pktf)
{
	return pktf_size(pktf) - pktf_len(pktf);
}


#endif /* __PKTF_H__*/

