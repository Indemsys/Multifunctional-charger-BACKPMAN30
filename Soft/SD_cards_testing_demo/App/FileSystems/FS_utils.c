// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2019.08.28
// 17:13:22
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"


extern SD_HandleTypeDef   hsd_sdmmc[];
HAL_SD_CardCSDTypeDef     SDcard_CSD;
HAL_SD_CardCIDTypeDef     SDcard_CID;

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint64_t
-----------------------------------------------------------------------------------------------------*/
uint64_t Get_media_total_sectors(void)
{
  return fat_fs_media.fx_media_total_sectors;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Get_media_bytes_per_sector(void)
{
  return fat_fs_media.fx_media_bytes_per_sector;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint64_t
-----------------------------------------------------------------------------------------------------*/
uint64_t Get_media_total_size(void)
{
  return fat_fs_media.fx_media_total_sectors * fat_fs_media.fx_media_bytes_per_sector;
}

/*-----------------------------------------------------------------------------------------------------



  \return uint64_t
-----------------------------------------------------------------------------------------------------*/
uint64_t Get_media_available_size(void)
{
  uint64_t sz;
  fx_media_extended_space_available(&fat_fs_media,&sz);
  return sz;
}

/*-------------------------------------------------------------------------------------------------------------
  Читать строку str не длинее len из файла file оканчивающуюся на CRLF (\r\n)
  Возвращает количество символов в прочитанной строке или -1 в случае ошибки
-------------------------------------------------------------------------------------------------------------*/
int32_t Read_line_from_file(FX_FILE  *fp, char *buf, uint32_t buf_len)
{
  int32_t  indx;
  char     ch;
  ULONG    actual_size;
  uint32_t status;

  buf[0] = 0;
  indx = 0;
  do
  {
    status = fx_file_read(fp,&ch, 1,&actual_size);
    if ((actual_size > 0) && (status == TX_SUCCESS))
    {
      buf[indx++] = ch;
      buf[indx]   = 0;
      if (indx > 1)
      {
        if (strcmp(&buf[indx-2],STR_CRLF) == 0)
        {
          buf[indx-2] = 0;
          return (indx-2);
        }
      }
      if (indx >= buf_len)
      {
        return (-1);
      }
    }
    else
    {
      if (indx == 0) return -1;
      return indx;
    }
  }while (1);
}

/*-----------------------------------------------------------------------------------------------------
  Прочитать строку из файла и получить из нее заданные переменные

  \param fp
  \param fmt_ptr

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Scanf_from_file(FX_FILE  *fp, int32_t *scan_res, char *tmp_buf, uint32_t tmp_buf_sz,  const char  *fmt_ptr, ...)
{
  va_list  ap;
  va_start(ap, fmt_ptr);

  if (Read_line_from_file(fp, tmp_buf, tmp_buf_sz) < 0)
  {
    return RES_ERROR;
  }
  *scan_res = vsscanf(tmp_buf, (char *)fmt_ptr, ap);
  va_end(ap);
  return RES_OK;

}

/*-----------------------------------------------------------------------------------------------------


  \param filename

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Recreate_file_for_write(FX_FILE  *f, CHAR *filename)
{
  uint32_t res;

  res = fx_file_create(&fat_fs_media, filename);
  if (res == FX_SUCCESS)
  {
    res = fx_file_open(&fat_fs_media, f, filename,  FX_OPEN_FOR_WRITE);
  }
  else if (res == FX_ALREADY_CREATED)
  {
    res = fx_file_delete(&fat_fs_media, filename);
    fx_media_flush(&fat_fs_media);
    if (res == FX_SUCCESS)
    {
      res = fx_file_create(&fat_fs_media,filename);
      if (res == FX_SUCCESS)
      {
        res = fx_file_open(&fat_fs_media, f, filename,  FX_OPEN_FOR_WRITE);
      }
    }
  }
  return res;
}


/*-----------------------------------------------------------------------------------------------------



  \return ssp_err_t
-----------------------------------------------------------------------------------------------------*/
uint32_t FS_format(void)
{
  uint32_t res = RES_OK;


//   uint32_t sector_size  = 512;       // Оценочная величина
//   uint32_t sector_count = 7618560;   // Оценочная величина
//
//    Получаем размеры вставленной SD карты
//    ssp_err_t error = Get_SD_card_info(fat_fs_media_cfg.p_config->p_lower_lvl_block_media,&sector_size,&sector_count);
//
//   if ((error != SSP_SUCCESS) || (sector_count <= 0))
//   {
//     return FX_MEDIA_INVALID;
//   }
//
//   sector_count -= 0;
//
//   // Блок памяти не должен быть слишком маленьким
//   if (Get_fs_memory_size() < 512)
//   {
//     return FX_MEDIA_INVALID;
//   }
//
//   res = fx_media_close(&fat_fs_media);
//   if (res != FX_SUCCESS)
//   {
//     return res;
//   }
//
//   res = fx_media_format(&fat_fs_media, // Pointer to FileX media control block.
//        SF_EL_FX_BlockDriver,           // Driver entry
//        &fat_fs_media_cfg,              // Pointer to Block Media Driver
//        fs_memory,                      // Media buffer pointer
//        Get_fs_memory_size(),           // Media buffer size
//        (CHAR *)"Volume 1",             // Volume Name
//        1,                              // Number of FATs
//        256,                            // Directory Entries
//        0,                              // Hidden sectors
//        sector_count,                   // Total sectors - Hidden Sectors
//        sector_size,                    // Sector size
//        64,                             // Sectors per cluster
//        1,                              // Heads
//        1);                             // Sectors per track


  return res;
}



/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t Возвращает RES_OK если все файлы удалены
-----------------------------------------------------------------------------------------------------*/
uint32_t Delete_all_files_in_current_dir(uint32_t   *p_err_cnt)
{
  uint32_t   res;

  CHAR       entry_name[FX_MAX_LONG_NAME_LEN];
  UINT       attributes;
  ULONG      size;
  UINT       year;
  UINT       month;
  UINT       day;
  UINT       hour;
  UINT       minute;
  UINT       second;

  *p_err_cnt = 0;

  // Проходим последовательно по всем файлам
  res = fx_directory_first_full_entry_find(&fat_fs_media,entry_name,&attributes,&size,&year,&month,&day,&hour,&minute,&second);
  while (res == FX_SUCCESS)
  {
    if ((attributes & (FX_DIRECTORY | FX_VOLUME)) == 0)
    {
      res = fx_file_delete(&fat_fs_media, entry_name);
      if (res  != FX_SUCCESS) *p_err_cnt++;
    }
    res = fx_directory_next_full_entry_find(&fat_fs_media,entry_name,&attributes,&size,&year,&month,&day,&hour,&minute,&second);
  }

  fx_media_flush(&fat_fs_media);
  if (*p_err_cnt == 0)
  {
    return RES_OK;
  }
  return RES_ERROR;
}


/*-----------------------------------------------------------------------------------------------------
  Проверяем соответствует ли расширение файла заданному

  \param file_name
  \param ext

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Check_file_extension(char *file_name, const char *const *ext_list)
{
  uint32_t name_len;
  uint32_t ext_pos;
  uint32_t ext_len;
  char const *ext;

  if (ext_list == 0)  return RES_ERROR;
  if (*ext_list == 0) return RES_ERROR;

  name_len = strlen(file_name);

  do
  {
    ext =*ext_list;
    ext_len  = strlen(ext);
    ext_pos = strcspn(file_name, ".");
    if  (name_len > ext_len+1)
    {
      if (ext_pos == (name_len-ext_len-1))
      {
        if (strcmp(&file_name[ext_pos+1], ext) == 0) return RES_OK;
      }
    }
    ext_list++;

  }while  (*ext_list != 0);

  return RES_ERROR;

}


/*-----------------------------------------------------------------------------------------------------
  Получаем строку расширения из имени файла
  Если расширения нет или оно длинее предоставленного буфера - нулевой смвол, то возвращаем пустую строку

  \param file_name
  \param ext
  \param ext_sz

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
void Get_file_extension(char *file_name, char *ext, uint32_t ext_buf_sz)
{
  uint32_t  ext_pos;
  uint32_t  ext_len;
  uint32_t  name_len;

  name_len = strlen(file_name);
  ext_pos  = strcspn(file_name, ".");  //

  if (name_len == ext_pos)
  {
    *ext = 0;
    return;
  }
  ext_pos++;
  ext_len = name_len - ext_pos;
  if (ext_len > (ext_buf_sz-1))
  {
    *ext = 0;
    return;
  }

  memcpy(ext,&file_name[ext_pos], ext_len);
  return;
}


/*-----------------------------------------------------------------------------------------------------
  Получить количество файлов в заданной директории c заданными расширениями и с фильтрацией по постфиксу с символом #


  \param dir_name          - название директории в которой будет осуществлен поиск
  \param valid_extensions  - список допустимый расширений
  \param filter_only_new   - если 1 то считать файлы бед потфикса c символом #

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Get_records_files_count(const char *dir_name, const char *const *valid_extensions, uint8_t filter_only_new)
{
  uint32_t        res = 0;
  uint32_t        cnt = 0;
  FX_LOCAL_PATH  *local_path_ptr = NULL;
  CHAR           *entry_name     = NULL;

  UINT            attributes;
  ULONG           size;
  UINT            year;
  UINT            month;
  UINT            day;
  UINT            hour;
  UINT            minute;
  UINT            second;



  local_path_ptr = App_malloc_pending(sizeof(FX_LOCAL_PATH), 10);
  if  (local_path_ptr == NULL) return 0;

  entry_name = App_malloc_pending(FX_MAX_LONG_NAME_LEN, 10);
  if  (entry_name == NULL)
  {
    App_free(local_path_ptr);
    return 0;
  }

  // Установим путь по умолчанию к файлам
  fx_directory_local_path_set(&fat_fs_media, local_path_ptr, (CHAR *)dir_name);

  // Проходим последовательно по всем файлам
  res = fx_directory_first_full_entry_find(&fat_fs_media,entry_name,&attributes,&size,&year,&month,&day,&hour,&minute,&second);

  while (res == FX_SUCCESS)
  {
    if ((attributes & (FX_DIRECTORY | FX_VOLUME)) == 0)
    {
      if (Check_file_extension(entry_name, valid_extensions) == RES_OK)
      {
        if ((!filter_only_new) || (filter_only_new && (strcspn(entry_name, "#") == strlen(entry_name))))
        {
          cnt++;
        }
      }
    }
    res = fx_directory_next_full_entry_find(&fat_fs_media,entry_name,&attributes,&size,&year,&month,&day,&hour,&minute,&second);
  }

  fx_directory_local_path_clear(&fat_fs_media);

  App_free(local_path_ptr);
  App_free(entry_name);
  return cnt;
}

/*-----------------------------------------------------------------------------------------------------
  Создаем файл с именем list_file_name со списком имен файлов из заданной директории dir_name c заданными расширениями
  Если filter_only_new = 1 , то игнорируем файлы с постфиксом начинающимся с символа #

  \param dir_name
  \param list_file_name
  \param filter_only_new
  \param files_cnt

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Create_files_list_from_dir(const char *dir_name, const char *list_file_name, const char *const *valid_extensions, uint8_t filter_only_new, uint32_t *files_cnt)
{
  uint32_t        res = 0;
  FX_FILE        *list_f         = NULL;
  FX_LOCAL_PATH  *local_path_ptr = NULL;
  CHAR           *entry_name     = NULL;

  UINT            attributes;
  ULONG           size;
  UINT            year;
  UINT            month;
  UINT            day;
  UINT            hour;
  UINT            minute;
  UINT            second;
  uint32_t        cnt = 0;

  local_path_ptr = App_malloc_pending(sizeof(FX_LOCAL_PATH), 10);
  if  (local_path_ptr == NULL) goto EXIT_ON_ERROR;

  entry_name = App_malloc_pending(FX_MAX_LONG_NAME_LEN, 10);
  if  (entry_name == NULL) goto EXIT_ON_ERROR;

  list_f = App_malloc_pending(sizeof(FX_FILE), 10);
  if (list_f == NULL) goto EXIT_ON_ERROR;

  // Открываем файл со списком имен на запись
  res = Recreate_file_for_write(list_f, (char *)list_file_name);
  if (res != FX_SUCCESS)
  {
    goto EXIT_ON_ERROR;
  }

  fx_file_write(list_f, "\\", 1); // Запись признака имени директории
  fx_file_write(list_f, (void *)dir_name, strlen(dir_name));
  fx_file_write(list_f,  STR_CRLF, strlen(STR_CRLF));


  // Установим путь по умолчанию к файлам
  fx_directory_local_path_set(&fat_fs_media, local_path_ptr, (CHAR *)dir_name);

  // Проходим последовательно по всем файлам
  res = fx_directory_first_full_entry_find(&fat_fs_media,entry_name,&attributes,&size,&year,&month,&day,&hour,&minute,&second);

  while (res == FX_SUCCESS)
  {
    if ((attributes & (FX_DIRECTORY | FX_VOLUME)) == 0)
    {
      if (Check_file_extension(entry_name, valid_extensions) == RES_OK)
      {
        if ((!filter_only_new) || (filter_only_new && (strcspn(entry_name, "#") == strlen(entry_name))))
        {
          if (fx_file_write(list_f, entry_name, strlen(entry_name)) != FX_SUCCESS) goto EXIT_ON_ERROR;
          if (fx_file_write(list_f, STR_CRLF, strlen(STR_CRLF)) != FX_SUCCESS) goto EXIT_ON_ERROR;
          cnt++;
        }
      }
    }
    res = fx_directory_next_full_entry_find(&fat_fs_media,entry_name,&attributes,&size,&year,&month,&day,&hour,&minute,&second);
  }

  fx_directory_local_path_clear(&fat_fs_media);
  if (list_f != NULL)
  {
    if (list_f->fx_file_id == FX_FILE_ID)
    {
      fx_file_close(list_f);
    }
    App_free(list_f);
  }
  App_free(entry_name);
  App_free(local_path_ptr);
  fx_media_flush(&fat_fs_media);
  if (files_cnt != NULL) *files_cnt = cnt;
  APPLOG("File %s created. Records count = %d", list_file_name, cnt);
  return RES_OK;

EXIT_ON_ERROR:
  APPLOG("Failed to create file %s. Errors %d", list_file_name,  res);


  if (list_f != NULL)
  {
    if (list_f->fx_file_id == FX_FILE_ID)
    {
      fx_file_close(list_f);
    }
    App_free(list_f);
  }

  App_free(entry_name);
  App_free(local_path_ptr);
  fx_media_flush(&fat_fs_media);
  if (files_cnt != NULL) *files_cnt = 0;
  return RES_ERROR;

}


/*-----------------------------------------------------------------------------------------------------
  Получить количество файлов и объем занимаемой ими памяти в заданной директории с именем начинающимся с заданного префикса

  \param path             - путь к директории
  \param name_prefix      - префикс файла, если передается 0 то префикс не учитываетя
  \param files_count      - указатель на количество файлов, если указатель 0, то количество не передается
  \param files_size       - указатель на общий размер файлов, если указатель 0, то общий размер не передается.

  \return uint32_t        - возвращаете RES_OK если функция выполнена успешно
-----------------------------------------------------------------------------------------------------*/
uint32_t Get_files_info_in_directory(char *path, char *name_prefix, uint32_t *files_count, uint64_t *files_size, uint32_t *file_max_num)
{
  uint32_t   res;

  CHAR       *entry_name;
  UINT       attributes;
  ULONG      size;
  UINT       year;
  UINT       month;
  UINT       day;
  UINT       hour;
  UINT       minute;
  UINT       second;
  FX_LOCAL_PATH  *local_path_ptr;
  uint32_t   cnt = 0;
  uint32_t   sz  = 0;
  uint32_t   max_file_number = 0;



  local_path_ptr = App_malloc_pending(sizeof(FX_LOCAL_PATH), 10);
  if  (local_path_ptr == NULL) return RES_ERROR;

  entry_name = App_malloc_pending(FX_MAX_LONG_NAME_LEN, 10);
  if  (entry_name == NULL)
  {
    App_free(local_path_ptr);
    return RES_ERROR;
  }

  // Установим путь по умолчанию к файлам
  fx_directory_local_path_set(&fat_fs_media, local_path_ptr, (CHAR *)path);

  // Проходим последовательно по всем файлам
  res = fx_directory_first_full_entry_find(&fat_fs_media,entry_name,&attributes,&size,&year,&month,&day,&hour,&minute,&second);
  while (res == FX_SUCCESS)
  {
    if ((attributes & (FX_DIRECTORY | FX_VOLUME)) == 0)
    {
      if (name_prefix != 0)
      {
        if (strstr(entry_name, name_prefix) == entry_name)
        {
          cnt++;
          sz= sz + size;
          if (file_max_num != 0)
          {
            // Выделяем номер и обновляем максимальное значение
            uint32_t file_number = Extract_number_from_string(&entry_name[strlen(name_prefix)]);
            if (file_number > max_file_number) max_file_number = file_number;
          }
        }
      }
      else
      {
        cnt++;
        sz= sz + size;
      }
    }
    res = fx_directory_next_full_entry_find(&fat_fs_media,entry_name,&attributes,&size,&year,&month,&day,&hour,&minute,&second);
  }

  if (files_count != 0) *files_count = cnt;
  if (files_size != 0) *files_size = sz;
  if (file_max_num != 0) *file_max_num = max_file_number;

  fx_directory_local_path_clear(&fat_fs_media);
  App_free(local_path_ptr);
  App_free(entry_name);
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------
  Удалить файл с минимальным номером

  \param path             - путь к директории
  \param name_prefix      - префикс файла, если передается 0 то префикс не учитываетя
  \param file_size        - указатель на размер удаленного файла
  \param file_num         - указатель на номер удаленного файла

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Delete_files_with_min_number(char *path, char *name_prefix, uint32_t *file_size, uint32_t *file_num)
{
  uint32_t   res;

  CHAR       *entry_name;
  CHAR       *last_entry_name;
  UINT       attributes;
  ULONG      size;
  UINT       year;
  UINT       month;
  UINT       day;
  UINT       hour;
  UINT       minute;
  UINT       second;
  FX_LOCAL_PATH  *local_path_ptr;
  uint32_t   last_sz  = 0;
  uint32_t   min_file_number = 0xFFFFFFFF;



  local_path_ptr = App_malloc_pending(sizeof(FX_LOCAL_PATH), 10);
  if  (local_path_ptr == NULL) return RES_ERROR;

  entry_name = App_malloc_pending(FX_MAX_LONG_NAME_LEN, 10);
  if  (entry_name == NULL)
  {
    App_free(local_path_ptr);
    return RES_ERROR;
  }

  last_entry_name = App_malloc_pending(FX_MAX_LONG_NAME_LEN, 10);
  if  (last_entry_name == NULL)
  {
    App_free(local_path_ptr);
    App_free(entry_name);
    return RES_ERROR;
  }

  // Установим путь по умолчанию к файлам
  fx_directory_local_path_set(&fat_fs_media, local_path_ptr, (CHAR *)path);

  // Проходим последовательно по всем файлам
  res = fx_directory_first_full_entry_find(&fat_fs_media,entry_name,&attributes,&size,&year,&month,&day,&hour,&minute,&second);
  while (res == FX_SUCCESS)
  {
    if ((attributes & (FX_DIRECTORY | FX_VOLUME)) == 0)
    {
      if (name_prefix != 0)
      {
        if (strstr(entry_name, name_prefix) == entry_name)
        {
          // Выделяем номер и обновляем максимальное значение
          uint32_t file_number = Extract_number_from_string(&entry_name[strlen(name_prefix)]);
          if (file_number < min_file_number)
          {
            strcpy(last_entry_name, entry_name);
            last_sz = size;
            min_file_number = file_number;
          }
        }
      }
    }
    res = fx_directory_next_full_entry_find(&fat_fs_media,entry_name,&attributes,&size,&year,&month,&day,&hour,&minute,&second);
  }

  res = RES_ERROR;
  if (min_file_number != 0xFFFFFFFF)
  {
    res = fx_file_delete(&fat_fs_media, last_entry_name);
    if (res == FX_SUCCESS)
    {
      if (file_size != 0) *file_size = last_sz;
      if (file_num != 0) *file_num = min_file_number;
      res = RES_OK;
    }
  }


  fx_directory_local_path_clear(&fat_fs_media);
  App_free(local_path_ptr);
  App_free(entry_name);
  App_free(last_entry_name);
  return res;
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Log_SD_card_FS_init_state(void)
{
  uint32_t status = Get_FS_init_res()->fx_media_open_result;
  if (status != FX_SUCCESS)
  {
    APPLOG("SD card. FS media error %d", status);
  }
  else
  {
    HAL_SD_GetCardCSD(&hsd_sdmmc[0], &SDcard_CSD);
    HAL_SD_GetCardCID(&hsd_sdmmc[0], &SDcard_CID);

    switch (hsd_sdmmc[0].SdCard.CardSpeed)
    {
    case CARD_ULTRA_HIGH_SPEED:
      APPLOG("Inserted UHS-I SD Card <50MB/s for SDR50, DDR5 Cards and <104MB/s for SDR104, Spec version 3.01");
      break;
    case CARD_HIGH_SPEED:
      APPLOG("Inserted High Speed Card <25MB/s , Spec version 2.00");
      break;
    case CARD_NORMAL_SPEED:
      APPLOG("Inserted Normal Speed Card <12.5MB/s , Spec Version 1.01");
      break;
    }

    switch (hsd_sdmmc[0].SdCard.CardType)
    {
    case CARD_SDSC:
      APPLOG("SD Standard Capacity <2GB ");
      break;
    case CARD_SDHC_SDXC:
      APPLOG("SD High Capacity <32GB, SD Extended Capacity <2TB");
      break;
    case CARD_SECURED:
      APPLOG("SD Secured");
      break;
    }
    APPLOG("SD card capacity %lld", (uint64_t)hsd_sdmmc[0].SdCard.BlockNbr * (uint64_t)hsd_sdmmc[0].SdCard.LogBlockSize);

    APPLOG("SD card. FS media opened successfully.");
    APPLOG("SD card. FS media total size       = %lld.", Get_media_total_size());
    APPLOG("SD card. Deleting win dir res      = %04X.", Get_FS_init_res()->deleting_win_dir_res);
    APPLOG("SD card. Deleting errors cnt       = %04X.", Get_FS_init_res()->deleting_errors_cnt);
    APPLOG("SD card. Creating misc dir res     = %04X.", Get_FS_init_res()->creation_misc_dir_res );
    APPLOG("SD card. Creating records dir res  = %04X.", Get_FS_init_res()->creation_records_dir_res);
    APPLOG("SD card. Creating log dir res      = %04X.", Get_FS_init_res()->creation_log_dir_res);
  }
}

