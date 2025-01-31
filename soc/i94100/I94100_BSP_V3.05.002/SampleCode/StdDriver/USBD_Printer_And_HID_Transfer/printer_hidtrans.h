/******************************************************************************
 * @file     micro_printer_and_hid_transfer.h
 * @brief    M480 series USB driver header file
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 16/08/31 5:34p $
 *
 * @note
 * Copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#ifndef __PRINTER_AND_HIDTRANS_H__
#define __PRINTER_AND_HIDTRANS_H__

#include "BUFCTRL.h"

/* Define HID transfer buffer */
#define HIDTRANS_PAGESIZE        2048
#define HIDTRANS_SECTORSIZE      4096

/* Declare HID transfer APIs */
void Printer_HIDTrans_Init(void);
void Printer_HIDTrans_ClassRequest(void);

// HID transfer callback function.
void HIDTrans_EraseSector(uint32_t u32StartSector,uint32_t u32Sectors);
void HIDTrans_PrepareReadPage(uint32_t* pu32Address,uint32_t u32StartPage,uint32_t u32Pages);
void HIDTrans_PrepareWritePage(uint32_t* pu32Address,uint32_t u32StartPage,uint32_t u32Pages);
void HIDTrans_GetWriteData(uint32_t u32Address,uint32_t u32Pages);

// Micro printer callback function.
void Printer_ReceiveData(uint8_t* pu8DataBuf, uint32_t u32DataCount);

/* Define the vendor id and product id */
#define USBD_VID        0x0416
#define USBD_PID        0x1003

/*!<Define HID Class Specific Request */
#define GET_REPORT          0x01
#define GET_IDLE            0x02
#define GET_PROTOCOL        0x03
#define SET_REPORT          0x09
#define SET_IDLE            0x0A
#define SET_PROTOCOL        0x0B

/*!<USB HID Interface Class protocol */
#define HID_NONE            0x00
#define HID_KEYBOARD        0x01
#define HID_MOUSE           0x02

/*!<USB HID Class Report Type */
#define HID_RPT_TYPE_INPUT      0x01
#define HID_RPT_TYPE_OUTPUT     0x02
#define HID_RPT_TYPE_FEATURE    0x03

/*-------------------------------------------------------------*/
/* Define EP maximum packet size */
#define EP0_MAX_PKT_SIZE    8
#define EP1_MAX_PKT_SIZE    EP0_MAX_PKT_SIZE
#define EP2_MAX_PKT_SIZE    64
#define EP3_MAX_PKT_SIZE    64
#define EP4_MAX_PKT_SIZE    8
#define EP5_MAX_PKT_SIZE    64
#define EP6_MAX_PKT_SIZE    64

#define SETUP_BUF_BASE      0
#define SETUP_BUF_LEN       8
#define EP0_BUF_BASE        (SETUP_BUF_BASE + SETUP_BUF_LEN)
#define EP0_BUF_LEN         EP0_MAX_PKT_SIZE
#define EP1_BUF_BASE        (SETUP_BUF_BASE + SETUP_BUF_LEN)
#define EP1_BUF_LEN         EP1_MAX_PKT_SIZE
#define EP2_BUF_BASE        (EP1_BUF_BASE + EP1_BUF_LEN)
#define EP2_BUF_LEN         EP2_MAX_PKT_SIZE
#define EP3_BUF_BASE        (EP2_BUF_BASE + EP2_BUF_LEN)
#define EP3_BUF_LEN         EP3_MAX_PKT_SIZE
#define EP4_BUF_BASE        (EP3_BUF_BASE + EP3_BUF_LEN)
#define EP4_BUF_LEN         EP4_MAX_PKT_SIZE
#define EP5_BUF_BASE        (EP4_BUF_BASE + EP4_BUF_LEN)
#define EP5_BUF_LEN         EP5_MAX_PKT_SIZE
#define EP6_BUF_BASE        (EP5_BUF_BASE + EP5_BUF_LEN)
#define EP6_BUF_LEN         EP6_MAX_PKT_SIZE

/* Define the interrupt In EP number */
#define BULK_IN_EP_NUM      0x01
#define BULK_OUT_EP_NUM     0x02
#define INT_IN_EP_NUM       0x03
#define INT_IN_EP_NUM_1     0x04
#define INT_OUT_EP_NUM      0x05

/* Define Descriptor information */
#define HID_DEFAULT_INT_IN_INTERVAL     1
#define USBD_SELF_POWERED               0
#define USBD_REMOTE_WAKEUP              0
#define USBD_MAX_POWER                  50  /* The unit is in 2mA. ex: 50 * 2mA = 100mA */

/************************************************/
#define  GET_PORT_STATUS           0x01

#endif  /* __USBD_PRINTER_H_ */

/*** (C) COPYRIGHT 2016 Nuvoton Technology Corp. ***/
