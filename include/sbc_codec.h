/*************************************************
 * Anthor  : LuoZhongYao@gmail.com
 * Modified: 2019/07/04
 ************************************************/
#ifndef __SBC_CODEC_H__
#define __SBC_CODEC_H__

#include <fifo.h>
#include <stdint.h>
#include <sbc/sbc.h>
#include <pt-1.4/pt.h>

#define SBC_SYNCWORD	0x9C

#define MSBC_SYNCWORD	0xAD
#define MSBC_BLOCKS	15

#define A2DP_SAMPLING_FREQ_16000		(1 << 3)
#define A2DP_SAMPLING_FREQ_32000		(1 << 2)
#define A2DP_SAMPLING_FREQ_44100		(1 << 1)
#define A2DP_SAMPLING_FREQ_48000		(1 << 0)

#define A2DP_CHANNEL_MODE_MONO			(1 << 3)
#define A2DP_CHANNEL_MODE_DUAL_CHANNEL		(1 << 2)
#define A2DP_CHANNEL_MODE_STEREO		(1 << 1)
#define A2DP_CHANNEL_MODE_JOINT_STEREO		(1 << 0)

#define A2DP_BLOCK_LENGTH_4			(1 << 3)
#define A2DP_BLOCK_LENGTH_8			(1 << 2)
#define A2DP_BLOCK_LENGTH_12			(1 << 1)
#define A2DP_BLOCK_LENGTH_16			(1 << 0)

#define A2DP_SUBBANDS_4				(1 << 1)
#define A2DP_SUBBANDS_8				(1 << 0)

#define A2DP_ALLOCATION_SNR			(1 << 1)
#define A2DP_ALLOCATION_LOUDNESS		(1 << 0)

#if __BYTE_ORDER == __LITTLE_ENDIAN

struct a2dp_sbc {
	uint8_t channel_mode:4;
	uint8_t frequency:4;
	uint8_t allocation_method:2;
	uint8_t subbands:2;
	uint8_t block_length:4;
	uint8_t min_bitpool;
	uint8_t max_bitpool;
} __attribute__ ((packed));

#elif __BYTE_ORDER == __BIG_ENDIAN

struct a2dp_sbc {
	uint8_t frequency:4;
	uint8_t channel_mode:4;
	uint8_t block_length:4;
	uint8_t subbands:2;
	uint8_t allocation_method:2;
	uint8_t min_bitpool;
	uint8_t max_bitpool;
} __attribute__ ((packed));

#else
#error "Unknown byte order"
#endif

struct codec_task
{
	unsigned ifrmlen:10;
	unsigned ofrmlen:10;

	struct pt pt;
	struct kfifo fifo;
	sbc_t	sbc;
	void (*prcoess)(const uint8_t *buf, unsigned size);
	uint8_t ebuf[1024];
	uint8_t kbuf[4 * 1024];
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

void sbc_codec_init(
	struct codec *code,
	struct a2dp_sbc *sbc, unsigned payloadsize,
	void (*enc)(const uint8_t *buf, unsigned size),
	void (*dec)(const uint8_t *buf, unsigned size));
void sbc_codec_reset(struct codec *codec);
void sbc_codec_workloop(struct codec *codec);

unsigned sbc_codec_dec_out(struct codec *codec, uint8_t *buf, unsigned size);
unsigned sbc_codec_enc_out(struct codec *codec, uint8_t *buf, unsigned size);

unsigned sbc_codec_enc(struct codec *cdeoc, const uint8_t *buf, unsigned size);
unsigned sbc_codec_dec(struct codec *cdeoc, const uint8_t *buf, unsigned size);

#endif /* __SBC_CODEC_H__*/

