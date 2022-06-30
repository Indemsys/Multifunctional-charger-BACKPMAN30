// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2018.09.03
// 23:06:41
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"



uint8_t    nv_store_ok;

/*-------------------------------------------------------------------------------------------
  Восстановление параметров по умолчанию, после сбоев системы или смены версии
---------------------------------------------------------------------------------------------*/
void Return_def_params(void)
{
  uint16_t  i;

  // Загрузить параметры значениями по умолчанию
  for (i = 0; i < DWVAR_SIZE; i++)
  {
    if ((dwvar[i].attr & VAL_NOINIT) == 0)
    {
      switch (dwvar[i].vartype)
      {
        // tint8u, tint16u, tuint32_t, tfloat, tarrofdouble, tarrofbyte
      case tint8u:
        *(uint8_t *)dwvar[i].val = (uint8_t)dwvar[i].defval;
        break;
      case tint16u:
        *(uint16_t *)dwvar[i].val = (uint16_t)dwvar[i].defval;
        break;
      case tint32u:
        *(uint32_t *)dwvar[i].val = (uint32_t)dwvar[i].defval;
        break;
      case tint32s:
        *(int32_t *)dwvar[i].val = (int32_t)dwvar[i].defval;
        break;
      case tfloat:
        *(float *)dwvar[i].val = (float)dwvar[i].defval;
        break;
      case tstring:
        {
          uint8_t *st;

          strncpy((char *)dwvar[i].val, (const char *)dwvar[i].pdefval, dwvar[i].varlen - 1);
          st = (uint8_t *)dwvar[i].val;
          st[dwvar[i].varlen - 1] = 0;
        }
        break;
      case tarrofbyte:
        memcpy(dwvar[i].val, dwvar[i].pdefval, dwvar[i].varlen);
        break;
      case tarrofdouble:
        break;
      }
    }
  }
}


/*-------------------------------------------------------------------------------------------
  Загрузка параметров из файла, после старта системы
---------------------------------------------------------------------------------------------*/
int32_t Restore_NV_parameters(void)
{
  int32_t status;

  nv_store_ok = true;

  Return_def_params(); // Сначала запишем в параметры значения по умолчанию
  status = Restore_from_STfs();
  if (status != RES_OK)
  {
    nv_store_ok = false;
    Return_def_params();
    Save_Params_to_STfs();
    LOGs(__FUNCTION__, __LINE__, SEVERITY_DEFAULT,"Parameters restoring error. Returned to defaults");
  }
  else
  {
    LOGs(__FUNCTION__, __LINE__, SEVERITY_DEFAULT,"Parameters restored successfully");
  }
  snprintf((char *)wvar.ver, sizeof(wvar.ver), "%s %s", __DATE__, __TIME__);
  LOGs(__FUNCTION__, __LINE__, SEVERITY_DEFAULT,"Compilation date time: %s %s", __DATE__, __TIME__);
  return (RES_OK);
}

/*-------------------------------------------------------------------------------------------

---------------------------------------------------------------------------------------------*/
int32_t Restore_from_STfs(void)
{
  T_file_info  finfo;
  int32_t      file;
  uint8_t      compressed = 0;
  uint32_t     data_buf_sz;
  uint8_t     *data_buf = NULL;
  uint8_t     *decompessed_data_ptr = NULL;
  uint32_t     decompessed_data_sz;
  int32_t      actual_read;
  uint32_t     res;

  // Искать файл с названием STFS_PARAMS_FILE_NAME

  if (STfs_find(INT_FLASH_BANK2,STFS_PARAMS_FILE_NAME,&finfo) == STFS_OK)
  {
    // Файл найден настроек
    if (STfs_open(INT_FLASH_BANK2,STFS_PARAMS_FILE_NAME, STFS_OPEN_READ,&file) != STFS_OK)
    {
      return RES_ERROR;
    }
  }
  else
  {

    if (STfs_find(INT_FLASH_BANK2,STFS_COMPRESSED_PARAMS_FILE_NAME,&finfo) == STFS_OK)
    {
      // Найден сжатый файл настроек
      compressed = 1;
      if (STfs_open(INT_FLASH_BANK2,STFS_COMPRESSED_PARAMS_FILE_NAME, STFS_OPEN_READ,&file) != STFS_OK)
      {
        return RES_ERROR;
      }
    }
    else return RES_ERROR;
  }

  data_buf_sz = finfo.size;
  data_buf = App_malloc_pending(data_buf_sz + 1,10);
  if (data_buf == NULL) goto EXIT_ON_ERROR;

  if (STfs_read(file, data_buf, data_buf_sz,&actual_read) != STFS_OK) goto EXIT_ON_ERROR;

  if (compressed)
  {
    // Проверка контрольной суммы
    uint16_t crc = Get_CRC16_of_block(data_buf,data_buf_sz-2, 0xFFFF);
    uint16_t ecrc = data_buf[data_buf_sz-2] +(data_buf[data_buf_sz-1]<<8);
    if (crc != ecrc) goto EXIT_ON_ERROR; // Выход если не совпала контрольная сумма

    decompessed_data_sz = data_buf[0] +(data_buf[1]<<8)+(data_buf[2]<<16)+(data_buf[3]<<24);
    if (decompessed_data_sz > MAX_SIZE_OF_PARAMS_FILE) goto EXIT_ON_ERROR; // Выход если после декомпрессии объем данных слишком большой
    decompessed_data_ptr = App_malloc_pending(decompessed_data_sz+1,10);
    if (decompessed_data_ptr == NULL) goto EXIT_ON_ERROR;
    // Декомпрессия
    if (Decompress_mem_to_mem(SIXPACK_ALG, data_buf, data_buf_sz-2, decompessed_data_ptr, decompessed_data_sz) != decompessed_data_sz) goto EXIT_ON_ERROR;

    App_free(data_buf);
    data_buf = 0;
    data_buf = decompessed_data_ptr;
    data_buf_sz = decompessed_data_sz;
    decompessed_data_ptr = 0;
  }

  data_buf[data_buf_sz] = 0; // Дополняем строку завершающим нулем

  // Парсим JSON формат данных
  res = JSON_Deser_settings((char *)data_buf);


  STfs_close(file);
  App_free(decompessed_data_ptr);
  App_free(data_buf);
  return res;

EXIT_ON_ERROR:
  STfs_close(file);
  App_free(decompessed_data_ptr);
  App_free(data_buf);
  return RES_ERROR;

}

/*-------------------------------------------------------------------------------------------
   Процедура сохранения в ini-файл параметров
---------------------------------------------------------------------------------------------*/
int32_t Save_Params_to_STfs(void)
{
  uint32_t   flags;
  char      *json_str = NULL;
  uint32_t   json_str_sz;
  uint8_t   *compessed_data_ptr = 0;
  uint32_t   compessed_data_sz;
  uint8_t   *buf;
  uint32_t   buf_sz;
  uint8_t    compressed = 1;
  char      *fname;
  int32_t    file;
  uint32_t   res;

  flags = JSON_INDENT(1) | JSON_ENSURE_ASCII;

  if (Serialze_settings_to_mem(&json_str,&json_str_sz, flags) != RES_OK) goto EXIT_ON_ERROR;

  if (compressed)
  {
    // Выделить память для сжатого файла
    compessed_data_ptr = App_malloc_pending(json_str_sz,10);
    if (compessed_data_ptr == NULL) goto EXIT_ON_ERROR;

    compessed_data_sz = json_str_sz;
    res = Compress_mem_to_mem(SIXPACK_ALG, json_str, json_str_sz,compessed_data_ptr,&compessed_data_sz);
    if (res != RES_OK) goto EXIT_ON_ERROR;
    // Добавляем контрольную сумму
    uint16_t crc = Get_CRC16_of_block(compessed_data_ptr,compessed_data_sz, 0xFFFF);
    buf           = compessed_data_ptr;
    buf_sz        = compessed_data_sz;
    buf[buf_sz]   = crc & 0xFF;
    buf[buf_sz+1] =(crc >> 8) & 0xFF;
    buf_sz += 2;
  }
  else
  {
    buf = (uint8_t *)json_str;
    buf_sz = json_str_sz;
  }

  //Save_settings_buf_to_file(file_name,buf, buf_sz);

  if (compressed)
  {
    fname = STFS_COMPRESSED_PARAMS_FILE_NAME;
  }
  else
  {
    fname = STFS_PARAMS_FILE_NAME;
  }

  uint32_t free_space;
  // Проверим есть ли место на диске для файла
  if (STfs_free_space(INT_FLASH_BANK2, &free_space)!= STFS_OK) goto EXIT_ON_ERROR;

  if (free_space <= STfs_estimate_file_phis_space(buf_sz))
  {
    // Пытаемся дефрагментировать систему
    if (STfs_defrag(INT_FLASH_BANK2) != STFS_OK)                 goto EXIT_ON_ERROR;
    if (STfs_free_space(INT_FLASH_BANK2, &free_space)!= STFS_OK) goto EXIT_ON_ERROR;
    // Если после дефрагментации все равно нет места, то выходим с ошибкой
    if (free_space <= STfs_estimate_file_phis_space(buf_sz))     goto EXIT_ON_ERROR;
  }

  if (STfs_open(INT_FLASH_BANK2,fname, STFS_OPEN_WRITE,&file) != STFS_OK) goto EXIT_ON_ERROR;

  if (STfs_write(file, buf, buf_sz)!= STFS_OK)
  {
    STfs_close(file);
    goto EXIT_ON_ERROR;
  }

  STfs_close(file);
  App_free(compessed_data_ptr);
  if (json_str != 0) App_free(json_str);
  return RES_OK;
EXIT_ON_ERROR:
  App_free(compessed_data_ptr);
  if (json_str != 0) App_free(json_str);
  return RES_ERROR;
}

