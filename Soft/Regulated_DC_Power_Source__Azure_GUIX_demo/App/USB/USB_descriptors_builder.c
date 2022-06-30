#include "App.h"

USBD_DevClassHandleTypeDef  USBD_Device_FS, USBD_Device_HS;

uint8_t UserClassInstance[USBD_MAX_CLASS_INTERFACES] =
{
#ifdef USBD_CDC_ECM_CLASS_ACTIVATED
  CLASS_TYPE_CDC_ECM,
#else
  CLASS_TYPE_RNDIS,
  CLASS_TYPE_CDC_ACM,
#endif
  //CLASS_TYPE_MSC,
};

/* The generic device descriptor buffer that will be filled by builder Size of the buffer is the maximum possible device FS descriptor size. */
#if defined ( __ICCARM__ ) /* IAR Compiler */
  #pragma data_alignment=4
#endif /* defined ( __ICCARM__ ) */
__ALIGN_BEGIN static uint8_t DevFrameWorkDesc_FS[USBD_FRAMEWORK_MAX_DESC_SZ] __ALIGN_END = {0};

/* The generic device descriptor buffer that will be filled by builder Size of the buffer is the maximum possible device HS descriptor size. */
#if defined ( __ICCARM__ ) /* IAR Compiler */
  #pragma data_alignment=4
#endif /* defined ( __ICCARM__ ) */
__ALIGN_BEGIN static uint8_t DevFrameWorkDesc_HS[USBD_FRAMEWORK_MAX_DESC_SZ] __ALIGN_END = {0};

static uint8_t *pDevFrameWorkDesc_FS = DevFrameWorkDesc_FS;

static uint8_t *pDevFrameWorkDesc_HS = DevFrameWorkDesc_HS;

/* String Device Framework :
 Byte 0 and 1 : Word containing the language ID : 0x0904 for US
 Byte 2       : Byte containing the index of the descriptor
 Byte 3       : Byte containing the length of the descriptor string
*/
#if defined ( __ICCARM__ ) /* IAR Compiler */
  #pragma data_alignment=4
#endif /* defined ( __ICCARM__ ) */
__ALIGN_BEGIN UCHAR USBD_string_framework[USBD_STRING_FRAMEWORK_MAX_LENGTH]
__ALIGN_END = {0};

/* Multiple languages are supported on the device, to add
   a language besides English, the Unicode language code must
   be appended to the language_id_framework array and the length
   adjusted accordingly. */

#if defined ( __ICCARM__ ) /* IAR Compiler */
  #pragma data_alignment=4
#endif /* defined ( __ICCARM__ ) */
UCHAR USBD_language_id_framework[LANGUAGE_ID_MAX_LENGTH] = {0};

static void     USBD_Desc_GetString(uint8_t *desc, uint8_t *Buffer, uint16_t *len);
static uint8_t  USBD_Desc_GetLen(uint8_t *buf);
static uint8_t* USBD_Device_Framework_Builder(USBD_DevClassHandleTypeDef *pdev, uint8_t *pDevFrameWorkDesc, uint8_t *UserClassInstance, uint8_t Speed);
static uint8_t  USBD_FrameWork_AddToConfDesc(USBD_DevClassHandleTypeDef *pdev, uint8_t Speed, uint8_t *pCmpstConfDesc);
static uint8_t  USBD_FrameWork_AddClass(USBD_DevClassHandleTypeDef *pdev, USBD_CompositeClassTypeDef class, uint8_t cfgidx, uint8_t Speed, uint8_t *pCmpstConfDesc);
static uint8_t  USBD_FrameWork_FindFreeIFNbr(USBD_DevClassHandleTypeDef *pdev);
static void     USBD_FrameWork_AddConfDesc(uint32_t Conf, uint32_t *pSze);
static void     USBD_FrameWork_AssignEp(USBD_DevClassHandleTypeDef *pdev, uint8_t Add, uint8_t Type, uint32_t Sze);

static void     USBD_FrameWork_MSCDesc(USBD_DevClassHandleTypeDef *pdev,  uint32_t descriptor_ptr, uint32_t *Sze, uint8_t use_iad);
static void     USBD_FrameWork_CDCDesc(USBD_DevClassHandleTypeDef *pdev,  uint32_t descriptor_ptr, uint32_t *Sze, uint8_t use_iad);
static void     USBD_FrameWork_RNDIS_Desc(USBD_DevClassHandleTypeDef *pdev, uint32_t descriptor_ptr, uint32_t *dscr_size, uint8_t use_iad);
static void     USBD_FrameWork_CDC_ECM_Desc(USBD_DevClassHandleTypeDef *pdev, uint32_t descriptor_ptr, uint32_t *dscr_size, uint8_t use_iad);
/*-----------------------------------------------------------------------------------------------------
  USBD_Get_Device_Framework_Speed Return the device speed descriptor

  \param Speed  HIGH or FULL SPEED flag
  \param Length length of HIGH or FULL SPEED array

  \return uint8_t* Pointer to descriptor buffer
-----------------------------------------------------------------------------------------------------*/
uint8_t* USBD_Get_Device_Framework_Speed(uint8_t Speed, ULONG *Length)
{
  uint8_t *pFrameWork = NULL;

  if (USBD_FULL_SPEED == Speed)
  {
    USBD_Device_Framework_Builder(&USBD_Device_FS, pDevFrameWorkDesc_FS, UserClassInstance, Speed);
    *Length = (ULONG)(USBD_Device_FS.CurrDevDescSz + USBD_Device_FS.CurrConfDescSz); // Get the length of USBD_device_framework_full_speed
    pFrameWork = pDevFrameWorkDesc_FS;
  }
  else
  {
    USBD_Device_Framework_Builder(&USBD_Device_HS, pDevFrameWorkDesc_HS, UserClassInstance, Speed);
    *Length = (ULONG)(USBD_Device_HS.CurrDevDescSz + USBD_Device_HS.CurrConfDescSz); // Get the length of USBD_device_framework_high_speed
    pFrameWork = pDevFrameWorkDesc_HS;
  }
  return pFrameWork;
}

/*-----------------------------------------------------------------------------------------------------
  USBD_Get_String_Framework Return the language_id_framework

  \param Length Length of String_Framework

  \return uint8_t* Pointer to language_id_framework buffer
-----------------------------------------------------------------------------------------------------*/
uint8_t* USBD_Get_String_Framework(ULONG *Length)
{
  uint16_t len = 0U;
  uint8_t count = 0U;

  /* Set the Manufacturer language Id and index in USBD_string_framework */
  USBD_string_framework[count++] = USBD_LANGID_STRING & 0xFF;
  USBD_string_framework[count++] = USBD_LANGID_STRING >> 8;
  USBD_string_framework[count++] = USBD_IDX_MFC_STR;

  /* Set the Manufacturer string in string_framework */
  USBD_Desc_GetString((uint8_t *)USBD_MANUFACTURER_STRING, USBD_string_framework + count,&len);

  /* Set the Product language Id and index in USBD_string_framework */
  count += len + 1;
  USBD_string_framework[count++] = USBD_LANGID_STRING & 0xFF;
  USBD_string_framework[count++] = USBD_LANGID_STRING >> 8;
  USBD_string_framework[count++] = USBD_IDX_PRODUCT_STR;

  /* Set the Product string in USBD_string_framework */
  USBD_Desc_GetString((uint8_t *)USBD_PRODUCT_STRING, USBD_string_framework + count,&len);

  /* Set Serial language Id and index in string_framework */
  count += len + 1;
  USBD_string_framework[count++] = USBD_LANGID_STRING & 0xFF;
  USBD_string_framework[count++] = USBD_LANGID_STRING >> 8;
  USBD_string_framework[count++] = USBD_IDX_SERIAL_STR;

  /* Set the Serial number in USBD_string_framework */
  USBD_Desc_GetString((uint8_t *)USBD_SERIAL_NUMBER, USBD_string_framework + count,&len);

#ifdef USBD_CDC_ECM_CLASS_ACTIVATED

  /* Set MAC_STRING_INDEX and MAC_STRING in string_framework */
  count += len + 1;
  USBD_string_framework[count++] = USBD_LANGID_STRING & 0xFF;
  USBD_string_framework[count++] = USBD_LANGID_STRING >> 8;
  USBD_string_framework[count++] = CDC_ECM_MAC_STRING_INDEX;

  /* Set the Mac address in USBD_string_framework */
  USBD_Desc_GetString((uint8_t *)CDC_ECM_MAC_STR_DESC, USBD_string_framework + count, &len);

#endif /* USBD_CDC_ECM_CLASS_ACTIVATED */


  /* Get the length of USBD_string_framework */
  *Length = strlen((const char *)USBD_string_framework);

  return USBD_string_framework;
}

/*-----------------------------------------------------------------------------------------------------
  USBD_Get_String_Framework Return the language_id_framework

  \param Length Length of Language_Id_Framework

  \return uint8_t* Pointer to language_id_framework buffer
-----------------------------------------------------------------------------------------------------*/
uint8_t* USBD_Get_Language_Id_Framework(ULONG *Length)
{
  uint8_t count = 0U;

  /* Set the language Id in USBD_language_id_framework */
  USBD_language_id_framework[count++] = USBD_LANGID_STRING & 0xFF;
  USBD_language_id_framework[count++] = USBD_LANGID_STRING >> 8;

  /* Get the length of USBD_language_id_framework */
  *Length = strlen((const char *)USBD_language_id_framework);

  return USBD_language_id_framework;
}

/*-----------------------------------------------------------------------------------------------------
  USBD_Desc_GetString Convert ASCII string into Unicode one

  \param desc descriptor buffer
  \param unicode Formatted string buffer (Unicode)
  \param len descriptor length
-----------------------------------------------------------------------------------------------------*/
static void USBD_Desc_GetString(uint8_t *desc, uint8_t *unicode, uint16_t *len)
{
  uint8_t idx = 0U;
  uint8_t *pdesc;

  if (desc == NULL)
  {
    return;
  }

  pdesc = desc;
  *len = (uint16_t)USBD_Desc_GetLen(pdesc);

  unicode[idx++] =*(uint8_t *)len;

  while (*pdesc != (uint8_t)'\0')
  {
    unicode[idx++] =*pdesc;
    pdesc++;
  }
}

/*-----------------------------------------------------------------------------------------------------
  USBD_Desc_GetLen return the string length

  \param buf pointer to the ASCII string buffer

  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
static uint8_t USBD_Desc_GetLen(uint8_t *buf)
{
  uint8_t  len = 0U;
  uint8_t *pbuff = buf;

  while (*pbuff != (uint8_t)'\0')
  {
    len++;
    pbuff++;
  }

  return len;
}

/*-----------------------------------------------------------------------------------------------------
  USBD_Device_Framework_Builder Device Framework builder

  \param pdev device instance
  \param pDevFrameWorkDesc Pointer to the device framework descriptor
  \param UserClassInstance type of the class to be added
  \param Speed Speed parameter HS or FS

  \return uint8_t*
-----------------------------------------------------------------------------------------------------*/
static uint8_t* USBD_Device_Framework_Builder(USBD_DevClassHandleTypeDef *pdev, uint8_t *pDevFrameWorkDesc, uint8_t *UserClassInstance, uint8_t Speed)
{
  static USBD_DeviceDescTypedef   *pDevDesc;
  static USBD_DevQualiDescTypedef *pDevQualDesc;
  uint8_t Idx_Instance = 0U;
  uint8_t NumberClass  = 1U;

  pdev->CurrConfDescSz = 0U;
  pdev->CurrDevDescSz  = 0U;


  pDevDesc = (USBD_DeviceDescTypedef *)pDevFrameWorkDesc;  /* Set the pointer to the device descriptor area*/

  /* Start building the generic device descriptor common part */
  pDevDesc->bLength            = (uint8_t)sizeof(USBD_DeviceDescTypedef);
  pDevDesc->bDescriptorType    = UX_DEVICE_DESCRIPTOR_ITEM;
  pDevDesc->bcdUSB             = USB_BCDUSB;
  pDevDesc->bDeviceClass       = 0x00;
  pDevDesc->bDeviceSubClass    = 0x00;
  pDevDesc->bDeviceProtocol    = 0x00;
  pDevDesc->bMaxPacketSize     = USBD_MAX_EP0_SIZE;
  pDevDesc->idVendor           = USBD_VID;
  pDevDesc->idProduct          = USBD_PID;
  pDevDesc->bcdDevice          = 0x0001;
  pDevDesc->iManufacturer      = USBD_IDX_MFC_STR;
  pDevDesc->iProduct           = USBD_IDX_PRODUCT_STR;
  pDevDesc->iSerialNumber      = USBD_IDX_SERIAL_STR;
  pDevDesc->bNumConfigurations = USBD_MAX_NUM_CONFIGURATION;
  pdev->CurrDevDescSz         += (uint32_t)sizeof(USBD_DeviceDescTypedef);

  /* Check if USBx is in high speed mode to add qualifier descriptor */
  if (Speed == USBD_HIGH_SPEED)
  {
    pDevQualDesc                     = (USBD_DevQualiDescTypedef *)(pDevFrameWorkDesc + pdev->CurrDevDescSz);
    pDevQualDesc->bLength            = (uint8_t)sizeof(USBD_DevQualiDescTypedef);
    pDevQualDesc->bDescriptorType    = UX_DEVICE_QUALIFIER_DESCRIPTOR_ITEM;
    pDevQualDesc->bcdDevice          = 0x0200;
    pDevQualDesc->Class              = 0x00;
    pDevQualDesc->SubClass           = 0x00;
    pDevQualDesc->Protocol           = 0x00;
    pDevQualDesc->bMaxPacketSize     = 0x40;
    pDevQualDesc->bNumConfigurations = 0x01;
    pDevQualDesc->bReserved          = 0x00;
    pdev->CurrDevDescSz += (uint32_t)sizeof(USBD_DevQualiDescTypedef);
  }

  while (Idx_Instance < USBD_MAX_SUPPORTED_CLASS)  /* Build the device framework */
  {
    if ((pdev->classId < USBD_MAX_SUPPORTED_CLASS) && (pdev->NumClasses < USBD_MAX_SUPPORTED_CLASS))
    {

      (void)USBD_FrameWork_AddClass(pdev, (USBD_CompositeClassTypeDef)UserClassInstance[Idx_Instance], 0, Speed,(pDevFrameWorkDesc + pdev->CurrDevDescSz));  /* Call the composite class builder */
      pdev->classId++;      /* Increment the ClassId for the next occurrence */
      pdev->NumClasses++;
    }
    Idx_Instance++;
    if (UserClassInstance[Idx_Instance] != CLASS_TYPE_NONE) /* Count the number of Classes different of CLASS_TYPE_NONE */
    {
      NumberClass++;
    }
  }
  if (NumberClass > 1)  /* Check if there is a composite class and update device class */
  {
    pDevDesc->bDeviceClass    = 0xEF;
    pDevDesc->bDeviceSubClass = 0x02;
    pDevDesc->bDeviceProtocol = 0x01;
  }
  else
  {
    if (UserClassInstance[0] == CLASS_TYPE_CDC_ACM)  /* Check if the CDC ACM class is set and update device class */
    {
      pDevDesc->bDeviceClass    = 0x02;
      pDevDesc->bDeviceSubClass = 0x02;
      pDevDesc->bDeviceProtocol = 0x00;
    }
  }
  return pDevFrameWorkDesc;
}

/*-----------------------------------------------------------------------------------------------------
  USBD_FrameWork_AddClass Register a class in the class builder

  \param pdev device instance
  \param cfgidx pointer to the class structure to be added
  \param Speed type of the class to be added (from USBD_CompositeClassTypeDef)
  \param pCmpstConfDesc configuration index

  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
uint8_t  USBD_FrameWork_AddClass(USBD_DevClassHandleTypeDef *pdev, USBD_CompositeClassTypeDef class_type, uint8_t cfgidx, uint8_t Speed, uint8_t *pCmpstConfDesc)
{
  if ((pdev->classId < USBD_MAX_SUPPORTED_CLASS) && (pdev->tclasslist[pdev->classId].Active == 0U))
  {
    /* Store the class parameters in the global tab */
    pdev->tclasslist[pdev->classId].ClassId   = pdev->classId;
    pdev->tclasslist[pdev->classId].Active    = 1U;
    pdev->tclasslist[pdev->classId].ClassType = class_type;

    if (USBD_FrameWork_AddToConfDesc(pdev, Speed, pCmpstConfDesc) != UX_SUCCESS) /* Call configuration descriptor builder and endpoint configuration builder */
    {
      return UX_ERROR;
    }
  }
  UNUSED(cfgidx);
  return UX_SUCCESS;
}

/*-----------------------------------------------------------------------------------------------------
  USBD_FrameWork_AddToConfDesc  Add a new class to the configuration descriptor

  \param pdev
  \param Speed
  \param pCmpstConfDesc

  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
uint8_t  USBD_FrameWork_AddToConfDesc(USBD_DevClassHandleTypeDef *pdev, uint8_t Speed, uint8_t *pCmpstConfDesc)
{
  uint8_t interface = 0U;

  pdev->Speed = Speed;  /* The USB drivers do not set the speed value, so set it here before starting */
  if (pdev->classId == 0U)  /* start building the config descriptor common part */
  {
    USBD_FrameWork_AddConfDesc((uint32_t)pCmpstConfDesc,&pdev->CurrConfDescSz); /* Add configuration and IAD descriptors */
  }

  switch (pdev->tclasslist[pdev->classId].ClassType)
  {
  case CLASS_TYPE_CDC_ACM:
    interface = USBD_FrameWork_FindFreeIFNbr(pdev);  /* Find the first available interface slot and Assign number of interfaces */
    pdev->tclasslist[pdev->classId].NumIf  = 2U;
    pdev->tclasslist[pdev->classId].Ifs[0] = interface;
    pdev->tclasslist[pdev->classId].Ifs[1] = (uint8_t)(interface + 1U);
    pdev->tclasslist[pdev->classId].NumEps = 3U;    /* Assign endpoint numbers */

    if (Speed == USBD_HIGH_SPEED)  /* Check the current speed to assign endpoints */
    {
      USBD_FrameWork_AssignEp(pdev, USBD_CDCACM_EPOUT_ADDR,   USBD_EP_TYPE_BULK, USBD_CDCACM_EPOUT_HS_MPS);   /* Assign OUT Endpoint */
      USBD_FrameWork_AssignEp(pdev, USBD_CDCACM_EPIN_ADDR,    USBD_EP_TYPE_BULK, USBD_CDCACM_EPIN_HS_MPS);    /* Assign IN Endpoint */
      USBD_FrameWork_AssignEp(pdev, USBD_CDCACM_EPINCMD_ADDR, USBD_EP_TYPE_INTR, USBD_CDCACM_EPINCMD_HS_MPS); /* Assign CMD Endpoint */
    }
    else
    {
      USBD_FrameWork_AssignEp(pdev, USBD_CDCACM_EPOUT_ADDR,   USBD_EP_TYPE_BULK, USBD_CDCACM_EPOUT_FS_MPS);   /* Assign OUT Endpoint */
      USBD_FrameWork_AssignEp(pdev, USBD_CDCACM_EPIN_ADDR,    USBD_EP_TYPE_BULK, USBD_CDCACM_EPIN_FS_MPS);    /* Assign IN Endpoint */
      USBD_FrameWork_AssignEp(pdev, USBD_CDCACM_EPINCMD_ADDR, USBD_EP_TYPE_INTR, USBD_CDCACM_EPINCMD_FS_MPS); /* Assign CMD Endpoint */
    }
    USBD_FrameWork_CDCDesc(pdev, (uint32_t)pCmpstConfDesc,&pdev->CurrConfDescSz, USE_IAD);  /* Configure and Append the Descriptor */
    break;

  case CLASS_TYPE_MSC:

    interface = USBD_FrameWork_FindFreeIFNbr(pdev);  /* Find the first available interface slot and Assign number of interfaces */
    pdev->tclasslist[pdev->classId].NumIf  = 1;
    pdev->tclasslist[pdev->classId].Ifs[0] = interface;
    pdev->tclasslist[pdev->classId].NumEps = 2; /* EP1_IN, EP1_OUT  Assign endpoint numbers */

    if (pdev->Speed == USBD_HIGH_SPEED) /* Check the current speed to assign endpoints */
    {
      USBD_FrameWork_AssignEp(pdev, USBD_MSC_EPIN_ADDR, USBD_EP_TYPE_BULK, USBD_MSC_EPIN_HS_MPS);   /* Assign IN Endpoint */
      USBD_FrameWork_AssignEp(pdev, USBD_MSC_EPOUT_ADDR, USBD_EP_TYPE_BULK, USBD_MSC_EPOUT_HS_MPS); /* Assign OUT Endpoint */
    }
    else
    {
      USBD_FrameWork_AssignEp(pdev, USBD_MSC_EPIN_ADDR, USBD_EP_TYPE_BULK, USBD_MSC_EPIN_FS_MPS); /* Assign IN Endpoint */
      USBD_FrameWork_AssignEp(pdev, USBD_MSC_EPOUT_ADDR, USBD_EP_TYPE_BULK, USBD_MSC_EPOUT_FS_MPS); /* Assign OUT Endpoint */
    }
    USBD_FrameWork_MSCDesc(pdev, (uint32_t)pCmpstConfDesc,&pdev->CurrConfDescSz, USE_IAD);  /* Configure and Append the Descriptor */
    break;

  case CLASS_TYPE_RNDIS:

    interface = USBD_FrameWork_FindFreeIFNbr(pdev);  /* Find the first available interface slot and Assign number of interfaces */
    pdev->tclasslist[pdev->classId].NumIf  = 2U;
    pdev->tclasslist[pdev->classId].Ifs[0] = interface;
    pdev->tclasslist[pdev->classId].Ifs[1] = (uint8_t)(interface + 1U);
    pdev->tclasslist[pdev->classId].NumEps = 3U;    /* Assign endpoint numbers */

    if (Speed == USBD_HIGH_SPEED)  /* Check the current speed to assign endpoints */
    {
      USBD_FrameWork_AssignEp(pdev, USBD_CDCECM_EPOUT_ADDR,   USBD_EP_TYPE_BULK, USBD_CDCECM_EPOUT_HS_MPS);   /* Assign OUT Endpoint */
      USBD_FrameWork_AssignEp(pdev, USBD_CDCECM_EPIN_ADDR,    USBD_EP_TYPE_BULK, USBD_CDCECM_EPIN_HS_MPS);    /* Assign IN Endpoint */
      USBD_FrameWork_AssignEp(pdev, USBD_CDCECM_EPINCMD_ADDR, USBD_EP_TYPE_INTR, USBD_CDCECM_EPINCMD_HS_MPS); /* Assign CMD Endpoint */
    }
    else
    {
      USBD_FrameWork_AssignEp(pdev, USBD_CDCECM_EPOUT_ADDR,   USBD_EP_TYPE_BULK, USBD_CDCECM_EPOUT_FS_MPS);   /* Assign OUT Endpoint */
      USBD_FrameWork_AssignEp(pdev, USBD_CDCECM_EPIN_ADDR,    USBD_EP_TYPE_BULK, USBD_CDCECM_EPIN_FS_MPS);    /* Assign IN Endpoint */
      USBD_FrameWork_AssignEp(pdev, USBD_CDCECM_EPINCMD_ADDR, USBD_EP_TYPE_INTR, USBD_CDCECM_EPINCMD_FS_MPS); /* Assign CMD Endpoint */
    }
    USBD_FrameWork_RNDIS_Desc(pdev, (uint32_t)pCmpstConfDesc,&pdev->CurrConfDescSz, USE_IAD);  /* Configure and Append the Descriptor */
    break;
  case CLASS_TYPE_CDC_ECM:

    interface = USBD_FrameWork_FindFreeIFNbr(pdev);  /* Find the first available interface slot and Assign number of interfaces */
    pdev->tclasslist[pdev->classId].NumIf  = 2U;
    pdev->tclasslist[pdev->classId].Ifs[0] = interface;
    pdev->tclasslist[pdev->classId].Ifs[1] = (uint8_t)(interface + 1U);
    pdev->tclasslist[pdev->classId].NumEps = 3U;    /* Assign endpoint numbers */

    if (Speed == USBD_HIGH_SPEED)  /* Check the current speed to assign endpoints */
    {
      USBD_FrameWork_AssignEp(pdev, USBD_CDCECM_EPOUT_ADDR,   USBD_EP_TYPE_BULK, USBD_CDCECM_EPOUT_HS_MPS);   /* Assign OUT Endpoint */
      USBD_FrameWork_AssignEp(pdev, USBD_CDCECM_EPIN_ADDR,    USBD_EP_TYPE_BULK, USBD_CDCECM_EPIN_HS_MPS);    /* Assign IN Endpoint */
      USBD_FrameWork_AssignEp(pdev, USBD_CDCECM_EPINCMD_ADDR, USBD_EP_TYPE_INTR, USBD_CDCECM_EPINCMD_HS_MPS); /* Assign CMD Endpoint */
    }
    else
    {
      USBD_FrameWork_AssignEp(pdev, USBD_CDCECM_EPOUT_ADDR,   USBD_EP_TYPE_BULK, USBD_CDCECM_EPOUT_FS_MPS);   /* Assign OUT Endpoint */
      USBD_FrameWork_AssignEp(pdev, USBD_CDCECM_EPIN_ADDR,    USBD_EP_TYPE_BULK, USBD_CDCECM_EPIN_FS_MPS);    /* Assign IN Endpoint */
      USBD_FrameWork_AssignEp(pdev, USBD_CDCECM_EPINCMD_ADDR, USBD_EP_TYPE_INTR, USBD_CDCECM_EPINCMD_FS_MPS); /* Assign CMD Endpoint */
    }
    USBD_FrameWork_CDC_ECM_Desc(pdev, (uint32_t)pCmpstConfDesc,&pdev->CurrConfDescSz, NOT_USE_IAD);  /* Configure and Append the Descriptor */
    break;

  default:
    break;
  }

  return UX_SUCCESS;
}

/*-----------------------------------------------------------------------------------------------------
  USBD_FrameWork_FindFreeIFNbr Find the first interface available slot

  \param pdev

  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
static uint8_t USBD_FrameWork_FindFreeIFNbr(USBD_DevClassHandleTypeDef *pdev)
{
  uint32_t idx = 0U;

  /* Unroll all already activated classes */
  for (uint32_t i = 0U; i < pdev->NumClasses; i++)
  {
    /* Unroll each class interfaces */
    for (uint32_t j = 0U; j < pdev->tclasslist[i].NumIf; j++)
    {
      /* Increment the interface counter index */
      idx++;
    }
  }

  /* Return the first available interface slot */
  return (uint8_t)idx;
}

/*-----------------------------------------------------------------------------------------------------
  USBD_FrameWork_AddToConfDesc Add a new class to the configuration descriptor

  \param Conf
  \param pSze
-----------------------------------------------------------------------------------------------------*/
static void  USBD_FrameWork_AddConfDesc(uint32_t Conf, uint32_t *pSze)
{
  /* Intermediate variable to comply with MISRA-C Rule 11.3 */
  USBD_ConfigDescTypedef *ptr = (USBD_ConfigDescTypedef *)Conf;

  ptr->bLength                = (uint8_t)sizeof(USBD_ConfigDescTypedef);
  ptr->bDescriptorType        = USB_DESC_TYPE_CONFIGURATION;
  ptr->wDescriptorLength      = 0U;
  ptr->bNumInterfaces         = 0U;
  ptr->bConfigurationValue    = 1U;
  ptr->iConfiguration         = USBD_CONFIG_STR_DESC_IDX;
  ptr->bmAttributes           = USBD_CONFIG_BMATTRIBUTES;
  ptr->bMaxPower              = USBD_CONFIG_MAXPOWER;
  *pSze                       += sizeof(USBD_ConfigDescTypedef);
}

/*-----------------------------------------------------------------------------------------------------
  USBD_FrameWork_AssignEp Assign and endpoint

  \param pdev device instance
  \param Add Endpoint address
  \param Type Endpoint type
  \param Sze Endpoint max packet size
-----------------------------------------------------------------------------------------------------*/
static void  USBD_FrameWork_AssignEp(USBD_DevClassHandleTypeDef *pdev, uint8_t Add, uint8_t Type, uint32_t Sze)
{
  uint32_t idx = 0U;

  // Find the first available endpoint slot
  while (((idx < (pdev->tclasslist[pdev->classId]).NumEps) && ((pdev->tclasslist[pdev->classId].Eps[idx].is_used) != 0U)))
  {
    idx++;  /* Increment the index */
  }

  // Configure the endpoint
  pdev->tclasslist[pdev->classId].Eps[idx].add = Add;
  pdev->tclasslist[pdev->classId].Eps[idx].type = Type;
  pdev->tclasslist[pdev->classId].Eps[idx].size = (uint16_t) Sze;
  pdev->tclasslist[pdev->classId].Eps[idx].is_used = 1U;
}




/*-----------------------------------------------------------------------------------------------------
  Append Endpoint descriptor to Configuration descriptor

  \param pdev
  \param Sze
  \param ep_num
  \param eptype
  \param HSinterval
  \param FSinterval
-----------------------------------------------------------------------------------------------------*/
static void USBD_framework_set_ep(uint32_t pConf, USBD_DevClassHandleTypeDef *pdev, uint32_t *Sze, uint8_t ep_num, uint8_t eptype, uint16_t HSinterval, uint16_t FSinterval)
{
  USBD_EpDescTypedef       *pEpDesc;
  uint8_t                   epadd  = pdev->tclasslist[pdev->classId].Eps[ep_num].add;
  uint16_t                  epsize = pdev->tclasslist[pdev->classId].Eps[ep_num].size;

  pEpDesc =((USBD_EpDescTypedef *)((uint32_t)pConf +*Sze));
  pEpDesc->bLength            = (uint8_t)sizeof(USBD_EpDescTypedef);
  pEpDesc->bDescriptorType    = USB_DESC_TYPE_ENDPOINT;
  pEpDesc->bEndpointAddress   = epadd;
  pEpDesc->bmAttributes       = eptype;
  pEpDesc->wMaxPacketSize     = epsize;
  if (pdev->Speed == USBD_HIGH_SPEED)
  {
    pEpDesc->bInterval        = HSinterval;
  }
  else
  {
    pEpDesc->bInterval        = FSinterval;
  }
  *Sze += (uint32_t)sizeof(USBD_EpDescTypedef);
}

/*-----------------------------------------------------------------------------------------------------
  Interface Descriptor

  \param ifnum
  \param alt
  \param eps
  \param subclass
  \param protocol
  \param istring
-----------------------------------------------------------------------------------------------------*/
static void USBD_framework_set_if(uint32_t pConf, USBD_DevClassHandleTypeDef *pdev, uint32_t *Sze, uint8_t if_number,  uint8_t alt, uint8_t NumEndpoints, uint8_t if_class, uint8_t if_subclass, uint8_t if_protocol, uint8_t istring)
{
  USBD_IfDescTypedef       *pIfDesc;

  pIfDesc =((USBD_IfDescTypedef *)((uint32_t)pConf +*Sze));
  pIfDesc->bLength            = (uint8_t)sizeof(USBD_IfDescTypedef);
  pIfDesc->bDescriptorType    = USB_DESC_TYPE_INTERFACE;
  pIfDesc->bInterfaceNumber   = if_number;
  pIfDesc->bAlternateSetting  = alt;
  pIfDesc->bNumEndpoints      = NumEndpoints;
  pIfDesc->bInterfaceClass    = if_class;
  pIfDesc->bInterfaceSubClass = if_subclass;
  pIfDesc->bInterfaceProtocol = if_protocol;
  pIfDesc->iInterface         = istring;
  *Sze += (uint32_t)sizeof(USBD_IfDescTypedef);
}

/*-----------------------------------------------------------------------------------------------------
  USBD_FrameWork_MSCDesc Configure and Append the MSC Descriptor

  \param pdev  device instance
  \param pConf  Configuration descriptor pointer
  \param Sze pointer to the current configuration descriptor size
-----------------------------------------------------------------------------------------------------*/
static void  USBD_FrameWork_MSCDesc(USBD_DevClassHandleTypeDef *pdev, uint32_t descriptor_ptr, uint32_t *dscr_size, uint8_t use_iad)
{
  static USBD_IadDescTypedef              *pIadDesc;

  if (use_iad)
  {
    pIadDesc                          =((USBD_IadDescTypedef *)(descriptor_ptr +*dscr_size));
    pIadDesc->bLength                 = (uint8_t)sizeof(USBD_IadDescTypedef);
    pIadDesc->bDescriptorType         = USB_DESC_TYPE_IAD;
    pIadDesc->bFirstInterface         = pdev->tclasslist[pdev->classId].Ifs[0];
    pIadDesc->bInterfaceCount         = 1;    // 1 interface
    pIadDesc->bFunctionClass          = 0x08;
    pIadDesc->bFunctionSubClass       = 0x06;
    pIadDesc->bFunctionProtocol       = 0x50;
    pIadDesc->iFunction               = 0; /* String Index */
    *dscr_size                              += (uint32_t)sizeof(USBD_IadDescTypedef);
  }

  USBD_framework_set_if(descriptor_ptr, pdev, dscr_size, pdev->tclasslist[pdev->classId].Ifs[0], 0, 2, 0x08, 0x06, 0x50, 0);     // Append MSC Interface descriptor
  USBD_framework_set_ep(descriptor_ptr, pdev, dscr_size, 0, USBD_EP_TYPE_BULK, 0, 0); // Append Endpoint descriptor to Configuration descriptor
  USBD_framework_set_ep(descriptor_ptr, pdev, dscr_size, 1, USBD_EP_TYPE_BULK, 0, 0); // Append Endpoint descriptor to Configuration descriptor


  ((USBD_ConfigDescTypedef *)descriptor_ptr)->bNumInterfaces += 1U;      /* Update Config Descriptor and IAD descriptor */
  ((USBD_ConfigDescTypedef *)descriptor_ptr)->wDescriptorLength  =*dscr_size;
}


/*-----------------------------------------------------------------------------------------------------


  \param pdev
  \param pConf
  \param Sze
-----------------------------------------------------------------------------------------------------*/
static void USBD_FrameWork_CDCDesc(USBD_DevClassHandleTypeDef *pdev, uint32_t descriptor_ptr, uint32_t *dscr_size, uint8_t use_iad)
{

  static USBD_CDCHeaderFuncDescTypedef    *pHeadDesc;
  static USBD_CDCCallMgmFuncDescTypedef   *pCallMgmDesc;
  static USBD_CDCACMFuncDescTypedef       *pACMDesc;
  static USBD_CDCUnionFuncDescTypedef     *pUnionDesc;
  static USBD_IadDescTypedef              *pIadDesc;

  if (use_iad)
  {
    pIadDesc                       =((USBD_IadDescTypedef *)(descriptor_ptr +*dscr_size));
    pIadDesc->bLength              = (uint8_t)sizeof(USBD_IadDescTypedef);
    pIadDesc->bDescriptorType      = USB_DESC_TYPE_IAD;
    pIadDesc->bFirstInterface      = pdev->tclasslist[pdev->classId].Ifs[0];
    pIadDesc->bInterfaceCount      = 2;    // 2 interfaces
    pIadDesc->bFunctionClass       = 0x02;
    pIadDesc->bFunctionSubClass    = 0x02;
    pIadDesc->bFunctionProtocol    = 0x01;
    pIadDesc->iFunction            = 0; /* String Index */
    *dscr_size                     += (uint32_t)sizeof(USBD_IadDescTypedef);
  }

  USBD_framework_set_if(descriptor_ptr, pdev, dscr_size, pdev->tclasslist[pdev->classId].Ifs[0], 0, 1, 0x02, 0x02, 0x01, 0);     // Control Interface Descriptor

  pHeadDesc =((USBD_CDCHeaderFuncDescTypedef *)((uint32_t)descriptor_ptr +*dscr_size));  // Control interface headers
  /* Header Functional Descriptor*/
  pHeadDesc->bLength               = 0x05;
  pHeadDesc->bDescriptorType       = 0x24;
  pHeadDesc->bDescriptorSubtype    = 0x00;
  pHeadDesc->bcdCDC                = 0x0110;
  *dscr_size += (uint32_t)sizeof(USBD_CDCHeaderFuncDescTypedef);

  /* Call Management Functional Descriptor*/
  pCallMgmDesc =((USBD_CDCCallMgmFuncDescTypedef *)((uint32_t)descriptor_ptr +*dscr_size));
  pCallMgmDesc->bLength            = 0x05;
  pCallMgmDesc->bDescriptorType    = 0x24;
  pCallMgmDesc->bDescriptorSubtype = 0x01;
  pCallMgmDesc->bmCapabilities     = 0x00;
  pCallMgmDesc->bDataInterface     = pdev->tclasslist[pdev->classId].Ifs[1];
  *dscr_size += (uint32_t)sizeof(USBD_CDCCallMgmFuncDescTypedef);

  /* ACM Functional Descriptor*/
  pACMDesc =((USBD_CDCACMFuncDescTypedef *)((uint32_t)descriptor_ptr +*dscr_size));
  pACMDesc->bLength                = 0x04;
  pACMDesc->bDescriptorType        = 0x24;
  pACMDesc->bDescriptorSubtype     = 0x02;
  pACMDesc->bmCapabilities         = 0x02;
  *dscr_size += (uint32_t)sizeof(USBD_CDCACMFuncDescTypedef);

  /* Union Functional Descriptor*/
  pUnionDesc =((USBD_CDCUnionFuncDescTypedef *)((uint32_t)descriptor_ptr +*dscr_size));
  pUnionDesc->bLength              = 0x05;
  pUnionDesc->bDescriptorType      = 0x24;
  pUnionDesc->bDescriptorSubtype   = 0x06;
  pUnionDesc->bMasterInterface     = pdev->tclasslist[pdev->classId].Ifs[0];
  pUnionDesc->bSlaveInterface      = pdev->tclasslist[pdev->classId].Ifs[1];
  *dscr_size += (uint32_t)sizeof(USBD_CDCUnionFuncDescTypedef);

  USBD_framework_set_ep(descriptor_ptr, pdev, dscr_size, 2, USBD_EP_TYPE_INTR, USBD_CDCACM_EPINCMD_HS_BINTERVAL, USBD_CDCACM_EPINCMD_FS_BINTERVAL);

  USBD_framework_set_if(descriptor_ptr, pdev, dscr_size, pdev->tclasslist[pdev->classId].Ifs[1], 0, 2, 0x0A, 0, 0, 0);     // Control Interface Descriptor
  USBD_framework_set_ep(descriptor_ptr, pdev, dscr_size, 0, USBD_EP_TYPE_BULK, 0, 0);
  USBD_framework_set_ep(descriptor_ptr, pdev, dscr_size, 1, USBD_EP_TYPE_BULK, 0, 0);

  ((USBD_ConfigDescTypedef *)descriptor_ptr)->bNumInterfaces += 2U;
  ((USBD_ConfigDescTypedef *)descriptor_ptr)->wDescriptorLength =*dscr_size;
}

/*-----------------------------------------------------------------------------------------------------


  \param pdev
  \param pConf
  \param Sze
-----------------------------------------------------------------------------------------------------*/
static void USBD_FrameWork_RNDIS_Desc(USBD_DevClassHandleTypeDef *pdev, uint32_t descriptor_ptr, uint32_t *dscr_size, uint8_t use_iad)
{

  static USBD_CDCHeaderFuncDescTypedef    *pHeadDesc;
  static USBD_CDCCallMgmFuncDescTypedef   *pCallMgmDesc;
  static USBD_CDCACMFuncDescTypedef       *pACMDesc;
  static USBD_CDCUnionFuncDescTypedef     *pUnionDesc;
  static USBD_IadDescTypedef              *pIadDesc;

  if (use_iad)
  {
    pIadDesc                       =((USBD_IadDescTypedef *)(descriptor_ptr +*dscr_size));
    pIadDesc->bLength              = (uint8_t)sizeof(USBD_IadDescTypedef);
    pIadDesc->bDescriptorType      = USB_DESC_TYPE_IAD;
    pIadDesc->bFirstInterface      = pdev->tclasslist[pdev->classId].Ifs[0];
    pIadDesc->bInterfaceCount      = 2;    // 2 interfaces
    pIadDesc->bFunctionClass       = 0xEF; // Wireless Controller
    pIadDesc->bFunctionSubClass    = 0x04; //
    pIadDesc->bFunctionProtocol    = 0x01; //
    pIadDesc->iFunction            = 0; /* String Index */
    *dscr_size                     += (uint32_t)sizeof(USBD_IadDescTypedef);
  }

  USBD_framework_set_if(descriptor_ptr, pdev, dscr_size, pdev->tclasslist[pdev->classId].Ifs[0], 0, 1, 0x02, 0x02, 0x00, 0);     // Control Interface Descriptor

  pHeadDesc =((USBD_CDCHeaderFuncDescTypedef *)((uint32_t)descriptor_ptr +*dscr_size));  // Control interface headers
  /* Header Functional Descriptor*/
  pHeadDesc->bLength               = 0x05;
  pHeadDesc->bDescriptorType       = 0x24;
  pHeadDesc->bDescriptorSubtype    = 0x00;
  pHeadDesc->bcdCDC                = 0x0110;
  *dscr_size += (uint32_t)sizeof(USBD_CDCHeaderFuncDescTypedef);

  /* Call Management Functional Descriptor*/
  pCallMgmDesc =((USBD_CDCCallMgmFuncDescTypedef *)((uint32_t)descriptor_ptr +*dscr_size));
  pCallMgmDesc->bLength            = 0x05;
  pCallMgmDesc->bDescriptorType    = 0x24;
  pCallMgmDesc->bDescriptorSubtype = 0x01;
  pCallMgmDesc->bmCapabilities     = 0x00;
  pCallMgmDesc->bDataInterface     = pdev->tclasslist[pdev->classId].Ifs[1];
  *dscr_size += (uint32_t)sizeof(USBD_CDCCallMgmFuncDescTypedef);

  /* Abstract Control Management Functional Descriptor (ACM) Functional Descriptor*/
  pACMDesc =((USBD_CDCACMFuncDescTypedef *)((uint32_t)descriptor_ptr +*dscr_size));
  pACMDesc->bLength                = 0x04;
  pACMDesc->bDescriptorType        = 0x24;
  pACMDesc->bDescriptorSubtype     = 0x02;
  pACMDesc->bmCapabilities         = 0x00; //
  *dscr_size += (uint32_t)sizeof(USBD_CDCACMFuncDescTypedef);

  /* Union Functional Descriptor*/
  pUnionDesc =((USBD_CDCUnionFuncDescTypedef *)((uint32_t)descriptor_ptr +*dscr_size));
  pUnionDesc->bLength              = 0x05;
  pUnionDesc->bDescriptorType      = 0x24;
  pUnionDesc->bDescriptorSubtype   = 0x06;
  pUnionDesc->bMasterInterface     = pdev->tclasslist[pdev->classId].Ifs[0];
  pUnionDesc->bSlaveInterface      = pdev->tclasslist[pdev->classId].Ifs[1];
  *dscr_size += (uint32_t)sizeof(USBD_CDCUnionFuncDescTypedef);

  USBD_framework_set_ep(descriptor_ptr, pdev, dscr_size, 2, USBD_EP_TYPE_INTR, USBD_CDCACM_EPINCMD_HS_BINTERVAL, USBD_CDCACM_EPINCMD_FS_BINTERVAL);

  USBD_framework_set_if(descriptor_ptr, pdev, dscr_size, pdev->tclasslist[pdev->classId].Ifs[1], 0, 2, 0x0A, 0, 0, 0);     // Control Interface Descriptor

  USBD_framework_set_ep(descriptor_ptr, pdev, dscr_size, 0, USBD_EP_TYPE_BULK, 0, 0);
  USBD_framework_set_ep(descriptor_ptr, pdev, dscr_size, 1, USBD_EP_TYPE_BULK, 0, 0);

  ((USBD_ConfigDescTypedef *)descriptor_ptr)->bNumInterfaces += 2U;
  ((USBD_ConfigDescTypedef *)descriptor_ptr)->wDescriptorLength =*dscr_size;
}

/*-----------------------------------------------------------------------------------------------------


  \param pdev
  \param descriptor_ptr
  \param dscr_size
  \param use_iad
-----------------------------------------------------------------------------------------------------*/
static void     USBD_FrameWork_CDC_ECM_Desc(USBD_DevClassHandleTypeDef *pdev, uint32_t descriptor_ptr, uint32_t *dscr_size, uint8_t use_iad)
{
  static USBD_CDCHeaderFuncDescTypedef    *pHeadDesc;
  static USBD_CDCUnionFuncDescTypedef     *pUnionDesc;
  static USBD_IadDescTypedef              *pIadDesc;
  static USBD_ECMFuncDescTypedef          *pFuncDesc;

  if (use_iad)
  {
    pIadDesc                       =((USBD_IadDescTypedef *)(descriptor_ptr +*dscr_size));
    pIadDesc->bLength              = (uint8_t)sizeof(USBD_IadDescTypedef);
    pIadDesc->bDescriptorType      = USB_DESC_TYPE_IAD;
    pIadDesc->bFirstInterface      = pdev->tclasslist[pdev->classId].Ifs[0];
    pIadDesc->bInterfaceCount      = 2;    // 2 interfaces
    pIadDesc->bFunctionClass       = 0x02; // Wireless Controller
    pIadDesc->bFunctionSubClass    = 0x06; //
    pIadDesc->bFunctionProtocol    = 0x00; //
    pIadDesc->iFunction            = 0; /* String Index */
    *dscr_size                     += (uint32_t)sizeof(USBD_IadDescTypedef);
  }

  USBD_framework_set_if(descriptor_ptr, pdev, dscr_size, pdev->tclasslist[pdev->classId].Ifs[0], 0, 1, 0x02, 0x06, 0x00, 0);     // Control Interface Descriptor

  pHeadDesc =((USBD_CDCHeaderFuncDescTypedef *)((uint32_t)descriptor_ptr +*dscr_size));  // Control interface headers
  /* Append ECM header functional descriptor to Configuration descriptor  */
  pHeadDesc->bLength               = (uint8_t)sizeof(USBD_CDCHeaderFuncDescTypedef);;
  pHeadDesc->bDescriptorType       = 0x24;
  pHeadDesc->bDescriptorSubtype    = 0x00;
  pHeadDesc->bcdCDC                = USBD_DESC_ECM_BCD; // 0x0110;
  *dscr_size += (uint32_t)sizeof(USBD_CDCHeaderFuncDescTypedef);

  /* Append ECM functional descriptor to Configuration descriptor */
  pFuncDesc = ((USBD_ECMFuncDescTypedef *)((uint32_t)descriptor_ptr + *dscr_size));
  pFuncDesc->bFunctionLength         = (uint8_t)sizeof(USBD_ECMFuncDescTypedef);
  pFuncDesc->bDescriptorType         = 0x24;
  pFuncDesc->bDescriptorSubType      = USBD_DESC_SUBTYPE_ACM;
  pFuncDesc->iMacAddress             = CDC_ECM_MAC_STRING_INDEX;
  pFuncDesc->bEthernetStatistics3    = CDC_ECM_ETH_STATS_BYTE3;
  pFuncDesc->bEthernetStatistics2    = CDC_ECM_ETH_STATS_BYTE2;
  pFuncDesc->bEthernetStatistics1    = CDC_ECM_ETH_STATS_BYTE1;
  pFuncDesc->bEthernetStatistics0    = CDC_ECM_ETH_STATS_BYTE0;
  pFuncDesc->wMaxSegmentSize         = CDC_ECM_ETH_MAX_SEGSZE;
  pFuncDesc->bNumberMCFiltes         = CDC_ECM_ETH_NBR_MACFILTERS;
  pFuncDesc->bNumberPowerFiltes      = CDC_ECM_ETH_NBR_PWRFILTERS;
  *dscr_size += (uint32_t)sizeof(USBD_ECMFuncDescTypedef);

  /* Append ECM Union functional descriptor to Configuration descriptor */
  pUnionDesc =((USBD_CDCUnionFuncDescTypedef *)((uint32_t)descriptor_ptr +*dscr_size));
  pUnionDesc->bLength              = (uint8_t)sizeof(USBD_CDCUnionFuncDescTypedef);;
  pUnionDesc->bDescriptorType      = 0x24;
  pUnionDesc->bDescriptorSubtype   = 0x06;
  pUnionDesc->bMasterInterface     = pdev->tclasslist[pdev->classId].Ifs[0];
  pUnionDesc->bSlaveInterface      = pdev->tclasslist[pdev->classId].Ifs[1];
  *dscr_size += (uint32_t)sizeof(USBD_CDCUnionFuncDescTypedef);

  USBD_framework_set_ep(descriptor_ptr, pdev, dscr_size, 2, USBD_EP_TYPE_INTR, USBD_CDCACM_EPINCMD_HS_BINTERVAL, USBD_CDCACM_EPINCMD_FS_BINTERVAL);

  USBD_framework_set_if(descriptor_ptr, pdev, dscr_size, pdev->tclasslist[pdev->classId].Ifs[1], 0, 2, 0x0A, 0, 0, 0);     // Control Interface Descriptor

  USBD_framework_set_ep(descriptor_ptr, pdev, dscr_size, 0, USBD_EP_TYPE_BULK, 0, 0);
  USBD_framework_set_ep(descriptor_ptr, pdev, dscr_size, 1, USBD_EP_TYPE_BULK, 0, 0);

  ((USBD_ConfigDescTypedef *)descriptor_ptr)->bNumInterfaces += 2U;
  ((USBD_ConfigDescTypedef *)descriptor_ptr)->wDescriptorLength =*dscr_size;
}

