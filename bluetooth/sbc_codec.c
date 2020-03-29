#include <fifo.h>
#include <pt-1.4/pt.h>
#include <sbc/sbc.h>
#include <sbc_codec.h>
#include <debug.h>
#include <time.h>
#include <string.h>
#include <bluetooth/l2cap.h>

struct codec_task
{
	unsigned ifrmlen:10;
	unsigned ofrmlen:10;

	struct pt pt;
	struct kfifo fifo;
	sbc_t	sbc;
	void (*prcoess)(const uint8_t *buf, unsigned size);
	uint8_t ebuf[1024];
	uint8_t kbuf[8 * 1024];
};

struct codec
{
	unsigned nrfrm:4;
	unsigned maxfrm:4;
	unsigned encoded:12;
	uint8_t  tbuf[1024];

	struct codec_task enc;
	struct codec_task dec;
};


static PT_THREAD(sbc_decoder_thread(struct codec *codec))
{
	unsigned decn, frmlen;

	PT_BEGIN(&codec->dec.pt);

	while (1) {
		PT_WAIT_UNTIL(&codec->dec.pt, kfifo_len(&codec->dec.fifo) >= codec->dec.ifrmlen);

		kfifo_out_peek(&codec->dec.fifo, codec->dec.ebuf, codec->dec.ifrmlen);

		frmlen = sbc_decode(&codec->dec.sbc, codec->dec.ebuf, codec->dec.ifrmlen,
			codec->tbuf, sizeof(codec->tbuf), &decn);

		kfifo_discard(&codec->dec.fifo, frmlen);
		/* TODO: assert(frmlen <= sizeof(ebuf) */
		codec->dec.ifrmlen = frmlen;

		//BLOGV("ifrmlen = %d, decn = %d\n", frmlen, decn);
		codec->dec.prcoess(codec->tbuf, decn);
	}

	PT_END(&codec->dec.pt);
}

static PT_THREAD(sbc_encoder_thread(struct codec *codec))
{
	int wn;
	PT_BEGIN(&codec->enc.pt);

	while (1) {

		PT_WAIT_UNTIL(&codec->enc.pt, kfifo_len(&codec->enc.fifo) >= codec->enc.ifrmlen);
		kfifo_out(&codec->enc.fifo, codec->tbuf, codec->enc.ifrmlen);
		sbc_encode(&codec->enc.sbc, codec->tbuf, codec->enc.ifrmlen,
			codec->enc.ebuf + codec->encoded, sizeof(codec->enc.ebuf) - codec->encoded, &wn);

		codec->encoded += wn;
		codec->nrfrm++;
		if (codec->nrfrm == codec->maxfrm) {

			codec->enc.ebuf[0] = (codec->encoded - 4) & 0xff;
			codec->enc.ebuf[1] = ((codec->encoded - 4) >> 8) & 0xff;
			codec->enc.ebuf[2] = L2CAP_SBC_CHANNEL & 0xff;
			codec->enc.ebuf[3] = (L2CAP_SBC_CHANNEL >> 8) & 0xff;
			codec->enc.prcoess(codec->enc.ebuf, codec->encoded);

			codec->encoded = 4;	/* addition l2cap hdr, ugly optimization */
			codec->nrfrm = 0;
		}
	}

	PT_END(&codec->enc.pt);
}

static struct codec codec = {0};

unsigned sbc_codec_enc(const uint8_t *buf, unsigned size)
{
	if (kfifo_avail(&codec.enc.fifo) < size)
		BLOGE("encodec overrun\n");

	return kfifo_in(&codec.enc.fifo, buf, size);
}

unsigned sbc_codec_enc_out(uint8_t *buf, unsigned size)
{
	return kfifo_out(&codec.enc.fifo, buf,size);
}

unsigned sbc_codec_dec(const uint8_t *buf, unsigned size)
{
	if (kfifo_avail(&codec.dec.fifo) < size)
		BLOGE("decode overrun\n");

	return kfifo_in(&codec.dec.fifo, buf, size);
}

unsigned sbc_codec_dec_out(uint8_t *buf, unsigned size)
{
	return kfifo_out(&codec.dec.fifo, buf, size);
}

void sbc_codec_run(void)
{
	sbc_decoder_thread(&codec);
	sbc_encoder_thread(&codec);
}

void sbc_codec_init(struct a2dp_sbc *sbc, unsigned payloadsize,
	void (*enc)(const uint8_t *buf, unsigned size),
	void (*dec)(const uint8_t *buf, unsigned size))
{
	memset(&codec, 0, sizeof(codec));

	sbc_init(&codec.dec.sbc,  0L);
	sbc_init_a2dp(&codec.enc.sbc, 0L, sbc, sizeof(*sbc));

	kfifo_init(&codec.enc.fifo, codec.enc.kbuf, sizeof(codec.enc.kbuf));
	kfifo_init(&codec.dec.fifo, codec.dec.kbuf, sizeof(codec.dec.kbuf));

	codec.dec.prcoess = dec;
	codec.enc.prcoess = enc;

	PT_INIT(&codec.enc.pt);
	PT_INIT(&codec.dec.pt);

	codec.encoded = 4;	/* addition l2cap hdr, ugly optimization */
	codec.enc.ifrmlen = sbc_get_codesize(&codec.enc.sbc);
	codec.enc.ofrmlen = sbc_get_frame_length(&codec.enc.sbc);
	codec.dec.ofrmlen = 512;
	codec.dec.ifrmlen = 513;
	codec.maxfrm = payloadsize / codec.enc.ofrmlen;
}

void sbc_codec_reset(void)
{
	kfifo_reset_out(&codec.enc.fifo);
	kfifo_reset_out(&codec.dec.fifo);
}
