#ifndef __UX_DEVICE_DESCRIPTORS_H__
  #define __UX_DEVICE_DESCRIPTORS_H__

  #ifdef __cplusplus
extern "C"
{
  #endif

#include "ux_api.h"
#include "ux_stm32_config.h"

#define USE_IAD        1
#define NOT_USE_IAD    0

#define USBD_MAX_NUM_CONFIGURATION                     1U
#define USBD_MAX_SUPPORTED_CLASS                       3U
#define USBD_MAX_CLASS_ENDPOINTS                       9U
#define USBD_MAX_CLASS_INTERFACES                      9U

#define USBD_CONFIG_MAXPOWER                           25U
#define USBD_COMPOSITE_USE_IAD                         0U
#define USBD_DEVICE_FRAMEWORK_BUILDER_ENABLED          1U

  typedef enum
  {
    CLASS_TYPE_NONE    = 0,
    CLASS_TYPE_HID     = 1,
    CLASS_TYPE_CDC_ACM = 2,
    CLASS_TYPE_MSC     = 3,
    CLASS_TYPE_CDC_ECM = 4,
    CLASS_TYPE_DFU     = 5,
    CLASS_TYPE_RNDIS   = 6,
  } USBD_CompositeClassTypeDef;

  /* USB Endpoint handle structure */
  typedef struct
  {
      uint32_t status;
      uint32_t total_length;
      uint32_t rem_length;
      uint32_t maxpacket;
      uint16_t is_used;
      uint16_t bInterval;
  } USBD_EndpointTypeDef;

  /* USB endpoint handle structure */
  typedef struct
  {
      uint8_t                     add;
      uint8_t                     type;
      uint16_t                    size;
      uint8_t                     is_used;
  } USBD_EPTypeDef;

  /* USB Composite handle structure */
  typedef struct
  {
      USBD_CompositeClassTypeDef   ClassType;
      uint32_t                     ClassId;
      uint32_t                     Active;
      uint32_t                     NumEps;
      USBD_EPTypeDef               Eps[USBD_MAX_CLASS_ENDPOINTS];
      uint32_t                     NumIf;
      uint8_t                      Ifs[USBD_MAX_CLASS_INTERFACES];
  } USBD_CompositeElementTypeDef;

  /* USB Device handle structure */
  typedef struct _USBD_DevClassHandleTypeDef
  {
      uint8_t                 Speed;
      uint32_t                classId;
      uint32_t                NumClasses;
      USBD_CompositeElementTypeDef tclasslist[USBD_MAX_SUPPORTED_CLASS];
      uint32_t                CurrDevDescSz;
      uint32_t                CurrConfDescSz;
  } USBD_DevClassHandleTypeDef;

  /* USB Device endpoint direction */
  typedef enum
  {
    OUT   = 0x00,
    IN    = 0x80,
  } USBD_EPDirectionTypeDef;

  /* USB Device descriptors structure */
  typedef struct
  {
      uint8_t           bLength;
      uint8_t           bDescriptorType;
      uint16_t          bcdUSB;
      uint8_t           bDeviceClass;
      uint8_t           bDeviceSubClass;
      uint8_t           bDeviceProtocol;
      uint8_t           bMaxPacketSize;
      uint16_t          idVendor;
      uint16_t          idProduct;
      uint16_t          bcdDevice;
      uint8_t           iManufacturer;
      uint8_t           iProduct;
      uint8_t           iSerialNumber;
      uint8_t           bNumConfigurations;
  } __PACKED USBD_DeviceDescTypedef;

  /* USB Iad descriptors structure */
  typedef struct
  {
      uint8_t           bLength;
      uint8_t           bDescriptorType;
      uint8_t           bFirstInterface;
      uint8_t           bInterfaceCount;
      uint8_t           bFunctionClass;
      uint8_t           bFunctionSubClass;
      uint8_t           bFunctionProtocol;
      uint8_t           iFunction;
  } __PACKED USBD_IadDescTypedef;

  /* USB interface descriptors structure */
  typedef struct
  {
      uint8_t           bLength;
      uint8_t           bDescriptorType;
      uint8_t           bInterfaceNumber;
      uint8_t           bAlternateSetting;
      uint8_t           bNumEndpoints;
      uint8_t           bInterfaceClass;
      uint8_t           bInterfaceSubClass;
      uint8_t           bInterfaceProtocol;
      uint8_t           iInterface;
  } __PACKED USBD_IfDescTypedef;

  /* USB endpoint descriptors structure */
  typedef struct
  {
      uint8_t           bLength;
      uint8_t           bDescriptorType;
      uint8_t           bEndpointAddress;
      uint8_t           bmAttributes;
      uint16_t          wMaxPacketSize;
      uint8_t           bInterval;
  } __PACKED USBD_EpDescTypedef;

  /* USB Config descriptors structure */
  typedef struct
  {
      uint8_t   bLength;
      uint8_t   bDescriptorType;
      uint16_t  wDescriptorLength;
      uint8_t   bNumInterfaces;
      uint8_t   bConfigurationValue;
      uint8_t   iConfiguration;
      uint8_t   bmAttributes;
      uint8_t   bMaxPower;
  } __PACKED USBD_ConfigDescTypedef;

  /* USB Qualifier descriptors structure */
  typedef struct
  {
      uint8_t           bLength;
      uint8_t           bDescriptorType;
      uint16_t          bcdDevice;
      uint8_t           Class;
      uint8_t           SubClass;
      uint8_t           Protocol;
      uint8_t           bMaxPacketSize;
      uint8_t           bNumConfigurations;
      uint8_t           bReserved;
  } __PACKED USBD_DevQualiDescTypedef;


  typedef struct
  {
      /* Header Functional Descriptor*/
      uint8_t           bLength;
      uint8_t           bDescriptorType;
      uint8_t           bDescriptorSubtype;
      uint16_t          bcdCDC;
  } __PACKED USBD_CDCHeaderFuncDescTypedef;

  typedef struct
  {
      /* Call Management Functional Descriptor*/
      uint8_t           bLength;
      uint8_t           bDescriptorType;
      uint8_t           bDescriptorSubtype;
      uint8_t           bmCapabilities;
      uint8_t           bDataInterface;
  } __PACKED USBD_CDCCallMgmFuncDescTypedef;

  typedef struct
  {
      /* ACM Functional Descriptor*/
      uint8_t           bLength;
      uint8_t           bDescriptorType;
      uint8_t           bDescriptorSubtype;
      uint8_t           bmCapabilities;
  } __PACKED USBD_CDCACMFuncDescTypedef;

  typedef struct
  {
      /* Union Functional Descriptor*/
      uint8_t           bLength;
      uint8_t           bDescriptorType;
      uint8_t           bDescriptorSubtype;
      uint8_t           bMasterInterface;
      uint8_t           bSlaveInterface;
  } __PACKED USBD_CDCUnionFuncDescTypedef;

  typedef struct
  {
      uint8_t bFunctionLength;
      uint8_t bDescriptorType;
      uint8_t bDescriptorSubType;
      uint8_t iMacAddress;
      uint8_t bEthernetStatistics3;
      uint8_t bEthernetStatistics2;
      uint8_t bEthernetStatistics1;
      uint8_t bEthernetStatistics0;
      uint16_t wMaxSegmentSize;
      uint16_t bNumberMCFiltes;
      uint8_t bNumberPowerFiltes;
  } __PACKED USBD_ECMFuncDescTypedef;

  uint8_t* USBD_Get_Device_Framework_Speed(uint8_t Speed, ULONG *Length);
  uint8_t* USBD_Get_String_Framework(ULONG *Length);
  uint8_t* USBD_Get_Language_Id_Framework(ULONG *Length);



#define USBD_RNDIS_INTERFACE_INDEX                     0
#define USBD_CDC_ACM_INTERFACE_INDEX                   2
#define USBD_MSC_INTERFACE_INDEX                       4

#define USBD_VID                                       0x045D //1155
#define USBD_PID                                       0x0009 //0x5750 // 22304
#define USBD_LANGID_STRING                             1033
#define USBD_MANUFACTURER_STRING                       "INDEMSYS"
#define USBD_PRODUCT_STRING                            "BACKPMAN3R10"
#define USBD_SERIAL_NUMBER                             "000000001"

#define USB_DESC_TYPE_INTERFACE                        0x04U
#define USB_DESC_TYPE_ENDPOINT                         0x05U
#define USB_DESC_TYPE_CONFIGURATION                    0x02U
#define USB_DESC_TYPE_IAD                              0x0BU

#define USBD_EP_TYPE_CTRL                              0x00U
#define USBD_EP_TYPE_ISOC                              0x01U
#define USBD_EP_TYPE_BULK                              0x02U
#define USBD_EP_TYPE_INTR                              0x03U

#define USBD_FULL_SPEED                                0x00U
#define USBD_HIGH_SPEED                                0x01U

#define USB_BCDUSB                                     0x0200U
#define LANGUAGE_ID_MAX_LENGTH                         2U

#define USBD_IDX_MFC_STR                               0x01U
#define USBD_IDX_PRODUCT_STR                           0x02U
#define USBD_IDX_SERIAL_STR                            0x03U

#define USBD_MAX_EP0_SIZE                              64U
#define USBD_DEVICE_QUALIFIER_DESC_SIZE                0x0AU

#define USBD_STRING_FRAMEWORK_MAX_LENGTH               256U

  /* Device Storage Class */
#define USBD_MSC_EPOUT_ADDR                            0x04U
#define USBD_MSC_EPIN_ADDR                             0x85U
#if (USBD_MSC_EPOUT_ADDR == (USBD_MSC_EPIN_ADDR & 0x0FU))
#error Address endpoint IN shall be different form endpoint OUT.
#endif
#define USBD_MSC_EPOUT_FS_MPS                          64U
#define USBD_MSC_EPOUT_HS_MPS                          512U
#define USBD_MSC_EPIN_FS_MPS                           64U
#define USBD_MSC_EPIN_HS_MPS                           512U

  /* This is the maximum supported configuration descriptor size
     User may redefine this value in order to optima */
#ifndef USBD_FRAMEWORK_MAX_DESC_SZ
#define USBD_FRAMEWORK_MAX_DESC_SZ                    200U
#endif /* USBD_FRAMEWORK_MAX_DESC_SZ */

#ifndef USBD_CONFIG_STR_DESC_IDX
#define USBD_CONFIG_STR_DESC_IDX                      0U
#endif /* USBD_CONFIG_STR_DESC_IDX */

#ifndef USBD_CONFIG_BMATTRIBUTES
#define USBD_CONFIG_BMATTRIBUTES                      0xC0U
#endif /* USBD_CONFIG_BMATTRIBUTES */

  /* Device CDC-ACM Class */
#define USBD_CDCACM_EPINCMD_ADDR                      0x87
#define USBD_CDCACM_EPINCMD_FS_MPS                    8U
#define USBD_CDCACM_EPINCMD_HS_MPS                    8U
#define USBD_CDCACM_EPIN_ADDR                         0x88
#define USBD_CDCACM_EPOUT_ADDR                        0x06
#if (USBD_CDCACM_EPIN_ADDR == (USBD_CDCACM_EPOUT_ADDR & 0x0FU))
#error Address endpoint IN shall be different form endpoint OUT.
#endif
#define USBD_CDCACM_EPIN_FS_MPS                       64U
#define USBD_CDCACM_EPIN_HS_MPS                       512U
#define USBD_CDCACM_EPOUT_FS_MPS                      64U
#define USBD_CDCACM_EPOUT_HS_MPS                      512U
#define USBD_CDCACM_EPINCMD_FS_BINTERVAL              5U
#define USBD_CDCACM_EPINCMD_HS_BINTERVAL              5U

  /* Device CDC-ECM Class */
//#define USBD_CDC_ECM_CLASS_ACTIVATED

  /* CDC_ECM parameters: you can fine tune these values depending on the needed baudrates and performance. */
#define USBD_DESC_ECM_BCD                             0x1000U
#define USBD_DESC_SUBTYPE_ACM                         0x0FU
#define CDC_ECM_ETH_STATS_BYTE3                       0U
#define CDC_ECM_ETH_STATS_BYTE2                       0U
#define CDC_ECM_ETH_STATS_BYTE1                       0U
#define CDC_ECM_ETH_STATS_BYTE0                       0U
  /* Ethernet Maximum Segment size, typically 1514 bytes */
#define CDC_ECM_ETH_MAX_SEGSZE                        1514U
  /* Number of Ethernet multicast filters */
#define CDC_ECM_ETH_NBR_MACFILTERS                    0U
  /* Number of wakeup power filters */
#define CDC_ECM_ETH_NBR_PWRFILTERS                    0U
  /* MAC String index */
#define CDC_ECM_MAC_STRING_INDEX                      0x04U// 10U
#define CDC_ECM_MAC_STR_DESC                (uint8_t *)"000202030000"

#define USBD_CDCECM_EPINCMD_ADDR                      0x82 //0x87U
#define USBD_CDCECM_EPINCMD_FS_MPS                    8U
#define USBD_CDCECM_EPINCMD_HS_MPS                    8U
#define USBD_CDCECM_EPIN_ADDR                         0x83 //0x88U
#define USBD_CDCECM_EPOUT_ADDR                        0x01 //0x06U
#if (USBD_CDCACM_EPIN_ADDR == (USBD_CDCACM_EPOUT_ADDR & 0x0FU))
#error Address endpoint IN shall be different form endpoint OUT.
#endif
#define USBD_CDCECM_EPIN_FS_MPS                       64U
#define USBD_CDCECM_EPIN_HS_MPS                       512U
#define USBD_CDCECM_EPOUT_FS_MPS                      64U
#define USBD_CDCECM_EPOUT_HS_MPS                      512U
#define USBD_CDCECM_EPINCMD_FS_BINTERVAL              5U
#define USBD_CDCECM_EPINCMD_HS_BINTERVAL              5U

#ifdef __cplusplus
}
#endif
#endif

