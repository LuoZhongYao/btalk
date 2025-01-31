/******************************************************************************
 * @file     massstorage.h
 * @brief    M480 series USB mass storage header file
 *
 * @note
 * Copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
 
 #include "stdint.h" 
 
 
/* Define the vendor id and product id */
#define USBD_VID     0x0416
#define USBD_PID     0x1204

/* Define EP maximum packet size */
// #define EP0_MAX_PKT_SIZE    64
// #define EP1_MAX_PKT_SIZE    EP0_MAX_PKT_SIZE
// #define EP2_MAX_PKT_SIZE    256
// #define EP3_MAX_PKT_SIZE    PLAY_RATE*PLAY_CHANNELS*2/1000

//8+8+576+288+4+16
#define EP0_MAX_PKT_SIZE    8     
#define EP1_MAX_PKT_SIZE    EP0_MAX_PKT_SIZE
//#define EP2_MAX_PKT_SIZE    48  // (REC_RATE*REC_CHANNELS*3/1000)			
#define EP3_MAX_PKT_SIZE    (PLAY_RATE_48K*PLAY_CHANNELS*3/1000)		
#define EP4_MAX_PKT_SIZE    4  
#define EP5_MAX_PKT_SIZE    16  
//#define EP6_MAX_PKT_SIZE    96
//#define EP7_MAX_PKT_SIZE    144
//#define EP8_MAX_PKT_SIZE    192
#define EP2_MAX_PKT_SIZE    (REC_RATE*REC_CHANNELNUM_1CH*3/1000)			
#define EP6_MAX_PKT_SIZE    (REC_RATE*REC_CHANNELNUM_2CH*3/1000)			
#define EP7_MAX_PKT_SIZE    (REC_RATE*REC_CHANNELNUM_3CH*3/1000)			
#define EP8_MAX_PKT_SIZE    (REC_RATE*REC_CHANNELNUM_4CH*3/1000)			


#define SETUP_BUF_BASE      0
#define SETUP_BUF_LEN       8
#define EP0_BUF_BASE        (SETUP_BUF_BASE + SETUP_BUF_LEN)
#define EP0_BUF_LEN         (EP0_MAX_PKT_SIZE)
#define EP1_BUF_BASE        (SETUP_BUF_BASE + SETUP_BUF_LEN)
#define EP1_BUF_LEN         (EP1_MAX_PKT_SIZE)
#define EP2_BUF_BASE        (EP1_BUF_BASE + EP1_BUF_LEN)
#define EP2_BUF_LEN         (EP2_MAX_PKT_SIZE)
#define EP3_BUF_BASE        (EP2_BUF_BASE + EP2_BUF_LEN)
#define EP3_BUF_LEN         (EP3_MAX_PKT_SIZE)
#define EP4_BUF_BASE        (EP3_BUF_BASE + EP3_BUF_LEN)
#define EP4_BUF_LEN         (EP4_MAX_PKT_SIZE)
#define EP5_BUF_BASE        (EP4_BUF_BASE + EP4_BUF_LEN)
#define EP5_BUF_LEN         (EP5_MAX_PKT_SIZE)
#define EP6_BUF_BASE        (EP5_BUF_BASE + EP5_BUF_LEN)
#define EP6_BUF_LEN         (EP6_MAX_PKT_SIZE)
#define EP7_BUF_BASE        (EP6_BUF_BASE + EP6_BUF_LEN)
#define EP7_BUF_LEN         (EP7_MAX_PKT_SIZE)
#define EP8_BUF_BASE        (EP7_BUF_BASE + EP7_BUF_LEN)
#define EP8_BUF_LEN         (EP8_MAX_PKT_SIZE)


/* Define Descriptor information */
#define USBD_SELF_POWERED               0
#define USBD_REMOTE_WAKEUP              0
#define USBD_MAX_POWER                  100  /* The unit is in 2mA. ex: 50 * 2mA = 100mA */

#define LEN_CONFIG_AND_SUBORDINATE      (LEN_CONFIG+LEN_INTERFACE+LEN_ENDPOINT*2)


/*!<Define AUDIO Class Specific Request */
#define GET_CUR_VOL		0x81
#define GET_MIN_VOL		0x82
#define GET_MAX_VOL		0x83
#define GET_RES_VOL		0x84


/* Endpoint Control Selectors */
#define EP_CONTROL_UNDEFINED        0x0
#define SAMPLING_FREQ_CONTROL       0x1


/******************************************************************************/
/*                USBD Mass Storage Structure                                 */
/******************************************************************************/
/** @addtogroup M480_USBD_Mass_Exported_Struct M480 USBD Mass Exported Struct
  M480 USBD Mass Specific Struct
  @{
*/


static __INLINE uint32_t get_be32(uint8_t *buf)
{
    return ((uint32_t) buf[0] << 24) | ((uint32_t) buf[1] << 16) |
           ((uint32_t) buf[2] << 8) | ((uint32_t) buf[3]);
}



/*-------------------------------------------------------------*/
void DataFlashWrite(uint32_t addr, uint32_t size, uint32_t buffer);
void DataFlashRead(uint32_t addr, uint32_t size, uint32_t buffer);
void AUDIO_Init(void);
void AUDIO_ClassRequest(void);
void AUDIO_SetInterface(void);
void AUDIO_SetConfig(void);

/*-------------------------------------------------------------*/
void EP2_Handler(void);
void EP3_Handler(void);
void EP4_Handler(void);
void EP6_Handler(void);
void EP7_Handler(void);
void EP8_Handler(void);


/*** (C) COPYRIGHT 2016 Nuvoton Technology Corp. ***/
