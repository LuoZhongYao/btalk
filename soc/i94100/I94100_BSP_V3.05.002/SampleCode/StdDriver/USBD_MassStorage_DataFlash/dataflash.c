/******************************************************************************
 * @file     DataFlash_RW.c
 * @brief    
 *
 * @note
 * Copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/

/*!<Includes */
#include <stdio.h>
#include <string.h>
#include "fat.h"
#include "Platform.h"
#include "massstorage.h"
#include "usbd_bot.h"
#include "dataflash.h"
#include "fmc.h"


uint32_t g_sectorBuf[FLASH_PAGE_SIZE / 4];
volatile uint8_t flash_sectors_manager[16][8] = {0};

//////////////////////////////////////////////////////////////////
/*
 *  Set Stall for MSC Endpoint
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *    Return Value:    None
 */

void MSC_SetStallEP (uint32_t EPNum) 
{          	
	EPNum &= 0x7F;		  
	USBD_SET_EP_STALL(EPNum);
}


/*
 *  MSC Mass Storage Reset Request Callback
 *   Called automatically on Mass Storage Reset Request
 *    Parameters:      None (global SetupPacket and EP0Buf)
 *    Return Value:    TRUE - Success, FALSE - Error
 */

uint32_t MSC_Reset (void) 
{  
	g_u32Offset = g_u32Length = 0;
	g_u8BotState = BOT_IDLE;
	g_sCBW.dSignature = BOT_CBW_SIGNATURE;
	return (TRUE);
}

#define HWORD(n) ((n) & 0xFF), (((n) >> 8) & 0xFF)
#define WORD(n)  HWORD(n & 0xFFFF), HWORD((n >> 16) & 0xFFFF)
#define DWORD(n) WORD(n & 0xFFFFFFFF), WORD((n >> 32) & 0xFFFFFFFF)

#define BYTES_PER_SEC	4096
#define SECTOR_NUMBER	(DATA_FLASH_STORAGE_SIZE / BYTES_PER_SEC)
#define SEC_PER_CLUSTER 1
#define FAT1_OFFSET		(1 * BYTES_PER_SEC)
#define FAT2_OFFSET		(2 * BYTES_PER_SEC)
#define ROOT_DIR_OFFSET	(3 * BYTES_PER_SEC)
#define ROOT_DIR_NUMBER	(BYTES_PER_SEC / 32)
#define KERNEL_OFFSET	(4 * BYTES_PER_SEC)
#define KERNEL_CLUSTER_NUMBER	124
#define KERNEL_SIZE		(KERNEL_CLUSTER_NUMBER * BYTES_PER_SEC)
#define CONFIG_SIZE		(1 * BYTES_PER_SEC)

static const uint8_t sector0[] = {
	0xEB, 0x3C, 0x90,			/* boot jump code (x86) */
	'M', 'S', 'D', 'O', 'S', '5', '.', '0',	/* OEM name*/
	HWORD(BYTES_PER_SEC),		/* bytes per sector */
	SEC_PER_CLUSTER,			/* sector per cluster */
	HWORD(1),					/* size of reserved area */
	2,							/* number of FATs */
	HWORD(ROOT_DIR_NUMBER),		/* number of root directory entries */
	HWORD(SECTOR_NUMBER),		/* volume size in 16-bit LBA */
	0xF8,						/* Media descriptor byte */
	HWORD(1),					/* Number of sector per FAT */
	HWORD(1),					/* Number of sectors per track */
	HWORD(1),					/* Number of heads */
	WORD(0),					/* volume offset in the physical drive */
	WORD(0),					/* volume size in 32-bit LBA */
	0x80,						/* Driver number */
	0x00,						/* reserved */
	0x29,						/* Extended boot signature */
	WORD(1),					/* volume serial number */
	'N', 'O', ' ', 'N', 'A', 'M', 'E', ' ', ' ', ' ', ' ',
	'F', 'A', 'T', '1', '2', ' ', ' ', ' ',
};

static uint8_t fat1[4096];

struct dir
{
	uint8_t d_name[8];
	uint8_t d_suffix[3];
	uint8_t d_attr;
	uint8_t reserved;
	uint8_t d_crttime;
	uint16_t d_crttime1;
	uint16_t d_crtdate;
	uint16_t d_assdate;
	uint16_t d_eaindex;
	uint16_t d_wrtime;
	uint16_t d_wrdate;
	uint16_t d_clus;
	uint32_t d_size;
} __attribute__((packed));

static const struct dir root_dir[] = {
	{
		.d_name = {'k', 'e', 'r', 'n', 'e', 'l', ' ', ' '},
		.d_suffix = {'i', 'm', 'g'},
		.d_attr = 0x20,
		.d_clus = 0x0002,
		.d_size = KERNEL_SIZE,
	},
	{
		.d_name = {'c', 'o', 'n', 'f', 'i', 'g', ' ', ' '},
		.d_suffix = {'i', 'm', 'g'},
		.d_attr = 0x20,
		.d_clus = 0x0003,
		.d_size = CONFIG_SIZE,
	},
};

#define MIN(a, b)	(a) < (b) ? (a) : (b)
#define MAX(a, b)	(a) > (b) ? (a) : (b)

void MSC_MemoryInit(void)
{
	uint32_t i;
	fat1[0] = 0xF8;
	fat1[1] = 0xFF;
	fat1[2] = 0xFF;
	for (i= 1; i < KERNEL_CLUSTER_NUMBER; i++) {
		fat1[3 * i + 0] = (i * 2) & 0xFF;
		fat1[3 * i + 1] = (((i * 2) >> 8) & 0xF) |  ((((i * 2) + 1) & 0xF) << 4);
		fat1[3 * i + 2] = (((i * 2) + 1) >> 4) & 0xFF;
	}

	fat1[3 * i + 0] = 0xF8;
	fat1[3 * i + 1] = 0xFF;
}

/*
 *  MSC Memory Read Callback
 *   Called automatically on Memory Read Event
 *    Parameters:      None (global variables)
 *    Return Value:    None
 */

void MSC_MemoryRead (void) 
{
	uint8_t *pu8Src, *pu8Dst;
	uint32_t i, pu32Buf[64/4];
	uint32_t Transmit_Len;

	Transmit_Len = GetMin(g_u32Length, MSC_MAX_PACKET);

	memset(pu32Buf, 0, Transmit_Len);
	printf("read offset %x, size = %x\n", g_u32Offset, Transmit_Len);
	if(g_u32Offset <= 0x1FE && ((g_u32Offset + Transmit_Len) >= 0x200))
	{
		uint32_t off = 0x1FE - g_u32Offset;
		uint32_t len = GetMin(Transmit_Len - off, 2);

		memcpy(pu32Buf + off, (uint8_t []){0x55, 0xAA}, len);
	} else if (g_u32Offset < sizeof(sector0)) {
		uint32_t len = GetMin(Transmit_Len, sizeof(sector0) - g_u32Offset);
		memcpy(pu32Buf, sector0 + g_u32Offset, len);
	} else if (g_u32Offset >= FAT1_OFFSET &&
		g_u32Offset < FAT1_OFFSET + sizeof(fat1))
	{
		uint32_t off = g_u32Offset - FAT1_OFFSET;
		uint32_t len = GetMin(Transmit_Len, sizeof(fat1) - off);
		memcpy(pu32Buf, fat1 + off, len);
	} else if (g_u32Offset >= FAT2_OFFSET &&
		g_u32Offset < FAT2_OFFSET + sizeof(fat1))
	{
		uint32_t off = g_u32Offset - FAT2_OFFSET;
		uint32_t len = GetMin(Transmit_Len, sizeof(fat1) - off);
		memcpy(pu32Buf, fat1 + off, len);
	} else if (g_u32Offset >= ROOT_DIR_OFFSET &&
		g_u32Offset < ROOT_DIR_OFFSET + sizeof(root_dir))
	{
		uint32_t off = g_u32Offset - ROOT_DIR_OFFSET;
		uint32_t len = GetMin(Transmit_Len, sizeof(root_dir) - off);

		memcpy(pu32Buf, root_dir + off, len);
	}

	pu8Src = (uint8_t *)&pu32Buf;
	pu8Dst = (uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(BULK_IN_EP));

	for(i = 0 ; i < Transmit_Len ;i++){
		pu8Dst[i] = pu8Src[i]; 
	}	

	USBD_SET_PAYLOAD_LEN(BULK_IN_EP, Transmit_Len);

	g_u32Offset += Transmit_Len;
	g_u32Length -= Transmit_Len;

	g_sCSW.dDataResidue -= Transmit_Len;

	if ( g_u32Length == 0 ) 	
	{

		g_u8BotState = BOT_DATA_IN_LAST;
	}

	if (g_u8BotState != BOT_DATA_IN) 
	{
		g_sCSW.bStatus = CSW_CMD_PASSED;
	}
}

/*
 *  MSC Memory Write Callback
 *   Called automatically on Memory Write Event
 *    Parameters:      None (global variables)
 *    Return Value:    None
 */

void MSC_MemoryWrite (void) 	
{
	uint32_t i;


	for (i = 0; i < g_u8BulkLen; i++) 
	{
		g_au8Memory[g_u32Offset + i] = g_au8BulkBuf[i];				
	}

	g_u32Offset += g_u8BulkLen;
	g_u32Length -= g_u8BulkLen;

	g_sCSW.dDataResidue -= g_u8BulkLen;

	USBD_SET_PAYLOAD_LEN(BULK_OUT_EP, EP3_MAX_PKT_SIZE);    

	/* 4KB Buffer full. Writer it to storage first. */
	if ( (g_u32Offset - g_u32DataFlashStartAddr) == UDC_SECTOR_SIZE )
	{
		//DataFlashWrite(g_u32DataFlashStartAddr, UDC_SECTOR_SIZE, (uint32_t)&g_au8Memory[g_u32DataFlashStartAddr] /*STORAGE_DATA_BUF*/);
		g_u32DataFlashStartAddr = g_u32Offset;		
	}

	if ((g_u32Length == 0) || (g_u8BotState == BOT_CSW_Send)) 
	{	
		Set_Pkt_CSW (CSW_CMD_PASSED, SEND_CSW_ENABLE);
	}

}


/*
 *  MSC Memory Verify Callback
 *   Called automatically on Memory Verify Event
 *    Parameters:      None (global variables)
 *    Return Value:    None
 */
void MSC_MemoryVerify (void) 
{
	uint32_t n;
	uint8_t CSW_Status;

	if ((g_u32Offset + g_u8BulkLen) > DATA_FLASH_STORAGE_SIZE) {
		g_u8BulkLen = DATA_FLASH_STORAGE_SIZE - g_u32Offset;
		g_u8BotState = BOT_CSW_Send;
		MSC_SetStallEP(BULK_OUT_EP);		
	}

	for (n = 0; n < g_u8BulkLen; n++) 
	{
		if (g_au8Memory[g_u32Offset + n] != g_au8BulkBuf[n]) 
		{
			g_u32MemOK = FALSE;
			break;
		}
	}

	g_u32Offset += g_u8BulkLen;
	g_u32Length -= g_u8BulkLen;

	g_sCSW.dDataResidue -= g_u8BulkLen;

	if ((g_u32Length == 0) || (g_u8BotState == BOT_CSW_Send)) 
	{
		CSW_Status = (g_u32MemOK) ? CSW_CMD_PASSED : CSW_CMD_FAILED;
		Set_Pkt_CSW (CSW_Status, SEND_CSW_ENABLE);
	}
}



