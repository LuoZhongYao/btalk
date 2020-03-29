/*************************************************
 * Anthor  : LuoZhongYao@gmail.com
 * Modified: 2019/06/28
 ************************************************/
#ifndef __FIFO_H__
#define __FIFO_H__

struct kfifo
{
	unsigned char *data;
	unsigned int mask;
	unsigned int in;
	unsigned int out;
};

int kfifo_init(struct kfifo *fifo, void *buffer, unsigned int size);
unsigned int kfifo_in(struct kfifo *fifo, const void *buf, unsigned int len);
unsigned int kfifo_put(struct kfifo *fifo, unsigned char ch);
unsigned int kfifo_get(struct kfifo *fifo, unsigned char *ch);
unsigned int kfifo_out_peek(struct kfifo *fifo, void *buf, unsigned int len);
unsigned int kfifo_out(struct kfifo *fifo, void *buf, unsigned int len);
unsigned int kfifo_len(struct kfifo *fifo);
unsigned int kfifo_avail(struct kfifo *fifo);
unsigned int kfifo_size(struct kfifo *fifo);
unsigned int kfifo_discard(struct kfifo *fifo, unsigned int len);
void kfifo_reset(struct kfifo *fifo);
void kfifo_reset_out(struct kfifo *fifo);

#endif /* __FIFO_H__*/

