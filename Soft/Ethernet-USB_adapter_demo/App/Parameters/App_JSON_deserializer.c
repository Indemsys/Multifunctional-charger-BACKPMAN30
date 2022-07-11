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


