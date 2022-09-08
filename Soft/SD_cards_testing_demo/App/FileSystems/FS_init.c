// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2019.08.29
// 18:42:07
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"

extern VOID  STM32_SD_driver(FX_MEDIA *media_ptr);


FX_MEDIA                           fat_fs_media;//Размер структуры 10032.

// Буфер для кэша FileX. В этот буфер считываются данные драйверм SD карты
// Буфер должен быть выровнен по границе 4 байта чтобы коректно работал DMA механизм драйвера SD карты
// И буфер должен быть выровнен по границе 32 байта чтобы коректно работала инвалидация кэша процессора если включено кэширование данных
ALIGN_32BYTES              (uint8_t fs_memory[FS_MEMORY_SZ]);
static T_file_system_init_results   fs_res;
static uint8_t                      fs_layer_initialized= 0;


#define ERROR_CORR_BUF_SZ           0x20000   // Размер буфера для функции коррекции ошибок
/*-----------------------------------------------------------------------------------------------------


  \param void

  \return T_file_system_init_results*
-----------------------------------------------------------------------------------------------------*/
T_file_system_init_results* Get_FS_init_res(void)
{
  return &fs_res;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Get_fs_memory_size(void)
{
  return FS_MEMORY_SZ;
}

/*-----------------------------------------------------------------------------------------------------


  \param dir_name

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Create_subdirectory(const char *dir_name)
{
  uint32_t res;
  // Создаем служебную директорию если ее еще нет.
  if (fx_directory_name_test(&fat_fs_media, (CHAR *)dir_name) != FX_SUCCESS)
  {
    // Создаем служебную директорию
    res = fx_directory_create(&fat_fs_media, (CHAR *)dir_name);
    if  (res != FX_SUCCESS) return RES_ERROR;
  }
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Init_exFAT(void)
{
  rtc_time_t   rt_time = {0};
  uint32_t     err_cnt = 0;
  uint32_t     status;

  //UCHAR       *buf;
  CHAR        *path;

  if (fs_layer_initialized == 0)
  {
    fx_system_initialize();
    fs_layer_initialized = 1;
  }


  rt_time.tm_mon++; // Renesas SSP счет месяцев начинается с 0

  fx_system_date_set(rt_time.tm_year+1900 , rt_time.tm_mon , rt_time.tm_mday);
  fx_system_time_set(rt_time.tm_hour, rt_time.tm_min, rt_time.tm_sec);


  fs_res.fx_media_open_result = fx_media_open(&fat_fs_media, "C:", STM32_SD_driver, 0, fs_memory, sizeof(fs_memory));
  if (fs_res.fx_media_open_result != FX_SUCCESS)
  {
    return RES_ERROR;
  }


//  Файловую систему не проверяем поскольку она уже должны быть проверена бутлодером
//  buf = App_malloc_pending(ERROR_CORR_BUF_SZ, 10);
//  if  (buf != NULL) // Выделеям 64 Кб памяти для функции исправления ошибок FAT
//  {
//    fs_res.fs_check_error_code = fx_media_check(&fat_fs_media,buf, ERROR_CORR_BUF_SZ, FX_FAT_CHAIN_ERROR | FX_DIRECTORY_ERROR | FX_LOST_CLUSTER_ERROR,(ULONG *)&(fs_res.fs_detected_errors));
//    App_free(buf);
//    if (fs_res.fs_check_error_code != FX_SUCCESS) return RES_ERROR;
//  }
//  else
//  {
//    fs_res.fx_corr_buff_alloc_res = RES_ERROR;
//  }

  // Получаем строку пути по умолчанию
  fx_directory_default_get(&fat_fs_media,&path);



  // Удаляем служебную директорию Windows
  if (fx_directory_name_test(&fat_fs_media, WINDOWS_DIR) == FX_SUCCESS)
  {
    fx_directory_default_set(&fat_fs_media, WINDOWS_DIR);
    if (Delete_all_files_in_current_dir(&err_cnt) == RES_OK)
    {
      fx_directory_default_set(&fat_fs_media, "/"); // Возвращаемся на уровень ROOT
      status = fx_directory_delete(&fat_fs_media, WINDOWS_DIR);
    }
    else
    {
      status = FX_IO_ERROR;
    }
  }
  else
  {
    // Директори Windows ненайдено
    status = FX_NOT_FOUND;
  }

  fs_res.deleting_errors_cnt      = err_cnt;
  fs_res.deleting_win_dir_res     = status;
  fs_res.creation_misc_dir_res    = Create_subdirectory(MISC_DIR_NAME);
  fs_res.creation_records_dir_res = Create_subdirectory(RECORDS_DIR_NAME);
  fs_res.creation_log_dir_res     = Create_subdirectory(LOG_DIR_NAME);
  fx_media_flush(&fat_fs_media);

  return RES_OK;
}


