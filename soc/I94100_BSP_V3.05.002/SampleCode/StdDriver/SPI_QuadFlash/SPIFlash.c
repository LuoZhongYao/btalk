/****************************************************************
 *                                                              *
 * Copyright (c) Nuvoton Technology Corp. All rights reserved. *
 *                                                              *
 ****************************************************************/
#include "SPIFlash.h"
#include <string.h>

#if defined ( __CC_ARM )
#pragma O0								// This pragma changes the optimization level, level Om range: O0~O3.
#elif defined ( __ICCARM__ )
#pragma optimize=medium					// optimization level: none, low, medium and high.
#elif defined ( __GNUC__ )
#pragma GCC optimization_level 2		// optimization level range: 0~3.
#endif

#if ( SPIFLASH_INTERFACE_MODE == 0)	//one-bit mode
#define SPIFLASH_READ_CMD					SPIFLASH_FAST_READ
#define SPIFLASH_WRITE_CMD					SPIFLASH_PAGE_PROGRAM
#define SPIFLASH_READ_MODE(spi)						
#define SPIFLASH_WRITE_MODE(spi)
#define SPIFLASH_DEFAULT_MODE(spi)
#define SPIFLASH_OUTPUT_DIRECTION(spi)	
#define SPIFLASH_INPUT_DIRECTION(spi)	


#elif ( SPIFLASH_INTERFACE_MODE == 1)	//dual mode
#define SPIFLASH_READ_CMD					SPIFLASH_FAST_DREAD
#define SPIFLASH_WRITE_CMD					SPIFLASH_PAGE_PROGRAM
#define SPIFLASH_READ_MODE(spi)				SPI_ENABLE_DUAL_INPUT_MODE(spi)
#define SPIFLASH_WRITE_MODE(spi)			SPI_ENABLE_DUAL_OUTPUT_MODE(spi)
#define SPIFLASH_DEFAULT_MODE(spi)			SPI_DISABLE_DUAL_MODE(spi)
#define SPIFLASH_OUTPUT_DIRECTION(spi)	// Write operation is standard mode, SPIFlash didn't supports Dual PROGRAM!
#define SPIFLASH_INPUT_DIRECTION(spi)		SPI_ENABLE_DUAL_INPUT_MODE(spi)

#elif ( SPIFLASH_INTERFACE_MODE == 2)	// quad mode 
#define SPIFLASH_READ_CMD					SPIFLASH_FAST_QREAD
#define SPIFLASH_WRITE_CMD					SPIFLASH_QPAGE_PROGRAM
#define SPIFLASH_READ_MODE(spi)				SPI_ENABLE_QUAD_INPUT_MODE(spi)
#define SPIFLASH_WRITE_MODE(spi)			SPI_ENABLE_QUAD_OUTPUT_MODE(spi)
#define SPIFLASH_DEFAULT_MODE(spi)			SPI_DISABLE_QUAD_MODE(spi)
#define SPIFLASH_OUTPUT_DIRECTION(spi)		SPI_ENABLE_QUAD_OUTPUT_MODE(spi)
#define SPIFLASH_INPUT_DIRECTION(spi)		SPI_ENABLE_QUAD_INPUT_MODE(spi)

#endif

#if (defined (__ISD9300__) || defined (__I91200__))
#define SPI0_SS_NONE		(0x0ul<<SPI_SSCTL_SS0_Pos)
#define SPI0_SET_SS(spi,u32SS)									( (spi)->SSCTL = ( (spi)->SSCTL & ~(SPI_SSCTL_SS0_Msk|SPI_SSCTL_SS1_Msk)) | u32SS )
#define SPI0_SET_SLAVE_ACTIVE_LEVEL(spi,u32Level)				( (spi)->SSCTL = ( (spi)->SSCTL & ~SPI0_SSCTL_SSACTPOL_Msk ) | u32Level )
#define SPI0_SET_TX_NUM(spi,u32TxNum)       //not available in I92100
#define SPI0_WRITE_TX0(spi, u32TxData)		SPI_WRITE_TX(spi, u32TxData)	
#define SPI0_WRITE_TX1(spi, u32TxData)		SPI_WRITE_TX(spi, u32TxData)
#define SPI0_READ_RX0(spi)					SPI0_READ_RX(spi)
#define SPI0_READ_RX1(spi)					SPI0_READ_RX(spi)
#define SPI0_GO(spi)						//I92100 SPI is triggered in  SPIFlash_Open and doesn't need to trigger every time when transaction begin
#endif

#if (defined (__I94100_SERIES__))
#define SPI0_SS_NONE																		(0x0ul<<SPI_SSCTL_SS0_Pos)
#define SPI0_SET_SS(spi,u32SS)													( (spi)->SSCTL = ( (spi)->SSCTL & ~(SPI_SSCTL_SS0_Msk|SPI_SSCTL_SS1_Msk)) | u32SS )
#define SPI0_SET_SLAVE_ACTIVE_LEVEL(spi,u32Level)				( (spi)->SSCTL = ( (spi)->SSCTL & ~SPI_SSCTL_SSACTPOL_Msk ) | u32Level )
#define SPI0_SET_TX_NUM(spi,u32TxNum)       						//not available in I94100
#define SPI0_WRITE_TX0(spi, u32TxData)									SPI_WRITE_TX(spi, u32TxData)	
#define SPI0_WRITE_TX1(spi, u32TxData)									SPI_WRITE_TX(spi, u32TxData)
#define SPI0_READ_RX0(spi)															SPI_READ_RX(spi)
#define SPI0_READ_RX1(spi)															SPI_READ_RX(spi)
#define SPI0_GO(spi)																		//I94100 SPI is triggered in  SPIFlash_Open and doesn't need to trigger every time when transaction begin
#endif


void
SPIFlash_3ByteAddr_Cmd(SPI_T	*psSpiHandler, UINT32 u32Cmd, UINT32 u32ByteAddr)
{
	SPI_SET_DATA_WIDTH(psSpiHandler,32);
	SPI0_SET_TX_NUM(psSpiHandler,SPI0_TXNUM_ONE);
	SPI0_WRITE_TX0(psSpiHandler,((UINT32)u32Cmd<<24)|(u32ByteAddr&0xffffff));
	SPI0_GO(psSpiHandler);
	while( SPI_IS_BUSY(psSpiHandler) );
}

void
SPIFlash_4ByteAddr_Cmd(SPI_T	*psSpiHandler, UINT32 u32Cmd, UINT32 u32ByteAddr)
{
	SPI_SET_DATA_WIDTH(psSpiHandler,20);
	SPI0_SET_TX_NUM(psSpiHandler,SPI0_TXNUM_TWO);
	SPI0_WRITE_TX0(psSpiHandler,((UINT32)u32Cmd<<12)|((u32ByteAddr&0xfff00000)>>20));
	SPI0_WRITE_TX1(psSpiHandler, (u32ByteAddr&0x000fffff));
	SPI0_GO(psSpiHandler);
	while( SPI_IS_BUSY(psSpiHandler) );
	SPI0_SET_TX_NUM(psSpiHandler,SPI0_TXNUM_ONE);
}

/*******************************************************************/
/*             Miscellaneous API code section                      */
/*******************************************************************/
void
SPIFlash_SendRecOneData(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT32 u32Data,
	UINT8  u8DataLen
)
{
	// Active chip select
	SPI0_SET_SS(psSpiFlashHandler->psSpiHandler, psSpiFlashHandler->u8SlaveDevice);
		
	// Set transmit Bit Length = u8DataLen
	SPI_SET_DATA_WIDTH(psSpiFlashHandler->psSpiHandler,u8DataLen);
	// Transmit/Receive Numbers = 1
	SPI0_SET_TX_NUM(psSpiFlashHandler->psSpiHandler,SPI0_TXNUM_ONE);
	// Write data to TX0 register
	SPI0_WRITE_TX0(psSpiFlashHandler->psSpiHandler,u32Data);

	SPI0_GO(psSpiFlashHandler->psSpiHandler);
	while( SPI_IS_BUSY(psSpiFlashHandler->psSpiHandler) );

	// Inactive chip select
	SPI0_SET_SS(psSpiFlashHandler->psSpiHandler, SPI0_SS_NONE);
}

BOOL
SPIFlash_CheckBusy(
	S_SPIFLASH_HANDLER *psSpiFlashHandler
)
{
	return (SPIFlash_ReadStatusReg(psSpiFlashHandler, eSTATUS_REG1)& SPIFLASH_BUSY);
}

void
SPIFlash_WaitReady(
	S_SPIFLASH_HANDLER *psSpiFlashHandler
)
{
	while( SPIFlash_ReadStatusReg(psSpiFlashHandler, eSTATUS_REG1)& SPIFLASH_BUSY );
}


UINT8
SPIFlash_ReadStatusReg(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	E_SPIFLASH_STATUS_REGISTER eStatusReg
)
{
#if (defined (__ISD9300__) || defined (__I91200__))
	SPI0_ClearRxFIFO(psSpiFlashHandler->psSpiHandler);
	// Wait for FIFO clear
	while (SPI0_GET_RX_FIFO_EMPTY_FLAG(psSpiFlashHandler->psSpiHandler) == 0);
#elif (defined (__I94100_SERIES__))
	SPI_ClearRxFIFO(psSpiFlashHandler->psSpiHandler);
	// Wait for FIFO clear
	while (SPI_GET_RX_FIFO_EMPTY_FLAG(psSpiFlashHandler->psSpiHandler) == 0);
#endif
	
	SPIFlash_SendRecOneData(psSpiFlashHandler,(SPIFLASH_READ_STATUS|eStatusReg)<<8, 16);

	return (UINT8)SPI0_READ_RX0(psSpiFlashHandler->psSpiHandler);
}

void
SPIFlash_WriteStatusRegEx(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	E_SPIFLASH_STATUS_REGISTER eStatusReg,
	UINT16 u16Status,
	UINT8 u8Length
)
{
    UINT8 shift = u8Length - 8;  // instruction occupied 8 bit, write data occupied u8Length - 8 bit
	SPIFlash_ChipWriteEnable(psSpiFlashHandler, TRUE);

	SPIFlash_SendRecOneData(psSpiFlashHandler,((SPIFLASH_WRITE_STATUS|eStatusReg)<<shift)|u16Status, u8Length);
	SPIFlash_WaitReady(psSpiFlashHandler);
}

UINT32
SPIFlash_GetVersion(void)
{
	return SPIFLASH_VERSION_NUM;
}

void
SPIFlash_EN4BAddress(
	S_SPIFLASH_HANDLER *psSpiFlashHandler
)
{
#if (SPIFLASH_OPERATION_MODE == 2)
	SPIFlash_SendRecOneData(psSpiFlashHandler,SPIFLASH_EN4B_MODE,8);
	psSpiFlashHandler->u8Flag = SPIFLASH_FLAG_HIGH_CAPACITY;
	psSpiFlashHandler->pfnSPIFlashMode = SPIFlash_4ByteAddr_Cmd;
#endif
}

void
SPIFlash_EX4BAddress(
	S_SPIFLASH_HANDLER *psSpiFlashHandler
)
{
#if (SPIFLASH_OPERATION_MODE == 2)
	SPIFlash_SendRecOneData(psSpiFlashHandler,SPIFLASH_EX4B_MODE,8);
	psSpiFlashHandler->u8Flag = SPIFLASH_FLAG_LOW_CAPACITY;
	psSpiFlashHandler->pfnSPIFlashMode = SPIFlash_3ByteAddr_Cmd;
#endif
}

void
SPIFlash_Open(
	SPI_T *psSpiHandler,
	UINT8 u8DrvSlaveDevice,
	UINT32 u32SpiClk,
	S_SPIFLASH_HANDLER *psSpiFlashHandler
)
{
	psSpiFlashHandler->u8SlaveDevice = u8DrvSlaveDevice;
	psSpiFlashHandler->psSpiHandler = psSpiHandler;
	
#if (defined (__N572F072__) || defined (__N572P072__) || defined (__N571P032__) )
	if (psSpiHandler == SPI0)// Enable high speed pins
	   SYS->GPA_HS = 0x1f;
	// Configure SPI parameters
	// Mode0--> SPI RX latched rising edge of clock; TX latced falling edge of clock; SCLK idle low
	SPI_Open(psSpiFlashHandler->psSpiHandler, SPI_MASTER, SPI_MODE_0, u32SpiClk);
#elif (defined (__N572F065__) || defined (__N572F064__))
	// Configure SPI parameters
	// Mode0--> SPI RX latched rising edge of clock; TX latced falling edge of clock; SCLK idle low
	SPI_Open(psSpiFlashHandler->psSpiHandler, SPI_MODE_0, u32SpiClk);
#elif (defined (__ISD9100__) || defined (__N575F145__) || defined (__N570F064__) || defined (__ISD9000__) || defined (__N569S__) )
	SPI_Open(psSpiFlashHandler->psSpiHandler, SPI_MASTER, SPI_MODE_0, u32SpiClk, 0);
#elif (defined (__ISD9300__) || defined (__I91200__))
	SPI0_Open(psSpiFlashHandler->psSpiHandler, SPI0_MASTER, SPI0_MODE_0, 8, u32SpiClk);
#elif	(defined (__I94100_SERIES__))
	SPI_Open(psSpiFlashHandler->psSpiHandler, SPI_MASTER, SPI_MODE_0, 8, u32SpiClk);
#endif

	// bit MSB first
	SPI_SET_MSB_FIRST(psSpiFlashHandler->psSpiHandler);
	// send/receve command in big endian; write/read data in little endian
	SPI_DISABLE_BYTE_REORDER(psSpiFlashHandler->psSpiHandler);
	// transmit/receive word will be executed in one transfer
	SPI0_SET_TX_NUM(psSpiFlashHandler->psSpiHandler, SPI0_TXNUM_ONE);
	// defalut width 8 bits
	SPI_SET_DATA_WIDTH(psSpiFlashHandler->psSpiHandler, 8);
	// set Slave active level as low selection
	SPI0_SET_SLAVE_ACTIVE_LEVEL(psSpiFlashHandler->psSpiHandler, SPI_SS_ACTIVE_LOW);
	// set Suspend Interval = 3 SCLK clock cycles for interval between two successive transmit/receive.
  // If for DUAL and QUAD transactions with REORDER, SUSPITV must be set to 0.
	SPI_SET_SUSPEND_CYCLE(psSpiFlashHandler->psSpiHandler, 0xf);
	
	psSpiFlashHandler->u32FlashSize = 0;
	psSpiFlashHandler->u8Flag = 0;
	// Inactive chip select
	SPI0_SET_SS(psSpiFlashHandler->psSpiHandler, SPI0_SS_NONE);

#if (defined (__ISD9300__) || defined (__I91200__)) 
	SPI0_TRIGGER(psSpiFlashHandler->psSpiHandler);
#endif
	
	/* Set defalut 3 byte-address */ 
	SPIFlash_EX4BAddress(psSpiFlashHandler);
}

void
SPIFlash_GetChipInfo(
	S_SPIFLASH_HANDLER *psSpiFlashHandler
)
{
	UINT8  u8CapacityOrder;
	UINT32 u32Value;

 
#if (defined (__ISD9300__) || defined (__I91200__))
	SPI0_ClearRxFIFO(psSpiFlashHandler->psSpiHandler);
#elif (defined (__I94100_SERIES__))
	SPI_ClearRxFIFO(psSpiFlashHandler->psSpiHandler);
#endif
	
	// Get JEDEC ID command to detect Winbond, MXIC and ATmel series
	// Only W25P serious not support JEDEC ID command
	SPIFlash_SendRecOneData(psSpiFlashHandler, (UINT32)SPIFLASH_JEDEC_ID<<24, 32);
	u32Value = SPI0_READ_RX0(psSpiFlashHandler->psSpiHandler);
	u8CapacityOrder = ((u32Value)&0x0f); // based on 512Kbytes order
	if( ((u32Value>>16)&0xff) == 0x1f ) // Atmel SPIFlash
	{
		u8CapacityOrder = ((u32Value>>8)&0x1f); 
		u8CapacityOrder -= 1; 
	}
	//psSpiFlashHandler->u32FlashSize = (1024*512/8)<<u16CapacityOrder;
	psSpiFlashHandler->u32FlashSize = (64*1024)<<u8CapacityOrder; // Unit: 64k block bytes
}

void
SPIFlash_PowerDown(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	BOOL	bEnable
)
{
	UINT8 u8Cmd;

	if ( bEnable )
		u8Cmd = SPIFLASH_POWER_DOWN;
	else
		u8Cmd = SPIFLASH_RELEASE_PD_ID;

	SPIFlash_SendRecOneData(psSpiFlashHandler,u8Cmd,8);
}


/*******************************************************************/
/*             Protection API code section                         */
/*******************************************************************/
void
SPIFlash_ChipWriteEnable(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	BOOL bEnableWrite
)
{
	UINT8 u8Cmd;

	if ( bEnableWrite == TRUE )
		u8Cmd = SPIFLASH_WRITE_ENABLE;
	else
		u8Cmd = SPIFLASH_WRITE_DISABLE;
	
	SPIFlash_SendRecOneData(psSpiFlashHandler, u8Cmd, 8);
	SPIFlash_WaitReady(psSpiFlashHandler);
}

void
SPIFlash_GlobalProtect(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	BOOL bEnableGlobalProtect
)
{
	#define SPIFLASH_ALLLOCK_MASK	(0x3c)
	#define SPIFLASH_CMP_MASK		(0x4000)
	
	
#if 1 // Some new SPIFlash needs to check CMP (Complement Protect bit) value. e.g. Winbond, CMP defalut is 0
	
	UINT16 u16Status=0;
	
	u16Status = (SPIFlash_ReadStatusReg(psSpiFlashHandler, eSTATUS_REG2)<<8)|SPIFlash_ReadStatusReg(psSpiFlashHandler, eSTATUS_REG1);
	
	if (u16Status&SPIFLASH_CMP_MASK)
	{
		if(bEnableGlobalProtect)
			u16Status &= ~SPIFLASH_ALLLOCK_MASK;
		else
			u16Status |= SPIFLASH_ALLLOCK_MASK;
	}else
	{
		if(bEnableGlobalProtect)
			u16Status |= SPIFLASH_ALLLOCK_MASK;
		else
			u16Status &= ~SPIFLASH_ALLLOCK_MASK;
	}
		
	SPIFlash_ChipWriteEnable(psSpiFlashHandler, TRUE);

	SPIFlash_SendRecOneData(psSpiFlashHandler,((SPIFLASH_WRITE_STATUS|eSTATUS_REG1)<<8)|(u16Status&0xFF), 16);
	SPIFlash_WaitReady(psSpiFlashHandler);
	
	SPIFlash_SendRecOneData(psSpiFlashHandler,((SPIFLASH_WRITE_STATUS|eSTATUS_REG2)<<8)|((u16Status&0xFF00)>>8), 16);
	SPIFlash_WaitReady(psSpiFlashHandler);
	
#else // for old
/*	UINT8 u8Status=0;

	u8Status = SPIFlash_ReadStatusReg(psSpiFlashHandler, eSTATUS_REG1);
	
	if(bEnableGlobalProtect)
		u8Status |= SPIFLASH_ALLLOCK_MASK;
	else
		u8Status &= ~SPIFLASH_ALLLOCK_MASK;
	
	SPIMFlash_WriteStatusReg(psSpiFlashHandler,u8Status);*/
#endif
}

/*******************************************************************/
/*             Erase API code section                              */
/*******************************************************************/
void
SPIFlash_EraseStart(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT8 u8Cmd,
	UINT32 u32Addr
)
{
	SPIFlash_ChipWriteEnable(psSpiFlashHandler, TRUE);
		
	// Active chip select
	SPI0_SET_SS(psSpiFlashHandler->psSpiHandler, psSpiFlashHandler->u8SlaveDevice);
	
	// Send erase command
#if (SPIFLASH_OPERATION_MODE == 0)
	SPIFlash_3ByteAddr_Cmd(psSpiFlashHandler->psSpiHandler, u8Cmd, u32Addr);
#elif (SPIFLASH_OPERATION_MODE == 1)
	SPIFlash_4ByteAddr_Cmd(psSpiFlashHandler->psSpiHandler, u8Cmd, u32Addr);
#else
	psSpiFlashHandler->pfnSPIFlashMode(psSpiFlashHandler->psSpiHandler, u8Cmd, u32Addr);
#endif

	// Inactive chip select
	SPI0_SET_SS(psSpiFlashHandler->psSpiHandler, SPI0_SS_NONE);
}

/*******************************************************************/
/*             Read API code section                               */
/*******************************************************************/
void
SPIFlash_Read(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT32 u32ByteAddr,
	PUINT8 pau8Data,
	UINT32 u32DataLen
)
{
	SPIFlash_ReadStart( psSpiFlashHandler, u32ByteAddr );
	// Read data
	SPIFlash_ReadData(psSpiFlashHandler, pau8Data, u32DataLen);
	SPIFlash_ReadEnd(psSpiFlashHandler);
}

void
SPIFlash_BurstRead(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT32 u32ByteAddr,
	PUINT8 pau8Data,
	UINT32 u32DataLen
)
{
	SPIFlash_ReadStart( psSpiFlashHandler, u32ByteAddr );
	SPIFlash_ReadDataAlign(psSpiFlashHandler, pau8Data, u32DataLen);
	SPIFlash_ReadEnd(psSpiFlashHandler);
}

void
SPIFlash_ReadStart(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT32 u32ByteAddr
)
{
	// Active chip select
	SPI0_SET_SS(psSpiFlashHandler->psSpiHandler, psSpiFlashHandler->u8SlaveDevice);

	// Send Fast Read Quad Output command
#if (SPIFLASH_OPERATION_MODE == 0)
	SPIFlash_3ByteAddr_Cmd(psSpiFlashHandler->psSpiHandler, (UINT32)SPIFLASH_READ_CMD, u32ByteAddr);
#elif (SPIFLASH_OPERATION_MODE == 1)
	SPIFlash_4ByteAddr_Cmd(psSpiFlashHandler->psSpiHandler, (UINT32)SPIFLASH_READ_CMD, u32ByteAddr);
#else
	psSpiFlashHandler->pfnSPIFlashMode(psSpiFlashHandler->psSpiHandler, (UINT32)SPIFLASH_FAST_READ_4ADD, u32ByteAddr);
#endif

	// send dummy clcok
	SPI_SET_DATA_WIDTH(psSpiFlashHandler->psSpiHandler,8);
	SPI0_WRITE_TX0(psSpiFlashHandler->psSpiHandler,0);

	//SPI_SET_TX_NUM(psSpiFlashHandler->psSpiHandler,SPI_TXNUM_ONE);
	SPI0_GO(psSpiFlashHandler->psSpiHandler);
	while( SPI_IS_BUSY(psSpiFlashHandler->psSpiHandler) );
    
	// Enable read interface 
	SPIFLASH_READ_MODE(psSpiFlashHandler->psSpiHandler);

#if (defined (__ISD9300__) || defined (__I91200__))
	SPI0_ClearRxFIFO(psSpiFlashHandler->psSpiHandler);
#elif (defined (__I94100_SERIES__))
	SPI_ClearRxFIFO(psSpiFlashHandler->psSpiHandler);
#endif
}

void
SPIFlash_ReadEnd(
	S_SPIFLASH_HANDLER *psSpiFlashHandler
)
{
	// Inactive all slave devices
	SPI0_SET_SS(psSpiFlashHandler->psSpiHandler, SPI0_SS_NONE);
	// Default mode : one-bit mode
	SPIFLASH_DEFAULT_MODE(psSpiFlashHandler->psSpiHandler);
	// send/receve command in big endian; write/read data in little endian
	SPI_DISABLE_BYTE_REORDER(psSpiFlashHandler->psSpiHandler);
}

void
SPIFlash_ReadData(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	PUINT8 pau8Data,
	UINT32 u32DataLen
)
{
	//	PA20 CYHuang12 speedup read function.
	UINT32 u32ReadData;
	UINT8  u8ProcBytes;

	SPI0_SET_TX_NUM(psSpiFlashHandler->psSpiHandler,SPI0_TXNUM_ONE);
	
	u8ProcBytes = ((UINT32)pau8Data)&0x3;//u8ProcBytes = ((UINT32)pau8Data)%4;
  
	if (u8ProcBytes!=0)
	{
		u8ProcBytes = 4 - u8ProcBytes;
		if ( u8ProcBytes > u32DataLen )
		u8ProcBytes = u32DataLen;

		SPI_SET_DATA_WIDTH(psSpiFlashHandler->psSpiHandler,u8ProcBytes<<3);
		#if (defined (__ISD9300__) || defined (__I91200__))
		SPI0_WRITE_TX0(psSpiFlashHandler->psSpiHandler,0);
		#elif (defined (__I94100_SERIES__))
		SPI0_WRITE_TX0(psSpiFlashHandler->psSpiHandler, 0);
		#endif
		SPI0_GO(psSpiFlashHandler->psSpiHandler);
		u32DataLen-=u8ProcBytes;

		while( SPI_IS_BUSY(psSpiFlashHandler->psSpiHandler) );
		u32ReadData = SPI0_READ_RX0(psSpiFlashHandler->psSpiHandler);

		// Not Byte Reorder.
//		*pau8Data++ = (UINT8)u32ReadData;
//		if ( u8ProcBytes >= 2 )
//		*pau8Data++ = (UINT8)(u32ReadData>>8);
//		if ( u8ProcBytes >= 3 )
//		*pau8Data++ = (UINT8)(u32ReadData>>16);
		// Byte Reorder. Do Not Use SPI_CTL.REORDER When USING Quad and Dual Mode.
		*pau8Data++ = (UINT8)(u32ReadData>>16);
		if ( u8ProcBytes >= 2 )
			*pau8Data++ = (UINT8)(u32ReadData>>8);
		if ( u8ProcBytes >= 3 )
			*pau8Data++ = (UINT8)u32ReadData;
	}

	SPI_SET_DATA_WIDTH(psSpiFlashHandler->psSpiHandler, 32);

	while (u32DataLen>=4)
	{
		#if (defined (__ISD9300__) || defined (__I91200__))
		SPI0_WRITE_TX0(psSpiFlashHandler->psSpiHandler,0);
		#elif (defined (__I94100_SERIES__))
		SPI0_WRITE_TX0(psSpiFlashHandler->psSpiHandler,0);
		#endif
		SPI0_GO(psSpiFlashHandler->psSpiHandler);
		while( SPI_IS_BUSY(psSpiFlashHandler->psSpiHandler) );
		u32ReadData = SPI0_READ_RX0(psSpiFlashHandler->psSpiHandler);
		
		// Byte Reorder. Do Not Use SPI_CTL.REORDER When USING Quad and Dual Mode.
		*((UINT32*)pau8Data) = __REV(u32ReadData);
		pau8Data+=4;
		u32DataLen-=4;
	}

	if (u32DataLen>0)
	{
		SPI_SET_DATA_WIDTH(psSpiFlashHandler->psSpiHandler, u32DataLen<<3);
		#if (defined (__ISD9300__) || defined (__I91200__))
		SPI0_WRITE_TX0(psSpiFlashHandler->psSpiHandler,0);
		#elif (defined (__I94100_SERIES__))
		SPI0_WRITE_TX0(psSpiFlashHandler->psSpiHandler,0);
		#endif
		SPI0_GO(psSpiFlashHandler->psSpiHandler);
		while( SPI_IS_BUSY(psSpiFlashHandler->psSpiHandler) );
		u32ReadData = SPI0_READ_RX0(psSpiFlashHandler->psSpiHandler);
		
		// Not Byte Reorder.
//		*pau8Data++ = (UINT8)u32ReadData;
//		if ( u32DataLen >= 2 )
//			*pau8Data++ = (UINT8)(u32ReadData>>8);
//		if ( u32DataLen >= 3 )
//			*pau8Data++ = (UINT8)(u32ReadData>>16);
		// Byte Reorder. Do Not Use SPI_CTL.REORDER When USING Quad and Dual Mode.
		*pau8Data++ = (UINT8)(u32ReadData>>16);
		if ( u8ProcBytes >= 2 )
			*pau8Data++ = (UINT8)(u32ReadData>>8);
		if ( u8ProcBytes >= 3 )
			*pau8Data++ = (UINT8)u32ReadData;
	}
}
	
void
SPIFlash_ReadDataAlign(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	PUINT8 pau8Data,
	UINT32 u32DataLen
)
{
	UINT32 *pu32Temp = (UINT32 *)pau8Data;
	// Read data
	SPI_SET_DATA_WIDTH(psSpiFlashHandler->psSpiHandler, 32);
	SPI0_SET_TX_NUM(psSpiFlashHandler->psSpiHandler,SPI0_TXNUM_TWO);
	do
	{
		#if (defined (__ISD9300__) || defined (__I91200__))
		SPI0_WRITE_TX0(psSpiFlashHandler->psSpiHandler,0);
		SPI0_WRITE_TX1(psSpiFlashHandler->psSpiHandler,0);
		#elif (defined (__I94100_SERIES__))
		SPI_WRITE_TX(psSpiFlashHandler->psSpiHandler, 0);
		SPI_WRITE_TX(psSpiFlashHandler->psSpiHandler, 0);
		#else
		SPI0_GO(psSpiFlashHandler->psSpiHandler);  
		#endif
		
		while( SPI_IS_BUSY(psSpiFlashHandler->psSpiHandler) );
		// Byte Reorder. Do Not Use SPI_CTL.REORDER When USING Quad and Dual Mode.
		*(UINT32 *)pu32Temp++ = __REV (SPI0_READ_RX0(psSpiFlashHandler->psSpiHandler));
		// Byte Reorder. Do Not Use SPI_CTL.REORDER When USING Quad and Dual Mode.
		*(UINT32 *)pu32Temp++ = __REV (SPI0_READ_RX1(psSpiFlashHandler->psSpiHandler));
		u32DataLen -= 8;
	}while(u32DataLen>0);
}

/*******************************************************************/
/*             Write API code section                              */
/*******************************************************************/
void
SPIFlash_Write(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT32 u32Addr,
	PUINT8 pau8Data,
	UINT32 u32DataLen
)
{
	UINT32 u32WriteCount;
	
	while(u32DataLen!=0)
	{
		SPIFlash_WriteStart(psSpiFlashHandler, u32Addr);
		u32WriteCount = SPIFlash_WriteData(psSpiFlashHandler,u32Addr, pau8Data, u32DataLen);
		u32Addr += u32WriteCount;
		pau8Data += u32WriteCount;
		u32DataLen -= u32WriteCount;
		SPIFlash_WriteEnd(psSpiFlashHandler);
		// Wait write completely
		SPIFlash_WaitReady(psSpiFlashHandler);
	}
}

void
SPIFlash_WritePage(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT32 u32PageAddr,
	PUINT8 pau8Data
)
{

	SPIFlash_WriteStart(psSpiFlashHandler, u32PageAddr);
	SPIFlash_WriteDataAlign(psSpiFlashHandler, pau8Data);
	SPIFlash_WriteEnd(psSpiFlashHandler);

	SPIFlash_WaitReady(psSpiFlashHandler);
}

void
SPIFlash_WriteStart(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT32 u32ByteAddr
)
{
	SPIFlash_ChipWriteEnable(psSpiFlashHandler, TRUE);

	// Active chip select
	SPI0_SET_SS(psSpiFlashHandler->psSpiHandler, psSpiFlashHandler->u8SlaveDevice);

	// Send Quad Input Page Program command
#if (SPIFLASH_OPERATION_MODE == 0)
	SPIFlash_3ByteAddr_Cmd(psSpiFlashHandler->psSpiHandler, (UINT32)SPIFLASH_WRITE_CMD, u32ByteAddr);
#elif (SPIFLASH_OPERATION_MODE == 1)
	SPIFlash_4ByteAddr_Cmd(psSpiFlashHandler->psSpiHandler, (UINT32)SPIFLASH_WRITE_CMD, u32ByteAddr);
#else
	psSpiFlashHandler->pfnSPIFlashMode(psSpiFlashHandler->psSpiHandler, (UINT32)SPIFLASH_WRITE_CMD, u32ByteAddr);
#endif

	// Enable write interface
	SPIFLASH_WRITE_MODE(psSpiFlashHandler->psSpiHandler);
	// Set data direction as output
	SPIFLASH_OUTPUT_DIRECTION(psSpiFlashHandler->psSpiHandler);
}

void
SPIFlash_WriteEnd(
	S_SPIFLASH_HANDLER *psSpiFlashHandler
)
{
	// Inactive all slave devices
	SPI0_SET_SS(psSpiFlashHandler->psSpiHandler, SPI0_SS_NONE);
	// Disable quad mode
	SPIFLASH_DEFAULT_MODE(psSpiFlashHandler->psSpiHandler);
	// send/receve command in big endian; write/read data in little endian
	SPI_DISABLE_BYTE_REORDER(psSpiFlashHandler->psSpiHandler);
}

UINT32
SPIFlash_WriteData(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT32 u32SPIAddr,
	PUINT8 pau8Data,
	UINT32 u32DataLen
)
{
	UINT32 u32WriteCount, u32TotalWriteCount, u32ProcessByte, u32WriteData;
	
	SPI0_SET_TX_NUM(psSpiFlashHandler->psSpiHandler,SPI0_TXNUM_ONE);
	
	u32WriteCount = 256;
	if ( u32SPIAddr&0xff )//if ( u32SPIAddr%256 )
		 u32WriteCount -=  u32SPIAddr&0xff;//u32SPIAddr%256;
	if ( u32WriteCount > u32DataLen )
		u32WriteCount = u32DataLen;
	u32TotalWriteCount = u32WriteCount;

	if ( ((UINT32)pau8Data)&0x3 )//if ( ((UINT32)pau8Data)%4 )&0x3
	{
		u32ProcessByte = 4 - ( ((UINT32)pau8Data)&0x3 ); //((UINT32)pau8Data)%4;
		if ( u32ProcessByte > u32WriteCount )
			u32ProcessByte = u32WriteCount;
		SPI_SET_DATA_WIDTH(psSpiFlashHandler->psSpiHandler, u32ProcessByte*8);
		u32WriteData = *pau8Data ++;
		if ( u32ProcessByte >= 2 )
			u32WriteData |= (*pau8Data ++)<<8;
		if ( u32ProcessByte == 3 )
			u32WriteData |= (*pau8Data ++)<<16;
		// Byte Reorder. Do Not Use SPI_CTL.REORDER When USING Quad and Dual Mode.
		SPI0_WRITE_TX0(psSpiFlashHandler->psSpiHandler,__REV(u32WriteData));
		SPI0_GO(psSpiFlashHandler->psSpiHandler);
		while( SPI_IS_BUSY(psSpiFlashHandler->psSpiHandler) );
		u32WriteCount -=  u32ProcessByte;
		//pau8Data += u32ProcessByte;
	}

	SPI_SET_DATA_WIDTH(psSpiFlashHandler->psSpiHandler, 32);
	SPI0_SET_TX_NUM(psSpiFlashHandler->psSpiHandler,SPI0_TXNUM_ONE);
	while(u32WriteCount >= 4)
	{
		// Byte Reorder. Do Not Use SPI_CTL.REORDER When USING Quad and Dual Mode.
		SPI0_WRITE_TX0(psSpiFlashHandler->psSpiHandler,__REV(*((PUINT32)pau8Data)));
		SPI0_GO(psSpiFlashHandler->psSpiHandler);
		pau8Data += 4;
		u32WriteCount -= 4;
		while( SPI_IS_BUSY(psSpiFlashHandler->psSpiHandler) );
	}
	if (u32WriteCount)
	{
		SPI_SET_DATA_WIDTH(psSpiFlashHandler->psSpiHandler, u32WriteCount*8);
		u32WriteData = *pau8Data ++;
		if ( u32WriteCount >= 2 )
			u32WriteData |= (*pau8Data ++)<<8;
		if ( u32WriteCount == 3 )
			u32WriteData |= (*pau8Data ++)<<16;
		
		// Byte Reorder. Do Not Use SPI_CTL.REORDER When USING Quad and Dual Mode.
		SPI0_WRITE_TX0(psSpiFlashHandler->psSpiHandler,__REV(u32WriteData));
		SPI0_GO(psSpiFlashHandler->psSpiHandler);
		while( SPI_IS_BUSY(psSpiFlashHandler->psSpiHandler) );
	}
	return u32TotalWriteCount;
}

void
SPIFlash_WriteDataAlign(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	PUINT8 pau8Data
)
{
	UINT32 u32DataLen;
	UINT32 *pu32Temp = (UINT32 *)pau8Data;
	
	SPI_SET_DATA_WIDTH(psSpiFlashHandler->psSpiHandler, 32);
	SPI0_SET_TX_NUM(psSpiFlashHandler->psSpiHandler,SPI0_TXNUM_TWO);
	u32DataLen = 256;
	do
	{
		//DrvSPI_BurstWriteData(psSpiHandler,(PUINT32)pau8Data);
		// Byte Reorder. Do Not Use SPI_CTL.REORDER When USING Quad and Dual Mode.
		SPI0_WRITE_TX0(psSpiFlashHandler->psSpiHandler, __REV(*pu32Temp++));
		// Byte Reorder. Do Not Use SPI_CTL.REORDER When USING Quad and Dual Mode.
		SPI0_WRITE_TX1(psSpiFlashHandler->psSpiHandler, __REV(*pu32Temp++));
		SPI0_GO(psSpiFlashHandler->psSpiHandler);
		//pau8Data += 8;
		while( SPI_IS_BUSY(psSpiFlashHandler->psSpiHandler) );
	}while(	(u32DataLen -= 8) != 0 );
}


UINT32
SPIFlash_GetJedecID(
	S_SPIFLASH_HANDLER *psSpiFlashHandler
)
{
	uint32_t u32Ret;
	
	SPIFlash_SendRecOneData(psSpiFlashHandler, (UINT32)SPIFLASH_JEDEC_ID<<24, 32);
	u32Ret = SPI_READ_RX(psSpiFlashHandler->psSpiHandler);
	u32Ret &= 0x00ffffff;
	return u32Ret;
}


#define SPIFLASH_CHECK_READ_COUNT	100
#define SPIFLASH_CHECK_ID_COUNT		10
void SPIFlash_WaitStable(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT32 u32MaxReadVerifyCount
)
{
	UINT32 u32VerifyCount;
	
	// Make sure SPI flash digital part is ready by checking JEDEC ID
	{
		UINT32 u32CheckJedecID;
		UINT32 u32ReadJedecID;
				
		u32VerifyCount = 0;
		u32CheckJedecID = 0;
		while(u32MaxReadVerifyCount)
		{
			u32MaxReadVerifyCount --;	
			u32ReadJedecID = SPIFlash_GetJedecID(psSpiFlashHandler);
						
			if (((UINT8)u32ReadJedecID) == 0 )
			{
				// memory capacity should not be 0
				continue;
			}
			
			if (u32CheckJedecID == u32ReadJedecID)
			{
				if ( (++u32VerifyCount) == SPIFLASH_CHECK_ID_COUNT )
					break;
			}
			else
			{
				u32CheckJedecID = u32ReadJedecID;
				u32VerifyCount = 0;
			}
		}		
	}
	
	// Make SPI flash leave power down mode if some where or some time had made it entring power down mode
	SPIFlash_PowerDown(psSpiFlashHandler, FALSE);
	
	// Check SPI flash is ready for accessing
	{
		UINT8 ui8ReadByte, ui8CheckByte;
		UINT16 u16Address;
		
		ui8CheckByte = 0;
		u32VerifyCount = 0;
		u16Address = 0;
		while(u32MaxReadVerifyCount)
		{
			u32MaxReadVerifyCount --;
			SPIFlash_Read(psSpiFlashHandler, u16Address, &ui8ReadByte, 1);
			
			if ( (ui8ReadByte==0) || (ui8ReadByte==0xff) )
			{
				u16Address ++;
				u32VerifyCount = 0;
				continue;
			}
			
			if ( ui8ReadByte != ui8CheckByte )
			{
				ui8CheckByte = ui8ReadByte;
				u32VerifyCount = 0;
			}
			else
			{
				if ((++u32VerifyCount) == SPIFLASH_CHECK_READ_COUNT)
					break;
			}
		}
	}
}

// Fast Read Quad IO, address is sent in quad mode and can be set in continuous read mode
/*void SPIFlash_ReadQuadIO(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT32 u32StartAddr,
	PUINT8 pau8Data,
	UINT32 u32DataLen
)
{
    UINT32 u32ReadData;

	// Active chip select
	SPI0_SET_SS(psSpiFlashHandler->psSpiHandler, psSpiFlashHandler->u8SlaveDevice);

	// Send fast read quad IO command in normal mode
	SPI0_SET_DATA_WIDTH(psSpiFlashHandler->psSpiHandler,8);
	SPI0_WRITE_TX0(psSpiFlashHandler->psSpiHandler,SPIFLASH_FAST_4READ);
	while( SPI0_IS_BUSY(psSpiFlashHandler->psSpiHandler) );

	// Enable quad mode
	SPI0_ENABLE_QUAD_MODE(psSpiFlashHandler->psSpiHandler);
	// Set data direction as output to send address
	SPI0_ENABLE_QUAD_OUTPUT_MODE(psSpiFlashHandler->psSpiHandler);
    
	// Send address in quad mode
	SPI0_SET_DATA_WIDTH(psSpiFlashHandler->psSpiHandler,32);
	SPI0_WRITE_TX0(psSpiFlashHandler->psSpiHandler,(u32StartAddr << 8));
    while( SPI0_IS_BUSY(psSpiFlashHandler->psSpiHandler) );

	// Send dummy load
	SPI0_SET_DATA_WIDTH(psSpiFlashHandler->psSpiHandler,16);
	SPI0_WRITE_TX0(psSpiFlashHandler->psSpiHandler, 0X0000);
    while( SPI0_IS_BUSY(psSpiFlashHandler->psSpiHandler) );
    
    // Set data direction as input to receive data
    SPI0_ENABLE_QUAD_INPUT_MODE(psSpiFlashHandler->psSpiHandler);
    // Set receive data width each transaction
    SPI0_SET_DATA_WIDTH(psSpiFlashHandler->psSpiHandler,32);
    // Clear RX FIFO for receiving data
    SPI0_ClearRxFIFO(psSpiFlashHandler->psSpiHandler);
    // Wait RX FIFO to be empty
    while (SPI0_GET_RX_FIFO_EMPTY_FLAG(psSpiFlashHandler->psSpiHandler) == 0);
    
    while (u32DataLen)
    {
        // Write to TX to generate SPI clock for receiving data
        SPI0_WRITE_TX0(psSpiFlashHandler->psSpiHandler,0);
        while( SPI0_IS_BUSY(psSpiFlashHandler->psSpiHandler) );
        // Read data from RX FIFO
        u32ReadData = SPI0_READ_RX0(psSpiFlashHandler->psSpiHandler);
        *((UINT32*)pau8Data) = u32ReadData;
        // Read 4 byte each time
        pau8Data+=4;
        u32DataLen-=4;
    }
    SPI0_ClearRxFIFO(psSpiFlashHandler->psSpiHandler);
    // Wait RX FIFO to be empty
    while (SPI0_GET_RX_FIFO_EMPTY_FLAG(psSpiFlashHandler->psSpiHandler) == 0);

    SPI0_SET_SS(psSpiFlashHandler->psSpiHandler, SPI0_SS_NONE);
    SPI0_DISABLE_QUAD_MODE(psSpiFlashHandler->psSpiHandler);
}*/
