#include <config.h>
#include "rdabt.h"
#include "bt_uart.h"
#include "btsel.h"
#include "hci_private.h"
#include <defs.h>

#if CONFIG_BT_MODULE == RDA5876

#define RDABT_VENDOR_AUTO_BAUDRATE 0x77

#if defined(M480)

# define BTPWR_ON() do {PA1 = 1;} while (0)
# define BTPWR_OFF() do {PA1 = 0;} while (0)
# define BTLDO_PIN_HIGH() do { PA3 = 1;} while (0)
# define BTLDO_PIN_LOW() do { PA3 = 0;} while (0)
# define BTPIN_INIT() do { \
	PA1 = 0; PA4 = 0; PA5 = 0; PA6 = 0; PA7 = 0; PA2 = 0;\
	GPIO_SetMode(PA, BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7, GPIO_MODE_OUTPUT);\
} while (0)

#elif defined(I94100)

# define BTPWR_ON() do {PA15 = 1;} while (0)
# define BTPWR_OFF() do {PA15 = 0;} while (0)
# define BTLDO_PIN_HIGH() do { PA0 = 1; } while (0)
# define BTLDO_PIN_LOW()  do { PA0 = 0; } while (0)
# define BTPIN_INIT() do { \
	PA15 = 0; PD15 = 0; PD14 = 0; PB3 = 0; PB4 = 0; PA1 = 0; PA0 = 1;\
	GPIO_SetMode(PA, BIT15 | BIT0 | BIT1, GPIO_MODE_OUTPUT);\
	GPIO_SetMode(PB, BIT3 | BIT4, GPIO_MODE_OUTPUT);\
	GPIO_SetMode(PD, BIT15 | BIT14, GPIO_MODE_OUTPUT);\
} while (0)

#endif

static const uint32_t rdabt_phone_init_12[][2] = 
{
	{0x3f,0x0001},//;page 1
#if (CHIP_ASIC_ID == CHIP_ASIC_ID_GALLITE)
	{0x32,0x0059},//;TM mod 001
#else // 8808 or later
	{0x32,0x0009},// Init UART IO status
#endif
	{0x3f,0x0000},//;page 0
};

static const uint32_t rdabt_rf_12[][2] = 
{
	{ 0x3f, 0x0000 }, //;page 0
	{ 0x01, 0x1FFF }, //;
	{ 0x06, 0x07F7 }, //;padrv_set,increase the power.
	{ 0x08, 0x01E7 }, //;
	{ 0x09, 0x0520 }, //;
	{ 0x0B, 0x03DF }, //;filter_cap_tuning<3:0>1101
	{ 0x0C, 0x85E8 }, //;
	{ 0x0F, 0x0DBC }, //; 0FH,16'h1D8C; 0FH,16'h1DBC;adc_clk_sel=1 20110314 ;adc_digi_pwr_reg<2:0>=011;
	{ 0x12, 0x07F7 }, //;padrv_set,increase the power.
	{ 0x13, 0x0327 }, //;agpio down pullen .
	{ 0x14, 0x0CCC }, //;h0CFE; bbdac_cm 00=vdd/2.
	{ 0x15, 0x0526 }, //;Pll_bypass_ontch:1,improve ACPR.
	{ 0x16, 0x8918 }, //;add div24 20101126
	{ 0x18, 0x8800 }, //;add div24 20101231
	{ 0x19, 0x10C8 }, //;pll_adcclk_en=1 20101126
	{ 0x1A, 0x9128 }, //;Mdll_adcclk_out_en=0
	{ 0x1B, 0x80C0 }, //;1BH,16'h80C2
	{ 0x1C, 0x361f }, //;
	{ 0x1D, 0x33fb }, //;Pll_cp_bit_tx<3:0>1110;13D3
	{ 0x1E, 0x303f }, //;Pll_lpf_gain_tx<1:0> 00;304C
	//{0x23,0x3335}, //;txgain
	//{0x24,0x8AAF}, //;
	//{0x27,0x1112}, //;
	//{0x28,0x345F}, //;
	{ 0x23, 0x2222 }, //;txgain
	{ 0x24, 0x359d }, //;
	{ 0x27, 0x0011 }, //;
	{ 0x28, 0x124f }, //;

	{ 0x39, 0xA5FC }, //;
	{ 0x3f, 0x0001 }, //;page 1
	{ 0x00, 0x043F }, //;agc
	{ 0x01, 0x467F }, //;agc
	{ 0x02, 0x28FF }, //;agc//20110323;82H,16'h68FF;agc
	{ 0x03, 0x67FF }, //;agc
	{ 0x04, 0x57FF }, //;agc
	{ 0x05, 0x7BFF }, //;agc
	{ 0x06, 0x3FFF }, //;agc
	{ 0x07, 0x7FFF }, //;agc
	{ 0x18, 0xf3f5 }, //;
	{ 0x19, 0xf3f5 }, //;
	{ 0x1A, 0xe7f3 }, //;
	{ 0x1B, 0xf1ff }, //;
	{ 0x1C, 0xffff }, //;
	{ 0x1D, 0xffff }, //;
	{ 0x1E, 0xffff }, //;
	{ 0x1F, 0xFFFF }, //;padrv_gain;9FH,16'hFFEC;padrv_gain20101103

#if 0
	{ 0x23, 0x4224 }, //;ext32k   //  //lugong modify,if mark it ,you can repeat open/close bt without 32k.
#endif

	{ 0x24, 0x0110 },
	{ 0x25, 0x43E1 }, //;ldo_vbit:110,2.04v
	{ 0x26, 0x4bb5 }, //;reg_ibit:100,reg_vbit:101,1.2v,reg_vbit_deepsleep:110,750mV
#if 0
	{ 0x32, 0x0079 }, //;TM mod
#endif
	{ 0x3f, 0x0000 }, //;page 0
};

static const uint32_t rdabt_pskey_rf_12[][2] =
{
	{0x40240000,0x2004f39c}, //; SPI2_CLK_EN PCLK_SPI2_EN
	{0x800000C0,0x00000021}, //; CHIP_PS PSKEY: Total number -----------------
	{0x800000C4,0x003F0000}, //;         PSKEY: Page 0
	{0x800000C8,0x00414003}, //;         PSKEY: Swch_clk_ADC
	{0x800000CC,0x004225BD}, //;         PSKEY: dig IF 625K IF  by lihua20101231
	{0x800000D0,0x004908E4}, //;         PSKEY: freq_offset_for rateconv  by lihua20101231(replace 47H)
	{0x800000D4,0x0043B074}, //;         PSKEY: AM dac gain, 20100605
	{0x800000D8,0x0044D01A}, //;         PSKEY: gfsk dac gain, 20100605//22
	{0x800000DC,0x004A0800}, //;         PSKEY: 4AH=0800
	{0x800000E0,0x0054A020}, //;         PSKEY: 54H=A020;agc_th_max=A0;agc_th_min=20
	{0x800000E4,0x0055A020}, //;         PSKEY: 55H=A020;agc_th_max_lg=A0;agc_th_min_lg=20
	{0x800000E8,0x0056A542}, //;         PSKEY: 56H=A542
	{0x800000EC,0x00574C18}, //;         PSKEY: 57H=4C18
	{0x800000F0,0x003F0001}, //;         PSKEY: Page 1               
	{0x800000F4,0x00410900}, //;         PSKEY: C1=0900;Phase Delay, 20101029 
	{0x800000F8,0x0046033F}, //;         PSKEY: C6=033F;modulation Index;delta f2=160KHZ,delta f1=164khz
	{0x800000FC,0x004C0000}, //;         PSKEY: CC=0000;20101108   
	{0x80000100,0x004D0015}, //;         PSKEY: CD=0015;
	{0x80000104,0x004E002B}, //;         PSKEY: CE=002B;           
	{0x80000108,0x004F0042}, //;         PSKEY: CF=0042            
	{0x8000010C,0x0050005A}, //;         PSKEY: D0=005A            
	{0x80000110,0x00510073}, //;         PSKEY: D1=0073            
	{0x80000114,0x0052008D}, //;         PSKEY: D2=008D            
	{0x80000118,0x005300A7}, //;         PSKEY: D3=00A7            
	{0x8000011C,0x005400C4}, //;         PSKEY: D4=00C4            
	{0x80000120,0x005500E3}, //;         PSKEY: D5=00E3            
	{0x80000124,0x00560103}, //;         PSKEY: D6=0103            
	{0x80000128,0x00570127}, //;         PSKEY: D7=0127            
	{0x8000012C,0x0058014E}, //;         PSKEY: D8=014E            
	{0x80000130,0x00590178}, //;         PSKEY: D9=0178            
	{0x80000134,0x005A01A1}, //;         PSKEY: DA=01A1            
	{0x80000138,0x005B01CE}, //;         PSKEY: DB=01CE            
	{0x8000013C,0x005C01FF}, //;         PSKEY: DC=01FF            
	{0x80000140,0x003F0000}, //;         PSKEY: Page 0

	{0x80000144,0x00000000}, //;         PSKEY: Page 0
	{0x80000040,0x10000000}, //;         PSKEY: Flage

	//adde for mini sleep (quick switch sco will failed)
	//{0x80000078,0x0f054001}, //for mini sleep 
	//{0x80000040,0x00004000}, //flag

	//{0x40240000,0x2000f29c}, //; SPI2_CLK_EN PCLK_SPI2_EN 
};


static const uint32_t rdabt_dccal_12[][2]=
{
	{0x0030,0x0129},
	{0x0030,0x012b}
};

static const uint32_t rdabt_pskey[][2] =
{
	{0x800004ec, 0xfe8fffff},
	{0x800004f0, 0x875b3fd8},	/* LMP featrues*/

	{0x80000070, 0x00002000},
	{0x80000074, 0x05025010},
	{0x8000007c, 0xb530b530},
	{0x800000a0, 0x00000000},	/* wake */
	{0x800000a4, 0x00000000},	/* wake */
	{0x800000a8, 0x0Bbaba30},	/* min power level */
	{0x80000040, 0x07009074},

};

static const uint32_t rda_trap_12[][2] = 
{
	{0x80000098, 0x9a610020},
	{0x80000040, 0x00400000},
	{0x40180100, 0x000068b0}, //inc power
	{0x40180120, 0x000068f4},
	{0x40180104, 0x000066b0}, //dec power
	{0x40180124, 0x000068f4},
	{0x40180108, 0x00017354}, // auth after encry
	{0x40180128, 0x00017368},
	{0x4018010c, 0x0001a338}, // auth after encry
	{0x4018012c, 0x00000014},
	{0x40180110, 0x0001936c}, // pair after auth fail
	{0x40180130, 0x00001ca8},
	{0x40180114, 0x0000f8c4}, ///all rxon
	{0x40180134, 0x00026948},
	{0x40180118, 0x000130b8}, ///qos PRH_CHN_QUALITY_MIN_NUM_PACKETS
	{0x40180138, 0x0001cbb4},
	{0x4018011c, 0x00010318}, //lu added for earphone 09.27
	{0x4018013c, 0x00001f04},

	{0x40180020, 0x00015a14}, // Set Wesco to 2
	{0x40180040, 0x000174b4},

	{0x8000000c, 0xea00003e},
	{0x80000104, 0xe51ff004},
	{0x80000108, 0x00001ec0},
	{0x8000010c, 0xe3a00000},
	{0x80000110, 0xe3a01000},
	{0x80000114, 0xebfffffa},
	{0x80000118, 0xe3500000},
	{0x8000011c, 0x03a0ebcb},
	{0x80000120, 0x028ef020},
	{0x80000124, 0xe3a02004},
	{0x80000128, 0xe3a0ebca},
	{0x8000012c, 0xe28effe9},
	{0x4018001c, 0x00032ba0},
	{0x4018003c, 0x00032d20},

	{0x80000014, 0xea000055},
	{0x80000170, 0xe59f0024},
	{0x80000174, 0xe5801000},
	{0x80000178, 0xe3a00002},
	{0x8000017c, 0xe59f100c},
	{0x80000180, 0xe1a02004},
	{0x80000184, 0xe3a03001},
	{0x80000188, 0xe59fe004},
	{0x8000018c, 0xe59ff004},
	{0x80000190, 0x800001a0},
	{0x80000194, 0x0000b6ec},
	{0x80000198, 0x0001bab8},
	{0x8000019c, 0x800001b4},
	{0x800001a0, 0xe59f1004},
	{0x800001a4, 0xe5911000},
	{0x800001a8, 0xe59ff008},
	{0x800001ac, 0x800001b4},
	{0x800001b0, 0x00030648},
	{0x800001b4, 0x00000000},
	{0x800001b8, 0x00030648},
	{0x40180018, 0x0000b6e8},
	{0x40180038, 0x00032d28},

	{0x80000018, 0xea000078},    //   sniff in sco
	{0x80000200, 0x05d41027},
	{0x80000204, 0x00611181},
	{0x80000208, 0x00811004},
	{0x8000020c, 0x05d1102a},
	{0x80000210, 0x01510000},
	{0x80000214, 0xe3a00bbb},
	{0x80000218, 0x1280ff79},
	{0x8000021c, 0x0280ff59},
	{0x40180014, 0x0002ed60},
	{0x40180034, 0x00032d2c},

	{0x40180004, 0x0002192c}, //
	{0x40180024, 0x00006b38}, //
//	{0x40180000,0x0000fff1},

	//repair ZTEU807 no audio question -- 20140210
	{0x40180008, 0x00016ac0}, // esco air mode
	{0x40180028, 0x0001fbf4},
	{0x40180000, 0x0000fff3},

	/*{0x4018000C,0x00024020},
	{0x4018002C,0x00000014},
	{0x40180000,0x0000fff7},*/

	{0x8000054c, 0x007803fd}, // set buffer size
	{0x80000550, 0x00030004},
	{0x80000554, 0x007803fd},
	{0x80000558, 0x00030002},
	{0x8000055c, 0x00010078},
	{0x80000560, 0x00000003},
};

static const uint32_t rdabt_enable_spi[][2] =
{
	{ 0x40240000, 0x2004f39c },
};

static const uint32_t rdabt_disable_spi[][2] =
{
	{ 0x40240000, 0x2000f29c },
};

static const uint32_t rdabt_flow_ctrl[][2] =
{
	{0x40200044, 0x0000003c},
	{0x50000010, 0x00000122},
};

static void rdabt_write_memory(uint32_t address, uint8_t type, const uint32_t *data, uint8_t length)
{
	uint8_t hdr[10];
	hdr[0] = 0x01;
	hdr[1] = 0x02;
	hdr[2] = 0xfd;
	hdr[3] = length * 4 + 6;
	hdr[4] = ((type == 0x10) ? 0x00 : type) + 0x80;
	address = (type == 0x01) ? (address << 2) + 0x200 : address;
	hdr[5]  = length;
	hdr[6] = address & 0xFF;
	hdr[7] = (address >> 8) & 0xFF;
	hdr[8] = (address >> 16) & 0xFF;
	hdr[9] = (address >> 24) & 0xFF;
	pktf_in2(&hci_conn.cmd_pktf.f, hdr, sizeof(hdr), data, 4 * length);
}

static void rdabt_write_data(uint32_t address, uint32_t data, uint8_t type)
{
	uint8_t *buf;

	while (!(buf = uart_dma_buffer()))
		DELAY(D_MILLISECOND(2));

	buf[0] = 0x01;
	buf[1] = 0x02;
	buf[2] = 0xfd;
	buf[3] = 0x0a;

	buf[4] = ((type == 0x10) ? 0x00 : type) + 0x80;
	address = (type == 0x01) ? (address << 2) + 0x200 : address;
	buf[5]  = 1;
	bt_put_le32(address, buf + 6);
	bt_put_le32(data, buf + 10);

	uart_dma_tx_transfer(14);
}

static void rdabt_write_array(const uint32_t buf[][2], unsigned size, uint8_t type)
{
	unsigned i;
	for (i = 0;i < size;i++) {
		rdabt_write_data(buf[i][0], buf[i][1], type);
		DELAY(D_MILLISECOND(12));
	}
}

#define RDA_WRITE_ARRAY(arr, type) rdabt_write_array(arr, ARRAY_SIZE(arr), type)

static void rdabt_auto_baudrate(void)
{
	uint8_t *buf;

	while (!(buf = uart_dma_buffer()))
		DELAY(D_MILLISECOND(2));

	buf[0] = 0x01;
	buf[1] = 0x77;
	buf[2] = 0xfc;
	buf[3] = 0x00;

	uart_dma_tx_transfer(0x4);
}

static void rdabt_hard_flow_ctrl(void)
{
	RDA_WRITE_ARRAY(rdabt_flow_ctrl, 0);
}

static void rdabt_rf_init(void)
{
	RDA_WRITE_ARRAY(rdabt_enable_spi, 0);
	RDA_WRITE_ARRAY(rdabt_rf_12, 1);
	DELAY(D_MILLISECOND(50));
}

static void rdabt_pskey_rf_init(void)
{
	RDA_WRITE_ARRAY(rdabt_pskey_rf_12, 0);
}

static void rdabt_dccal(void)
{
	RDA_WRITE_ARRAY(rdabt_dccal_12, 1);
	RDA_WRITE_ARRAY(rdabt_disable_spi, 0);
}

static void rdabt_pskey_misc(void)
{
	RDA_WRITE_ARRAY(rdabt_pskey, 0);
}

static void rdabt_write_bdaddr(bdaddr_t *ba)
{
	uint32_t pskey_bdaddr[][2] = {
		{0x80000044, ba->b[0] << 24 | ba->b[1] << 16 | ba->b[2] << 8 | ba->b[3]},
		{0x80000048, ba->b[4] << 8 | ba->b[5]},
		{0x80000040, 0x00000006}
	};
	RDA_WRITE_ARRAY(pskey_bdaddr, 0);
}

static void rdabt_write_buadrate(uint32_t baudrate)
{
	uint32_t pskey_baudrate[][2] = {
		{0x80000060, baudrate},
		{0x80000040, 0x00000100},
	};
	RDA_WRITE_ARRAY(pskey_baudrate, 0);
}

static void rdabt_trap(void)
{
	RDA_WRITE_ARRAY(rda_trap_12, 0);
}

void rdabt_reset_complete(bdaddr_t *ba)
{
	rdabt_write_bdaddr(ba);
	hci_conn_bthw_connected();
}

void rdabt_vendor_cmd_complete(evt_cmd_complete *ev)
{
}

void rdabt_vendor_evt(const uint8_t *ev, unsigned size)
{
}

void rdabt_init(void)
{
	BTPIN_INIT();
	BTPWR_OFF();
	BTLDO_PIN_HIGH();
	DELAY(D_MILLISECOND(500));
	BTLDO_PIN_LOW();

	BTPWR_ON();
	DELAY(D_MILLISECOND(20));
	BTLDO_PIN_HIGH();
	DELAY(D_MILLISECOND(20));
	UART0_Init(115200);

	rdabt_rf_init();
	rdabt_pskey_rf_init();

	BTLDO_PIN_LOW();
	DELAY(D_MILLISECOND(40));
	BTLDO_PIN_HIGH();
	DELAY(D_MILLISECOND(50));

	rdabt_rf_init();
	rdabt_pskey_rf_init();

	rdabt_dccal();
	rdabt_trap();
	rdabt_pskey_misc();
	rdabt_hard_flow_ctrl();
	DELAY(D_MILLISECOND(50));
	UART_EnableFlowCtrl(UART0);
	rdabt_write_buadrate(HCI_BAUDRATE);
	DELAY(D_MILLISECOND(50));
	Uart_Reset(HCI_BAUDRATE);
	DELAY(D_MILLISECOND(50));
}

#endif
