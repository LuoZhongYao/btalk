/******************************************************************************
 * @file     main.c
 * @brief
 *           Demonstrate how to transfer data between USB device and PC through USB HID interface.
 *           A windows tool is also included in this sample code to connect with USB device.
 *
 * @note
 * Copyright (C) 2014~2015 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include "I94100.h"
#include "targetdev.h"

#define PLLCON_SETTING  		CLK_PLLCTL_144MHz_HXT
#define PLL_CLOCK       		144000000
#define HCLK_DIV 						2
#define USBD_DIV 						3

#define BIT(n)	(1 << n)

#define USBD_HID_PIN_MASK (SYS_GPB_MFPH_PB13MFP_Msk|SYS_GPB_MFPH_PB14MFP_Msk|SYS_GPB_MFPH_PB15MFP_Msk)
#define USBD_HID_PIN      (SYS_GPB_MFPH_PB13MFP_USBD_DN|SYS_GPB_MFPH_PB14MFP_USBD_DP|SYS_GPB_MFPH_PB15MFP_USBD_VBUS)

/*--------------------------------------------------------------------------*/
void SYS_Init(void)
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Enable Internal RC clock and external XTAL clock */
    CLK->PWRCTL |= (CLK_PWRCTL_HXTEN_Msk | CLK_PWRCTL_HIRCEN_Msk);

    /* Waiting for external XTAL clock ready */
    while (!(CLK->STATUS & CLK_STATUS_HXTSTB_Msk));

    /* Set core clock as PLL_CLOCK from PLL */
    CLK->PLLCTL = PLLCON_SETTING;

    while (!(CLK->STATUS & CLK_STATUS_PLLSTB_Msk));

    CLK->CLKDIV0 &= ~CLK_CLKDIV0_HCLKDIV_Msk;
    CLK->CLKDIV0 |= CLK_CLKDIV0_HCLK(HCLK_DIV);
    CLK->CLKDIV0 &= ~CLK_CLKDIV0_USBDIV_Msk;
    CLK->CLKDIV0 |= CLK_CLKDIV0_USBD(USBD_DIV);
    CLK->CLKSEL0 &= (~CLK_CLKSEL0_HCLKSEL_Msk);
    CLK->CLKSEL0 |= CLK_CLKSEL0_HCLKSEL_PLL;
    CLK->CLKSEL4 &= CLK_CLKSEL4_USBSEL_Msk;
    CLK->CLKSEL4 |= CLK_CLKSEL4_USBSEL_PLL;
		
    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate PllClock, SystemCoreClock and CycylesPerUs automatically. */
    //SystemCoreClockUpdate();
    PllClock        = PLL_CLOCK;            			// PLL
    SystemCoreClock = PLL_CLOCK / HCLK_DIV; 			// HCLK
    CyclesPerUs     = SystemCoreClock / 1000000;  // For SYS_SysTickDelay()
    /* Enable module clock */
    CLK->APBCLK0 |= CLK_APBCLK0_USBDCKEN_Msk;
    CLK->AHBCLK |= CLK_AHBCLK_ISPCKEN_Msk;
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    // gpio multi-function configuration.
    SYS->GPB_MFPH = (SYS->GPB_MFPH & (~USBD_HID_PIN_MASK)) | USBD_HID_PIN;
	SYS->GPA_MFPH &= ~(SYS_GPA_MFPH_PA12MFP_Msk);
	SYS->GPA_MFPH |= SYS_GPA_MFPH_PA12MFP_GPIO;

	PA->MODE  = (PA->MODE  & ~(0x3 << (12 << 1))) | (GPIO_MODE_OUTPUT << (12 << 1));
	PA12 = 1;

	SYS->GPD_MFPL &= ~(SYS_GPD_MFPL_PD4MFP_Msk);
	SYS->GPD_MFPL |= SYS_GPD_MFPL_PD4MFP_GPIO;

	PD->MODE  = (PD->MODE  & ~(0x3 << (4 << 1))) | (GPIO_MODE_INPUT << (4 << 1));
	PD->PUSEL = (PD->PUSEL & ~(0x3 << (4 << 1))) | (GPIO_PUSEL_PULL_DOWN << (4 << 1));
}

void USBD_IRQHandler(void);
/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main(void)
{
    /* Unlock write-protected registers */
    SYS_UnlockReg();
    /* Init system and multi-funcition I/O */
    SYS_Init();
    FMC->ISPCTL |= FMC_ISPCTL_ISPEN_Msk;	// (1ul << 0)
    GetDataFlashInfo(&g_dataFlashAddr, &g_dataFlashSize);

    while (DetectPin == 1) {
        /* Open USB controller */
        USBD_Open(&gsInfo, HIDTrans_ClassRequest, NULL);
        /*Init Endpoint configuration for HID */
        HID_Init();
        /* Start USB device */
        USBD_Start();

        /* Using polling mode and Removed Interrupt Table to reduce code size for M480 */

        /* DO NOT Enable USB device interrupt */
        // NVIC_EnableIRQ(USBD_IRQn);

        while (DetectPin == 1) {
            // polling USBD interrupt flag
            USBD_IRQHandler();

            if (bUsbDataReady == TRUE) {
                ParseCmd((uint8_t *)usb_rcvbuf, 64);
                EP2_Handler();
                bUsbDataReady = FALSE;
            }
        }

        goto _APROM;
    }

    //CLK_SysTickDelay(300000);
    SysTick->LOAD = 300000 * CyclesPerUs;
    SysTick->VAL  = (0x00);
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;

    /* Waiting for down-count to zero */
    while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0);

_APROM:
	PA12 = 0;
    outpw(&SYS->RSTSTS, 3);//clear bit
    outpw(&FMC->ISPCTL, inpw(&FMC->ISPCTL) & 0xFFFFFFFC);
    outpw(&SCB->AIRCR, (V6M_AIRCR_VECTKEY_DATA | V6M_AIRCR_SYSRESETREQ));

    /* Trap the CPU */
    while (1);
}
/*** (C) COPYRIGHT 2017 Nuvoton Technology Corp. ***/
