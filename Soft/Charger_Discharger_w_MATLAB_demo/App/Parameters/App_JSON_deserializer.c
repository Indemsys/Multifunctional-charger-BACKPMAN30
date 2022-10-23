// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2019.09.26
// 23:11:19
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"


static  uint32_t _JSON_Deser_params(json_t  *root);
static  uint32_t _JSON_Deser_table(json_t  *root,  uint32_t table_id);
static  uint32_t _JSON_Deser_tables(json_t *root);
static  uint32_t _JSON_find_array(json_t  *root, json_t  **object, char const *key_name);



/*-----------------------------------------------------------------------------------------------------


  \param root

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t _JSON_Deser_tables(json_t *root)
{
  uint32_t res;
  uint32_t err = 0;
  res = _JSON_Deser_params(root);
  if (res != RES_OK) err++;
  res = _JSON_Deser_table(root,  TYPE_MISC_TABLE);
  if (res != RES_OK) err++;

  if (err > 0)
  {
    APPLOG("Deserialization fail count = %d.", err);
    return RES_ERROR;
  }
  else
  {
    APPLOG("Deserialization done sucessfully.");
    return RES_OK;
  }

}

/*-----------------------------------------------------------------------------------------------------


  \param root
  \param params

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t _JSON_find_array(json_t  *root, json_t  **object, char const *key_name)
{
  json_t       *item;
  uint32_t      err = 1;
  uint32_t      n   = 0;
  uint32_t      i;

  err = 1;
  n   = json_array_size(root);
  // Проходим по всем элементам макссива root
  for (i = 0; i < n; i++)
  {
    item = json_array_get(root, i); // Получаем элемет массива
    *object = json_object_get(item, key_name); // Ищем в элементе массива объект с заданным именем
    // json_decref(item); Не декременитируем ссылку поскольку она  не инкрементировалась после вызова json_array_get
    if (json_is_array(*object))
    {
      err = 0;
      break;
    }
  }
  if (err == 1)
  {
    APPLOG("Error. Key %s", key_name);
    // json_decref(*object); Не декременитируем ссылку поскольку она  не инкрементировалась после вызова json_object_get
    return RES_ERROR;
  }
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param root

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static  uint32_t _JSON_Deser_params(json_t  *root)
{
  json_t       *params = 0;
  json_t       *item   = 0;
  int32_t       indx;
  char         *var_name;
  char         *val;
  uint32_t      i;
  uint32_t      err;
  uint32_t      hits_cnt = 0;

  err = _JSON_find_array(root,&params, MAIN_PARAMETERS_KEY);
  if (err != RES_OK) return RES_ERROR;


  for (i = 0; i < json_array_size(params); i++)
  {
    item = json_array_get(params, i);
    if (json_is_array(item))
    {
      if (json_unpack(item, "[s,s]" ,&var_name,&val) == 0)
      {
        indx = Find_param_by_name(var_name);
        if (indx >= 0)
        {
          Str_to_param((uint8_t *)val , indx);
          hits_cnt++;
        }
      }
      else
      {
        APPLOG("Error.");
      }
    }
    // json_decref(item); Не декременитируем ссылку поскольку она  не инкрементировалась
  }
  if (hits_cnt > 0)
  {
    APPLOG("Updated %d params.",hits_cnt);
  }
  // json_decref(params); Не декременитируем ссылку поскольку она  не инкрементировалась
  return RES_OK;
}


/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t _JSON_Unpac_value(void *rec_ptr, T_table_fields_props const *flds_props, json_t *field)
{
  uint32_t  v;
  double    df;
  float     f;
  char      *src_str;
  char      *dst_str;

  switch (flds_props->fld_type)
  {
  case tint8u:
    json_unpack(field, "i",&v);
    memcpy((void *)((uint32_t)rec_ptr + flds_props->fld_offset),&v,1);
    break;
  case tint16u:
    json_unpack(field, "i",&v);
    memcpy((void *)((uint32_t)rec_ptr + flds_props->fld_offset),&v,2);
    break;
  case tint32u:
    json_unpack(field, "i",&v);
    memcpy((void *)((uint32_t)rec_ptr + flds_props->fld_offset),&v,4);
    break;
  case tfloat:
    json_unpack(field, "f",&df);
    f = (float)df;
    memcpy((void *)((uint32_t)rec_ptr + flds_props->fld_offset),&f,4);
    break;
  case tstring:
    dst_str = (char *)((uint32_t)rec_ptr + flds_props->fld_offset);
    json_unpack(field, "s",&src_str);
    strcpy(dst_str, src_str);
    break;
  case tarrofdouble:
  case tarrofbyte:
  case tint32s:
    break;
  }

  return 0;
}

/*-----------------------------------------------------------------------------------------------------
  Десериализация дополнительной таблицы параметров
  Тип таблицы задается параметром  tbl_type


  \param root

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static  uint32_t _JSON_Deser_table(json_t  *root,  uint32_t table_id)
{
  json_t       *recs_array;
  json_t       *record;
  json_t       *field;

  uint32_t     rec_num;
  uint8_t      fields_num;
  uint32_t     tbl_fields_num;
  uint32_t     arr_sz;
  uint32_t     i,j;
  uint32_t     err;
  T_table_fields_props const *flds_props;


  err = _JSON_find_array(root,&recs_array, Get_table_JSON_key(table_id));
  if (err != RES_OK)
  {
    APPLOG("Error.");
    return RES_ERROR;
  }


  rec_num = Get_table_records_count(table_id);                   // Получаем количество записей в таблице
  arr_sz  = json_array_size(recs_array);
  if (arr_sz < rec_num) rec_num = arr_sz;                        // Если в JSON записей меньше, то берем меньшее количество

  flds_props = Get_table_fields_props(table_id,&tbl_fields_num); // Получаем указатель на массив с перечислением свойств каждого поля таблицы

  // Проходим по записям таблицы
  for (i = 0; i < rec_num; i++)
  {
    record = json_array_get(recs_array, i);
    if (json_is_array(record))
    {
      fields_num = json_array_size(record);                         // Получаем количество объектов в массиве JSON содержащем значения полей записи таблицы
      if (fields_num > tbl_fields_num) fields_num = tbl_fields_num; // Если в JSON объектов менбше чем полей в таблице то выбираем меньшее число

      // Проходим по полям таблицы
      for (j = 0; j < fields_num; j++)
      {
        field = json_array_get(record, j);
        _JSON_Unpac_value(Get_table_records_ptr(table_id, i),&flds_props[j],field);
        // json_decref(field); Не декременитируем ссылку поскольку она  не инкрементировалась
      }
    }
    else
    {
      APPLOG("Error.");
    }
    // json_decref(record); Не декременитируем ссылку поскольку она  не инкрементировалась
  }

  // json_decref(recs_array); Не декременитируем ссылку поскольку она  не инкрементировалась
  return RES_OK;
}


/*-----------------------------------------------------------------------------------------------------
  Десериализация параметров из хранилища

  \param text

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t JSON_Deser_settings(char *text)
{
  json_t       *root;
  json_error_t  error;
  uint32_t      res;

  root = json_loads(text, 0,&error);

  if (!root)
  {
    APPLOG("JSON decoding error: on line %d: %s", error.line, error.text);
    return RES_ERROR;
  }

  if (!json_is_array(root))
  {
    APPLOG("JSON decoding error: root is not array.");
    json_decref(root);
    return RES_ERROR;
  }

  res = _JSON_Deser_tables(root);
  json_decref(root);

  return res;

}

/*-----------------------------------------------------------------------------------------------------


  \param root
  \param object
  \param key_name

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t _JSON_find_cmd(json_t  *root, json_t  **object, char const *key_name)
{
  json_t       *item;
  uint32_t      err = 1;
  uint32_t      n   = 0;
  uint32_t      i;

  err = 1;
  n   = json_array_size(root);
  // Проходим по всем элементам макссива root
  for (i = 0; i < n; i++)
  {
    item = json_array_get(root, i); // Получаем элемет массива
    *object = json_object_get(item, key_name); // Ищем в элементе массива объект с заданным именем
    //json_decref(item); Не декременитируем ссылку поскольку она  не инкрементировалась после вызова json_array_get
    if (*object != NULL)
    {
      if (json_is_integer(*object))
      {
        err = 0;
        break;
      }
    }
  }
  if (err == 1)
  {
    //json_decref(*object); Не декременитируем ссылку поскольку она  не инкрементировалась после вызова json_object_get
    *object = NULL;
    return RES_ERROR;
  }
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------
  Десериализация и выполнение команды содержимое которой из-за ее размера было сохранено в файл
  Вместо штатного парсера JSON производим собственный построчный разбор JSON структуры из файла
  Это команды:
    OPCODE_GET_FILES_BY_LIST
    OPCODE_DELETE_FILES_BY_LIST

  Список файлов из кодировки JSON переносится обычный список имен файлов заканчивающихся  STR_CRLF
  Этот спиок переносится в файл LIST_OF_FILENAMES и выдается сообщение задаче FTP_sender о необходимости выполнения операции



  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t JSON_Deser_long_command(void)
{
  FX_FILE        *file = NULL;
  char           *str  = NULL;
  int32_t        res;
  ULONG          actual_size;
  char           b;
  uint32_t       indx;
  uint32_t       opid;


  str =  App_malloc_pending(128, 10);
  if (str == NULL) goto EXIT_ON_ERROR;
  file = App_malloc_pending(sizeof(FX_FILE), 10);
  if (file == NULL) goto EXIT_ON_ERROR;
  // Открываем файл
  res = fx_file_open(&fat_fs_media, file, UNCOMPESSED_STREAM_FILE_NAME,  FX_OPEN_FOR_READ);
  if (res != FX_SUCCESS) goto EXIT_ON_ERROR;

  // Читаем стороку символов [{"OpCode":104}

  res= fx_file_read(file,(void *)str,11,&actual_size);
  if (res != FX_SUCCESS) goto EXIT_ON_ERROR;
  if (strncmp(str, "[{\"OpCode\":",11) != 0) goto EXIT_ON_ERROR;

  indx = 0;
  do
  {
    res= fx_file_read(file,(void *)&b,1,&actual_size);
    if (res != FX_SUCCESS) goto EXIT_ON_ERROR;
    str[indx] = b;
    indx++;

  } while ((isdigit(b) != 0) && (indx < 6)); // Up to 5 digits in OpCode are allowed
  if (b != '}') goto EXIT_ON_ERROR;
  str[indx] = 0;

  opid = atoi(str);

  if (!((opid == OPCODE_GET_FILES_BY_LIST) || (opid == OPCODE_DELETE_FILES_BY_LIST))) goto EXIT_ON_ERROR;

  // Читаем символы ,{"List":[
  //res= fx_file_read(file,(void *)str,10,&actual_size);
  //if (res != FX_SUCCESS) goto EXIT_ON_ERROR;
  //
  //if (str[9] != '[') goto EXIT_ON_ERROR;
  //
  //res = RES_ERROR;
  //
  //if (Is_FTP_sender_task_busy() == 0)
  //{
  //  res = MQTTMC_create_files_list_from_JSONchank(RECORDS_DIR_NAME, file);
  //  if (res == RES_OK)
  //  {
  //    if (opid == OPCODE_GET_FILES_BY_LIST)
  //    {
  //      FTP_sender_FLAG_START_FILES_SEND_BY_LIST();
  //    }
  //    else if (opid == OPCODE_DELETE_FILES_BY_LIST)
  //    {
  //      FTP_sender_FLAG_DELETE_FILES_BY_LIST();
  //    }
  //  }
  //}

  MQTTMC_Send_Ack(res);
  fx_file_close(file);
  App_free(str);
  App_free(file);
  return RES_OK;

EXIT_ON_ERROR:
  if (file != 0)
  {
    if (file->fx_file_id == FX_FILE_ID)
    {
      fx_file_close(file);
    }
    App_free(file);
  }
  App_free(str);
  return RES_ERROR;
}


/*-----------------------------------------------------------------------------------------------------
  Декодирование и сохранение параметров из JSON

  \param json

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t JSON_Deser_and_Exec_command(char *text, uint32_t f_long_stream)
{
  json_t       *root;
  json_error_t  error;
  json_t       *opc;
  uint32_t      opcode;
  uint32_t      res = RES_ERROR;


  if (f_long_stream)
  {
    if  (JSON_Deser_long_command() == RES_OK)
    {
      MQTTMC_Send_Ack(RES_OK);
      return RES_OK;
    }
    else
    {
      MQTTMC_Send_Ack(RES_ERROR);
      return RES_ERROR;
    }
  }

  root = json_loads(text, 0,&error);

  if (!root)
  {
    APPLOG("JSON decoding error: on line %d: %s", error.line, error.text);
    return RES_ERROR;
  }

  if (!json_is_array(root))
  {
    APPLOG("JSON decoding error: root is not array.");
    json_decref(root);
    return RES_ERROR;
  }

  res = _JSON_find_cmd(root,&opc, COMMAND_KEY);
  if (res != RES_OK)
  {
    APPLOG("JSON decoding error: 'OpCode' key is not a string.");
    json_decref(root);
    return RES_ERROR;
  }
  opcode = json_integer_value(opc);

  res = RES_ERROR;
  switch (opcode)
  {
  case OPCODE_SAVE_PARAMETERS_TO_RAM:
    APPLOG("MQTT command: SAVE_PARAMETERS_TO_RAM");
    res = _JSON_Deser_tables(root);
    MQTTMC_Send_Ack(res);
    break;
  case OPCODE_SAVE_PARAMETERS_TO_NV:
    APPLOG("MQTT command: SAVE_PARAMETERS_TO_NV");
    res = _JSON_Deser_tables(root);
    MQTTMC_Send_Ack(res);
    if (res == RES_OK) request_save_to_NV_mem = 1;
    break;


  default:
    MQTTMC_set_opcode(opcode); // Записываем код команды для обработки в главной процедуре. Так сделано для того чтобы при обработке не осталось захваченных парсером ресурсов памяти
    res = RES_OK;
    break;
  }
  json_decref(root);
  return res;

}
