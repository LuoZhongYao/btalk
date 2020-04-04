/*
 * Written by ZhongYao Luo <luozhongyao@gmail.com>
 *
 * Copyright 2020 ZhongYao Luo
 */
#include <config.h>
#include <time.h>
#include <soc.h>
#include <Platform.h>
#include "bt_uart.h"

#if defined(CSR8811)

void csr_init(void)
{
	UART0_Init(921600);
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
}

#endif
