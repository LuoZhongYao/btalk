/**************************************************************************//**
 * @file     main.c
 * @version  V3.00
 * @brief    Use PDMA channel 2 to transfer data from memory to memory.
 *
 * @copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include <stdio.h>
#include "I94100.h"
#include "ConfigSysClk.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Macro, type and constant definitions                                                                    */
/*---------------------------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/
uint32_t PDMA_TEST_LENGTH = 64;
uint8_t au8SrcArray[256];
uint8_t au8DestArray[256];
uint32_t volatile g_u32IsTestOver = 0;

/**
 * @brief       DMA IRQ
 *
 * @param       None
 *
 * @return      None
 *
 * @details     The DMA default IRQ, declared in startup_I94100.s.
 */
void PDMA_IRQHandler(void)
{
    uint32_t status = PDMA_GET_INT_STATUS();

    if(status & PDMA_INTSTS_ABTIF_Msk) {  /* abort */
        /* Check if channel 2 has abort error */
        if(PDMA_GET_ABORT_STS() & PDMA_CH2_MASK)
            g_u32IsTestOver = 2;
        /* Clear abort flag of channel 2 */
        PDMA_CLR_ABORT_FLAG(PDMA_CH2_MASK);
    } else if(status & PDMA_INTSTS_TDIF_Msk) {  /* done */
        /* Check transmission of channel 2 has been transfer done */
        if(PDMA_GET_TD_STS() & PDMA_CH2_MASK)
            g_u32IsTestOver = 1;
        /* Clear transfer done flag of channel 2 */
        PDMA_CLR_TD_FLAG(PDMA_CH2_MASK);
    } else
        printf("unknown interrupt !!\n");
}

void UART0_Init()
{
	/* Enable UART module clock */
	CLK_EnableModuleClock(UART0_MODULE);

	/* Select UART module clock source as HXT and UART module clock divider as 1 */
	CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_PLL, CLK_CLKDIV0_UART0(1));
	
	/* Set PB multi-function pins for UART0 RXD(PB.9) and TXD(PB.8) */
	SYS->GPB_MFPH &= ~(SYS_GPB_MFPH_PB8MFP_Msk | SYS_GPB_MFPH_PB9MFP_Msk);
	SYS->GPB_MFPH |= (SYS_GPB_MFPH_PB8MFP_UART0_TXD | SYS_GPB_MFPH_PB9MFP_UART0_RXD);

	/* Reset UART module */
	SYS_ResetModule(UART0_RST);

	/* Configure UART0 and set UART0 baud rate */
	UART_Open(UART0, 115200);
}


/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int main(void)
{
	uint32_t u32i;
	
	// Initiate system clock(Configure in ConfigSysClk.h)
	SYSCLK_INITIATE();

	/* Init UART for printf */
	UART0_Init();

	printf("\n\nCPU @ %dHz\n", SystemCoreClock);
	printf("+------------------------------------------------------+ \n");
	printf("|    PDMA Memory to Memory Driver Sample Code          | \n");
	printf("+------------------------------------------------------+ \n");

	// Set Source Array Content
	for(u32i = 0; u32i < 256; u32i++)
		au8SrcArray[u32i] = u32i;
	
	/* Enable PDMA clock source */
	CLK_EnableModuleClock(PDMA_MODULE);

	/* Reset PDMA module */
	SYS_ResetModule(PDMA_RST);

	/*------------------------------------------------------------------------------------------------------

											 au8SrcArray                         au8DestArray
											 ---------------------------   -->   ---------------------------
										 /| [0]  | [1]  |  [2] |  [3] |       | [0]  | [1]  |  [2] |  [3] |\
											|      |      |      |      |       |      |      |      |      |
		 PDMA_TEST_LENGTH |            ...            |       |            ...            | PDMA_TEST_LENGTH
											|      |      |      |      |       |      |      |      |      |
										 \| [60] | [61] | [62] | [63] |       | [60] | [61] | [62] | [63] |/
											 ---------------------------         ---------------------------
											 \                         /         \                         /
														 32bits(one word)                     32bits(one word)

		PDMA transfer configuration:

			Channel = 2
			Operation mode = basic mode
			Request source = PDMA_MEM(memory to memory)
			transfer done and table empty interrupt = enable

			Transfer count = PDMA_TEST_LENGTH
			Transfer width = 32 bits(one word)
			Source address = au8SrcArray
			Source address increment size = 32 bits(one word)
			Destination address = au8DestArray
			Destination address increment size = 32 bits(one word)
			Transfer type = burst transfer

			Total transfer length = PDMA_TEST_LENGTH * 32 bits
	------------------------------------------------------------------------------------------------------*/

	/* Open Channel 2 */
	PDMA_Open(PDMA_CH2_MASK);
	/* Transfer count is PDMA_TEST_LENGTH, transfer width is 32 bits(one word) */
	PDMA_SetTransferCnt(2, PDMA_WIDTH_32, PDMA_TEST_LENGTH);
	/* Set source address is au8SrcArray, destination address is au8DestArray, Source/Destination increment size is 32 bits(one word) */
	PDMA_SetTransferAddr(2, (uint32_t)au8SrcArray, PDMA_SAR_INC, (uint32_t)au8DestArray, PDMA_DAR_INC);
	/* Request source is memory to memory */
	PDMA_SetTransferMode(2, PDMA_MEM, FALSE, 0);
	/* Transfer type is burst transfer and burst size is 4 */
	PDMA_SetBurstType(2, PDMA_REQ_BURST, PDMA_BURST_4);

	/* Enable interrupt */
	PDMA_EnableInt(2, PDMA_INT_TRANS_DONE);

	/* Enable NVIC for PDMA */
	NVIC_EnableIRQ(PDMA_IRQn);
	g_u32IsTestOver = 0;

	/* Generate a software request to trigger transfer with PDMA channel 2  */
	PDMA_Trigger(2);

	/* Waiting for transfer done */
	while(g_u32IsTestOver == 0);

	/* Check transfer result */
	if(g_u32IsTestOver == 1)
			printf("test done...\n");
	else if(g_u32IsTestOver == 2)
			printf("target abort...\n");

	/* Close channel 2 */
	PDMA_Close();

	while(1);
}
