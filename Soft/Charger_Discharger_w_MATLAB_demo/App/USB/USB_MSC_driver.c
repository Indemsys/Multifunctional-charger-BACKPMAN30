#include "App.h"
#include "fx_stm32_sd_driver.h"

SD_HandleTypeDef hsd_sdmmc[1];

extern PCD_HandleTypeDef hpcd_USB_OTG_FS;

UX_SLAVE_CLASS_STORAGE_PARAMETER    storage_parameter;


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
UINT Register_USB_MSC_class(void)
{
  SD_HandleTypeDef  *p_hsd1;
  UINT               ret = UX_SUCCESS;

  p_hsd1 =&hsd_sdmmc[SD_INSTANCE];
  storage_parameter.ux_slave_class_storage_parameter_number_lun = 1;                                                                            /* Store the number of LUN in this device storage instance. */
  storage_parameter.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_last_lba       = (ULONG)(p_hsd1->SdCard.BlockNbr - 1); /* Initialize the storage class parameters for reading/writing to the Flash Disk. */
  storage_parameter.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_block_length   = p_hsd1->SdCard.BlockSize;
  storage_parameter.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_type           = 0;
  storage_parameter.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_removable_flag = 0x80;
  storage_parameter.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_read           = STORAGE_Read;
  storage_parameter.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_write          = STORAGE_Write;
  storage_parameter.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_status         = STORAGE_Status;

  ret =  _ux_device_stack_class_register(_ux_system_slave_class_storage_name, _ux_device_class_storage_entry, 1, USBD_MSC_INTERFACE_INDEX, (VOID *)&storage_parameter);
  return ret;
}


/*-----------------------------------------------------------------------------------------------------


  \param storage
  \param lun
  \param media_id
  \param media_status

  \return UINT
-----------------------------------------------------------------------------------------------------*/
UINT STORAGE_Status(VOID *storage, ULONG lun, ULONG media_id, ULONG *media_status)
{
  /* The ATA drive never fails. This is just for app_usb_device only !!!! */
  return (UX_SUCCESS);
}

/*-----------------------------------------------------------------------------------------------------


  \param storage
  \param lun
  \param data_pointer
  \param number_blocks
  \param lba
  \param media_status

  \return UINT
-----------------------------------------------------------------------------------------------------*/
UINT STORAGE_Read(VOID *storage, ULONG lun, UCHAR *pData, ULONG NumberOfBlocks, ULONG BlockAdd, ULONG *media_status)
{
  uint32_t           res;

  res = SD_read_data(pData, BlockAdd, NumberOfBlocks);
  if (res != FX_SUCCESS)
  {
    *media_status = UX_DEVICE_CLASS_STORAGE_SENSE_STATUS(UX_SLAVE_CLASS_STORAGE_SENSE_KEY_HARDWARE_ERROR,0xFF,0xFF);
    return UX_ERROR;
  }
  *media_status = 0;
  return UX_SUCCESS;
}

/*-----------------------------------------------------------------------------------------------------


  \param storage
  \param lun
  \param data_pointer
  \param number_blocks
  \param lba
  \param media_status

  \return UINT
-----------------------------------------------------------------------------------------------------*/
UINT STORAGE_Write(VOID *storage, ULONG lun, UCHAR *pData, ULONG NumberOfBlocks, ULONG BlockAdd, ULONG *media_status)
{
  uint32_t           res;

  res = SD_write_data(pData, BlockAdd, NumberOfBlocks);
  if (res != FX_SUCCESS)
  {
    *media_status = UX_DEVICE_CLASS_STORAGE_SENSE_STATUS(UX_SLAVE_CLASS_STORAGE_SENSE_KEY_HARDWARE_ERROR,0xFF,0xFF);
    return UX_ERROR;
  }
  *media_status = 0;
  return UX_SUCCESS;
}




