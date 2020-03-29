/*
 * Written by ZhongYao Luo <luozhongyao@gmail.com>
 *
 * Copyright 2020 ZhongYao Luo
 */

#ifndef __APP_ECHO_H__
#define __APP_ECHO_H__

void app_echo_init(void);
void app_echo_run(void);
void app_echo_reset(void);

unsigned app_echo_playback(void *sample, unsigned size);
unsigned app_echo_capture(void *sample, unsigned size);

#endif /* __APP_ECHO_H__*/

