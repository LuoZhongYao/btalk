#include "app.h"
#include "app_led.h"

__attribute__((weak)) void app_led_enter_idle(void) {}
__attribute__((weak)) void app_led_enter_init(void) {}
__attribute__((weak)) void app_led_enter_discovery(void) { }
__attribute__((weak)) void app_led_enter_inquirying(void) {}
__attribute__((weak)) void app_led_enter_local_connecting(void) {}
__attribute__((weak)) void app_led_enter_remote_connecting(void) {}
__attribute__((weak)) void app_led_enter_connected(void) {}

void app_led_set_status(app_status_t status)
{
	switch (status) {
	case APP_STATUS_IDLE:
		app_led_enter_idle();
	break;

	case APP_STATUS_INIT:
		app_led_enter_init();
	break;

	default:
	break;
	}
}
