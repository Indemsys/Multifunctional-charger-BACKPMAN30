#include   "App.h"


// Описание дерева меню
// Требование к структуре дерева - на один пункт не должны ссылаться два радительских пункта
extern const T_parmenu parmenu[];


#define   TMP_BUF_SZ       512

#define   MAX_VALUE_STR_SZ 100

static const char *PARAM_EDITOR_HELP = "\033[5C Press digit key to select menu item.\r\n"
                                      "\033[5C <M> - Main menu, <R> - return on prev. level\r\n"
                                      "\033[5C Enter - Accept, Esc - Cancel, BackSp - erase\r\n";

static const char *TABLE_EDITOR_HELP = "\033[5C Press digit key to start edit field.\r\n"
                                      "\033[5C <R> - return to menu, <N> то prev record, <M> - to next record\r\n"
                                      "\033[5C Enter - Accept, Esc - Cancel\r\n";



static void     Goto_to_edit_param(void);
static uint8_t* Get_mn_caption(void);
static uint8_t  Get_mn_prevlev(void);


#define MTYPE_NONE      0
#define MTYPE_SUBMENU   1
#define MTYPE_SUBTABLE  2
#define MTYPE_PARAMETER 3


/*-------------------------------------------------------------------------------------------
  Найти название меню по его идентификатору
---------------------------------------------------------------------------------------------*/
static uint8_t* Get_mn_caption(void)
{
  int i;
  GET_MCBL;

  for (i = 0; i < PARMNU_ITEM_NUM; i++)
  {
    if (parmenu[i].currlev == mcbl->current_level) return(uint8_t *)parmenu[i].name;
  }
  return(uint8_t *)parmenu[0].name;
}

/*-------------------------------------------------------------------------------------------
  Найти короткое название меню по его идентификатору
---------------------------------------------------------------------------------------------*/
uint8_t* Get_mn_shrtcapt(enum enm_parmnlev menu_lev)
{
  uint16_t i;
  for (i = 0; i < PARMNU_ITEM_NUM; i++)
  {
    if (parmenu[i].currlev == menu_lev) return(uint8_t *)parmenu[i].shrtname;
  }
  return(uint8_t *)parmenu[0].shrtname;
}

/*-------------------------------------------------------------------------------------------
  Найти название меню по его идентификатору
---------------------------------------------------------------------------------------------*/
uint8_t* Get_mn_name(enum enm_parmnlev menu_lev)
{
  uint16_t i;
  for (i = 0; i < PARMNU_ITEM_NUM; i++)
  {
    if (parmenu[i].currlev == menu_lev) return(uint8_t *)parmenu[i].name;
  }
  return(uint8_t *)parmenu[0].name;
}


/*-------------------------------------------------------------------------------------------

---------------------------------------------------------------------------------------------*/
static uint8_t Get_mn_prevlev(void)
{
  int i;
  GET_MCBL;

  for (i = 0; i < PARMNU_ITEM_NUM; i++)
  {
    if (parmenu[i].currlev == mcbl->current_level) return (parmenu[i].prevlev);
  }
  return (MAIN_PARAMS_ROOT);
}

/*-------------------------------------------------------------------------------------------
  Функция вызываемая один раз при входе в режим редактирования параметров
---------------------------------------------------------------------------------------------*/
void Do_Params_editor(uint8_t keycode)
{
  GET_MCBL;

  mcbl->current_level = MAIN_PARAMS_ROOT;
  Display_submenus_and_params_vals();                   // Показать меню параметров
  Set_monitor_func(Params_editor_press_key_handler);    // Переназначение обработчика нажатий в главном цикле монитора
}

/*-------------------------------------------------------------------------------------------
  Вывод на экран
---------------------------------------------------------------------------------------------*/
void Display_submenus_and_params_vals(void)
{
  int               i;
  uint32_t          n;
  uint8_t           str[MAX_VALUE_STR_SZ];
  uint8_t          *st;
  GET_MCBL;

  MPRINTF(VT100_CLEAR_AND_HOME);
  // Вывод заголовка меню параметров
  st = Get_mn_caption();
  VT100_send_str_to_pos(st, 1, Find_str_center(st));
  VT100_send_str_to_pos(DASH_LINE, 2, 0);
  VT100_send_str_to_pos((uint8_t *)PARAM_EDITOR_HELP, 3, 0);
  MPRINTF("\r\n");
  MPRINTF(DASH_LINE);

  //..........................................
  // Вывести список субменю на данном уровне
  //..........................................
  n = 0;
  for (i = 0; i < PARMNU_ITEM_NUM; i++)
  {
    if (parmenu[i].prevlev != mcbl->current_level) continue;
    sprintf((char *)str, "%1X - %s", n, parmenu[i].name);

    if ((strlen((char *)str)+ SCR_ITEMS_HOR_OFFS) > COLCOUNT)
    {
      str[COLCOUNT - SCR_ITEMS_HOR_OFFS - 3] = '.';
      str[COLCOUNT - SCR_ITEMS_HOR_OFFS - 2] = '>';
      str[COLCOUNT - SCR_ITEMS_HOR_OFFS - 1] = 0;
    }
    VT100_send_str_to_pos(str, SCR_ITEMS_VERT_OFFS + n, SCR_ITEMS_HOR_OFFS);
    mcbl->item_indexes[n] = i;
    n++;
  }
  mcbl->current_menu_submenus_count = n;

  //..........................................
  // Вывести список таблиц параметров на данном уровне
  //..........................................
#ifdef PARAMS_TABLES_ENABLED
  for (i = 0; i < Get_params_tables_count(); i++)
  {
    T_params_table_info  *rec = Get_params_table_record_ptr(i);
    if (rec->level == mcbl->current_level)
    {
      sprintf((char *)str, "%1X - %s[%d]", n, rec->name, rec->records_num);
      VT100_send_str_to_pos(str, SCR_ITEMS_VERT_OFFS + n, SCR_ITEMS_HOR_OFFS);
      mcbl->item_indexes[n] = i;
      n++;
    }
  }
  mcbl->current_menu_subtables_count = n - mcbl->current_menu_submenus_count;
#endif

  //..........................................
  // Вывести список всех параметров на данном уровне
  //..........................................
  for (i = 0; i < DWVAR_SIZE; i++)
  {
    int len;
    if (dwvar[i].parmnlev != mcbl->current_level) continue;
    VT100_set_cursor_pos(SCR_ITEMS_VERT_OFFS + n, SCR_ITEMS_HOR_OFFS);
    sprintf((char *)str, "%1X - ", n);
    MPRINTF((char *)str);
    len = strlen((char *)str)+ SCR_ITEMS_HOR_OFFS;
    MPRINTF(VT100_REVERSE_ON);
    MPRINTF((char *)dwvar[i].var_description);
    MPRINTF("= ");
    len = len + strlen((char *)dwvar[i].var_description)+ 2;
    MPRINTF(VT100_REVERSE_OFF);
    // Преобразовать параметр в строку
    Param_to_str(str, MAX_VALUE_STR_SZ-1, i);
    if ((strlen((char *)str)+ len) > COLCOUNT)
    {
      str[COLCOUNT - len - 3] = '.';
      str[COLCOUNT - len - 2] = '>';
      str[COLCOUNT - len - 1] = 0;
    }
    MPRINTF((char *)str);
    mcbl->item_indexes[n] = i;
    n++;
  }
  mcbl->current_menu_items_count = n;

}

/*-----------------------------------------------------------------------------------------------------


  \param tbl_fields_props
  \param rec_start_ptr
  \param str
-----------------------------------------------------------------------------------------------------*/
void Get_table_field_value_string(uint32_t field_num, const T_table_fields_props  *tbl_fields_props, uint8_t *rec_start_ptr, char *str)
{
  switch (tbl_fields_props[field_num].fld_type)
  {
  case tint8u        :
    {
      uint8_t v;
      memcpy(&v, rec_start_ptr + tbl_fields_props[field_num].fld_offset, tbl_fields_props[field_num].fld_sz);
      sprintf((char *)str, "%d", v);
    }
    break;
  case tint16u       :
    {
      uint16_t v;
      memcpy(&v, rec_start_ptr + tbl_fields_props[field_num].fld_offset, tbl_fields_props[field_num].fld_sz);
      sprintf((char *)str, "%d", v);
    }
    break;
  case tint32u       :
    {
      uint32_t v;
      memcpy(&v, rec_start_ptr + tbl_fields_props[field_num].fld_offset, tbl_fields_props[field_num].fld_sz);
      sprintf((char *)str, "%d", v);
    }
    break;
  case tfloat        :
    {
      float v;
      memcpy(&v, rec_start_ptr + tbl_fields_props[field_num].fld_offset, tbl_fields_props[field_num].fld_sz);
      sprintf((char *)str, "%f", v);
    }
    break;
  case tarrofdouble  :
    break;
  case tstring       :
    {
      char *v;
      v = (char *)rec_start_ptr + tbl_fields_props[field_num].fld_offset;
      snprintf((char *)str, MAX_VALUE_STR_SZ-1,  "%s", v);
    }
    break;
  case tarrofbyte    :
    break;
  case tint32s       :
    break;
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param table_id
  \param rec_indx
  \param row
  \param col
  \param str
-----------------------------------------------------------------------------------------------------*/
void Edit_table_record(uint32_t table_id, uint32_t rec_indx, uint32_t field_num, uint8_t row, uint8_t col, char *str)
{
  GET_MCBL;
  uint32_t                     table_fields_count;
  const T_table_fields_props  *tbl_fields_props;
  uint8_t                     *rec_start_ptr;

  tbl_fields_props = Get_table_fields_props(table_id,&table_fields_count);
  rec_start_ptr    = (uint8_t *)Get_table_records_ptr(table_id, rec_indx);

  VT100_set_cursor_pos(row, col);
  MPRINTF("Edited field: %s\r\n",  tbl_fields_props[field_num].field_name);
  row++;
  MPRINTF(DASH_LINE);
  row++;
  VT100_set_cursor_pos(row, col);
  Get_table_field_value_string(field_num, tbl_fields_props, rec_start_ptr, str);

  uint32_t  sz;
  if (tbl_fields_props[field_num].fld_type == tstring)
  {
    sz = tbl_fields_props[field_num].fld_sz;
  }
  else
  {
    sz = 32;
  }

  uint32_t res;

  MPRINTF(VT100_CURSOR_ON);
  res = Edit_string(str, sz, str);
  MPRINTF(VT100_CURSOR_OFF);

  if (res == RES_OK)
  {
    // Конвертировать строку обратно в значение
    switch (tbl_fields_props[field_num].fld_type)
    {
    case tint8u        :
      {
        uint32_t iv;
        if (sscanf(str,"%d",&iv) == 1)
        {
          uint8_t v = (uint8_t)iv;
          memcpy(rec_start_ptr + tbl_fields_props[field_num].fld_offset,&v, tbl_fields_props[field_num].fld_sz);
          Req_to_save_settings();
        }
      }
      break;
    case tint16u       :
      {
        uint32_t iv;
        if (sscanf(str,"%d",&iv) == 1)
        {
          uint16_t v = (uint16_t)iv;
          memcpy(rec_start_ptr + tbl_fields_props[field_num].fld_offset,&v, tbl_fields_props[field_num].fld_sz);
          Req_to_save_settings();
        }
      }
      break;
    case tint32u       :
      {
        uint32_t v;
        if (sscanf(str,"%d",&v) == 1)
        {
          memcpy(rec_start_ptr + tbl_fields_props[field_num].fld_offset,&v, tbl_fields_props[field_num].fld_sz);
          Req_to_save_settings();
        }
      }
      break;
    case tfloat        :
      {
        float v;
        if (sscanf(str,"%f",&v) == 1)
        {
          memcpy(rec_start_ptr + tbl_fields_props[field_num].fld_offset,&v, tbl_fields_props[field_num].fld_sz);
          Req_to_save_settings();
        }
      }
      break;
    case tarrofdouble  :
      break;
    case tstring       :
      {
        char *s;
        s = (char *)rec_start_ptr + tbl_fields_props[field_num].fld_offset;
        strcpy(s, str);
        Req_to_save_settings();
      }
      break;
    case tarrofbyte    :
      break;
    case tint32s       :
      {
        uint32_t iv;
        if (sscanf(str,"%d",&iv) == 1)
        {
          int32_t v = (int32_t)iv;
          memcpy(rec_start_ptr + tbl_fields_props[field_num].fld_offset,&v, tbl_fields_props[field_num].fld_sz);
          Req_to_save_settings();
        }
      }
      break;
    }

  }

}

/*-----------------------------------------------------------------------------------------------------


  \param table_id
  \param rec_indx
  \param row
  \param col
-----------------------------------------------------------------------------------------------------*/
void Show_table_fields(uint32_t table_id, uint32_t rec_indx, uint8_t row, uint8_t col, uint8_t *str)
{
  GET_MCBL;

  uint32_t                     table_fields_count;
  const T_table_fields_props  *tbl_fields_props;
  uint8_t                     *rec_start_ptr;

  VT100_set_cursor_pos(row, col);
  sprintf((char *)str, ">>> RECORD %d <<< ", rec_indx);
  MPRINTF((char *)str);
  row++;
  tbl_fields_props = Get_table_fields_props(table_id,&table_fields_count);
  rec_start_ptr    = (uint8_t *)Get_table_records_ptr(table_id, rec_indx);
  // Вывести все поля таблицы
  for (uint32_t i=0; i < table_fields_count; i++)
  {
    VT100_set_cursor_pos(row + i, col);
    // Выводим номер поля
    sprintf((char *)str, VT100_CLL_FM_CRSR"%1X - ", i);
    MPRINTF((char *)str);
    // Выводим название поля
    sprintf((char *)str, "%s = ", tbl_fields_props[i].field_name);
    MPRINTF((char *)str);
    // Выводим значение
    Get_table_field_value_string(i, tbl_fields_props, rec_start_ptr, (char *)str);
    MPRINTF((char *)str);
  }
}

/*-----------------------------------------------------------------------------------------------------
  Вывод

  \param void
-----------------------------------------------------------------------------------------------------*/
void Table_editor(uint32_t tbl_indx)
{
  GET_MCBL;
  uint32_t                     table_id;
  uint32_t                     table_records_count;
  T_params_table_info         *tbl_info;


  uint32_t                     curr_rec_indx = 0; // Индекс текущей записи в таблице
  uint8_t                      b;
  uint8_t                      row;
  uint8_t                      col;
  uint8_t                      str[MAX_VALUE_STR_SZ];

  MPRINTF(VT100_CLEAR_AND_HOME);
  // Вывод заголовка меню редактирования таблицы
  tbl_info = Get_params_table_record_ptr(tbl_indx);
  VT100_send_str_to_pos((uint8_t *)tbl_info->name, 1, Find_str_center((uint8_t *)tbl_info->name));
  VT100_send_str_to_pos(DASH_LINE, 2, 0);
  VT100_send_str_to_pos((uint8_t *)TABLE_EDITOR_HELP, 3, 0);
  MPRINTF("\r\n");
  MPRINTF(DASH_LINE);

  table_id = tbl_info->table_id;
  table_records_count = tbl_info->records_num;
  row      = 8;
  col      = 1;

  Show_table_fields(table_id, curr_rec_indx, row, col, str);

  do
  {
    if (WAIT_CHAR(&b, 1000) == RES_OK)
    {
      switch (b)
      {
      case 'R':
      case 'r':
        return;

      case 'M':
      case 'm':
        // Перейти к предыдущей записи
        if (curr_rec_indx >= (table_records_count - 1))
        {
          curr_rec_indx = 0;
        }
        else
        {
          curr_rec_indx++;
        }
        Show_table_fields(table_id, curr_rec_indx, row, col, str);
        break;

      case 'N':
      case 'n':
        // Перейти к следующей записи
        if (curr_rec_indx == 0)
        {
          curr_rec_indx = table_records_count - 1;
        }
        else
        {
          curr_rec_indx--;
        }
        Show_table_fields(table_id, curr_rec_indx, row, col, str);
        break;
      default:
        {
          uint32_t field_num;
          uint32_t table_fields_count;
          if ((b >= '0') && (b <= '9'))
          {
            field_num = b - '0';
          }
          else if ((b >= 'a') && (b <= 'f'))
          {
            field_num = b - 'a';
          }
          else if ((b >= 'A') && (b <= 'F'))
          {
            field_num = b - 'A';
          }
          else
          {
            break;
          }
          Get_table_fields_props(table_id,&table_fields_count);
          if (field_num < table_fields_count)
          {
            Edit_table_record(table_id, curr_rec_indx, field_num, row + 17, 1, (char *)str);
            // Очищаем нижнюю часть экрана от строк процесса редактирования
            VT100_set_cursor_pos(row + 17, 1);
            MPRINTF(VT100_CLR_FM_CRSR);
            Show_table_fields(table_id, curr_rec_indx, row, col, str);
          }
        }
        break;
      }
    }
  } while (1);


}


/*-------------------------------------------------------------------------------------------
  Функция периодически вызываемая в качестве обработчика нажатий из главного цикла монитора

  Принимаются коды клавиш:
    от 0 до F - вызывают соответствующий пункт меню
    M и R     - вызываю переход на другой уровень меню
---------------------------------------------------------------------------------------------*/
void Params_editor_press_key_handler(uint8_t b)
{

  GET_MCBL;

  if (b < 0x30) return; // Игнорируем служебные коды

  if ((b == 'M') || (b == 'm'))
  {
    Goto_main_menu();
    return;
  }
  if ((b == 'R') || (b == 'r'))
  {
    if (mcbl->current_level != MAIN_PARAMS_ROOT)
    {
      mcbl->current_level = Get_mn_prevlev();
      Display_submenus_and_params_vals();
      return;
    }
    else
    {
      // Выход на пункт меню из которого вошли в редактор параметров
      Return_to_prev_menu();
      Display_menu();
      return;
    }

  }

  b = ascii_to_hex(b); // Из кода клавиши получаем 16-и ричную цифру
  if (b >= MAX_ITEMS_COUNT) return;


  if (b >= mcbl->current_menu_items_count) return; // Выход если код меньше чем есть в ниличии пунктов

  // Определяем к какому типу пункта меню относится код клавиши
  uint8_t mtype = MTYPE_NONE;

  if  (b < mcbl->current_menu_submenus_count)
  {
    mtype = MTYPE_SUBMENU;
  }
  else if (b < (mcbl->current_menu_subtables_count + mcbl->current_menu_submenus_count))
  {
    mtype = MTYPE_SUBTABLE;
  }
  else
  {
    mtype = MTYPE_PARAMETER;
  }


  switch (mtype)
  {
  case MTYPE_SUBMENU:

    // Выбран пункт субменю
    mcbl->current_level = parmenu[mcbl->item_indexes[b]].currlev;
    Display_submenus_and_params_vals();
    break;

  case MTYPE_SUBTABLE:
    Table_editor(mcbl->item_indexes[b]);
    Display_submenus_and_params_vals();
    break;

  case MTYPE_PARAMETER:

    // Выбран параметр для редактирования
    mcbl->current_parameter_indx = mcbl->item_indexes[b];
    Goto_to_edit_param();
    break;
  }
}



/*-------------------------------------------------------------------------------------------
  Переход к редактированиб параметра

  Параметры:
             n    - индекс параметра в массиве параметров
             item - номер параметра в меню
---------------------------------------------------------------------------------------------*/
static void Goto_to_edit_param(void)
{
  int n;
  GET_MCBL;

  mcbl->rowcount =(MAX_PARAMETER_STRING_LEN + 1) / COLCOUNT;
  if (((MAX_PARAMETER_STRING_LEN + 1)% COLCOUNT) > 0) mcbl->rowcount++;
  if (mcbl->rowcount > ROWCOUNT - 2) return;  // Слишком большая область редактирования
  mcbl->firstrow = ROWCOUNT - mcbl->rowcount + 1;
  VT100_set_cursor_pos(mcbl->firstrow - 2, 0);
  MPRINTF("Edited parameter: '%s'\r\n", dwvar[mcbl->current_parameter_indx].var_description);
  MPRINTF(DASH_LINE);
  Param_to_str(mcbl->param_str, MAX_PARAMETER_STRING_LEN, mcbl->current_parameter_indx);
  VT100_set_cursor_pos(mcbl->firstrow, 0);
  MPRINTF(VT100_CLR_FM_CRSR);
  VT100_set_cursor_pos(mcbl->firstrow, 0);
  MPRINTF(">");
  MPRINTF(VT100_CURSOR_ON);
  MPRINTF((char *)mcbl->param_str);
  // Вычислим где сейчас находится курсор
  n = strlen((char *)mcbl->param_str);
  if (n != 0)
  {
    mcbl->current_row =(n + 2) / COLCOUNT;
    mcbl->current_col =(n + 2)% COLCOUNT;
    mcbl->current_pos =  n;
  }
  else
  {
    mcbl->current_row = 0;
    mcbl->current_col = 2;
    mcbl->current_pos = 0;
  }
  VT100_set_cursor_pos(mcbl->firstrow + mcbl->current_row, mcbl->current_col);
  Set_monitor_func(Edit_func);
}

/*-------------------------------------------------------------------------------------------
   Функция редактирования параметра в окне терминала
---------------------------------------------------------------------------------------------*/
void Edit_func(uint8_t b)
{
  GET_MCBL;

  switch (b)
  {
  case 0x08:  // Back Space
    if (mcbl->current_pos > 0)
    {
      mcbl->current_pos--;
      mcbl->param_str[mcbl->current_pos] = 0;

      if (mcbl->current_col < 2)
      {
        mcbl->current_col = COLCOUNT;
        mcbl->current_row--;
      }
      else
      {
        mcbl->current_col--;
      }
    }
    VT100_set_cursor_pos(mcbl->firstrow + mcbl->current_row, mcbl->current_col);
    MPRINTF(" ");
    VT100_set_cursor_pos(mcbl->firstrow + mcbl->current_row, mcbl->current_col);
    break;
  case 0x7E:  // DEL
  case 0x7F:  // DEL
    mcbl->current_row = 0;
    mcbl->current_col = 2;
    mcbl->current_pos = 0;
    mcbl->param_str[mcbl->current_pos] = 0;
    VT100_set_cursor_pos(mcbl->firstrow + mcbl->current_row, mcbl->current_col);
    MPRINTF(VT100_CLR_FM_CRSR);
    break;
  case 0x1B:  // ESC

    MPRINTF(VT100_CURSOR_OFF);
    Display_submenus_and_params_vals();
    Set_monitor_func(Params_editor_press_key_handler);

    break;
  case 0x0D:  // Enter
    mcbl->param_str[mcbl->current_pos] = 0;

    Str_to_param(mcbl->param_str, mcbl->current_parameter_indx);
    Execute_param_func(mcbl->current_parameter_indx);

    Req_to_save_settings();

    MPRINTF(VT100_CURSOR_OFF);
    Display_submenus_and_params_vals();
    Set_monitor_func(Params_editor_press_key_handler);
    break;
  default:
    if (isspace(b) || isgraph(b))
    {
      mcbl->param_str[mcbl->current_pos] = b;
      MPRINTF("%c", b);
      if (mcbl->current_pos < (MAX_PARAMETER_STRING_LEN - 1))
      {
        mcbl->current_pos++;
        if (mcbl->current_col == (COLCOUNT))
        {
          mcbl->current_col = 1;
          mcbl->current_row++;
        }
        else mcbl->current_col++;
      }
      VT100_set_cursor_pos(mcbl->firstrow + mcbl->current_row, mcbl->current_col);
    }
    break;

  }

}


/*-------------------------------------------------------------------------------------------
   Преобразовать строку в параметр

   Входная строка должна позволять модификацию
---------------------------------------------------------------------------------------------*/
void Str_to_param(uint8_t *in_str, uint16_t indx)
{
  uint8_t  uch_tmp;
  uint16_t uin_tmp;
  uint32_t ulg_tmp;
  int32_t slg_tmp;
  char *end;
  float  d_tmp;

  // Откорректируем преобразование строк True и False в 1 и 0 соответственно
  if (dwvar[indx].vartype != tstring)
  {
    if (strcmp((char*)in_str, "True") == 0)
    {
      in_str[0] = '1';
      in_str[1] = 0;
    }
    else if (strcmp((char*)in_str, "False") == 0)
    {
      in_str[0] = '0';
      in_str[1] = 0;
    }
  }


  switch (dwvar[indx].vartype)
  {
  case tint8u:
    uch_tmp = strtol((char *)in_str,&end, 10);
    if (uch_tmp > ((uint8_t)dwvar[indx].maxval)) uch_tmp = (uint8_t)dwvar[indx].maxval;
    if (uch_tmp < ((uint8_t)dwvar[indx].minval)) uch_tmp = (uint8_t)dwvar[indx].minval;
    *(uint8_t *)dwvar[indx].val = uch_tmp;
    break;
  case tint16u:
    uin_tmp = strtol((char *)in_str,&end, 10);
    if (uin_tmp > ((uint16_t)dwvar[indx].maxval)) uin_tmp = (uint16_t)dwvar[indx].maxval;
    if (uin_tmp < ((uint16_t)dwvar[indx].minval)) uin_tmp = (uint16_t)dwvar[indx].minval;
    *(uint16_t *)dwvar[indx].val = uin_tmp;
    break;
  case tint32u:
    ulg_tmp = strtol((char *)in_str,&end, 10);
    if (ulg_tmp > ((uint32_t)dwvar[indx].maxval)) ulg_tmp = (uint32_t)dwvar[indx].maxval;
    if (ulg_tmp < ((uint32_t)dwvar[indx].minval)) ulg_tmp = (uint32_t)dwvar[indx].minval;
    *(uint32_t *)dwvar[indx].val = ulg_tmp;
    break;
  case tint32s:
    slg_tmp = strtol((char *)in_str,&end, 10);
    if (slg_tmp > ((int32_t)dwvar[indx].maxval)) slg_tmp = (uint32_t)dwvar[indx].maxval;
    if (slg_tmp < ((int32_t)dwvar[indx].minval)) slg_tmp = (uint32_t)dwvar[indx].minval;
    *(uint32_t *)dwvar[indx].val = slg_tmp;
    break;
  case tfloat:
    d_tmp = strtod((char *)in_str,&end);
    if (d_tmp > ((float)dwvar[indx].maxval)) d_tmp = (float)dwvar[indx].maxval;
    if (d_tmp < ((float)dwvar[indx].minval)) d_tmp = (float)dwvar[indx].minval;
    *(float *)dwvar[indx].val = d_tmp;
    break;
  case tstring:
    {
      uint8_t *st;
      strncpy(dwvar[indx].val, (char *)in_str, dwvar[indx].varlen - 1);
      st = dwvar[indx].val;
      st[dwvar[indx].varlen - 1] = 0;
    }
    break;
  case tarrofbyte:
    break;
  case tarrofdouble:
    break;
  }
}

/*-------------------------------------------------------------------------------------------
   Преобразовать параметр в строку
---------------------------------------------------------------------------------------------*/
void Param_to_str(uint8_t *buf, uint16_t maxlen, uint16_t indx)
{
  void *val;

  // tint8u, tint16u, tint32u, tdouble, tstring, tarrofdouble, tarrofbyte
  val = dwvar[indx].val;
  switch (dwvar[indx].vartype)
  {
  case tint8u:
    snprintf((char *)buf, maxlen, dwvar[indx].format,*(uint8_t *)val);
    break;
  case tint16u:
    snprintf((char *)buf, maxlen, dwvar[indx].format,*(uint16_t *)val);
    break;
  case tint32u:
    snprintf((char *)buf, maxlen, dwvar[indx].format,*(uint32_t *)val);
    break;
  case tint32s:
    snprintf((char *)buf, maxlen, dwvar[indx].format,*(int32_t *)val);
    break;
  case tfloat:
    {
      float f;
      f =*((float *)val);
      snprintf((char *)buf, maxlen, dwvar[indx].format, f);
      break;
    }
  case tstring:
    {
      int len;
      if (dwvar[indx].varlen > maxlen)
      {
        len = maxlen - 1;
      }
      else
      {
        len = dwvar[indx].varlen - 1;
      }
      strncpy((char *)buf, (char *)val, len);
      buf[len] = 0;
    }
    break;
  default:
    break;
  }
}

/*-------------------------------------------------------------------------------------------
  Найти индекс параметра по псевдониму
---------------------------------------------------------------------------------------------*/
int32_t Find_param_by_alias(char *alias)
{
  int i;
  for (i = 0; i < DWVAR_SIZE; i++)
  {
    if (strcmp((char *)dwvar[i].var_alias, alias) == 0) return (i);
  }
  return (-1);
}

/*-------------------------------------------------------------------------------------------
  Найти индекс параметра по аббревиатуре
---------------------------------------------------------------------------------------------*/
int32_t Find_param_by_name(char *name)
{
  int i;
  for (i = 0; i < DWVAR_SIZE; i++)
  {
    if (strcmp((char *)dwvar[i].var_name, name) == 0) return (i);
  }
  return (-1);
}

/*-----------------------------------------------------------------------------------------------------
    Найти индекс параметра по адресу переменной

  \param ptr

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
int32_t Find_param_by_var_ptr(void *ptr)
{
  int i;
  for (i = 0; i < DWVAR_SIZE; i++)
  {
    if (dwvar[i].val == ptr) return (i);
  }
  return (-1);
}




/*-------------------------------------------------------------------------------------------
  Получить тип параметра
---------------------------------------------------------------------------------------------*/
enum  vartypes Get_param_type(int n)
{
  return (dwvar[n].vartype);
}

/*-------------------------------------------------------------------------------------------
   Найти сколько подпунктов есть на данном уровне меню
---------------------------------------------------------------------------------------------*/
int16_t Get_menu_items_count(enum enm_parmnlev menu_item)
{
  int16_t i;
  int16_t n;
  n = 0;
  // Считаем подменю
  for (i = 0; i < PARMNU_ITEM_NUM; i++)
  {
    if (parmenu[i].prevlev != menu_item) continue;
    n++;
  }
  // Считаем сами параметры входящие в меню
  for (i = 0; i < DWVAR_SIZE; i++)
  {
    if (dwvar[i].parmnlev != menu_item) continue;
    n++;
  }
  return (n);
}

/*-------------------------------------------------------------------------------------------
   Возвратить указатель на название пункта подменю или параметра для заданного пункта меню
   curr_menu - идентификатор текущего меню
   item      - номер текущего подпункта в меню
   возвращает:
   submenu   - возвращает субменю, но если этот пункт не подменю, то возвращает 0
---------------------------------------------------------------------------------------------*/
uint8_t* Get_item_capt(enum enm_parmnlev curr_menu, uint8_t item, enum enm_parmnlev *submenu, uint16_t *parnum)
{
  int16_t i;
  int16_t n;
  n = 0;
  // Считаем подменю
  for (i = 0; i < PARMNU_ITEM_NUM; i++)
  {
    if (parmenu[i].prevlev != curr_menu) continue;
    if (n == item)
    {
      *submenu = parmenu[i].currlev;
      return(uint8_t *)parmenu[i].shrtname;
    }
    n++;
  }
  // Считаем сами параметры входящие в меню
  for (i = 0; i < DWVAR_SIZE; i++)
  {
    if (dwvar[i].parmnlev != curr_menu) continue;
    if ((dwvar[i].parmnlev & VAL_UNVISIBLE) != 0) continue;
    if (n == item)
    {
      *submenu = PARAMS_ROOT;
      *parnum  = i;
      return(uint8_t *)dwvar[i].var_alias;
    }
    n++;
  }
  return (0);
}

/*-------------------------------------------------------------------------------------------
  Получить идентификатор родительского меню по идентификатору субменю
---------------------------------------------------------------------------------------------*/
enum enm_parmnlev Get_parent_menu(enum enm_parmnlev curr_menu)
{
  int16_t i;
  for (i = 0; i < PARMNU_ITEM_NUM; i++)
  {
    if (parmenu[i].currlev == curr_menu) return (parmenu[i].prevlev);
  }
  return (MAIN_PARAMS_ROOT);
}





/*-------------------------------------------------------------------------------------------
  Функция возвращает указатель на массив дерева меню.
  В переменной *sz возвращается количество записей в массиве описывающем дерево
---------------------------------------------------------------------------------------------*/
T_parmenu* Get_parmenu(int16_t *sz)
{
  *sz = PARMNU_ITEM_NUM;
  return(T_parmenu *)parmenu;

}


/*-------------------------------------------------------------------------------------------
  Функция возвращает указатель на массив определений рабочих параметров.
  В переменной *sz возвращается количество записей в массиве
---------------------------------------------------------------------------------------------*/
T_work_params* Get_params(int16_t *sz)
{
  *sz = DWVAR_SIZE;
  return(T_work_params *)dwvar;
}

/*-------------------------------------------------------------------------------------------

---------------------------------------------------------------------------------------------*/
void Execute_param_func(uint16_t indx)
{
  if (dwvar[indx].func != 0) dwvar[indx].func();
}

/*-----------------------------------------------------------------------------------------------------



  \return const char*
-----------------------------------------------------------------------------------------------------*/
const char* Convrt_var_type_to_str(enum  vartypes  vartype)
{
  switch (vartype)
  {
  case tint8u        :
    return "tint8u";
  case tint16u       :
    return "tint16u";
  case tint32u       :
    return "tint32u";
  case tfloat        :
    return "tfloat";
  case tarrofdouble  :
    return "tarrofdouble";
  case tstring       :
    return "tstring";
  case tarrofbyte    :
    return "tarrofbyte";
  case tint32s       :
    return "tint32s";
  default:
    return "unknown";
  }
}



