/**************************************************************************//**
 * @file     uart.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 16/08/11 10:26a $
 * @brief    I94100 series UART driver source file
 *
 * @note
 * Copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include "I94100.h"

/** @addtogroup I94100_Device_Driver I94100 Device Driver
  @{
*/

/** @addtogroup I94100_UART_Driver UART Driver
  @{
*/

/** @addtogroup I94100_UART_EXPORTED_FUNCTIONS UART Exported Functions
  @{
*/

/**
 *    @brief        Clear UART specified interrupt flag
 *
 *    @param[in]    uart                The pointer of the specified UART module.
 *    @param[in]    u32InterruptFlag    The specified interrupt of UART module.
 *                                      - \ref UART_INTSTS_BUFERRINT_Msk : Buffer Error interrupt
 *                                      - \ref UART_INTSTS_MODEMINT_Msk  : Modem Status interrupt
 *                                      - \ref UART_INTSTS_RLSINT_Msk    : Receive Line Status interrupt
 *
 *    @return       None
 *
 *    @details      The function is used to clear UART specified interrupt flag.
 */

void UART_ClearIntFlag(UART_T* uart , uint32_t u32InterruptFlag)
{

    if(u32InterruptFlag & UART_INTSTS_RLSINT_Msk)   /* Clear Receive Line Status Interrupt */
    {
        uart->FIFOSTS = UART_FIFOSTS_BIF_Msk | UART_FIFOSTS_FEF_Msk | UART_FIFOSTS_FEF_Msk;
        uart->FIFOSTS = UART_FIFOSTS_ADDRDETF_Msk;
    }

    if(u32InterruptFlag & UART_INTSTS_MODEMINT_Msk)  /* Clear Modem Status Interrupt */
        uart->MODEMSTS |= UART_MODEMSTS_CTSDETF_Msk;

    if(u32InterruptFlag & UART_INTSTS_BUFERRINT_Msk)   /* Clear Buffer Error Interrupt */
    {
        uart->FIFOSTS = UART_FIFOSTS_RXOVIF_Msk | UART_FIFOSTS_TXOVIF_Msk;
    }
}


/**
 *  @brief      Disable UART interrupt
 *
 *  @param[in]  uart The pointer of the specified UART module.
 *
 *  @return     None
 *
 *  @details    The function is used to disable UART interrupt.
 */
void UART_Close(UART_T* uart)
{
    uart->INTEN = 0;
}


/**
 *  @brief      Disable UART auto flow control function
 *
 *  @param[in]  uart The pointer of the specified UART module.
 *
 *  @return     None
 *
 *  @details    The function is used to disable UART auto flow control.
 */
void UART_DisableFlowCtrl(UART_T* uart)
{
    uart->INTEN &= ~(UART_INTEN_ATORTSEN_Msk | UART_INTEN_ATOCTSEN_Msk);
}


/**
 *    @brief        Disable UART specified interrupt
 *
 *    @param[in]    uart                The pointer of the specified UART module.
 *    @param[in]    u32InterruptFlag    The specified interrupt of UART module.
 *								- \ref UART_INTEN_TXENDIEN_Msk	 : Transmitter Empty Interrupt 
 *                              - \ref UART_INTEN_ABRIEN_Msk     : Auto baud rate interrupt
 *                              - \ref UART_INTEN_WKIEN_Msk      : Wake-up Interrupt
 *                              - \ref UART_INTEN_BUFERRIEN_Msk  : Buffer Error interrupt
 *                              - \ref UART_INTEN_RXTOIEN_Msk    : Rx time-out interrupt
 *                              - \ref UART_INTEN_MODEMIEN_Msk   : Modem interrupt
 *                              - \ref UART_INTEN_RLSIEN_Msk     : Rx Line status interrupt
 *                              - \ref UART_INTEN_THREIEN_Msk    : Tx empty interrupt
 *                              - \ref UART_INTEN_RDAIEN_Msk     : Rx ready interrupt
 *
 *    @return       None
 *
 *    @details      The function is used to disable UART specified interrupt and disable NVIC UART IRQ.
 */
void UART_DisableInt(UART_T*  uart, uint32_t u32InterruptFlag)
{
    /* Disable UART specified interrupt */
    UART_DISABLE_INT(uart, u32InterruptFlag);

    /* Disable NVIC UART IRQ */
    if(uart == UART0)
        NVIC_DisableIRQ(UART0_IRQn);
//    else if(uart == UART1)
//        NVIC_DisableIRQ(UART1_IRQn);
//    else if(uart == UART2)
//        NVIC_DisableIRQ(UART2_IRQn);
//    else
//        NVIC_DisableIRQ(UART3_IRQn);
}


/**
 *    @brief        Enable UART auto flow control function
 *
 *    @param[in]    uart    The pointer of the specified UART module.
 *
 *    @return       None
 *
 *    @details      The function is used to Enable UART auto flow control.
 */
void UART_EnableFlowCtrl(UART_T* uart)
{
    /* Set RTS pin output is low level active */
    uart->MODEM |= UART_MODEM_RTSACTLV_Msk;

    /* Set CTS pin input is low level active */
    uart->MODEMSTS |= UART_MODEMSTS_CTSACTLV_Msk;

    /* Set RTS and CTS auto flow control enable */
    uart->INTEN |= UART_INTEN_ATORTSEN_Msk | UART_INTEN_ATOCTSEN_Msk;
}


/**
 *    @brief        The function is used to enable UART specified interrupt and enable NVIC UART IRQ.
 *
 *    @param[in]    uart                The pointer of the specified UART module.
 *    @param[in]    u32InterruptFlag    The specified interrupt of UART module:
 *								- \ref UART_INTEN_TXENDIEN_Msk	 : Transmitter Empty Interrupt 
 *                              - \ref UART_INTEN_ABRIEN_Msk     : Auto baud rate interrupt
 *                              - \ref UART_INTEN_WKIEN_Msk      : Wake-up Interrupt
 *                              - \ref UART_INTEN_BUFERRIEN_Msk  : Buffer Error interrupt
 *                              - \ref UART_INTEN_RXTOIEN_Msk    : Rx time-out interrupt
 *                              - \ref UART_INTEN_MODEMIEN_Msk   : Modem interrupt
 *                              - \ref UART_INTEN_RLSIEN_Msk     : Rx Line status interrupt
 *                              - \ref UART_INTEN_THREIEN_Msk    : Tx empty interrupt
 *                              - \ref UART_INTEN_RDAIEN_Msk     : Rx ready interrupt
 *
 *    @return       None
 *
 *    @details      The function is used to enable UART specified interrupt and enable NVIC UART IRQ.
 */
void UART_EnableInt(UART_T*  uart, uint32_t u32InterruptFlag)
{

    /* Enable UART specified interrupt */
    UART_ENABLE_INT(uart, u32InterruptFlag);

    /* Enable NVIC UART IRQ */
    if(uart == UART0)
        NVIC_EnableIRQ(UART0_IRQn);
//    else if(uart == UART1)
//        NVIC_EnableIRQ(UART1_IRQn);
//    else if(uart == UART2)
//        NVIC_EnableIRQ(UART2_IRQn);
//    else
//        NVIC_EnableIRQ(UART3_IRQn);

}


/**
 *    @brief        Open and set UART function
 *
 *    @param[in]    uart            The pointer of the specified UART module.
 *    @param[in]    u32baudrate     The baudrate of UART module.
 *
 *    @return       None
 *
 *    @details      This function use to enable UART function and set baud-rate.
 */
void UART_Open(UART_T* uart, uint32_t u32baudrate)
{
    uint8_t u8UartClkSrcSel, u8UartClkDivNum;
    uint32_t u32ClkTbl[4] = {__HXT, 0, __LXT, 0};
    uint32_t u32Baud_Div = 0;

    /* Get UART clock source selection */
    u8UartClkSrcSel = (CLK->CLKSEL1 & CLK_CLKSEL1_UART0SEL_Msk) >> CLK_CLKSEL1_UART0SEL_Pos;
	
    /* Get UART clock divider number */
    u8UartClkDivNum = (CLK->CLKDIV0 & CLK_CLKDIV0_UART0DIV_Msk) >> CLK_CLKDIV0_UART0DIV_Pos;

    /* Select UART function */
    uart->FUNCSEL = UART_FUNCSEL_UART;

    /* Set UART line configuration */
    uart->LINE = UART_WORD_LEN_8 | UART_PARITY_NONE | UART_STOP_BIT_1;

    /* Set UART Rx and RTS trigger level */
    uart->FIFO &= ~(UART_FIFO_RFITL_Msk | UART_FIFO_RTSTRGLV_Msk);

    /* Get PLL clock frequency if UART clock source selection is PLL */
    if(u8UartClkSrcSel == 1)
        u32ClkTbl[u8UartClkSrcSel] = CLK_GetPLLClockFreq();
	 
	/* Get HIRC clock frequency if UART clock source selection is HIRC */
	u32ClkTbl[3] = __HIRC;
	
    /* Set UART baud rate */
    if(u32baudrate != 0)
    {
        u32Baud_Div = UART_BAUD_MODE2_DIVIDER((u32ClkTbl[u8UartClkSrcSel]) / (u8UartClkDivNum + 1), u32baudrate);

        if(u32Baud_Div > 0xFFFF)
            uart->BAUD = (UART_BAUD_MODE0 | UART_BAUD_MODE0_DIVIDER((u32ClkTbl[u8UartClkSrcSel]) / (u8UartClkDivNum + 1), u32baudrate));
        else
            uart->BAUD = (UART_BAUD_MODE2 | u32Baud_Div);
    }
}


/**
 *    @brief        Read UART data
 *
 *    @param[in]    uart            The pointer of the specified UART module.
 *    @param[in]    pu8RxBuf        The buffer to receive the data of receive FIFO.
 *    @param[in]    u32ReadBytes    The the read bytes number of data.
 *
 *    @return       u32Count Receive byte count
 *
 *    @details      The function is used to read Rx data from RX FIFO and the data will be stored in pu8RxBuf.
 */
uint32_t UART_Read(UART_T* uart, uint8_t *pu8RxBuf, uint32_t u32ReadBytes)
{
    uint32_t  u32Count, u32delayno;

    for(u32Count = 0; u32Count < u32ReadBytes; u32Count++)
    {
        u32delayno = 0;

        while(uart->FIFOSTS & UART_FIFOSTS_RXEMPTY_Msk)   /* Check RX empty => failed */
        {
            u32delayno++;
            if(u32delayno >= 0x40000000)
                return FALSE;
        }
        pu8RxBuf[u32Count] = uart->DAT;    /* Get Data from UART RX  */
    }

    return u32Count;

}


/**
 *    @brief        Set UART line configuration
 *
 *    @param[in]    uart            The pointer of the specified UART module.
 *    @param[in]    u32baudrate     The register value of baudrate of UART module.
 *                                  If u32baudrate = 0, UART baudrate will not change.
 *    @param[in]    u32data_width   The data length of UART module.
 *                                  - \ref UART_WORD_LEN_5
 *                                  - \ref UART_WORD_LEN_6
 *                                  - \ref UART_WORD_LEN_7
 *                                  - \ref UART_WORD_LEN_8
 *    @param[in]    u32parity       The parity setting (none/odd/even/mark/space) of UART module.
 *                                  - \ref UART_PARITY_NONE
 *                                  - \ref UART_PARITY_ODD
 *                                  - \ref UART_PARITY_EVEN
 *                                  - \ref UART_PARITY_MARK
 *                                  - \ref UART_PARITY_SPACE
 *    @param[in]    u32stop_bits    The stop bit length (1/1.5/2 bit) of UART module.
 *                                  - \ref UART_STOP_BIT_1
 *                                  - \ref UART_STOP_BIT_1_5
 *                                  - \ref UART_STOP_BIT_2
 *
 *    @return       None
 *
 *    @details      This function use to config UART line setting.
 */
void UART_SetLine_Config(UART_T* uart, uint32_t u32baudrate, uint32_t u32data_width, uint32_t u32parity, uint32_t  u32stop_bits)
{
    uint8_t u8UartClkSrcSel, u8UartClkDivNum;
    uint32_t u32ClkTbl[4] = {__HXT, 0, __LXT, 0};
    uint32_t u32Baud_Div = 0;

    /* Get UART clock source selection */
    u8UartClkSrcSel = (CLK->CLKSEL1 & CLK_CLKSEL1_UART0SEL_Msk) >> CLK_CLKSEL1_UART0SEL_Pos;

    /* Get UART clock divider number */
    u8UartClkDivNum = (CLK->CLKDIV0 & CLK_CLKDIV0_UART0DIV_Msk) >> CLK_CLKDIV0_UART0DIV_Pos;

    /* Get PLL clock frequency if UART clock source selection is PLL */
    if(u8UartClkSrcSel == 1)
        u32ClkTbl[u8UartClkSrcSel] = CLK_GetPLLClockFreq();
	
	/* Get HIRC clock frequency if UART clock source selection is HIRC */
	u32ClkTbl[3] = __HIRC;
    /* Set UART baud rate */
	
    if(u32baudrate != 0)
    {
        u32Baud_Div = UART_BAUD_MODE2_DIVIDER((u32ClkTbl[u8UartClkSrcSel]) / (u8UartClkDivNum + 1), u32baudrate);

        if(u32Baud_Div > 0xFFFF)
            uart->BAUD = (UART_BAUD_MODE0 | UART_BAUD_MODE0_DIVIDER((u32ClkTbl[u8UartClkSrcSel]) / (u8UartClkDivNum + 1), u32baudrate));
        else
            uart->BAUD = (UART_BAUD_MODE2 | u32Baud_Div);
    }

    /* Set UART line configuration */
    uart->LINE = u32data_width | u32parity | u32stop_bits;
}


/**
 *    @brief        Set Rx timeout count
 *
 *    @param[in]    uart    The pointer of the specified UART module.
 *    @param[in]    u32TOC  Rx timeout counter.
 *
 *    @return       None
 *
 *    @details      This function use to set Rx timeout count.
 */
void UART_SetTimeoutCnt(UART_T* uart, uint32_t u32TOC)
{
    /* Set time-out interrupt comparator */
    uart->TOUT = (uart->TOUT & ~UART_TOUT_TOIC_Msk) | (u32TOC);

    /* Set time-out counter enable */
    uart->INTEN |= UART_INTEN_TOCNTEN_Msk;
}


/**
 *    @brief        Select and configure RS485 function
 *
 *    @param[in]    uart        The pointer of the specified UART module.
 *    @param[in]    u32Mode     The operation mode(NMM/AUD/AAD).
 *                              - \ref UART_ALTCTL_RS485NMM_Msk
 *                              - \ref UART_ALTCTL_RS485AUD_Msk
 *                              - \ref UART_ALTCTL_RS485AAD_Msk
 *    @param[in]    u32Addr     The RS485 address.
 *
 *    @return       None
 *
 *    @details      The function is used to set RS485 relative setting.
 */
void UART_SelectRS485Mode(UART_T* uart, uint32_t u32Mode, uint32_t u32Addr)
{
    /* Select UART RS485 function mode */
    uart->FUNCSEL = UART_FUNCSEL_RS485;

    /* Set RS585 configuration */
    uart->ALTCTL &= ~(UART_ALTCTL_RS485NMM_Msk | UART_ALTCTL_RS485AUD_Msk | UART_ALTCTL_RS485AAD_Msk | UART_ALTCTL_ADDRMV_Msk);
    uart->ALTCTL |= (u32Mode | (u32Addr << UART_ALTCTL_ADDRMV_Pos));
}


/**
 *    @brief        Write UART data
 *
 *    @param[in]    uart            The pointer of the specified UART module.
 *    @param[in]    pu8TxBuf        The buffer to send the data to UART transmission FIFO.
 *    @param[out]   u32WriteBytes   The byte number of data.
 *
 *    @return       u32Count transfer byte count
 *
 *    @details      The function is to write data into TX buffer to transmit data by UART.
 */
uint32_t UART_Write(UART_T* uart, uint8_t *pu8TxBuf, uint32_t u32WriteBytes)
{
    uint32_t  u32Count, u32delayno;

    for(u32Count = 0; u32Count != u32WriteBytes; u32Count++)
    {
        u32delayno = 0;
        while((uart->FIFOSTS & UART_FIFOSTS_TXEMPTYF_Msk) == 0)   /* Wait Tx empty and Time-out manner */
        {
            u32delayno++;
            if(u32delayno >= 0x40000000)
                return FALSE;
        }
        uart->DAT = pu8TxBuf[u32Count];    /* Send UART Data from buffer */
    }

    return u32Count;

}


/*@}*/ /* end of group I94100_UART_EXPORTED_FUNCTIONS */

/*@}*/ /* end of group I94100_UART_Driver */

/*@}*/ /* end of group I94100_Device_Driver */

/*** (C) COPYRIGHT 2016 Nuvoton Technology Corp. ***/



