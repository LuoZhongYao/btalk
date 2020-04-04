/****************************************************************
 *                                                              *
 * Copyright (c) Nuvoton Technology Corp. All rights reserved.  *
 *                                                              *
 ****************************************************************/

#ifndef __SPIFLASH_H__
#define __SPIFLASH_H__

// Include header file
#include "Platform.h"
//#include "SysInfra.h"

#ifdef  __cplusplus
extern "C"
{
#endif
	
// Version number
#define SPIFLASH_MAJOR_NUM	5
#define SPIFLASH_MINOR_NUM	3
#define SPIFLASH_BUILD_NUM	0
#define _SYSINFRA_VERSION(MAJOR_NUM, MINOR_NUM, BUILD_NUM)			 (((MAJOR_NUM) << 16) | ((MINOR_NUM) << 8) | (BUILD_NUM))	
#define SPIFLASH_VERSION_NUM	_SYSINFRA_VERSION(SPIFLASH_MAJOR_NUM, SPIFLASH_MINOR_NUM, SPIFLASH_BUILD_NUM)

#define SPIFLASH_INTERFACE_MODE	(2)		//0: device is one-bit mode; 1: device is dual-mode; 2: device is quad mode.
//SPI Flash 3byte, 4byte address mode selection
#define SPIFLASH_OPERATION_MODE	(0)		//0: device is 3byte address; 1: device is 4byte address; 2: device can change 3byte/4byte address.
	
// ------------------------------------------------------------------------------
// Define the Error Code
// ------------------------------------------------------------------------------
// E_SPIFLASH_BUSY				Read/Write Data Busy
#define E_SPIFLASH_BUSY         _SYSINFRA_ERRCODE(TRUE, MODULE_ID_SPIFLASH, 1)

// ------------------------------------------------------------------------------
// Define SPI Flash Status1
// ------------------------------------------------------------------------------
#define SPIFLASH_SPR			0x80	// Status Register Protect
#define SPIFLASH_R				0x40	// Reserved Bit
#define SPIFLASH_BP3     		0x20	// Block Protect Bit 3
#define SPIFLASH_BP2			0x10	// Block Protect Bit 2
#define SPIFLASH_BP1			0x08	// Block Protect Bit 1
#define SPIFLASH_BP0			0x04	// Block Protect Bit 0
#define SPIFLASH_WEL			0x02	// Write Enable Latch
#define SPIFLASH_BUSY			0x01	// BUSY
#define SPIFLASH_BP				(SPIFLASH_BP3|SPIFLASH_BP2|SPIFLASH_BP1|SPIFLASH_BP0)

// ------------------------------------------------------------------------------
// Define SPI Flash Instruction Set
// ------------------------------------------------------------------------------
#define SPIFLASH_ZERO			0x00
#define SPIFLASH_DUMMY			0xFF

#define SPIFLASH_WRITE_ENABLE	0x06
#define SPIFLASH_WRITE_DISABLE	0x04
#define SPIFLASH_READ_STATUS	0x05
#define SPIFLASH_WRITE_STATUS	0x01

#define SPIFLASH_READ_DATA		0x03	// Using fast read to replace normal read
#define SPIFLASH_FAST_READ		0x0B
#define SPIFLASH_FAST_READ_4ADD		0x0C
#define SPIFLASH_FAST_DREAD		0x3B	// Address is one bits per clock.
#define SPIFLASH_FAST_2READ		0xBB	// Address is two bits per clock, This reduced instruction overhead
#define SPIFLASH_FAST_QREAD		0x6B	// Address is one bits per clock.
#define SPIFLASH_FAST_4READ		0xEB	// Address is four bits per clock, This reduced instruction overhead.

#define SPIFLASH_PAGE_PROGRAM	0x02
#define SPIFLASH_QPAGE_PROGRAM  0x32

#define SPIFLASH_4K_ERASE		0x20
#define SPIFLASH_32K_ERASE		0x52
#define SPIFLASH_64K_ERASE		0xD8
#define SPIFLASH_SUSPEND_EP		0x75
#define SPIFLASH_RESUME_EP		0x7A

#define SPIFLASH_CHIP_ERASE		0xC7
#define SPIFLASH_POWER_DOWN		0xB9
#define SPIFLASH_RELEASE_PD_ID	0xAB
#define SPIFLASH_DEVICE_ID		0x90
#define SPIFLASH_JEDEC_ID		0x9F
#define SPIFLASH_READ_SFDP		0x5A
#define SPIFLASH_EN4B_MODE		0xB7
#define SPIFLASH_EX4B_MODE		0xE9

// ------------------------------------------------------------------------------
// Define SPI Flash Page Size
// ------------------------------------------------------------------------------
#define SPIFLASH_PAGE_SIZE	256
#define SPIFLASH_SECTOR_SIZE 4096
#define SPIFLASH_BLOCK32_SIZE 32768
#define SPIFLASH_BLOCK64_SIZE 65536

// ------------------------------------------------------------------------------
// Define SPI Flash Property Flag
// ------------------------------------------------------------------------------
#define SPIFLASH_FLAG_LOW_CAPACITY		0x00  
#define SPIFLASH_FLAG_HIGH_CAPACITY		0x01
#define SPIFLASH_FLAG_DUAL				0x02
#define SPIFLASH_FLAG_QUAD				0x04

typedef void (*PFN_SPIFLASH_MODE)(SPI_T  *psSpiHandler, UINT32 u32Cmd, UINT32 u32ByteAddr);

typedef struct
{
	SPI_T   *psSpiHandler;				// SPI access handler
#if (SPIFLASH_OPERATION_MODE == 2)
	PFN_SPIFLASH_MODE pfnSPIFlashMode;	// for change 3 or 4 byte address command function
#endif
//#if (SPIFLASH_INTERFACE_MODE == 2)		// for quad mode
	UINT32 pu32MISO1Port;
	UINT32 u32MISO1Pin;
	UINT32 u32MISO1MFP;
//#endif
	UINT32  u32FlashSize;				// SPIFlash memory size
	UINT8   u8SlaveDevice;              // SPIFlash is on device1/2
	UINT8   u8Flag;
} S_SPIFLASH_HANDLER;

typedef enum
{
	eSTATUS_REG1 = 0x00,
	eSTATUS_REG2 = 0x30,
	eSTATUS_REG3 = 0x10
} E_SPIFLASH_STATUS_REGISTER;


// APIs declaration

/*******************************************************************/
/*             Miscellaneous API declaration                       */
/*******************************************************************/
void
SPIFlash_Open(
	SPI_T *psSpiHandler,
	UINT8 u8DrvSlaveDevice,
	UINT32 u32SpiClk,
	S_SPIFLASH_HANDLER *psSpiFlashHandler
);

__STATIC_INLINE void
SPIFlash_Close(
	S_SPIFLASH_HANDLER *psSpiFlashHandler
)
{
	SPI_Close(psSpiFlashHandler->psSpiHandler);
}

__STATIC_INLINE UINT32
SPIFlash_GetSPIClock(
   S_SPIFLASH_HANDLER *psSpiFlashHandler
)
{
	return SPI_GetBusClock(psSpiFlashHandler->psSpiHandler);
}

void
SPIFlash_SendRecOneData(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT32 u32Data,
	UINT8  u8DataLen
);

void
SPIFlash_GetChipInfo(
	S_SPIFLASH_HANDLER *psSpiFlashHandler
);

UINT8
SPIFlash_ReadStatusReg(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	E_SPIFLASH_STATUS_REGISTER eStatusReg
);

void
SPIFlash_WriteStatusRegEx(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	E_SPIFLASH_STATUS_REGISTER eStatusReg,
	UINT16 u16Status,
	UINT8 u8Length
);

__STATIC_INLINE void
SPIMFlash_WriteStatusReg(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT8 u8Status
)
{
	SPIFlash_WriteStatusRegEx(psSpiFlashHandler, eSTATUS_REG1, u8Status, 16);
}

void
SPIFlash_PowerDown(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	BOOL	bEnable
);

void
SPIFlash_WaitReady(
	S_SPIFLASH_HANDLER *psSpiFlashHandler
);

BOOL
SPIFlash_CheckBusy(
	S_SPIFLASH_HANDLER *psSpiFlashHandler
);

UINT32
SPIFlash_GetVersion(void);

void
SPIFlash_ReadSFDP(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT8 u8ByteAddr,
	UINT8 u8ByteLen
);

void
SPIFlash_EN4BAddress(
	S_SPIFLASH_HANDLER *psSpiFlashHandler
);

void
SPIFlash_EX4BAddress(
	S_SPIFLASH_HANDLER *psSpiFlashHandler
);


UINT32
SPIFlash_GetJedecID(
	S_SPIFLASH_HANDLER *psSpiFlashHandler
);

void SPIFlash_WaitStable(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT32 u32MaxReadVerifyCount
);

/*******************************************************************/
/*             Protection API declaration                          */
/*******************************************************************/
void
SPIFlash_ChipWriteEnable(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	BOOL bEnableWrite
);

void
SPIFlash_GlobalProtect(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	BOOL bEnableGlobalProtect
);

/*******************************************************************/
/*             Write API declaration                               */
/*******************************************************************/
void
SPIFlash_Write(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT32 u32Addr,
	PUINT8 pau8Data,
	UINT32 u32DataLen
);

void
SPIFlash_WritePage(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT32 u32PageAddr,
	PUINT8 pau8Data
);

void
SPIFlash_WriteStart(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT32 u32ByteAddr
);

void
SPIFlash_WriteEnd(
	S_SPIFLASH_HANDLER *psSpiFlashHandler
);

UINT32
SPIFlash_WriteData(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT32 u32SPIAddr,
	PUINT8 pau8Data,
	UINT32 u32DataLen
);

void
SPIFlash_WriteDataAlign(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	PUINT8 pau8Data
);

/*******************************************************************/
/*             Read API declaration                                */
/*******************************************************************/
void
SPIFlash_Read(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT32 u32ByteAddr,
	PUINT8 pau8Data,
	UINT32 u32DataLen
);

void
SPIFlash_BurstRead(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT32 u32ByteAddr,
	PUINT8 pau8Data,
	UINT32 u32DataLen
);

void
SPIFlash_ReadStart(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT32 u32ByteAddr
);

void
SPIFlash_ReadEnd(
	S_SPIFLASH_HANDLER *psSpiFlashHandler
);

void
SPIFlash_ReadData(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	PUINT8 pau8Data,
	UINT32 u32DataLen
);

void
SPIFlash_ReadDataAlign(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	PUINT8 pau8Data,
	UINT32 u32DataLen
);

void SPIFlash_ReadQuadIO(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT32 u32StartAddr,
	PUINT8 pau8Data,
	UINT32 u32DataLen
);

/*******************************************************************/
/*             Erase API declaration                               */
/*******************************************************************/
/********* none blocking releate APIs *********/
void
SPIFlash_EraseStart(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT8 u8Cmd,
	UINT32 u32Addr
);

__STATIC_INLINE void
SPIFlash_Erase64KStart(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT16  u16IndexOf64K
)
{
	SPIFlash_EraseStart(psSpiFlashHandler, SPIFLASH_64K_ERASE, (u16IndexOf64K<<16));
}

__STATIC_INLINE void
SPIFlash_Erase4KStart(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT16 u16IndexOf4K
)
{
	SPIFlash_EraseStart(psSpiFlashHandler, SPIFLASH_4K_ERASE, (u16IndexOf4K<<12));
}

__STATIC_INLINE void
SPIFlash_Erase32KStart(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT16 u16IndexOf32K
)
{
	SPIFlash_EraseStart(psSpiFlashHandler, SPIFLASH_32K_ERASE, (u16IndexOf32K<<15));
}

__STATIC_INLINE void
SPIFlash_EraseChipStart(
	S_SPIFLASH_HANDLER *psSpiFlashHandler
)
{
	SPIFlash_ChipWriteEnable(psSpiFlashHandler, TRUE);
	SPIFlash_SendRecOneData(psSpiFlashHandler,SPIFLASH_CHIP_ERASE,8);
}

/********* blocking releate APIs *********/
__STATIC_INLINE void
SPIFlash_Erase(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT8 u8Cmd,
	UINT32 u32Addr
)
{
	SPIFlash_EraseStart(psSpiFlashHandler, u8Cmd, u32Addr);
	// Wait erase complete
	SPIFlash_WaitReady(psSpiFlashHandler);
}

__STATIC_INLINE void
SPIFlash_Erase64K(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT16  u16IndexOf64K
)
{
	SPIFlash_Erase(psSpiFlashHandler, SPIFLASH_64K_ERASE, (u16IndexOf64K<<16));
}

__STATIC_INLINE void
SPIFlash_Erase4K(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT16 u16IndexOf4K
)
{
	SPIFlash_Erase(psSpiFlashHandler, SPIFLASH_4K_ERASE, (u16IndexOf4K<<12));
}

__STATIC_INLINE void
SPIFlash_Erase32K(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT16 u16IndexOf32K
)
{
	SPIFlash_Erase(psSpiFlashHandler, SPIFLASH_32K_ERASE, (u16IndexOf32K<<15));
}

__STATIC_INLINE void
SPIFlash_EraseChip(
	S_SPIFLASH_HANDLER *psSpiFlashHandler
)
{
	SPIFlash_EraseChipStart(psSpiFlashHandler);
	// Wait erase complete
	SPIFlash_WaitReady(psSpiFlashHandler);
}

/*******************************************************************/
/*             Quad Mode Eanbel API By Manufacturer                */
/*******************************************************************/
__STATIC_INLINE void
SPIFlash_QuadMode_W25Q16BV(S_SPIFLASH_HANDLER *psSpiFlashHandler)
{
	UINT16 u16Status;
	u16Status = (SPIFlash_ReadStatusReg(psSpiFlashHandler, eSTATUS_REG1) << 8) | SPIFlash_ReadStatusReg(psSpiFlashHandler, eSTATUS_REG2);
	SPIFlash_WriteStatusRegEx(psSpiFlashHandler, eSTATUS_REG1, u16Status|0x02, 24);           
}

__STATIC_INLINE void
SPIFlash_QuadMode_W25Q64DW(S_SPIFLASH_HANDLER *psSpiFlashHandler)
{
	SPIFlash_QuadMode_W25Q16BV(psSpiFlashHandler);
}

__STATIC_INLINE void
SPIFlash_QuadMode_W25Q80BV(S_SPIFLASH_HANDLER *psSpiFlashHandler)
{
	SPIFlash_QuadMode_W25Q16BV(psSpiFlashHandler);
}

__STATIC_INLINE void
SPIFlash_QuadMode_GD25Q16C(S_SPIFLASH_HANDLER *psSpiFlashHandler)
{
	SPIFlash_QuadMode_W25Q16BV(psSpiFlashHandler);
}

__STATIC_INLINE void
SPIFlash_QuadMode_W25Q16FW(S_SPIFLASH_HANDLER *psSpiFlashHandler)
{
	UINT8 u8Status;
	u8Status = SPIFlash_ReadStatusReg(psSpiFlashHandler, eSTATUS_REG2);
	SPIFlash_WriteStatusRegEx(psSpiFlashHandler, eSTATUS_REG2, (UINT16)(u8Status|0x02), 16);
}

__STATIC_INLINE void
SPIFlash_QuadMode_W25Q256FV(S_SPIFLASH_HANDLER *psSpiFlashHandler)
{
	UINT8 u8Status;

	u8Status = SPIFlash_ReadStatusReg(psSpiFlashHandler, eSTATUS_REG2);
		
	SPIFlash_ChipWriteEnable(psSpiFlashHandler, TRUE);
	// Status2 Bit-1(S9): Quad-mode enable.
	SPIFlash_SendRecOneData(psSpiFlashHandler,((SPIFLASH_WRITE_STATUS|eSTATUS_REG2)<<8)|u8Status|0x02, 16);

	SPIFlash_WaitReady(psSpiFlashHandler);
}

__STATIC_INLINE void
SPIFlash_QuadMode_W25Q64FV(S_SPIFLASH_HANDLER *psSpiFlashHandler)
{
	UINT8 u8Status1;
	UINT8 u8Status2;
	u8Status1 = SPIFlash_ReadStatusReg(psSpiFlashHandler, eSTATUS_REG1);
	u8Status2 = SPIFlash_ReadStatusReg(psSpiFlashHandler, eSTATUS_REG2);
		
	SPIFlash_ChipWriteEnable(psSpiFlashHandler, TRUE);
	// Status2 Bit-1(S9): Quad-mode enable.

	SPIFlash_SendRecOneData(psSpiFlashHandler,((SPIFLASH_WRITE_STATUS<<16)|(u8Status1<<8)|(u8Status2|0x2)), 24);

	SPIFlash_WaitReady(psSpiFlashHandler);
}

__STATIC_INLINE void
SPIFlash_QuadMode_EN25QH256(S_SPIFLASH_HANDLER *psSpiFlashHandler)
{
	UINT8 u8Status;
	u8Status = SPIFlash_ReadStatusReg(psSpiFlashHandler, eSTATUS_REG1);
	
	//SPIFlash_WriteStatusRegEx(psSpiFlashHandler, eSTATUS_REG2, (UINT16)(u8Status|0x02), 16);
	
	SPIFlash_ChipWriteEnable(psSpiFlashHandler, TRUE);
	// Status2 Bit-2: Quad-mode enable.
	SPIFlash_SendRecOneData(psSpiFlashHandler,((SPIFLASH_WRITE_STATUS|eSTATUS_REG1)<<8)|u8Status|0x20, 16);
	SPIFlash_WaitReady(psSpiFlashHandler);
	
	// Check if the QE bit is enable.
	u8Status = SPIFlash_ReadStatusReg(psSpiFlashHandler, eSTATUS_REG1);
	if(u8Status & 0x20)
	{
		uint32_t *pu32Port;
		
		// If the SPI Flash quad-mode isenable, set the MFP back.
		#if (defined (__I94100_SERIES__))
		pu32Port = (uint32_t *)(psSpiFlashHandler->pu32MISO1Port);
		*pu32Port = (*pu32Port & ~(psSpiFlashHandler->u32MISO1Pin)) | psSpiFlashHandler->u32MISO1MFP;
		#endif
	}
}

__STATIC_INLINE void
SPIFlash_QuadMode_MX25L3235E(S_SPIFLASH_HANDLER *psSpiFlashHandler)
{
	UINT8 u8Status;
	u8Status = SPIFlash_ReadStatusReg(psSpiFlashHandler, eSTATUS_REG1);
	SPIFlash_WriteStatusRegEx(psSpiFlashHandler, eSTATUS_REG1, (u8Status|0x40)<<8, 24);
}

#ifdef  __cplusplus
}
#endif

#endif	// __SPIFLASH_H__

