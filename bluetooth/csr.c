#include <config.h>

#if defined(CSR8811)

#include <string.h>
#include "csr.h"
#include "bt_uart.h"
#include "hci_private.h"
#include "pt-1.4/pt.h"
#include "debug.h"
#include "time.h"

static uint16_t seqnum = 0x0000;

static void csr_write_bccmd(uint16_t opcode, uint16_t varid, uint16_t status, uint8_t *param, uint16_t psize)
{
	uint8_t cmd[256];

	memset(cmd, 0, sizeof(cmd));
	memcpy(cmd + 11, param, psize);
	if (psize < 8)
		psize = 8;
	psize = (psize + 1) & 0xFFFE;

	//cmd[0] = HCI_COMMAND_PKT;
	//cmd[1] = 0x00;      /* CSR command */
	//cmd[2] = 0xfc;      /* MANUFACTURER_SPEC */
	//cmd[3] = 1 + 10 + psize,

	/* CSR MSG header */
	cmd[0] = 0xC2;      /* first+last+channel=BCC */
	/* CSR BCC header */
	cmd[1] = opcode & 0xFF;      /* type = GET-REQ */
	cmd[2] = (opcode >> 8) & 0xFF;      /* - msB */
	cmd[3] = 5 + (psize / 2);     /* len */
	cmd[4] = 0x00;      /* - msB */
	cmd[5] = seqnum & 0xFF;/* seq num */
	cmd[6] = (seqnum >> 8) & 0xFF;    /* - msB */
	seqnum++;
	cmd[7] = varid & 0xFF;     /* var_id = CSR_CMD_BUILD_ID */
	cmd[8] = (varid >> 8) & 0xFF;     /* - msB */
	cmd[9] = status & 0xFF;     /* status = STATUS_OK */
	cmd[10] = (status >> 8) & 0xFF;     /* - msB */
	//uart_write(cmd, 15 + psize);
	hci_send_cmd(OGF_VENDOR_CMD, 0x0000, cmd, psize + 10 + 1);
}

void csr_write_pskey(uint16_t pskey, uint16_t stores, uint16_t value[], uint16_t size)
{
	uint8_t array[128];

	memset(array, 0, sizeof(array));
	array[0] = pskey & 0xff;
	array[1] = pskey >> 8;
	array[2] = size & 0xff;
	array[3] = size >> 8;
	array[4] = stores & 0xff;
	array[5] = stores >> 8;

	memcpy(array + 6, value, size * 2);

	csr_write_bccmd(0x0002, CSR_VARID_PS, 0x0000, array, 6 + size * 2);
}

static void csr_write_bdaddr(bdaddr_t *ba)
{
	uint8_t array[8];

	array[0] = ba->b[3];
	array[1] = 0x00;
	array[2] = ba->b[5];
	array[3] = ba->b[4];
	array[4] = ba->b[2];
	array[5] = 0x00;
	array[6] = ba->b[1];
	array[7] = ba->b[0];
	csr_write_pskey(CSR_PSKEY_BDADDR, 0x0000, (uint16_t*)array, sizeof(array) / 2);
}

static void csr_write_ana_freq(uint16_t freq)
{
	//uint16_t freq = 0x6590;
	csr_write_pskey(CSR_PSKEY_ANA_FREQ, 0x0000, &freq, sizeof(freq) / 2);
}

static void csr_write_host_inferface(uint16_t interface)
{
	//uint16_t interface = 0x0003;
	csr_write_pskey(CSR_PSKEY_HOST_INTERFACE, 0x0000, &interface, sizeof(interface) / 2);
}

static void csr_write_uart_config(uint16_t pskey, uint16_t config)
{
	//uint16_t h4_config = 0x08a8;
	if (pskey != CSR_PSKEY_UART_CONFIG_H4 && pskey != CSR_PSKEY_UART_CONFIG_H5) {
		BLOGE("uart config pskey must be CSR_PSKEY_UART_CONFIG_H5/CSR_PSKEY_UART_CONFIG_H4\n");
		return ;
	}
	csr_write_pskey(pskey, 0x0000, &config, sizeof(config) / 2);
}

static void csr_write_max_acl_link(uint16_t max_acl_link)
{
	csr_write_pskey(CSR_PSKEY_MAX_ACLS, 0x0000, &max_acl_link, sizeof(max_acl_link) / 2);
}

static void csr_write_max_acl_pkt_len(uint16_t pkt_len)
{
	csr_write_pskey(CSR_PSKEY_H_HC_FC_MAX_ACL_PKT_LEN, 0x0000, &pkt_len, sizeof(pkt_len) / 2);
}

static void csr_write_max_sco_link(uint16_t max_sco_link)
{
	csr_write_pskey(CSR_PSKEY_MAX_SCOS, 0x0000, &max_sco_link, sizeof(max_sco_link) / 2);
}

static void csr_write_max_sco_nr_pkt(uint16_t nr)
{
	csr_write_pskey(CSR_PSKEY_H_HC_FC_MAX_SCO_PKTS, 0x0000, &nr, sizeof(nr) / 2);
}

static void csr_write_uart_baudrate(uint32_t baudrate)
{
	uint8_t value[4];
	value[0] = (baudrate >> 16) & 0xFF;
	value[1] = (baudrate >> 24) & 0xFF;
	value[2] = (baudrate >> 0) & 0xFF;
	value[3] = (baudrate >> 8) & 0xFF;

	csr_write_pskey(0x01ea, 0x0000, (uint16_t*)value, 2);
}

static void csr_warn_reset(void)
{
	csr_write_bccmd(0x0002, CSR_VARID_WARM_RESET, 0x0000, NULL, 0);
}

static int csr_varid_ps(uint8_t *buf)
{
	int ret = 0;
	uint16_t pskey = buf[0] | buf[1] << 8;
	switch (pskey) {
	case CSR_PSKEY_BDADDR:
		BLOGD("CSR write bluetooth address complete, write ana freq\n");
		csr_write_ana_freq(0x6590);
	break;

	case CSR_PSKEY_HOST_INTERFACE:
#if defined(ENABLE_HCI_H5)
# define CSR_HOST_INTERFACE CSR_PHYS_BUS_H5
		csr_write_uart_config(CSR_PSKEY_UART_CONFIG_H5, 0x1806);
#elif defined(ENABLE_HCI_H4)
# define CSR_HOST_INTERFACE CSR_PHYS_BUS_H4
		csr_write_uart_config(CSR_PSKEY_UART_CONFIG_H4, 0x08a8);
#endif
	break;

	case CSR_PSKEY_ANA_FREQ:
		//csr_write_host_inferface(CSR_HOST_INTERFACE);
		csr_write_max_acl_link(1);
	break;

	case CSR_PSKEY_MAX_ACLS:
		csr_write_max_acl_pkt_len(679);
	break;

	case CSR_PSKEY_H_HC_FC_MAX_ACL_PKT_LEN:
		csr_write_max_sco_link(0);
	break;

	case CSR_PSKEY_MAX_SCOS:
		csr_write_max_sco_nr_pkt(0);
	break;

	case CSR_PSKEY_H_HC_FC_MAX_SCO_PKTS:
		csr_write_host_inferface(CSR_HOST_INTERFACE);
	break;

	case CSR_PSKEY_UART_CONFIG_H4:
	case CSR_PSKEY_UART_CONFIG_H5:
		csr_write_uart_baudrate(HCI_BAUDRATE);
	break;

	case 0x01ea:
		BLOGD("CSR write uart baudrate complete, warn reset\n");
		csr_warn_reset();
		//CLK_SysTickDelay(5000);
		//Uart_Reset(HCI_BAUDRATE);
		hci_conn.reset_uart = 1;
		TIMER_SETUP(hci_conn.timeout, D_MILLISECOND(100));
		ret = 1;
	break;

	}
	return ret;
}

void csr_reset_complete(bdaddr_t *ba)
{
	csr_write_bdaddr(ba);
}

int csr_vendor_evt(uint8_t *buf, unsigned size)
{
	int ret = 0;
	uint16_t status, varid, opcode;

	if (size < 11)
		return -1;

	if (buf[0] != 0xc2)
		return -1;


	opcode = buf[1] | buf[2] << 8;
	varid  = buf[7] | buf[8] << 8;
	status = buf[9] | buf[10] << 8;

	//BLOGD("csr_vendor_evt opcode = %04x, length = %04x, seq = %04x, varid = %04x, status = %04x\n",
	//	opcode, length, seq, varid,status);

	if (status != 0) {
		BLOGE("csr_vendor_evt failure: status = %d\n", status);
		return -1;
	}

	if (opcode != 0x0001) {
		BLOGE("csr_vendor_evt opcode\n");
		return -1;
	}

	switch (varid) {
	case CSR_VARID_PS:
		ret = csr_varid_ps(buf + 11);
	break;

	case CSR_VARID_WARM_RESET:
	break;
	}

	return ret;
}

void csr_init(void)
{
	UART0_Init(921600);
#if 0
	UART_EnableFlowCtrl(UART0);

#if defined(M480)
	GPIO_SetMode(PA, BIT3, GPIO_MODE_OUTPUT);//BT_RST
	GPIO_SetMode(PA, BIT2, GPIO_MODE_OUTPUT);//BT_VREG_EN
	PA3 = 0;
	PA2 = 0;
	DELAY(D_MILLISECOND(100));
	PA3 = 1;
	PA2 = 1;
#elif defined(I94100)
	GPIO_SetMode(PA, BIT15, GPIO_MODE_OUTPUT);//BT_POWER
	GPIO_SetMode(PA, BIT0, GPIO_MODE_OUTPUT);//BT_RST
	GPIO_SetMode(PA, BIT1, GPIO_MODE_OUTPUT);//BT_VREG_EN
	PA0 = 0;
	PA1 = 0;
	PA15 = 0;
	DELAY(D_MILLISECOND(100));
	PA0 = 1;
	PA1 = 1;
	PA15 = 1;
#endif
#endif
}

#endif
