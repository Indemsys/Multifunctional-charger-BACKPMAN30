#include   "App.h"
#include   "Flash_media_driver.h"
#include   "STfs_int.h"
#include   "STfs_api.h"


#ifdef   SFFS_H753_FLASH_DRIVER

  #define  H753_SECTORS_NUM  8
  #define  H753_SECTOR_SIZE       (1024*128)

const T_flash_sect_map H753_DataFlash_map[H753_SECTORS_NUM] =
{
  {0x08100000, 0x08120000 - sizeof(T_stfs_sector_descriptor)},  // 128 K
  {0x08120000, 0x08140000 - sizeof(T_stfs_sector_descriptor)},  // 128 K
  {0x08140000, 0x08160000 - sizeof(T_stfs_sector_descriptor)},  // 128 K
  {0x08160000, 0x08180000 - sizeof(T_stfs_sector_descriptor)},  // 128 K
  {0x08180000, 0x081A0000 - sizeof(T_stfs_sector_descriptor)},  // 128 K
  {0x081A0000, 0x081C0000 - sizeof(T_stfs_sector_descriptor)},  // 128 K
  {0x081C0000, 0x081E0000 - sizeof(T_stfs_sector_descriptor)},  // 128 K
  {0x081E0000, 0x08200000 - sizeof(T_stfs_sector_descriptor)},  // 128 K
};

const T_DataFlash_configuration H753_DataFlash_Config =
{
  0,
  H753_SECTOR_SIZE,
  H753_SECTORS_NUM,
  H753_DataFlash_map,
};

static TX_MUTEX                    stfs_mutex;
static TX_EVENT_FLAGS_GROUP        stfs_flag_grp;
static uint8_t                     stfs_driver_initialised = 0;
static uint32_t                    sectors_erase_counters[H753_SECTORS_NUM];


  #define STFS_MUTEX_TIMEOUT         MS_TO_TICKS(1000)
  #define STFS_FLAG_OP_DONE          BIT(0)
  #define STFS_FLAG_OP_ERROR         BIT(1)

#endif


#ifdef   STFS_SIMULATOR_FLASH_DRIVER

  #define  EMPTY_PTTRN            (0xFFFFFFFF)
  #define  SIMULATOR_SECTORS_NUM  8
  #define  SIMULATOR_SECTOR_SIZE  (1024*128)


T_sim_state           sim_state_area[SIMULATOR_SECTORS_NUM][SIMULATOR_SECTOR_SIZE];
uint32_t              sectors_erase_counters[SIMULATOR_SECTORS_NUM];

  #define SEC_ALIGN  0x10000
uint8_t               sim_flash_area[SIMULATOR_SECTORS_NUM*SIMULATOR_SECTOR_SIZE + SEC_ALIGN];
T_flash_sect_map      Simulator_DataFlash_map[SIMULATOR_SECTORS_NUM];

const T_DataFlash_configuration Simulator_DataFlash_Config =
{
  0,
  SIMULATOR_SECTOR_SIZE,
  SIMULATOR_SECTORS_NUM,
  Simulator_DataFlash_map,
};


#endif

T_DataFlash_driver_stat  stfs_drv_stat;


/*-----------------------------------------------------------------------------------------------------


  \param void

  \return T_DataFlash_driver_stat*
-----------------------------------------------------------------------------------------------------*/
T_DataFlash_driver_stat* FlashDriver_get_stat(void)
{
  return &stfs_drv_stat;
}


#ifdef   SFFS_H753_FLASH_DRIVER

/*-----------------------------------------------------------------------------------------------------


  \param sector_num

  \return uint8_t*
-----------------------------------------------------------------------------------------------------*/
uint8_t* STfs_get_sector(uint32_t sector_num)
{
  return (uint8_t *)(H753_DataFlash_map[sector_num].start_adr);
}


/*-----------------------------------------------------------------------------------------------------


  \param sector_num

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t STfs_get_sector_size(uint32_t sector_num)
{
  return H753_SECTOR_SIZE;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t STfs_get_sec_num(void)
{
  return H753_SECTORS_NUM;
}

/*-----------------------------------------------------------------------------------------------------


  \param sector

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t STfs_get_sector_erase_num(uint32_t sector)
{
  return sectors_erase_counters[sector];
}

/*-----------------------------------------------------------------------------------------------------


  \param flag
-----------------------------------------------------------------------------------------------------*/
static void STfs_set_flags(uint32_t flags)
{
  tx_event_flags_set(&stfs_flag_grp, flags,  TX_OR);
}

/*-----------------------------------------------------------------------------------------------------


  \param flag
  \param timeout
-----------------------------------------------------------------------------------------------------*/
static uint32_t  STfs_wait_flags(uint32_t flags, uint32_t *p_actual_flags, uint32_t timeout_ms)
{
  return tx_event_flags_get(&stfs_flag_grp, flags , TX_OR_CLEAR, (ULONG *)p_actual_flags, MS_TO_TICKS(timeout_ms));
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void FLASH_IRQHandler(void)
{
  HAL_FLASH_IRQHandler();
}

/*-----------------------------------------------------------------------------------------------------
 FLASH end of operation interrupt callback

 param  ReturnValue The value saved in this parameter depends on the ongoing procedure
                 Mass Erase: Bank number which has been requested to erase
                 Sectors Erase: Sector which has been erased
                   (if 0xFFFFFFFF, it means that all the selected sectors have been erased)
                 Program: Address which was selected for data program
 retval None


  \param ReturnValue
-----------------------------------------------------------------------------------------------------*/
void HAL_FLASH_EndOfOperationCallback(uint32_t ReturnValue)
{
  STfs_set_flags(STFS_FLAG_OP_DONE);
}

/*-----------------------------------------------------------------------------------------------------
  FLASH operation error interrupt callback

  param  ReturnValue The value saved in this parameter depends on the ongoing procedure
                 Mass Erase: Bank number which has been requested to erase
                 Sectors Erase: Sector number which returned an error
                 Program: Address which was selected for data program
  retval None

  \param ReturnValue
-----------------------------------------------------------------------------------------------------*/
void HAL_FLASH_OperationErrorCallback(uint32_t ReturnValue)
{
  STfs_set_flags(STFS_FLAG_OP_ERROR);
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t STfs_get_sync_obj(void)
{
  if (tx_mutex_get(&stfs_mutex, STFS_MUTEX_TIMEOUT) != TX_SUCCESS) return STFS_ACCESS_ERROR;
  return STFS_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t STfs_put_sync_obj(void)
{
  if (tx_mutex_put(&stfs_mutex) != TX_SUCCESS) return STFS_ACCESS_ERROR;
  return STFS_OK;
}


/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t STfs_flash_driver_init(void)
{

  stfs_drv_stat.stfs_last_flash_progr_err = 0;
  stfs_drv_stat.addr   = 0;
  stfs_drv_stat.stfs_last_sec_erasing_err = 0;
  stfs_drv_stat.sector = 0;

  if (stfs_driver_initialised) return STFS_OK;

  // Создаем мьютекс для достпа к API в многозадачной среде
  if (tx_mutex_create(&stfs_mutex, "SFFS", TX_INHERIT) != TX_SUCCESS)
  {
    return STFS_ERROR;
  }

  if (tx_event_flags_create(&stfs_flag_grp, "SFFS") != TX_SUCCESS)
  {
    tx_mutex_delete(&stfs_mutex);
    return STFS_ERROR;
  }

  // Разрешить прерывания от FLASH
  HAL_NVIC_SetPriority(FLASH_IRQn, 15, 0);
  HAL_NVIC_EnableIRQ(FLASH_IRQn);

  HAL_FLASH_Unlock();
  HAL_FLASHEx_Unlock_Bank2();
  __HAL_FLASH_ENABLE_IT(FLASH_IT_ALL_BANK2);

  stfs_driver_initialised = 1;
  return STFS_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param pdev_map
  \param empty_pttrn
  \param number_of_sectors

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
int32_t  FlashDriver_get_flash_mem_params(void **pdev_map, uint32_t *number_of_sectors)
{
  *pdev_map = (void *)H753_DataFlash_Config.sect_map;
  *number_of_sectors = H753_DataFlash_Config.flash_sect_num;
  return 0;
}


/*-----------------------------------------------------------------------------------------------------


  \param sector  - номер стираемого сектора

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
int32_t FlashDriver_erase_sector(uint32_t sector)
{
  FLASH_EraseInitTypeDef  erase_cmd;
  HAL_StatusTypeDef       status;
  T_sys_timestump         tstmp1;
  T_sys_timestump         tstmp2;
  uint32_t                res = STFS_OK;

  erase_cmd.Banks        = FLASH_BANK_2;
  erase_cmd.NbSectors    = 1;
  erase_cmd.Sector       = sector;
  erase_cmd.TypeErase    = FLASH_TYPEERASE_SECTORS;
  erase_cmd.VoltageRange = FLASH_VOLTAGE_RANGE_4;

  STFS_ERASELOG("Erasing sector %d", sector);
  sectors_erase_counters[sector]++;

  Get_hw_timestump(&tstmp1);

  status = HAL_FLASHEx_Erase_IT(&erase_cmd);
  if (status == HAL_OK)
  {
    uint32_t actual_flags;
    uint32_t res;
    res = STfs_wait_flags(STFS_FLAG_OP_DONE + STFS_FLAG_OP_ERROR,&actual_flags, 50000);
    if (res != TX_SUCCESS)
    {
      stfs_drv_stat.stfs_last_sec_erasing_err = STFS_SECTOR_ERASE_ERROR2;
      stfs_drv_stat.sector = sector;
      STFS_ERASELOG("Erasing error 2");
      res = STFS_SECTOR_ERASE_ERROR2;
    }
    else
    {
      if (actual_flags & STFS_FLAG_OP_ERROR)
      {
        stfs_drv_stat.stfs_last_sec_erasing_err = STFS_SECTOR_ERASE_ERROR3;
        stfs_drv_stat.sector = sector;
        STFS_ERASELOG("Erasing error 3");
        res = STFS_SECTOR_ERASE_ERROR3;
      }
      else
      {
        STFS_ERASELOG("Erasing Ok");

        Get_hw_timestump(&tstmp2);
        uint64_t dt = Hw_timestump_diff64_us(&tstmp1, &tstmp2);
        if (stfs_drv_stat.min_sec_erasing_time ==0)
        {
          stfs_drv_stat.min_sec_erasing_time = dt;
        }
        else if (stfs_drv_stat.min_sec_erasing_time> dt)
        {
          stfs_drv_stat.min_sec_erasing_time = dt;
        }

        if (stfs_drv_stat.max_sec_erasing_time ==0)
        {
          stfs_drv_stat.max_sec_erasing_time = dt;
        }
        else if (stfs_drv_stat.max_sec_erasing_time < dt)
        {
          stfs_drv_stat.max_sec_erasing_time = dt;
        }

      }
    }
  }
  else
  {
    stfs_drv_stat.stfs_last_sec_erasing_err = STFS_SECTOR_ERASE_ERROR1;
    stfs_drv_stat.sector = sector;
    STFS_ERASELOG("Erasing error 1");
    res = STFS_SECTOR_ERASE_ERROR1;
  }

  return res;
}

/*-----------------------------------------------------------------------------------------------------
  Запись слова во Flash размером FLASH_WORD_SIZE

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t FlashDriver_progr_word(uint32_t FlashAddress,  uint8_t *DataAddress)
{
  HAL_StatusTypeDef       status;

  if ((FlashAddress % STFS_FLASH_WORD_SIZE) != 0)
  {
    return STFS_ERROR;
  }

  status = HAL_FLASH_Program_IT(FLASH_TYPEPROGRAM_FLASHWORD,  FlashAddress,  (uint32_t)DataAddress);
  if (status == HAL_OK)
  {
    uint32_t actual_flags;
    uint32_t res;
    res = STfs_wait_flags(STFS_FLAG_OP_DONE + STFS_FLAG_OP_ERROR,&actual_flags, 100);
    if (res != TX_SUCCESS)
    {
      stfs_drv_stat.stfs_last_flash_progr_err = STFS_FLASH_PROGR_ERROR2;
      stfs_drv_stat.addr = FlashAddress;
      return STFS_ERROR;
    }
    else
    {
      if (actual_flags & STFS_FLAG_OP_ERROR)
      {
        stfs_drv_stat.stfs_last_flash_progr_err = STFS_FLASH_PROGR_ERROR3;
        stfs_drv_stat.addr = FlashAddress;
        return STFS_ERROR;
      }
    }
  }
  else
  {
    stfs_drv_stat.stfs_last_flash_progr_err = STFS_FLASH_PROGR_ERROR1;
    stfs_drv_stat.addr = FlashAddress;
    return STFS_ERROR;
  }
  return STFS_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param adr
  \param sz
  \param buf

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
int32_t FlashDriver_program_pages(uint32_t  addr, uint32_t data_size, uint8_t *buf, uint8_t *prgwrd_buf, uint8_t *prog_word_cnt)
{
  uint32_t  rem;
  uint32_t  mod;
  uint32_t  curr_addr;
  uint32_t  sz;

  rem =  addr % STFS_FLASH_WORD_SIZE;
  mod =  addr / STFS_FLASH_WORD_SIZE;
  if (rem != 0)
  {
    // Адрес начала не выровнен по границе FLASH_WORD_SIZE
    // Модифицируем слово на которое приходится невыровненная граница
    curr_addr = mod * STFS_FLASH_WORD_SIZE;
    // Определим сколько байт еще не дописано в слове
    sz = STFS_FLASH_WORD_SIZE - rem;

    if (data_size < sz)
    {
      // Если данных недостаточно чтобы дописать слово, то отправляем их в буфер слова и не программируем
      memcpy(&prgwrd_buf[rem],buf, data_size);
      *prog_word_cnt += data_size;
      return STFS_OK;
    }

    memcpy(&prgwrd_buf[rem],buf, sz);

    // Программируем модифицированное слово  во Flash
    if (FlashDriver_progr_word(curr_addr, prgwrd_buf) != STFS_OK)
    {
      return STFS_ERROR;
    }
    data_size      = data_size - sz;
    buf            = buf   + sz;
    curr_addr            = curr_addr   + STFS_FLASH_WORD_SIZE;
  }
  else
  {
    curr_addr = addr;
  }

  *prog_word_cnt = 0;

  while (data_size != 0)
  {
    sz = STFS_FLASH_WORD_SIZE;
    if (data_size < STFS_FLASH_WORD_SIZE)
    {

      memcpy(prgwrd_buf, buf, data_size);
      *prog_word_cnt = data_size;
      return STFS_OK;
    }
    else
    {
      if (FlashDriver_progr_word(curr_addr, buf) != STFS_OK)
      {
        return STFS_ERROR;
      }
    }
    data_size = data_size - sz;
    buf       = buf   + sz;
    curr_addr       = curr_addr   + STFS_FLASH_WORD_SIZE;
  }
  return STFS_OK;
}


/*-----------------------------------------------------------------------------------------------------


  \param start_adr
  \param bufsz
  \param buf

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
int32_t FlashDriver_program_aligned_pages(uint32_t  addr, uint32_t bufsz, uint8_t *buf)
{
  while (bufsz != 0)
  {
    if (FlashDriver_progr_word(addr, buf) != STFS_OK)
    {
      return STFS_ERROR;
    }
    bufsz = bufsz - STFS_FLASH_WORD_SIZE;
    buf   = buf   + STFS_FLASH_WORD_SIZE;
    addr  = addr  + STFS_FLASH_WORD_SIZE;
  }
  return STFS_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param adr
  \param sz
  \param buf

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
int32_t FlashDriver_read_data(uint32_t addr, uint32_t sz, uint8_t *buf)
{
  memcpy(buf, (void *)addr, sz);
  return STFS_OK;
}


#endif //  SFFS_H753_FLASH_DRIVER


#ifdef   STFS_SIMULATOR_FLASH_DRIVER

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t STfs_get_sync_obj(void)
{
  return STFS_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t STfs_put_sync_obj(void)
{
  return STFS_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param sector_num

  \return uint8_t*
-----------------------------------------------------------------------------------------------------*/
uint8_t* STfs_get_sector(uint32_t sector_num)
{
  return (uint8_t *)(Simulator_DataFlash_map[sector_num].start_adr);
}

/*-----------------------------------------------------------------------------------------------------


  \param sector_num

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t STfs_get_sector_size(uint32_t sector_num)
{
  return SIMULATOR_SECTOR_SIZE;
}
/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t STfs_flash_driver_init(void)
{
  uint32_t flash_area_addr =(((uint32_t)sim_flash_area / SEC_ALIGN)+ 1) * SEC_ALIGN;
  uint8_t  *flash_area_ptr =  (uint8_t *)flash_area_addr;

  memset(sim_flash_area, EMPTY_PTTRN, sizeof(sim_flash_area));
  for (uint32_t i=0; i < SIMULATOR_SECTORS_NUM; i++)
  {
    Simulator_DataFlash_map[i].start_adr = (uint32_t)&flash_area_ptr[i * SIMULATOR_SECTOR_SIZE];
    Simulator_DataFlash_map[i].end_adr   = (uint32_t)&flash_area_ptr[(i+1) * SIMULATOR_SECTOR_SIZE] -  sizeof(T_stfs_sector_descriptor);
    sectors_erase_counters[i] = 0;
  }
  return STFS_OK;
}
/*-----------------------------------------------------------------------------------------------------


  \param pdev_map
  \param empty_pttrn
  \param number_of_sectors

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
int32_t  FlashDriver_get_flash_mem_params(void **pdev_map, uint32_t *number_of_sectors)
{
  *pdev_map = (void *)Simulator_DataFlash_Config.sect_map;
  *number_of_sectors = Simulator_DataFlash_Config.flash_sect_num;
  return 0;
}

/*-----------------------------------------------------------------------------------------------------


  \param sector  - номер стираемого сектора

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
int32_t FlashDriver_erase_sector(uint32_t sector)
{

  memset((void *)(Simulator_DataFlash_map[sector].start_adr), EMPTY_PTTRN, SIMULATOR_SECTOR_SIZE);
  sectors_erase_counters[sector]++;

  //APPLOG("Erase sector _______________________ %d.", sector);
  return STFS_OK;
}

/*-----------------------------------------------------------------------------------------------------
  Эмуляция записи слова во Flash размером FLASH_WORD_SIZE
  Следим чтобы:
  - значение adr должно быть выровнено по границе FLASH_WORD_SIZE,
  - размер слова должен точно равняться FLASH_WORD_SIZE
  - область в которую производится запись должна быть заполнена байтами 0xFF

  \param adr
  \param bufsz
  \param buf

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t FlashDriver_progr_word(uint32_t adr, uint32_t bufsz, uint8_t *buf)
{
  if ((adr % STFS_FLASH_WORD_SIZE) != 0)
  {
    return STFS_ERROR;
  }
  if (bufsz  != STFS_FLASH_WORD_SIZE)
  {
    return STFS_ERROR;
  }

  for (uint32_t i=0; i < STFS_FLASH_WORD_SIZE; i++)
  {
    uint8_t dbyte =((uint8_t *)adr)[i];
    if (dbyte != 0xFF)
    {
      return STFS_ERROR;
    }
  }

  memcpy((void *)adr, (const void *)buf, bufsz);
  return STFS_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param adr
  \param sz
  \param buf

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
int32_t FlashDriver_program_pages(uint32_t  addr, uint32_t data_size, uint8_t *buf, uint8_t *prgwrd_buf, uint8_t *prog_word_cnt)
{
  uint32_t rem;
  uint32_t mod;
  uint32_t curr_addr;
  uint32_t sz;

  rem =  addr % STFS_FLASH_WORD_SIZE;
  mod =  addr / STFS_FLASH_WORD_SIZE;
  if (rem != 0)
  {
    // Адрес начала не выровнен по границе FLASH_WORD_SIZE
    // Модифицируем слово на которое приходится невыровненная граница
    curr_addr = mod * STFS_FLASH_WORD_SIZE;
    // Определим сколько байт еще не дописано в слове
    sz = STFS_FLASH_WORD_SIZE - rem;

    if (data_size < sz)
    {
      // Если данных недостаточно чтобы дописать слово, то отправляем их в буфер слова и не программируем
      memcpy(&prgwrd_buf[rem],buf, data_size);
      *prog_word_cnt += data_size;
      goto ok_exit;
    }

    memcpy(&prgwrd_buf[rem],buf, sz);

    // Программируем модифицированное слово обратно во Flash
    if (FlashDriver_progr_word(curr_addr, STFS_FLASH_WORD_SIZE, prgwrd_buf) != STFS_OK)
    {
      goto error_exit;
    }
    data_size = data_size - sz;
    buf   = buf   + sz;
    curr_addr   = curr_addr   + STFS_FLASH_WORD_SIZE;
  }
  else
  {
    curr_addr = addr;
  }

  *prog_word_cnt = 0;

  while (data_size != 0)
  {
    sz = STFS_FLASH_WORD_SIZE;
    if (data_size < STFS_FLASH_WORD_SIZE)
    {

      memcpy(prgwrd_buf, buf, data_size);
      *prog_word_cnt = data_size;
      goto ok_exit;
    }
    else
    {
      if (FlashDriver_progr_word(curr_addr, STFS_FLASH_WORD_SIZE, buf) != STFS_OK)
      {
        goto error_exit;
      }
    }
    data_size = data_size - sz;
    buf       = buf   + sz;
    curr_addr       = curr_addr   + STFS_FLASH_WORD_SIZE;
  }

ok_exit:
  return STFS_OK;
error_exit:
  return STFS_ERROR;
}

/*-----------------------------------------------------------------------------------------------------


  \param start_adr
  \param bufsz
  \param buf

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
int32_t FlashDriver_program_aligned_pages(uint32_t  addr, uint32_t bufsz, uint8_t *buf)
{
  while (bufsz != 0)
  {
    if (FlashDriver_progr_word(addr, STFS_FLASH_WORD_SIZE, buf) != STFS_OK)
    {
      return STFS_ERROR;
    }
    bufsz = bufsz - STFS_FLASH_WORD_SIZE;
    buf   = buf   + STFS_FLASH_WORD_SIZE;
    addr  = addr  + STFS_FLASH_WORD_SIZE;
  }
  return STFS_OK;
}


/*-----------------------------------------------------------------------------------------------------


  \param adr
  \param sz
  \param buf

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
int32_t FlashDriver_read_data(uint32_t addr, uint32_t sz, uint8_t *buf)
{
  memcpy((void *)buf, (const void *)addr, sz);
  return STFS_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param sector
  \param offset
-----------------------------------------------------------------------------------------------------*/
void Mark_Empty_Descriptor(uint32_t sector, uint32_t offset)
{
  //APPLOG("Sector %d. Mark_Empty_Descriptor %d.", sector, offset);
  for (uint32_t i=0; i < sizeof(T_stfs_file_descriptor); i++)
  {
    sim_state_area[sector][i+offset].color = EMPTY_DESCRIPTOR_COLOR;
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param sector
  \param offset
-----------------------------------------------------------------------------------------------------*/
void Mark_Valid_Descriptor(uint32_t sector, uint32_t offset)
{
  //APPLOG("Sector %d. Mark_Valid_Descriptor %d.", sector, offset);
  for (uint32_t i=0; i < sizeof(T_stfs_file_descriptor); i++)
  {
    sim_state_area[sector][i+offset].color = VALID_DESCRIPTOR_COLOR;
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param sector
  \param offset
-----------------------------------------------------------------------------------------------------*/
void Mark_Invalid_Descriptor(uint32_t sector, uint32_t offset)
{
  //APPLOG("Sector %d. Mark_Invalid_Descriptor %d.", sector, offset);
  for (uint32_t i=0; i < sizeof(T_stfs_file_descriptor); i++)
  {
    sim_state_area[sector][i+offset].color = INVALID_DESCRIPTOR_COLOR;
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param sector
  \param start_offset
  \param end_offset
-----------------------------------------------------------------------------------------------------*/
void Mark_Invalid_Data(uint32_t sector, uint32_t start_offset, uint32_t end_offset)
{
  //APPLOG("Sector %d. Mark_Invalid_Data %d - %d", sector, start_offset, end_offset);
  for (uint32_t i=start_offset; i < end_offset; i++)
  {
    sim_state_area[sector][i].color = INVALID_DATA_CHUNK_COLOR;
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param sector
  \param start_offset
  \param end_offset
-----------------------------------------------------------------------------------------------------*/
void Mark_Valid_Data(uint32_t sector, uint32_t start_offset, uint32_t end_offset)
{
  //APPLOG("Sector %d. Mark_Valid_Data %d - %d", sector, start_offset, end_offset);
  for (uint32_t i=start_offset; i < end_offset; i++)
  {
    sim_state_area[sector][i].color = VALID_DATA_CHUNK_COLOR;
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param sector
  \param start_offset
  \param end_offset
-----------------------------------------------------------------------------------------------------*/
void Mark_Empty_Data(uint32_t sector, uint32_t start_offset, uint32_t end_offset)
{
  //APPLOG("Sector %d. Mark_Empty_Data %d - %d", sector, start_offset, end_offset);
  for (uint32_t i=start_offset; i < end_offset; i++)
  {
    sim_state_area[sector][i].color = EMPTY_DATA_CHUNK_COLOR;
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param sector
  \param position

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Get_STfs_mark(uint32_t sector, uint32_t position)
{
  return sim_state_area[sector][position].color;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t STfs_get_sec_num(void)
{
  return SIMULATOR_SECTORS_NUM;
}

/*-----------------------------------------------------------------------------------------------------


  \param sector

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t STfs_get_sector_erase_num(uint32_t sector)
{
  return sectors_erase_counters[sector];
}

#endif // STFS_SIMULATOR_FLASH_DRIVER
