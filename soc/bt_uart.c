#include "debug.h"
#include "fifo.h"
#include "bt_uart.h"
#include <stdbool.h>
#include <string.h>
#include "soc_config.h"
#include "chipsel.h"

struct uart
{
	volatile bool	tx_busy;
	uint8_t tx_buff[2048];
	uint8_t rx_buff[720];
	uint8_t	rx_fifo_buf[4 * 1024];
	struct kfifo rx_fifo;
};

static struct uart uart;

static void UART0_RX_Recv(void *buf, uint32_t size)
{
	/* UART Rx PDMA channel configuration */
	/* Set transfer width (8 bits) and transfer count */
	PDMA_SetTransferCnt(PDMA, UART_RX_PDMA_CH, PDMA_WIDTH_8, size);
	/* Set source/destination address and attributes */
	PDMA_SetTransferAddr(PDMA, UART_RX_PDMA_CH, (uint32_t)&UART0->DAT, PDMA_SAR_FIX,
		(uint32_t)buf, PDMA_DAR_INC);
	/* Set service selection; set Memory-to-Peripheral mode. */
	PDMA_SetTransferMode(PDMA, UART_RX_PDMA_CH, PDMA_UART0_RX, FALSE, 0);
	/* Single request type */
	PDMA_SetBurstType(PDMA, UART_RX_PDMA_CH, PDMA_REQ_SINGLE, 0);
	/* Disable table interrupt */
	PDMA->DSCT[UART_RX_PDMA_CH].CTL |= PDMA_DSCT_CTL_TBINTDIS_Msk;

	/* Set PDMA CH 1 timeout to about 2 ms (5/(72M/(2^15))) */
	PDMA->TOUTPSC = (PDMA->TOUTPSC & (~PDMA_TOUTPSC_TOUTPSC1_Msk)) | (0x2 << PDMA_TOUTPSC_TOUTPSC1_Pos);
	PDMA_SetTimeOut(PDMA, UART_RX_PDMA_CH, 1, 3);

	/* Enable PDMA timeout for UART RX*/
	PDMA_EnableInt(PDMA, UART_RX_PDMA_CH, PDMA_INT_TIMEOUT);
	PDMA_EnableTimeout(PDMA, (1 << UART_RX_PDMA_CH));

	/* Trigger PDMA */
	UART0->INTEN |= UART_INTEN_RXPDMAEN_Msk;
}

static void UART0_TX_Send(uint8_t *ptr, unsigned size)
{
	//PDMA->CHCTL |= (1 << UART_TX_PDMA_CH);
	/* UART Tx PDMA channel configuration */
	/* Set transfer width (8 bits) and transfer count */
	PDMA_SetTransferCnt(PDMA, UART_TX_PDMA_CH, PDMA_WIDTH_8, size);
	/* Set source/destination address and attributes */
	PDMA_SetTransferAddr(PDMA, UART_TX_PDMA_CH, (uint32_t)ptr, PDMA_SAR_INC,
		(uint32_t)&UART0->DAT, PDMA_DAR_FIX);
	/* Set service selection; set Memory-to-Peripheral mode. */
	PDMA_SetTransferMode(PDMA, UART_TX_PDMA_CH, PDMA_UART0_TX, FALSE, 0);
	/* Single request type */
	PDMA_SetBurstType(PDMA, UART_TX_PDMA_CH, PDMA_REQ_SINGLE, 0);
	/* Trigger PDMA */
	UART0->INTEN |= UART_INTEN_TXPDMAEN_Msk;
}

void uart0_pdma_irqhandler(void)
{
	unsigned rn;
	uint32_t status = __PDMA_GET_INT_STATUS(PDMA);
	if (status & (1 << (PDMA_INTSTS_REQTOF0_Pos + UART_RX_PDMA_CH))) {
		__PDMA_CLR_TMOUT_FLAG(PDMA, UART_RX_PDMA_CH);
		UART0->INTEN &= ~UART_INTEN_RXPDMAEN_Msk;

		PDMA->TOUTEN &= ~(1 << UART_RX_PDMA_CH);
		PDMA->TOUTEN |= (1 << UART_RX_PDMA_CH);

		rn = sizeof(uart.rx_buff) - 1 - ((PDMA->DSCT[UART_RX_PDMA_CH].CTL & PDMA_DSCT_CTL_TXCNT_Msk) >> PDMA_DSCT_CTL_TXCNT_Pos);

		if (kfifo_avail(&uart.rx_fifo) < rn)
			BLOGE("UART FIFO overrun req = %d, avail = %d\n", rn, kfifo_avail(&uart.rx_fifo));

		kfifo_in(&uart.rx_fifo, uart.rx_buff, rn);

		PDMA_SetTransferCnt(PDMA, UART_RX_PDMA_CH, PDMA_WIDTH_8, sizeof(uart.rx_buff));
		UART0->INTEN |= UART_INTEN_RXPDMAEN_Msk;

		status = __PDMA_GET_INT_STATUS(PDMA);
	}

	if (status & PDMA_INTSTS_TDIF_Msk) {
		if (__PDMA_GET_TD_STS(PDMA) & (1 << UART_RX_PDMA_CH)) {
			__PDMA_CLR_TD_FLAG(PDMA, (1 << UART_RX_PDMA_CH));
			UART0->INTEN &= ~UART_INTEN_RXPDMAEN_Msk;

			if (kfifo_avail(&uart.rx_fifo) < sizeof(uart.rx_buff))
				BLOGE("UART FIFO overrun req = %d, avail = %d\n", sizeof(uart.rx_buff), kfifo_avail(&uart.rx_fifo));
			kfifo_in(&uart.rx_fifo, uart.rx_buff, sizeof(uart.rx_buff));
			UART0_RX_Recv(uart.rx_buff, sizeof(uart.rx_buff));
		}

		if (__PDMA_GET_TD_STS(PDMA) & (1 << UART_TX_PDMA_CH)) {
			__PDMA_CLR_TD_FLAG(PDMA, (1 << UART_TX_PDMA_CH));
			UART0->INTEN &= ~UART_INTEN_TXPDMAEN_Msk;
			uart.tx_busy = false;
		}
	}
}

void UART0_Init(uint32_t baudrate)
{
#if defined(M480)
	/* Select UART clock source is HXT */
    CLK->CLKSEL1 = (CLK->CLKSEL1 & ~CLK_CLKSEL1_UART0SEL_Msk) | (0x1 << CLK_CLKSEL1_UART0SEL_Pos);
	/* Set PA multi-function pins for UART0 RXD(PA.6) and TXD(PA.7) CTS(A.5) and RTS(A.4)*/
    SYS->GPA_MFPL &= ~(SYS_GPA_MFPL_PA4MFP_Msk | SYS_GPA_MFPL_PA5MFP_Msk | SYS_GPA_MFPL_PA6MFP_Msk | SYS_GPA_MFPL_PA7MFP_Msk);
    SYS->GPA_MFPL |= (SYS_GPA_MFPL_PA6MFP_UART0_RXD | SYS_GPA_MFPL_PA7MFP_UART0_TXD | SYS_GPA_MFPL_PA5MFP_UART0_nCTS | SYS_GPA_MFPL_PA4MFP_UART0_nRTS);
#elif defined(I94100)
	SYS->GPB_MFPL &= ~(SYS_GPB_MFPL_PB3MFP_Msk | SYS_GPB_MFPL_PB4MFP_Msk);
	SYS->GPB_MFPL |= (SYS_GPB_MFPL_PB4MFP_UART0_TXD | SYS_GPB_MFPL_PB3MFP_UART_RXD);
	SYS->GPD_MFPH &= ~(SYS_GPD_MFPH_PD14MFP_Msk | SYS_GPD_MFPH_PD15MFP_Msk);
	SYS->GPD_MFPH |= (SYS_GPD_MFPH_PD14MFP_UART0_nCTS | SYS_GPD_MFPH_PD15MFP_UART0_nRTS);
#endif

	SYS_ResetModule(UART0_RST);
	UART_Open(UART0, baudrate);
	/* UART_EnableFlowCtrl(UART0); */

#if defined(ENABLE_HCI_H5)
	UART_SetLineConfig(UART0, baudrate, UART_WORD_LEN_8, UART_PARITY_EVEN, UART_STOP_BIT_1);
#endif

	PDMA_Open(PDMA, (1 << UART_RX_PDMA_CH) | (1 << UART_TX_PDMA_CH));
	PDMA_EnableInt(PDMA, UART_RX_PDMA_CH, PDMA_INT_TRANS_DONE);
	PDMA_EnableInt(PDMA, UART_TX_PDMA_CH, PDMA_INT_TRANS_DONE);

	UART0_RX_Recv(uart.rx_buff, sizeof(uart.rx_buff));

	NVIC_EnableIRQ(UART0_IRQn);
	NVIC_SetPriority(UART0_IRQn, 1);
	kfifo_init(&uart.rx_fifo, uart.rx_fifo_buf, sizeof(uart.rx_fifo_buf));
}

unsigned uart_rx_len(void)
{
	return kfifo_len(&uart.rx_fifo);
}

unsigned uart_rx_out(uint8_t *buf, unsigned size)
{
	return kfifo_out(&uart.rx_fifo, buf, size);
}

unsigned uart_rx_get(uint8_t *ch)
{
	return kfifo_get(&uart.rx_fifo, ch);
}

bool uart_tx_busy(void)
{
	return uart.tx_busy;
}

#if 0
unsigned uart_write(const uint8_t *buf, unsigned size)
{
	if (!uart.tx_busy) {
		uart.tx_busy = true;
		memmove(uart.tx_buff, buf, size);
		UART0_TX_Send(uart.tx_buff, size);
		return size;
	}
	return 0;
}
#endif

void *uart_dma_buffer(void)
{
	return uart.tx_busy ? NULL : uart.tx_buff;
}

void uart_dma_tx_transfer(unsigned size)
{
	if (!uart.tx_busy) {
		uart.tx_busy = true;
		UART0_TX_Send(uart.tx_buff, size);
	}
}

struct kfifo *uart_rx_fifo(void)
{
	return &uart.rx_fifo;
}

void UART_Reset(uint32_t baudrate)
{
    UART_Open(UART0, baudrate);

	/* UART_EnableFlowCtrl(UART0); */
#if defined(ENABLE_HCI_H5)
	UART_SetLineConfig(UART0, baudrate, UART_WORD_LEN_8, UART_PARITY_EVEN, UART_STOP_BIT_1);
#endif
#if 0

	UART0->FIFO |= UART_FIFO_RTSTRGLV_14BYTES;
	UART0->FIFO |= UART_FIFO_RFITL_14BYTES;
	UART0->TOUT = 40;
	UART0->INTEN = UART_INTEN_RDAIEN_Msk | UART_INTEN_TOCNTEN_Msk | UART_INTEN_RXTOIEN_Msk | UART_INTEN_ATORTSEN_Msk | UART_INTEN_ATOCTSEN_Msk;
    NVIC_EnableIRQ(UART0_IRQn);
	NVIC_SetPriority(UART0_IRQn, 1);
#endif
}

