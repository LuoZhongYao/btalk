/*************************************************
 * Anthor  : LuoZhongYao@gmail.com
 * Modified: 2019/07/09
 ************************************************/
#ifndef __APP_L2CAP_H__
#define __APP_L2CAP_H__

#include <stdint.h>

typedef enum {
	APP_OPCODE_AUTH,
	APP_OPCODE_STATUS,
	APP_OPCODE_CTRL_REPORT,
	APP_OPCODE_MEDIA_STATUS_CHANGE,
	APP_OPCODE_OUTPUT_REPORT,
} app_opcode_t;

typedef struct {
	uint8_t report[43];
} __attribute__((packed)) app_control_report;

typedef struct {
} __attribute__((packed)) app_status_report;

#define APP_AUTH_SET_AUTH	0x00
#define APP_AUTH_STATUS		0x01
#define APP_AUTH_RSP_DATA	0x02

typedef struct {
	uint8_t operator;
	uint8_t payload[0];
} __attribute__((packed)) app_ps4_auth;

#define APP_MEDIA_STATUS_OPEN				(1 << 0)
#define APP_MEDIA_STATUS_PLAY				(1 << 1)
#define APP_MEDIA_STATUS_RECORD			(1 << 2)
#define APP_MEDIA_STATUS_SPEAKER_MUTE		(1 << 3)
#define APP_MEDIA_STATUS_MICPHONE_MUTE	(1 << 4)
#define APP_MEDIA_STATUS_VOLUME			(1 << 5)

typedef struct {
	uint8_t status;
} __attribute__((packed)) app_media_status;

typedef struct {
	uint16_t opcode;
	uint8_t  payload[0];
} __attribute__((packed)) app_command_hdr;

typedef struct {
	uint8_t report_id;
	uint8_t flag;
	uint8_t reserved0;
	uint8_t reserved1;
	uint8_t small_motor;
	uint8_t large_motor;
	uint8_t light_bar_red;
	uint8_t light_bar_green;
	uint8_t light_bar_blue;
	uint8_t reserved2[10];
	uint8_t left_speaker_volume;
	uint8_t right_speaker_volume;
	uint8_t microphone_volume;
	uint8_t speaker_volume;
	uint8_t reserved3[9];
} __attribute__((packed)) app_output_report;

#define APP_L2CAP_DEF(name, subtype) \
	uint8_t __buf[sizeof(app_command_hdr) + sizeof(subtype)]; \
	app_command_hdr *name = (app_command_hdr*)__buf

void app_l2cap_handle_auth(app_ps4_auth *auth);
void app_l2cap_handler(app_command_hdr *hdr, uint16_t size);

#ifdef M480

uint8_t app_get_media_status(void);
void app_media_status_change(uint8_t new_status);
void app_l2cap_handle_media_status_change(app_media_status *ctrl);
void app_l2cap_handle_output_report(app_output_report *rpt);

void app_auth_send_status(uint8_t status);
void app_auth2_send_data(void *buf, unsigned size);

#endif

#ifdef I94100

uint8_t app_auth_get_status(void);
void app_auth_set_data(void *buf, unsigned size);
uint8_t *app_output_report_buf(void);
void app_l2cap_handle_control_report(app_control_report *report);

#endif

#endif /* __APP_L2CAP_H__*/

