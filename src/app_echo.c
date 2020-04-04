/*
 * Written by ZhongYao Luo <luozhongyao@gmail.com>
 *
 * Copyright 2020 ZhongYao Luo
 */
#include "app.h"
#include "fifo.h"
#include <debug.h>
#include <time.h>
#include <string.h>
#include "app_echo.h"
#include "sbc_codec.h"
#include "pt-1.4/pt.h"
#include "assert.h"
#include "resampler.h"
#include "webrtc/modules/audio_processing/aecm/include/echo_control_mobile.h"
#include "webrtc/modules/audio_processing/agc/legacy/analog_agc.h"

#define NN16K	160
#define NN32K	320
#define AGC_MODE kAgcModeAdaptiveAnalog

struct app_echo
{
	void *aecm;
	void *agc;
	int32_t mic_level;
	struct pt echo_work;
	struct kfifo c;
	struct kfifo p;
	uint8_t cbuf[4096];
	uint8_t pbuf[4096];
	struct Resampler c32to16;
	struct Resampler p32to16;
	struct Resampler c16to32;

};

static struct app_echo echo;
#ifdef ENABLE_DEBUG
#define PanicFalse(expr) do {						\
	if (!(expr)) {									\
		BLOGE(#expr "%s:%d\n", __FILE__, __LINE__);	\
		while(1);									\
	}												\
} while(0)
#else
#define PanicFalse(expr) expr
#endif

static void s2m(int16_t *dst, int32_t *src, unsigned nr)
{
	unsigned i = 0;

	for (i = 0; i < nr; i++) {
		dst[i] = (uint16_t)((src[i] >> 16) & 0xffff);
	}
}

static void m2s(int32_t *dst, int16_t *left, int16_t *right, unsigned nr)
{
	unsigned i = 0;
	uint32_t *p = (uint32_t*)dst;
	uint16_t *l = (uint16_t *)left, *r = (uint16_t *)right;

	for (i = 0; i < nr; i++) {
		p[i] = l[i] << 16 | r[i];
	}
}

static PT_THREAD(app_echo_work(struct app_echo *app))
{
	uint8_t warn;
	int32_t cs32[NN32K], ps32[NN32K];

	PT_BEGIN(&app->echo_work);

	while (1) {
		PT_WAIT_UNTIL(&app->echo_work, kfifo_len(&app->p) >= sizeof(ps32) &&
			kfifo_len(&app->c) >= sizeof(cs32));

		kfifo_out(&app->c, cs32, sizeof(cs32));
		kfifo_out(&app->p, ps32, sizeof(ps32));

		if (1 /* app_media_is_record() */) {

			size_t olen;
			uint32_t ilen;
			int16_t cm32[NN32K], pm32[NN32K];
			int16_t cm16[NN16K], pm16[NN16K], om16[NN16K];

			BLOGT({

				s2m(cm32, cs32, NN32K);
				s2m(pm32, ps32, NN32K);

				ilen = NN32K; olen = NN16K; 
				ResamplerPush(&app->p32to16, pm32, NN32K, pm16, NN16K, &olen);
				WebRtcAgc_AddFarend(app->agc, pm16, NN16K);
				WebRtcAecm_BufferFarend(app->aecm, pm16, NN16K);

				ilen = NN32K; olen = NN16K; 
				ResamplerPush(&app->c32to16, cm32, NN32K, cm16, NN16K, &olen);
				if (AGC_MODE == kAgcModeAdaptiveDigital) {
					WebRtcAgc_VirtualMic(app->agc, (int16_t *const []){cm16}, 1,
						NN16K, app->mic_level, &app->mic_level);
				} else if (AGC_MODE == kAgcModeAdaptiveAnalog) {
					WebRtcAgc_AddMic(app->agc, (int16_t *const []){cm16}, 1,
						NN16K);
				}

				WebRtcAecm_Process(app->aecm, cm16, NULL, om16, NN16K, 0);
				WebRtcAgc_Process(app->agc, (const int16_t *const[]){om16}, 1,
					NN16K, (int16_t *const []){om16}, app->mic_level, &app->mic_level,
					1, &warn);
				//speex_echo_cancellation(app->echo, c, p, o);
				//memcpy(o, c, sizeof(c));
				//BLOGT(speex_echo_cancellation(app->echo, c, p, o), "Echo");

				ilen = NN16K; olen = NN32K; 
				ResamplerPush(&app->c16to32, om16, NN16K, cm32, NN32K, &olen);
				//speex_resampler_process_int(app->resampler3, 1, om16, &ilen, cm32, &olen);

				m2s(cs32, cm32, cm32, NN32K);
			}, "Echo");

			//sbc_codec_enc((void*)cs32, sizeof(cs32));
		}
	}

	PT_END(&app->echo_work);
}

unsigned app_echo_playback(void *sample, unsigned size)
{
	if (kfifo_avail(&echo.p) < size) {
		BLOGW("playback %d\n", kfifo_avail(&echo.p));
	}

	return kfifo_in(&echo.p, sample, size);
}

unsigned app_echo_capture(void *sample, unsigned size)
{
#if 0
	if (!app_media_is_play()) {
		return sbc_codec_enc(sample, size);
	}
#endif

	if (kfifo_avail(&echo.c) < size) {
		BLOGW("capture %d\n", kfifo_avail(&echo.c));
	}

	return kfifo_in(&echo.c, sample, size);
}

void app_echo_reset(void)
{
	kfifo_reset(&echo.c);
	kfifo_reset(&echo.p);
	//speex_echo_state_reset(echo.echo);
}

void app_echo_init(void)
{
	AecmConfig config = {AecmTrue, 0};
	WebRtcAgcConfig agc_cfg = {3, 15, kAgcTrue};
	
	memset(&echo, 0, sizeof(echo));
	PT_INIT(&echo.echo_work);
	kfifo_init(&echo.p, echo.pbuf, sizeof(echo.pbuf));
	kfifo_init(&echo.c, echo.cbuf, sizeof(echo.cbuf));

	PanicFalse(echo.aecm = WebRtcAecm_Create());
	PanicFalse(0 == WebRtcAecm_Init(echo.aecm, 16000));
	WebRtcAecm_set_config(echo.aecm, config);

	PanicFalse(echo.agc = WebRtcAgc_Create());
	PanicFalse(0 == WebRtcAgc_Init(echo.agc, 0, 255, kAgcModeAdaptiveAnalog, 16000));
	WebRtcAgc_set_config(echo.agc, agc_cfg);


	ResamplerInit(&echo.c32to16, 32000, 16000, 1);
	ResamplerInit(&echo.p32to16, 32000, 16000, 1);
	ResamplerInit(&echo.c16to32, 16000, 32000, 1);
}

void app_echo_run(void)
{
	app_echo_work(&echo);
}
