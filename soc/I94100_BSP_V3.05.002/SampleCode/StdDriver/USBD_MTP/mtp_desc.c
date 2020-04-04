#include <stdint.h>
#include <stddef.h>
#include <Platform.h>

/*!<USB Device Descriptor */
static const uint8_t MTP_device_descriptor[] =
{
	0x12,			/* bLength */
	0x01,			/* bDescriptorType */
	0x00, 0x02,     /* bcdUSB */
	0x00,           /* bDeviceClass */
	0x00,           /* bDeviceSubClass */
	0x00,           /* bDeviceProtocol */
	0x40,			/* bMaxPacketSize0 */
	/* idVendor */
	0x04, 0x2e,
	/* idProduct */
	0x26, 0xc0,
	0x04, 0x04,     /* bcdDevice */
	0x01,           /* iManufacture */
	0x02,           /* iProduct */
	0x03,           /* iSerialNumber */
	0x01            /* bNumConfigurations */
};

/*----------------------------------------------------------------------------*/
/*!<USB Configure Descriptor */
static const uint8_t MTP_configure_descriptor[] =
{
	0x09,			/* bLength */
	0x02,			/* bDescriptorType */
	39, 0x00,		/* wTotalLength */
	0x01,           /* bNumInterfaces */
	0x01,           /* bConfigurationValue */
	0x00,           /* iConfiguration */
	0x80,			/* bmAttributes */
	50,				/* MaxPower */

	/* I/F descr: MTP */
	0x09,			/* bLength */
	0x04,			/* bDescriptorType */
	0x00,           /* bInterfaceNumber */
	0x00,           /* bAlternateSetting */
	0x03,           /* bNumEndpoints */
	0xFF,           /* bInterfaceClass */
	0xFF,           /* bInterfaceSubClass */
	0x00,           /* bInterfaceProtocol */
	0x00,           /* iInterface */

	/* EP1 Descriptor: bulk in. */
	0x07,			/* bLength */
	0x05,			/* bDescriptorType */
	0x81,			/* bEndpointAddress */
	0x02,   		/* bmAttributes */
	0x00, 0x02,		/* wMaxPacketSize */
	0x00,		    /* bInterval */

	/* EP1 Descriptor: bulk out. */
	0x07,		/* bLength */
	0x05,		/* bDescriptorType */
	0x01,		/* bEndpointAddress */
	0x02,       /* bmAttributes */
	0x00, 0x02, /* wMaxPacketSize */
	0x00,		/* bInterval */

	/* EP2 Descriptor: interrupt in */
	0x07,		/* bLength */
	0x05,		/* bDescriptorType */
	0x82,		/* bEndpointAddress */
	0x03,       /* bmAttributes */
	0x40, 0x00, /* wMaxPacketSize */
	0x06,		/* bInterval */
};

static const uint8_t MTP_qualifier_descriptor[] =
{
	0x0A,		/* bLength */
	0x06,		/* bDescriptorType */
	0x00, 0x02,	/* bcdUSB	*/
	0x00,		/* bDeviceClass */
	0x00,		/* bDeviceSubClass */
	0x00,		/* bDeviceProtocol */
	0x00, 0x40,	/* bMaxPacketSize0 */
	0x01,		/* bNumConfigurations */
};

/*----------------------------------------------------------------------------*/
/*!<USB Language String Descriptor */
static const uint8_t MTP_string_lang[4] =
{
	0x04,       /* bLength */
	0x03,		/* bDescriptorType */
	0x09, 0x04
};

/*----------------------------------------------------------------------------*/
/*!<USB Vendor String Descriptor */
static const uint8_t MTP_string_vendor_desc[] =
{
	16,
	0x03,
	'D', 0, 'A', 0, 'S', 0, 'H', 0, 'I', 0, 'N', 0, 'E', 0
};

/*----------------------------------------------------------------------------*/
/*!<USB Product String Descriptor */
static const uint8_t MTP_string_product_desc[] =
{
	0x08,     /* bLength          */
	0x03,    /* bDescriptorType  */
	'M', 0, 'T', 0, 'P', 0,
};

/*----------------------------------------------------------------------------*/
static const uint8_t MTP_string_serial[26] =
{
	22,             // bLength
	0x03,    // bDescriptorType
	'I', 0, '9', 0, '4', 0, '1', 0, '0', 0, '0', 0, ' ', 0, 'H', 0, 'I', 0, 'D', 0, 
};

static const uint8_t *MTP_usb_string[4] =
{
	MTP_string_lang,
	MTP_string_vendor_desc,
	MTP_string_product_desc,
	MTP_string_serial
};

const S_USBD_INFO_T gsInfo =
{
	MTP_device_descriptor,
	MTP_configure_descriptor,
	MTP_usb_string,
	NULL,
	NULL,
	NULL,
	NULL,
	MTP_qualifier_descriptor,
};

