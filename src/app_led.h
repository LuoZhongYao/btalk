/*************************************************
 * Anthor  : LuoZhongYao@gmail.com
 * Modified: 2019/07/08
 ************************************************/
#ifndef __APP_LED_H__
#define __APP_LED_H__

#include "app.h"

void app_led_set_status(app_status_t status);
void app_led_enter_init(void);
void app_led_enter_idle(void);
void app_led_enter_discovery(void);
void app_led_enter_inquirying(void);
void app_led_enter_local_connecting(void);
void app_led_enter_remote_connecting(void);
void app_led_enter_connected(void);

#endif /* __APP_LED_H__*/

