#include "fx_api.h"
#include "stm32h7xx_hal.h"
#include "fx_stm32_sd_driver.h"
#include "sdmmc.h"
#include "SEGGER_RTT.h"

// Вспомогательный буфер. В этот буфер считываются данные драйверм SD карты если основной буфер не выровнен по границе 8 байт
// Буфер должен быть выровнен по границе 4 байта чтобы коректно работал DMA механизм драйвера SD карты
// И буфер должен быть выровнен по границе 32 байта чтобы коректно работала инвалидация кэша процессора если включено кэширование данных
ALIGN_32BYTES(static UCHAR scratch_buf[FX_STM32_SD_DEFAULT_SECTOR_SIZE]);

UINT  _fx_partition_offset_calculate(void  *partition_sector, UINT partition, ULONG *partition_start, ULONG *partition_size);



static UINT         is_initialized = 0;

static TX_SEMAPHORE transfer_semaphore;

/*-----------------------------------------------------------------------------------------------------


  \param instance

  \return INT
-----------------------------------------------------------------------------------------------------*/
static INT check_sd_status(uint32_t instance)
{
  uint32_t start = tx_time_get();

  while (tx_time_get()- start < FX_STM32_SD_DEFAULT_TIMEOUT)
  {
    if (MX_SDMMC1_SD_GetStatus() == HAL_OK)
    {
      return 0;
    }
  }
  return 1;
}

/*-----------------------------------------------------------------------------------------------------


  \param media_ptr
-----------------------------------------------------------------------------------------------------*/
VOID  STM32_SD_driver(FX_MEDIA *media_ptr)
{
  UINT status;
  UINT unaligned_buffer;
  ULONG partition_start;
  ULONG partition_size;

#if (FX_STM32_SD_INIT == 0)
  /* the SD was initialized by the application */
  is_initialized = 1;
#endif
  /* before performing any operation, check the status of the SD IP */
  if (is_initialized == 1)
  {
    if (check_sd_status(FX_STM32_SD_INSTANCE) != 0)
    {
      media_ptr->fx_media_driver_status =  FX_IO_ERROR;
      return;
    }
  }

  /* the SD DMA requires a 4-byte aligned buffers */
  unaligned_buffer = (UINT)(media_ptr->fx_media_driver_buffer) & 0x3;

  /* Process the driver request specified in the media control block.  */
  switch (media_ptr->fx_media_driver_request)
  {
  case FX_DRIVER_INIT:
    {
      media_ptr->fx_media_driver_status = FX_SUCCESS;

      if (tx_semaphore_create(&transfer_semaphore, "sd transfer semaphore", 1) != TX_SUCCESS)
      {
        media_ptr->fx_media_driver_status = FX_IO_ERROR;
      }

#if (FX_STM32_SD_INIT == 1)
      /* Initialize the SD instance */
      if (is_initialized == 0)
      {
        status = MX_SDMMC1_SD_Init();

        if (status == HAL_OK)
        {
          is_initialized = 1;
#endif

#if (FX_STM32_SD_INIT == 1)
        }
        else
        {
          media_ptr->fx_media_driver_status =  FX_IO_ERROR;
        }
      }
#endif
      break;
    }

  case FX_DRIVER_UNINIT:
    {
#if (FX_STM32_SD_INIT == 1)
      if (MX_SDMMC1_SD_Deinit() != HAL_OK)
      {
        media_ptr->fx_media_driver_status = FX_IO_ERROR;
      }
      else
      {
        media_ptr->fx_media_driver_status = FX_SUCCESS;
      }

      is_initialized = 0;
#endif

      tx_semaphore_delete(&transfer_semaphore);
      break;
    }

  case FX_DRIVER_READ:
    {
      media_ptr->fx_media_driver_status = FX_IO_ERROR;

      if (SD_FX_read_data(media_ptr, media_ptr->fx_media_driver_logical_sector + media_ptr->fx_media_hidden_sectors,media_ptr->fx_media_driver_sectors, unaligned_buffer) == FX_SUCCESS)
      {
        media_ptr->fx_media_driver_status = FX_SUCCESS;
      }

      break;
    }

  case FX_DRIVER_WRITE:
    {
      media_ptr->fx_media_driver_status = FX_IO_ERROR;

      if (SD_FX_write_data(media_ptr, media_ptr->fx_media_driver_logical_sector + media_ptr->fx_media_hidden_sectors,media_ptr->fx_media_driver_sectors, unaligned_buffer) == FX_SUCCESS)
      {
        media_ptr->fx_media_driver_status = FX_SUCCESS;
      }

      break;
    }

  case FX_DRIVER_FLUSH:
    {
      /* Return driver success.  */
      media_ptr->fx_media_driver_status =  FX_SUCCESS;
      break;
    }

  case FX_DRIVER_ABORT:
    {
      /* Return driver success.  */
      media_ptr->fx_media_driver_status =  FX_SUCCESS;
      break;
    }

  case FX_DRIVER_BOOT_READ:
    {
      /* the boot sector is the sector zero */
      status = SD_FX_read_data(media_ptr, 0, media_ptr->fx_media_driver_sectors, unaligned_buffer);

      if (status != FX_SUCCESS)
      {
        media_ptr->fx_media_driver_status = status;
        break;
      }

      /* Check if the sector 0 is the actual boot sector, otherwise calculate the offset into it.
      Please note that this should belong to higher level of MW to do this check and it is here
      as a temporary work solution */

      partition_start =  0;

      status =  _fx_partition_offset_calculate(media_ptr->fx_media_driver_buffer, 0, &partition_start,&partition_size);

      /* Check partition read error.  */
      if (status)
      {
        /* Unsuccessful driver request.  */
        media_ptr->fx_media_driver_status =  FX_IO_ERROR;
        break;
      }

      /* Now determine if there is a partition...   */
      if (partition_start)
      {

        if (check_sd_status(FX_STM32_SD_INSTANCE) != 0)
        {
          media_ptr->fx_media_driver_status =  FX_IO_ERROR;
          break;
        }

        /* Yes, now lets read the actual boot record.  */
        status = SD_FX_read_data(media_ptr, partition_start, media_ptr->fx_media_driver_sectors, unaligned_buffer);

        if (status != FX_SUCCESS)
        {
          media_ptr->fx_media_driver_status = status;
          break;
        }
      }

      /* Successful driver request.  */
      media_ptr->fx_media_driver_status =  FX_SUCCESS;
      break;
    }

  case FX_DRIVER_BOOT_WRITE:
    {
      status = SD_FX_write_data(media_ptr, 0, media_ptr->fx_media_driver_sectors, unaligned_buffer);

      media_ptr->fx_media_driver_status = status;

      break;
    }

  default:
    {
      media_ptr->fx_media_driver_status =  FX_IO_ERROR;
      break;
    }
  }
}

/*-----------------------------------------------------------------------------------------------------
  Чтение данных с помощью DMA в буфер файловой системы

  \param media_ptr
  \param start_sector
  \param num_sectors
  \param use_scratch_buffer - use_scratch_buffer to enable scratch_buf buffer usage or not.

  \return UINT
-----------------------------------------------------------------------------------------------------*/
UINT SD_FX_read_data(FX_MEDIA *media_ptr, ULONG start_sector, UINT num_sectors, UINT use_scratch_buffer)
{
  INT i = 0;
  UINT status;
  UCHAR *read_addr = media_ptr->fx_media_driver_buffer;
  //uint16_t crc;


  // Если область памяти назначени выходит за пределы специальной области памяти для работы с SDMMC то использовать для чтения промежуточный буфер
  //if (((uint32_t)read_addr < 0x24078000) || ((uint32_t)read_addr > 0x2407FFFF))
  //{
  //  use_scratch_buffer  = 1;
  //}

  if (tx_semaphore_get(&transfer_semaphore, FX_STM32_SD_DEFAULT_TIMEOUT) != TX_SUCCESS)
  {
    return FX_IO_ERROR;
  }

  if (use_scratch_buffer)
  {



    //crc = 0xFFFF;
    for (i = 0; i < num_sectors; i++)
    {
      /* Start reading into the scratch_buf buffer */
      status = MX_SDMMC1_SD_ReadBlocks_DMA(scratch_buf, (UINT)start_sector++, 1);


      if (status != HAL_OK)
      {
        /* read error occurred, call the error handler code then return immediately */
        tx_semaphore_put(&transfer_semaphore);
        status = FX_IO_ERROR;
        return FX_IO_ERROR;
      }

      /* wait for read transfer notification */
      if (tx_semaphore_get(&transfer_semaphore, FX_STM32_SD_DEFAULT_TIMEOUT) != TX_SUCCESS)
      {
        return FX_IO_ERROR;
      }

#if (FX_STM32_SD_CACHE_MAINTENANCE == 1)
      SCB_InvalidateDCache_by_Addr((void *)scratch_buf, (int32_t)FX_STM32_SD_DEFAULT_SECTOR_SIZE);
#endif

      //crc = Get_CRC16_of_block(scratch_buf,512, crc);

      _fx_utility_memory_copy(scratch_buf, read_addr, FX_STM32_SD_DEFAULT_SECTOR_SIZE);
      read_addr += FX_STM32_SD_DEFAULT_SECTOR_SIZE;
    }

    // Отладочный вывод
    //RTT_terminal_printf("Sec:%07d Num:%04d %01d %08X crc=%04X\r\n", start_sector, num_sectors, use_scratch_buffer, (uint32_t)scratch_buf, crc);


    /* Check if all sectors were read */
    if (i == num_sectors)
    {
      status = FX_SUCCESS;
    }
    else
    {
      status = FX_BUFFER_ERROR;
    }
  }
  else
  {

    status = MX_SDMMC1_SD_ReadBlocks_DMA(media_ptr->fx_media_driver_buffer, (UINT)start_sector, num_sectors);


    if (status != HAL_OK)
    {
      /* read error occurred, call the error handler code then return immediately */
      tx_semaphore_put(&transfer_semaphore);
      status = FX_IO_ERROR;

      return FX_IO_ERROR;
    }

    /* wait for read transfer notification */
    if (tx_semaphore_get(&transfer_semaphore, FX_STM32_SD_DEFAULT_TIMEOUT) != TX_SUCCESS)
    {
      return FX_IO_ERROR;
    }


#if (FX_STM32_SD_CACHE_MAINTENANCE == 1)
    SCB_InvalidateDCache_by_Addr((void *)media_ptr->fx_media_driver_buffer, (int32_t)num_sectors * FX_STM32_SD_DEFAULT_SECTOR_SIZE);
#endif

    // Отладочный вывод
    //crc = Get_CRC16_of_block(media_ptr->fx_media_driver_buffer,512*num_sectors, 0xFFFF);
    //RTT_terminal_printf("Sec:%07d Num:%04d %01d %08X crc=%04X\r\n", start_sector, num_sectors, use_scratch_buffer, (uint32_t)media_ptr->fx_media_driver_buffer, crc);

    status = FX_SUCCESS;
  }

  if (tx_semaphore_put(&transfer_semaphore) != TX_SUCCESS)
  {
    return FX_IO_ERROR;
  }

  return status;
}

/*-----------------------------------------------------------------------------------------------------
  Запись данных с помощью DMA из буфера файловой системы


  \param media_ptr
  \param start_sector
  \param num_sectors
  \param use_scratch_buffer - use_scratch_buffer to enable scratch_buf buffer usage or not.

  \return UINT
-----------------------------------------------------------------------------------------------------*/
UINT SD_FX_write_data(FX_MEDIA *media_ptr, ULONG start_sector, UINT num_sectors, UINT use_scratch_buffer)
{
  INT i = 0;
  UINT status;
  UCHAR *write_addr = media_ptr->fx_media_driver_buffer;

  if (tx_semaphore_get(&transfer_semaphore, FX_STM32_SD_DEFAULT_TIMEOUT) != TX_SUCCESS)
  {
    return FX_IO_ERROR;
  }

  // Если область памяти назначени выходит за пределы специальной области памяти для работы с SDMMC то использовать для чтения промежуточный буфер
  //if (((uint32_t)write_addr < 0x24078000) || ((uint32_t)write_addr > 0x2407FFFF))
  //{
  //  use_scratch_buffer  = 1;
  //}

  if (use_scratch_buffer)
  {


    for (i = 0; i < num_sectors; i++)
    {
      _fx_utility_memory_copy(write_addr, scratch_buf, FX_STM32_SD_DEFAULT_SECTOR_SIZE);
      write_addr += FX_STM32_SD_DEFAULT_SECTOR_SIZE;

#if (FX_STM32_SD_CACHE_MAINTENANCE == 1)
      /* Clean the DCache to make the SD DMA see the actual content of the scratch_buf buffer */
      SCB_CleanDCache_by_Addr((uint32_t *)scratch_buf, (int32_t)FX_STM32_SD_DEFAULT_SECTOR_SIZE);
#endif

      status = MX_SDMMC1_SD_WriteBlocks_DMA(scratch_buf, (UINT)start_sector++, 1);

      if (status != HAL_OK)
      {
        /* in case of error call the error handling macro */
        tx_semaphore_put(&transfer_semaphore);
        status = FX_IO_ERROR;
        return FX_IO_ERROR;
      }

      if (tx_semaphore_get(&transfer_semaphore, FX_STM32_SD_DEFAULT_TIMEOUT) != TX_SUCCESS)
      {
        return FX_IO_ERROR;
      }

    }

    if (i == num_sectors)
    {
      status = FX_SUCCESS;
    }
    else
    {
      status = FX_BUFFER_ERROR;
    }
  }
  else
  {
#if (FX_STM32_SD_CACHE_MAINTENANCE == 1)
    SCB_CleanDCache_by_Addr((uint32_t *)media_ptr->fx_media_driver_buffer, (int32_t)num_sectors * FX_STM32_SD_DEFAULT_SECTOR_SIZE);
#endif
    status = MX_SDMMC1_SD_WriteBlocks_DMA(media_ptr->fx_media_driver_buffer, (UINT)start_sector, num_sectors);

    if (status != HAL_OK)
    {
      tx_semaphore_put(&transfer_semaphore);
      status = FX_IO_ERROR;
      return FX_IO_ERROR;
    }

    /* when defined, wait for the write notification */
    if (tx_semaphore_get(&transfer_semaphore, FX_STM32_SD_DEFAULT_TIMEOUT) != TX_SUCCESS)
    {
      return FX_IO_ERROR;
    }

    status = FX_SUCCESS;
  }

  if (tx_semaphore_put(&transfer_semaphore) != TX_SUCCESS)
  {
    return FX_IO_ERROR;
  }

  return status;
}

/*-----------------------------------------------------------------------------------------------------
  Чтение данных с помощью DMA в буфер

  \param media_ptr
  \param start_sector
  \param num_sectors

  \return UINT
-----------------------------------------------------------------------------------------------------*/
UINT SD_read_data(uint8_t *buf, ULONG start_sector, UINT num_sectors)
{
  int32_t    i = 0;
  uint32_t   status;
  uint8_t    *read_addr;
  uint8_t     use_scratch_buffer;

  if (tx_semaphore_get(&transfer_semaphore, FX_STM32_SD_DEFAULT_TIMEOUT) != TX_SUCCESS)
  {
    return FX_IO_ERROR;
  }

  /* the SD DMA requires a 4-byte aligned buffers */
  use_scratch_buffer = (UINT)(buf) & 0x3;


  if (use_scratch_buffer)
  {
    read_addr = buf;

    for (i = 0; i < num_sectors; i++)
    {
      /* Start reading into the scratch_buf buffer */
      status = MX_SDMMC1_SD_ReadBlocks_DMA(scratch_buf, (UINT)start_sector++, 1);

      if (status != HAL_OK)
      {
        /* read error occurred, call the error handler code then return immediately */
        tx_semaphore_put(&transfer_semaphore);
        status = FX_IO_ERROR;
        return FX_IO_ERROR;
      }

      /* wait for read transfer notification */
      if (tx_semaphore_get(&transfer_semaphore, FX_STM32_SD_DEFAULT_TIMEOUT) != TX_SUCCESS)
      {
        return FX_IO_ERROR;
      }

#if (FX_STM32_SD_CACHE_MAINTENANCE == 1)
      SCB_InvalidateDCache_by_Addr((void *)scratch_buf, (int32_t)FX_STM32_SD_DEFAULT_SECTOR_SIZE);
#endif

      _fx_utility_memory_copy(scratch_buf, read_addr, FX_STM32_SD_DEFAULT_SECTOR_SIZE);
      read_addr += FX_STM32_SD_DEFAULT_SECTOR_SIZE;
    }

    /* Check if all sectors were read */
    if (i == num_sectors)
    {
      status = FX_SUCCESS;
    }
    else
    {
      status = FX_BUFFER_ERROR;
    }
  }
  else
  {

    status = MX_SDMMC1_SD_ReadBlocks_DMA(buf, (UINT)start_sector, num_sectors);

    if (status != HAL_OK)
    {
      /* read error occurred, call the error handler code then return immediately */
      tx_semaphore_put(&transfer_semaphore);
      status = FX_IO_ERROR;

      return FX_IO_ERROR;
    }

    /* wait for read transfer notification */
    if (tx_semaphore_get(&transfer_semaphore, FX_STM32_SD_DEFAULT_TIMEOUT) != TX_SUCCESS)
    {
      return FX_IO_ERROR;
    }


#if (FX_STM32_SD_CACHE_MAINTENANCE == 1)
    SCB_InvalidateDCache_by_Addr((void *)buf, (int32_t)num_sectors * FX_STM32_SD_DEFAULT_SECTOR_SIZE);
#endif

    status = FX_SUCCESS;
  }

  if (tx_semaphore_put(&transfer_semaphore) != TX_SUCCESS)
  {
    return FX_IO_ERROR;
  }

  return status;
}


/*-----------------------------------------------------------------------------------------------------
  Запись данных с помощью DMA из буфера файловой системы


  \param media_ptr
  \param start_sector
  \param num_sectors
  \param use_scratch_buffer - use_scratch_buffer to enable scratch_buf buffer usage or not.

  \return UINT
-----------------------------------------------------------------------------------------------------*/
UINT SD_write_data(uint8_t *buf, ULONG start_sector, UINT num_sectors)
{
  int32_t    i = 0;
  uint32_t   status;
  uint8_t    *write_addr;
  uint8_t     use_scratch_buffer;

  if (tx_semaphore_get(&transfer_semaphore, FX_STM32_SD_DEFAULT_TIMEOUT) != TX_SUCCESS)
  {
    return FX_IO_ERROR;
  }

  /* the SD DMA requires a 4-byte aligned buffers */
  use_scratch_buffer = (UINT)(buf) & 0x3;

  if (use_scratch_buffer)
  {
    write_addr = buf;

    for (i = 0; i < num_sectors; i++)
    {
      _fx_utility_memory_copy(write_addr, scratch_buf, FX_STM32_SD_DEFAULT_SECTOR_SIZE);
      write_addr += FX_STM32_SD_DEFAULT_SECTOR_SIZE;

#if (FX_STM32_SD_CACHE_MAINTENANCE == 1)
      /* Clean the DCache to make the SD DMA see the actual content of the scratch_buf buffer */
      SCB_CleanDCache_by_Addr((uint32_t *)scratch_buf, (int32_t)FX_STM32_SD_DEFAULT_SECTOR_SIZE);
#endif

      status = MX_SDMMC1_SD_WriteBlocks_DMA(scratch_buf, (UINT)start_sector++, 1);

      if (status != HAL_OK)
      {
        /* in case of error call the error handling macro */
        tx_semaphore_put(&transfer_semaphore);
        status = FX_IO_ERROR;
        return FX_IO_ERROR;
      }

      if (tx_semaphore_get(&transfer_semaphore, FX_STM32_SD_DEFAULT_TIMEOUT) != TX_SUCCESS)
      {
        return FX_IO_ERROR;
      }

    }

    if (i == num_sectors)
    {
      status = FX_SUCCESS;
    }
    else
    {
      status = FX_BUFFER_ERROR;
    }
  }
  else
  {
#if (FX_STM32_SD_CACHE_MAINTENANCE == 1)
    SCB_CleanDCache_by_Addr((uint32_t *)buf, (int32_t)num_sectors * FX_STM32_SD_DEFAULT_SECTOR_SIZE);
#endif
    status = MX_SDMMC1_SD_WriteBlocks_DMA(buf, (UINT)start_sector, num_sectors);

    if (status != HAL_OK)
    {
      tx_semaphore_put(&transfer_semaphore);
      status = FX_IO_ERROR;
      return FX_IO_ERROR;
    }

    /* when defined, wait for the write notification */
    if (tx_semaphore_get(&transfer_semaphore, FX_STM32_SD_DEFAULT_TIMEOUT) != TX_SUCCESS)
    {
      return FX_IO_ERROR;
    }

    status = FX_SUCCESS;
  }

  if (tx_semaphore_put(&transfer_semaphore) != TX_SUCCESS)
  {
    return FX_IO_ERROR;
  }

  return status;
}

/*-----------------------------------------------------------------------------------------------------


  \param hsd
-----------------------------------------------------------------------------------------------------*/
void HAL_SD_TxCpltCallback(SD_HandleTypeDef *hsd)
{
  tx_semaphore_put(&transfer_semaphore);
}

/*-----------------------------------------------------------------------------------------------------


  \param hsd
-----------------------------------------------------------------------------------------------------*/
void HAL_SD_RxCpltCallback(SD_HandleTypeDef *hsd)
{
  tx_semaphore_put(&transfer_semaphore);
}

