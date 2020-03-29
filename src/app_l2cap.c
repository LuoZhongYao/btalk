#include "app.h"
#include "app_l2cap.h"
#include <string.h>
#include <bluetooth/l2cap.h>


void app_l2cap_handler(app_command_hdr *hdr, uint16_t size)
{
	switch (hdr->opcode) {

#ifdef I94100
	case APP_OPCODE_CTRL_REPORT:
		app_l2cap_handle_control_report((app_control_report*)hdr->payload);
	break;
#endif

#ifdef M480
	case APP_OPCODE_MEDIA_STATUS_CHANGE:
		app_l2cap_handle_media_status_change((app_media_status*)hdr->payload);
	break;

	case APP_OPCODE_OUTPUT_REPORT:
		app_l2cap_handle_output_report((app_output_report*)hdr->payload);
	break;

#endif

	case APP_OPCODE_AUTH:
	break;

	case APP_OPCODE_STATUS:
	break;

	}
}
