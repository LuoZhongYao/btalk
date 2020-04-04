/*
 * Written by ZhongYao Luo <luozhongyao@gmail.com>
 *
 * Copyright 2020 ZhongYao Luo
 */
#include <time.h>
#include "ConfigSysClk.h"

volatile uint64_t jiffies = 0;
void uart0_pdma_irqhandler(void);

void PDMA_IRQHandler(void)
{
	uart0_pdma_irqhandler();
}

void TMR0_IRQHandler(void)
{
	if (TIMER_GetIntFlag(TIMER0) == 1) {
		TIMER_ClearIntFlag(TIMER0);
		jiffies++;
	}
}

void soc_init(void)
{
	SYSCLK_INITIATE();

	CLK_EnableModuleClock(TMR0_MODULE);
	CLK_SetModuleClock(TMR0_MODULE, CLK_CLKSEL1_TMR0SEL_HXT, 0);

	TIMER_Open(TIMER0, TIMER_PERIODIC_MODE, 25000);

	TIMER_EnableInt(TIMER0);
	NVIC_EnableIRQ(TMR0_IRQn);

	TIMER_Start(TIMER0);

	CLK_EnableModuleClock(PDMA_MODULE);
	SYS_ResetModule(PDMA_MODULE);
	NVIC_EnableIRQ(PDMA_IRQn);
}
