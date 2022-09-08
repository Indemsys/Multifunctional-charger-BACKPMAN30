#ifndef USB_MSC_DRIVER_H__
  #define USB_MSC_DRIVER_H__

  #ifdef __cplusplus
extern "C"
{
  #endif
  UINT  STORAGE_Status(VOID *storage, ULONG lun, ULONG media_id, ULONG *media_status);
  UINT  STORAGE_Read(VOID *storage, ULONG lun, UCHAR *data_pointer, ULONG number_blocks, ULONG lba, ULONG *media_status);
  UINT  STORAGE_Write(VOID *storage, ULONG lun, UCHAR *pData, ULONG NumberOfBlocks, ULONG BlockAdd, ULONG *media_status);
  void HAL_SD_TxCpltCallback(SD_HandleTypeDef *hsd);
  void HAL_SD_RxCpltCallback(SD_HandleTypeDef *hsd);

  UINT  Register_USB_MSC_class(void);

#ifdef __cplusplus
}
#endif
#endif

