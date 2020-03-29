#if defined(RTL8761)

#include "sys_config.h"
#include "chipsel.h"
#include "bt_uart.h"
#include "hci_private.h"
#include "rtl8761b_fw.h"
#include "rtlbt.h"
#include <time.h>

#if defined(M480)

# define BTVREG_PIN_HIGH()
# define BTVREG_PIN_LOW()
# define PWR_PIN_HIGH()
# define PWR_PIN_LOW()
# define BTRST_PIN_HIGH() do { PA3 = 1; } while(0)
# define BTRST_PIN_LOW()	do{ PA3 = 0;} while (0)
# define BTPIN_INIT()	do { GPIO_SetMode(PA, BIT3, GPIO_MODE_OUTPUT); } while (0)

#elif defined(I94100)

# define BTVREG_PIN_HIGH()
# define BTVREG_PIN_LOW()
# define PWR_PIN_HIGH()			do {PA15 = 1;} while (0)
# define PWR_PIN_LOW()			do {PA15 = 0;} while (0)
# define BTRST_PIN_HIGH()		do {PA0 = 1;} while(0)
# define BTRST_PIN_LOW()		do {PA0 = 0;} while (0)
# define BTPIN_INIT()	do { GPIO_SetMode(PA, BIT0 | BIT15, GPIO_MODE_OUTPUT); } while (0)

#endif

#if HCI_BAUDRATE == 115200
# define RTL_BAUDRATE 0x0252C014
#elif HCI_BAUDRATE == 230400
# define RTL_BAUDRATE 0x0252C00A
#elif HCI_BAUDRATE == 921600
# define RTL_BAUDRATE 0x05F75004
#elif HCI_BAUDRATE == 1000000
# define RTL_BAUDRATE 0x00005004
#elif HCI_BAUDRATE == 1500000
# define RTL_BAUDRATE 0x04928002
#elif HCI_BAUDRATE == 1500000
# define RTL_BAUDRATE 0x01128002
#elif HCI_BAUDRATE == 2000000
# define RTL_BAUDRATE 0x00005002
#elif HCI_BAUDRATE == 2500000
# define RTL_BAUDRATE 0x0000B001
#elif HCI_BAUDRATE == 3000000
# define RTL_BAUDRATE 0x04928001
#elif HCI_BAUDRATE == 3500000
# define RTL_BAUDRATE 0x052A6001
#elif HCI_BAUDRATE == 4000000
# define RTL_BAUDRATE 0x00005001
#else
# error "Unsupport baudrate" HCI_BAUDRATE
#endif

#define HCI_VENDOR_CHANGE_BAUD	0x17
#define HCI_VENDOR_DOWNLOAD		0x20
#define HCI_VENDOR_READ_CHIP_TYPE	0x61
#define HCI_VENDOR_READ_ROM_VER	0x6d
#define HCI_VENDOR_READ_EVERSION 0x6f
#define HCI_VENDOR_WRITE_FREQ_OFFSET 0xea

static void rtlbt_read_chip_type(void)
{
	const uint8_t params[5] = {0x00, 0x94, 0xa0, 0x00, 0xb0};
	hci_send_cmd(OGF_VENDOR_CMD, HCI_VENDOR_READ_CHIP_TYPE, params, sizeof(params));
}

static void rtlbt_write_freq_offset(uint8_t off)
{
	hci_send_cmd(OGF_VENDOR_CMD, HCI_VENDOR_WRITE_FREQ_OFFSET, &off, sizeof(off));
}

static void rtlbt_read_eversion(void)
{
	hci_send_cmd(OGF_VENDOR_CMD, HCI_VENDOR_READ_ROM_VER, NULL, 0);
}

static void rtlbt_change_baudrate(uint32_t baudrate)
{
	hci_send_cmd(OGF_VENDOR_CMD, HCI_VENDOR_CHANGE_BAUD, &baudrate, sizeof(baudrate));
}

static inline uint16_t rtlbt_fw_next_offset(uint8_t index)
{
	uint16_t off = hci_conn.hw_used << 7 | index + 1;
	hci_conn.hw_used = off >> 7;
	return off;
}

static void rtlbt_fw_download(uint16_t off, bool last)
{
	uint8_t hdr[5];
	const uint8_t total = (sizeof(rtl8761b_fw) + 251) / 252;
	uint16_t opcode = cmd_opcode_pack(OGF_VENDOR_CMD, HCI_VENDOR_DOWNLOAD);

	//BLOGD("fw download index = %x, offset = %x, total = %x, size = %x, hw_used = %d\n",
	//	off & 0x7f, off, total, sizeof(rtl8761b_fw), hci_conn.hw_used);

	if (last) {
		BLOGD("fw download done\n");
		hci_conn_bthw_connected();
		return ;
	}

	if (off > (total - 1))
		return ;

	hdr[0] = 0x01;
	hdr[1] = opcode & 0xFF;
	hdr[2] = opcode >> 8;

	if (off == total - 1) {
		hdr[3] = (sizeof(rtl8761b_fw) - off * 252) + 1;
		hdr[4] = (off & 0x7f) | 0x80;
	} else  {
		hdr[3] = 253;
		hdr[4] = off & 0x7f;
	}

	pktf_in2(&hci_conn.cmd_pktf.f, hdr, sizeof(hdr), rtl8761b_fw + off * 252, hdr[3] - 1);
}

void rtlbt_reset_complete(bdaddr_t *ba)
{
	rtlbt_fw_download(0, false);
}

void rtlbt_vendor_evt(const uint8_t *ev, unsigned size)
{
}

void rtlbt_vendor_cmd_complete(evt_cmd_complete *ev)
{
	uint8_t status, index;

	switch (ev->opcode) {
	case cmd_opcode_pack(OGF_VENDOR_CMD, HCI_VENDOR_READ_CHIP_TYPE):
		rtlbt_change_baudrate(RTL_BAUDRATE);
	break;

	case cmd_opcode_pack(OGF_VENDOR_CMD, HCI_VENDOR_WRITE_FREQ_OFFSET):
		rtlbt_read_chip_type();
	break;

	case cmd_opcode_pack(OGF_VENDOR_CMD, HCI_VENDOR_CHANGE_BAUD):
		BLOGD("change baud complete, tx busy = %d, h5 flag = %x\n",
			uart_tx_busy(), hci_conn.h5.flags);
		while (uart_tx_busy()) 
			DELAY(D_MILLISECOND(1));
		//hci_conn.reset_uart = 1;
		//TIMER_SETUP(hci_conn.timeout, D_MILLISECOND(50));
		Uart_Reset(HCI_BAUDRATE);
		UART_EnableFlowCtrl(UART0);
		DELAY(D_MILLISECOND(50));
		hci_conn_bthw_connected();
		//hci_conn_No_Operation();
		//hci_send_cmd(OGF_HOST_CTL, OCF_RESET, NULL, 0);
	break;

	case cmd_opcode_pack(OGF_VENDOR_CMD, HCI_VENDOR_DOWNLOAD):
		status = ((uint8_t*)ev)[3];
		index = ((uint8_t*)ev)[4];
		rtlbt_fw_download(rtlbt_fw_next_offset(index), !!(index & 0x80));
	break;

	}
}

void rtlbt_init(void)
{
	BTPIN_INIT();
	PWR_PIN_LOW();
	BTRST_PIN_LOW();
	DELAY(D_SEC(1));

	PWR_PIN_HIGH();
	BTRST_PIN_HIGH();
	DELAY(D_MILLISECOND(300));

	UART0_Init(115200);
	UART_EnableFlowCtrl(UART0);
	rtlbt_write_freq_offset(0x20);
	//rtlbt_change_baudrate(RTL_BAUDRATE);
	//hci_conn_read_local_version();
	//rtlbt_read_chip_type();
	//rtlbt_read_eversion();
}

#endif
