#include "fifo.h"
#include <string.h>

#define min(a, b) ((a) < (b) ? (a) : (b))

static unsigned int kfifo_unused(struct kfifo *fifo)
{
	return (fifo->mask + 1) - (fifo->in - fifo->out);
}

int kfifo_init(struct kfifo *fifo, void *buffer, unsigned int size)
{
	if (size & (size - 1))
		return -1;

	fifo->in = 0;
	fifo->out = 0;
	fifo->mask = size - 1;
	fifo->data = buffer;

	return 0;
}

static void kfifo_copy_in(struct kfifo *fifo, const void *src,
	unsigned int len, unsigned int off)
{
	unsigned int size = fifo->mask + 1;
	unsigned int l;

	off &= fifo->mask;
	l = min(len, size - off);

	memcpy(fifo->data + off, src, l);
	memcpy(fifo->data, ((char*)src) + l, len - l);
}

unsigned int kfifo_in(struct kfifo *fifo,
	const void *buf, unsigned int len)
{
	unsigned int l;

	l = kfifo_unused(fifo);
	if (len > l)
		len = l;

	kfifo_copy_in(fifo, buf, len, fifo->in);
	fifo->in += len;
	return len;
}

unsigned int kfifo_put(struct kfifo *fifo, unsigned char ch)
{
	if (kfifo_unused(fifo)) {
		fifo->data[fifo->in & fifo->mask] = ch;
		fifo->in++;
		return 1;
	}
	return 0;
}

unsigned int kfifo_get(struct kfifo *fifo, unsigned char *ch)
{
	if (kfifo_len(fifo)) {
		*ch = fifo->data[fifo->out & fifo->mask];
		fifo->out++;
		return 1;
	}
	return 0;
}

static void kfifo_copy_out(struct kfifo *fifo, void *dst,
		unsigned int len, unsigned int off)
{
	unsigned int size = fifo->mask + 1;
	unsigned int l;

	off &= fifo->mask;
	l = min(len, size - off);

	memcpy(dst, fifo->data + off, l);
	memcpy(((char*)dst) + l, fifo->data, len - l);
}

unsigned int kfifo_out_peek(struct kfifo *fifo,
		void *buf, unsigned int len)
{
	unsigned int l;

	l = fifo->in - fifo->out;
	if (len > l)
		len = l;

	kfifo_copy_out(fifo, buf, len, fifo->out);
	return len;
}

unsigned int kfifo_out(struct kfifo *fifo,
		void *buf, unsigned int len)
{
	len = kfifo_out_peek(fifo, buf, len);
	fifo->out += len;
	return len;
}

unsigned int kfifo_len(struct kfifo *fifo)
{
	return fifo->in - fifo->out;
}

unsigned int kfifo_avail(struct kfifo *fifo)
{
	return kfifo_size(fifo) - kfifo_len(fifo);
}

unsigned int kfifo_size(struct kfifo *fifo)
{
	return fifo->mask + 1;
}

unsigned int kfifo_discard(struct kfifo *fifo, unsigned int len)
{
	len = min(len, fifo->in - fifo->out);
	fifo->out += len;
	return len;
}

void kfifo_reset(struct kfifo *fifo)
{
	fifo->out = fifo->in = 0;
}

void kfifo_reset_out(struct kfifo *fifo)
{
	fifo->out = fifo->in;
}
