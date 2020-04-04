/******************************************************************************
 * @file     hid_trans.c
 * @brief    I94100 series USBD HID transfer sample file
 *
 * @note
 * Copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/

/*!<Includes */
#include <stdio.h>
#include <string.h>
#include "mtp.h"
#include "usb_mtp.h"
#include "Platform.h"

struct mtp {
	uint32_t session_id;
	uint32_t transaction_id;
};

static struct mtp mtp;

uint16_t get_u16(uint8_t *ptr)
{
	return ptr[0] | ptr[1] << 8;
}

uint32_t get_u32(uint8_t *ptr)
{
	return ptr[0] | ptr[1] << 8 | ptr[2] << 16 | ptr[3] << 24;
}

void put_u16(uint8_t *ptr, uint16_t value)
{
	ptr[0] = value & 0xff;
	ptr[1] = (value >> 8) & 0xff;
}

void put_u32(uint8_t *ptr, uint32_t value)
{
	ptr[0] = value & 0xff;
	ptr[1] = (value >> 8) & 0xff;
	ptr[2] = (value >> 16) & 0xff;
	ptr[3] = (value >> 24) & 0xff;
}

#define SetContainerCode(ptr, code) put_u16(ptr + MTP_CONTAINER_CODE_OFFSET, code)
#define SetContainerLength(ptr, length) put_u32(ptr + MTP_CONTAINER_LENGTH_OFFSET, length)
#define SetContainerType(ptr, type) put_u16(ptr + MTP_CONTAINER_TYPE_OFFSET, type)
#define SetTransactionID(ptr, id) put_u32(ptr + MTP_CONTAINER_TRANSACTION_ID_OFFSET, id)

void sendResponse(uint16_t type, uint16_t code, void *params, unsigned size)
{
	uint8_t *ptr = (uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP2));
	SetContainerCode(ptr, code);
	SetContainerType(ptr, type);
	SetContainerLength(ptr, size + MTP_CONTAINER_HEADER_SIZE);
	SetTransactionID(ptr, mtp.transaction_id);
	USBD_MemCopy(ptr + MTP_CONTAINER_PARAMETER_OFFSET, params, size);
	USBD_SET_PAYLOAD_LEN(EP2, size + MTP_CONTAINER_HEADER_SIZE);
}

void EP2_Handler(void)  
{
	printf("EP2\n");
}

void EP3_Handler(void)  
{
	uint16_t code,type;
	uint32_t len = USBD_GET_PAYLOAD_LEN(EP3);
	uint8_t *ptr = (uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP3));

	code = get_u16(ptr + MTP_CONTAINER_CODE_OFFSET);
	type = get_u16(ptr + MTP_CONTAINER_TYPE_OFFSET);

	printf("EP3 length = %d, .length = %d, .type = %x, .code = %x\n",
		len, get_u32(ptr + MTP_CONTAINER_LENGTH_OFFSET), type, code);
	if (code == MTP_OPERATION_OPEN_SESSION) {
		mtp.transaction_id = get_u32(ptr + MTP_CONTAINER_TRANSACTION_ID_OFFSET);
		mtp.session_id = get_u32(ptr + MTP_CONTAINER_PARAMETER_OFFSET);
		sendResponse(MTP_CONTAINER_TYPE_RESPONSE, MTP_RESPONSE_OK, NULL, 0);
	}
}

void EP4_Handler(void)
{
	printf("EP4\n");
}

void USBD_IRQHandler(void)
{
    uint32_t u32IntSts = USBD_GET_INT_FLAG();
    uint32_t u32State = USBD_GET_BUS_STATE();

//------------------------------------------------------------------
	if(u32IntSts & USBD_INTSTS_FLDET)
	{
		// Floating detect
		USBD_CLR_INT_FLAG(USBD_INTSTS_FLDET);

		if(USBD_IS_ATTACHED())
		{
			// Enable GPB15(VBUS) pull down state to solute suspend event issue.
			GPIO_EnablePullState(PB,BIT15,GPIO_PUSEL_PULL_DOWN); 	
			/* USB Plug In */
			USBD_ENABLE_USB();
		}
		else
		{
			// Disable GPB15 pull down state.
			GPIO_DisablePullState(PB,BIT15); 
			/* USB Un-plug */
			USBD_DISABLE_USB();
		}
	}

//------------------------------------------------------------------
	if ( u32IntSts & USBD_INTSTS_SOFIF_Msk )
	{
		// Clear event flag 
		USBD_CLR_INT_FLAG(USBD_INTSTS_SOFIF_Msk);
	}

//------------------------------------------------------------------
	if(u32IntSts & USBD_INTSTS_BUS)
	{
			/* Clear event flag */
			USBD_CLR_INT_FLAG(USBD_INTSTS_BUS);

			if(u32State & USBD_STATE_USBRST)
			{
					/* Bus reset */
					USBD_ENABLE_USB();
					USBD_SwReset();
			}
			if(u32State & USBD_STATE_SUSPEND)
			{
					/* Enable USB but disable PHY */
					USBD_DISABLE_PHY(); 
			}
			if(u32State & USBD_STATE_RESUME)
			{
					/* Enable USB and enable PHY */
					USBD_ENABLE_USB();
			}
	}

//------------------------------------------------------------------
	if(u32IntSts & USBD_INTSTS_USB)
	{
		// USB event
		if(u32IntSts & USBD_INTSTS_SETUP)
		{
			// Setup packet
			/* Clear event flag */
			USBD_CLR_INT_FLAG(USBD_INTSTS_SETUP);

			/* Clear the data IN/OUT ready flag of control end-points */
			USBD_STOP_TRANSACTION(EP0);
			USBD_STOP_TRANSACTION(EP1);

			USBD_ProcessSetupPacket();
		}
		// EP events
		if(u32IntSts & USBD_INTSTS_EP0)
		{
			/* Clear event flag */
			USBD_CLR_INT_FLAG(USBD_INTSTS_EP0);
			// Control IN
			USBD_CtrlIn();
		}
		if(u32IntSts & USBD_INTSTS_EP1)
		{
			/* Clear event flag */
			USBD_CLR_INT_FLAG(USBD_INTSTS_EP1);
			// Control OUT
			USBD_CtrlOut();
		}
		if(u32IntSts & USBD_INTSTS_EP2)
		{
			/* Clear event flag */
			USBD_CLR_INT_FLAG(USBD_INTSTS_EP2);
			// Bulk IN
			EP2_Handler();
		}
		if(u32IntSts & USBD_INTSTS_EP3)
		{
			/* Clear event flag */
			USBD_CLR_INT_FLAG(USBD_INTSTS_EP3);
			// Bulk OUT
			EP3_Handler();
		}
		if(u32IntSts & USBD_INTSTS_EP4)
		{
			/* Clear event flag */
			USBD_CLR_INT_FLAG(USBD_INTSTS_EP4);
			// Interrupt in
			EP4_Handler();
		}
	}
	
	/* clear unknown event */
	USBD_CLR_INT_FLAG(u32IntSts);
}
void HIDTrans_Initiate(void)
{
    /* Init setup packet buffer */
    /* Buffer range for setup packet -> [0 ~ 0x7] */
    USBD->STBUFSEG = SETUP_BUF_BASE;

    /*****************************************************/
    /* EP0 ==> control IN endpoint, address 0 */
    USBD_CONFIG_EP(EP0, USBD_CFG_CSTALL | USBD_CFG_EPMODE_IN | 0);
    /* Buffer range for EP0 */
    USBD_SET_EP_BUF_ADDR(EP0, EP0_BUF_BASE);

    /* EP1 ==> control OUT endpoint, address 0 */
    USBD_CONFIG_EP(EP1, USBD_CFG_CSTALL | USBD_CFG_EPMODE_OUT | 0);
    /* Buffer range for EP1 */
    USBD_SET_EP_BUF_ADDR(EP1, EP1_BUF_BASE);

    /*****************************************************/
    /* EP2 ==> Bulk IN endpoint, address 1 */
    USBD_CONFIG_EP(EP2, USBD_CFG_EPMODE_IN | 0x01);
    /* Buffer range for EP2 */
    //USBD_SET_PAYLOAD_LEN(EP2, EP2_MAX_PKT_SIZE);
    USBD_SET_EP_BUF_ADDR(EP2, EP2_BUF_BASE);

    /* EP3 ==> Bulk OUT endpoint, address 1 */
    USBD_CONFIG_EP(EP3, USBD_CFG_EPMODE_OUT | 0x01);
    /* Buffer range for EP3 */
    USBD_SET_EP_BUF_ADDR(EP3, EP3_BUF_BASE);
    /* trigger to receive OUT data */
    USBD_SET_PAYLOAD_LEN(EP3, EP3_MAX_PKT_SIZE);

    USBD_CONFIG_EP(EP4, USBD_CFG_EPMODE_IN | 0x02);
    /* Buffer range for EP4 */
    USBD_SET_EP_BUF_ADDR(EP4, EP4_BUF_BASE);
}
