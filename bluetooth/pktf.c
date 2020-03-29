#include "pktf.h"
#include "debug.h"
#include <string.h>

int pktf_init(struct pktf *pktf, void *buf, unsigned bsize, unsigned pkt_num)
{
	if (pkt_num & (pkt_num - 1) || (bsize & (bsize - 1)))
		return -1;
	kfifo_init(&pktf->fifo, buf, bsize);
	pktf->in = 0;
	pktf->out = 0;
	pktf->mask = pkt_num - 1;

#if defined(ENABLE_DEBUG)
	pktf->name = "Nil";
#endif

	return 0;
}

unsigned pktf_out_peek(struct pktf *pktf, void *buf)
{
	unsigned size = pktf->pkt_size[pktf->out & pktf->mask];

	if (pktf_len(pktf) == 0)
		return 0;

	if (kfifo_len(&pktf->fifo) < size) {
		BLOGE("%s out peek, pktf state that should not occur, size %d, length = %d\n",
			pktf->name, size, kfifo_len(&pktf->fifo));
		return 0;
	}
	kfifo_out_peek(&pktf->fifo, buf, size);

	return size;
}

unsigned pktf_out(struct pktf *pktf, void *buf)
{
	unsigned size = pktf->pkt_size[pktf->out & pktf->mask];
	if (pktf_len(pktf) == 0)
		return 0;

	if (kfifo_len(&pktf->fifo) < size) {
		BLOGE("%s out, pktf state that should not occur, size %d, length = %d\n",
			pktf->name, size, kfifo_len(&pktf->fifo));
		return 0;
	}
	kfifo_out(&pktf->fifo, buf, size);
	pktf->out++;

	return size;
}

unsigned pktf_in(struct pktf *pktf, const void *buf, unsigned size)
{
	if (kfifo_avail(&pktf->fifo) < size || pktf_avail(pktf) == 0) {
		return 0;
	}

	pktf->pkt_size[pktf->in & pktf->mask] = size;
	kfifo_in(&pktf->fifo, buf, size);
	pktf->in++;
	return size;
}

unsigned pktf_in2(struct pktf *pktf, const void *buf0, unsigned size0, const void *buf1, unsigned size1)
{
	unsigned size = size0 + size1;
	if (kfifo_avail(&pktf->fifo) < size || pktf_avail(pktf) == 0) {
		return 0;
	}

	pktf->pkt_size[pktf->in & pktf->mask] = size;
	kfifo_in(&pktf->fifo, buf0, size0);
	kfifo_in(&pktf->fifo, buf1, size1);
	pktf->in++;

	return size;
}

unsigned pktf_discard(struct pktf *pktf)
{
	unsigned size = pktf->pkt_size[pktf->out & pktf->mask];

	if (pktf_len(pktf) == 0)
		return 0;

	if (kfifo_len(&pktf->fifo) < size) {
		BLOGE("%s discard, pktf state that should not occur, size %d, length = %d\n",
			pktf->name, size, kfifo_len(&pktf->fifo));
		return 0;
	}

	kfifo_discard(&pktf->fifo, size);
	pktf->out++;

	return size;
}

void pktf_reset(struct pktf *pktf)
{
	pktf->out = pktf->in;
	kfifo_reset_out(&pktf->fifo);
}
