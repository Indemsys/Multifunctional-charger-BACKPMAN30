// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2021-09-27
// 16:08:42 
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"


#define USBX_MEMORY_SIZE                      (48 * 1024)


/*-----------------------------------------------------------------------------------------------------


  \param memory_ptr

  \return UINT
-----------------------------------------------------------------------------------------------------*/
UINT App_USBX_Device_Init(void)
{
  UINT               ret = UX_SUCCESS;
  CHAR              *ptr;

  ULONG device_framework_hs_length;
  ULONG device_framework_fs_length;
  ULONG string_framework_length;
  ULONG languge_id_framework_length;
  UCHAR *device_framework_high_speed;
  UCHAR *device_framework_full_speed;
  UCHAR *string_framework;
  UCHAR *language_id_framework;

  ptr = App_malloc(USBX_MEMORY_SIZE);
  ux_system_initialize(ptr, USBX_MEMORY_SIZE, UX_NULL, 0);                                                     /* Initialize USBX Memory */
  device_framework_high_speed = USBD_Get_Device_Framework_Speed(USBD_HIGH_SPEED,&device_framework_hs_length); /* Get_Device_Framework_High_Speed and get the length */
  device_framework_full_speed = USBD_Get_Device_Framework_Speed(USBD_FULL_SPEED,&device_framework_fs_length); /* Get_Device_Framework_Full_Speed and get the length */
  string_framework            = USBD_Get_String_Framework(&string_framework_length);                           /* Get_String_Framework and get the length */
  language_id_framework       = USBD_Get_Language_Id_Framework(&languge_id_framework_length);                  /* Get_Language_Id_Framework and get the length */

  ret =  _ux_device_stack_initialize(device_framework_high_speed,
       device_framework_hs_length,
       device_framework_full_speed,
       device_framework_fs_length,
       string_framework,
       string_framework_length,
       language_id_framework,
       languge_id_framework_length, UX_NULL);

  if (ret != UX_SUCCESS)
  {
    return ret;
  }

#ifdef USBD_CDC_ECM_CLASS_ACTIVATED
  Register_cdc_ecm_class();
#else
  Register_rndis_class();
  Register_USB_CDC_ACM_class();
#endif
  //Register_USB_MSC_class();


  tx_thread_sleep(0.1 * TX_TIMER_TICKS_PER_SECOND); /* Sleep for 100 ms */

  //HAL_PWREx_EnableUSBVoltageDetector();

  MX_USB_OTG_FS_PCD_Init();                                             // initialize the device controller HAL driver
  HAL_PCDEx_SetRxFiFo(&hpcd_USB_OTG_FS, 0x200);
  HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 0, 0x40);
  HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 1, 0x40);
  HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 2, 0x40);
  HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 3, 0x40);
  HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 4, 0x40);
  HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 5, 0x40);
  HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 6, 0x40);
  HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 7, 0x40);
  HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 8, 0x40);

  _ux_dcd_stm32_initialize((ULONG)USB_OTG_FS, (ULONG)&hpcd_USB_OTG_FS); // initialize and link controller HAL driver to USBx

  HAL_PCD_Start(&hpcd_USB_OTG_FS);                                      // Start USB device by connecting the DP pullup

  return ret;
}


