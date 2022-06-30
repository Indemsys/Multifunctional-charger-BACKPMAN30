#include   "App.h"


// Объявление таблицы параметров
// Демонстрационная реализация

T_misc_table   tbl_client_ap_list[MISC_TABLE_REC_NUM] =
{
  {0, "val1", },
  {0, "val2", },
  {0, "val3", },
  {0, "val4", },
  {0, "val5", },
};

const T_table_fields_props Client_misc_table_props[7] =
{
  {"field1  ",  tint8u  , 1  , offsetof(T_misc_table, field1)},  //
  {"text1   ",  tstring , 32 , offsetof(T_misc_table, text1)},   //
};


T_params_table_info  params_tables_info[]=
{
  { BACKPMAN3_2_1_General,  TYPE_MISC_TABLE , "Misc table"  , MISC_TABLE_REC_NUM  },
};

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Restore_default_dyn_tables(void)
{
  for (uint32_t i=0; i < MISC_TABLE_REC_NUM; i++)
  {
    tbl_client_ap_list[i].field1 = 0;
    strcpy(tbl_client_ap_list[i].text1,"none");
  }
}
/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Get_params_tables_count(void)
{
#ifdef PARAMS_TABLES_ENABLED
  return (sizeof(params_tables_info) / sizeof(params_tables_info[0]));
#else
  return 0;
#endif
}

/*-----------------------------------------------------------------------------------------------------


  \param indx

  \return T_params_table_info*
-----------------------------------------------------------------------------------------------------*/
T_params_table_info* Get_params_table_record_ptr(uint32_t indx)
{
  if (indx < Get_params_tables_count())
  {
    return &params_tables_info[indx];
  }
  return &params_tables_info[0];
}


/*-----------------------------------------------------------------------------------------------------
  Получение указателя на таблицу с перечислением свойств полей таблицы параметров и колическтва полей в таблице параметров

  \param table_id

  \return T_table_fields_props const*
-----------------------------------------------------------------------------------------------------*/
T_table_fields_props const* Get_table_fields_props(uint32_t table_id, uint32_t *filds_cnt)
{
  switch (table_id)
  {
  case TYPE_MISC_TABLE:
    *filds_cnt = sizeof(Client_misc_table_props) / sizeof(Client_misc_table_props[0]);
    return Client_misc_table_props;
  }
  *filds_cnt = 0;
  return 0;
}

/*-----------------------------------------------------------------------------------------------------
  Получение количества записей в таблице параметров

  \param table_id

  \return void*
-----------------------------------------------------------------------------------------------------*/
uint32_t Get_table_records_count(uint32_t table_id)
{
  switch (table_id)
  {
  case TYPE_MISC_TABLE:
    return MISC_TABLE_REC_NUM;
  }
  return 0;
}

/*-----------------------------------------------------------------------------------------------------
   Получение указателя на запись в таблице по индексу записи

  \param table_id

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
void* Get_table_records_ptr(uint32_t table_id, uint32_t indx)
{
  switch (table_id)
  {
  case TYPE_MISC_TABLE:
    if (indx > MISC_TABLE_REC_NUM) return 0;
    return &tbl_client_ap_list[indx];
  }
  return 0;
}

/*-----------------------------------------------------------------------------------------------------


  \param table_id

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
char const* Get_table_JSON_key(uint32_t table_id)
{
  switch (table_id)
  {
  case TYPE_MISC_TABLE:
    return MISC_LIST_KEY;
  }
  return " ";
}



