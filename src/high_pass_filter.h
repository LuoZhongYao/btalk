/*
 * Written by ZhongYao Luo <luozhongyao@gmail.com>
 *
 * Copyright 2020 ZhongYao Luo
 */

#ifndef __HIGH_PASS_FILTER_H__
#define __HIGH_PASS_FILTER_H__

#include "webrtc/typedefs.h"

struct FilterState {
  int16_t y[4];
  int16_t x[2];
  const int16_t* ba;
};

int InitializeFilter(struct FilterState* hpf, int sample_rate_hz);
int Filter(struct FilterState* hpf, int16_t* data, size_t length);

#endif /* __HIGH_PASS_FILTER_H__*/

