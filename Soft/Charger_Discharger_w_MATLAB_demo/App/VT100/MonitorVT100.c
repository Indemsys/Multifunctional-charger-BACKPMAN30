#include "App.h"


#define COL        80   /* Maximum column size       */
#define EDSTLEN    17

extern void Do_System_info(uint8_t keycode);
static void Do_FileX_block_rw_test(uint8_t keycode);
static void Do_FileX_test(uint8_t keycode);
static void Do_Reset(uint8_t keycode);

static void Do_show_event_log(uint8_t keycode);

extern const T_VT100_Menu MENU_MAIN;
extern const T_VT100_Menu MENU_PARAMETERS;
extern const T_VT100_Menu MENU_SPEC;


static int32_t Lookup_menu_item(T_VT100_Menu_item **item, uint8_t b);



/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

      Пункты имеющие свое подменю располагаются на следующем уровне вложенности
      Их функция подменяет в главном цикле обработчик нажатий по умолчанию и должна
      отдавать управление периодически в главный цикл

      Пункты не имеющие функции просто переходят на следующий уровень подменю

      Пункты не имеющие подменю полностью захватывают управление и на следующий уровень не переходят

      Пункты без подменю и функции означают возврат на предыдущий уровень
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

//-------------------------------------------------------------------------------------
const T_VT100_Menu_item MENU_MAIN_ITEMS[] =
{
  { '1', Do_Params_editor  , (void *)&MENU_PARAMETERS },
  { '2', 0                 , (void *)&MENU_SPEC       },
  { '3', Do_show_event_log , 0                        },
  { '4', Do_Reset          , 0                        },
  { 'R', 0                 , 0                        },
  { 'M', 0                 , (void *)&MENU_MAIN       },
  { 0                                                 }
};

const T_VT100_Menu      MENU_MAIN             =
{
  "BACKPMAN v3 Rev1.0 board",
  "\033[5C MAIN MENU \r\n"
  "\033[5C <1> - Adjustable parameters and settings\r\n"
  "\033[5C <2> - Board diagnostic\r\n"
  "\033[5C <3> - System log\r\n"
  "\033[5C <4> - Reset\r\n",
  MENU_MAIN_ITEMS,
};

//-------------------------------------------------------------------------------------
const T_VT100_Menu      MENU_PARAMETERS       =
{
  "",
  "",
  0
};

const T_VT100_Menu_item MENU_SPEC_ITEMS[] =
{
  { '1', Do_FileX_test     , 0},
  { '2', Do_FileX_block_rw_test, 0},
  { 'R', 0, 0 },
  { 'M', 0, (void *)&MENU_MAIN },
  { 0 }
};

const T_VT100_Menu      MENU_SPEC  =
{
  "BACKPMAN v3 Rev1.0 board",
  "\033[5C Perepherial tests \r\n"
  "\033[5C <1> - FileX test\r\n"
  "\033[5C <2> - FileX block rw perfomance test\r\n"
  "\033[5C <R> - Display previous menu\r\n"
  "\033[5C <M> - Display main menu\r\n",
  MENU_SPEC_ITEMS,
};


const char              *_days_abbrev[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
const char              *_months_abbrev[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

extern uint32_t           test_attempt;


static uint8_t           vt100_allocations[VT100_TASKS_MAX_NUM] = {0};
static uint8_t          *p_vt100_stacks[VT100_TASKS_MAX_NUM];
T_VT100_task_cbl        *p_vt100_task_cbls[VT100_TASKS_MAX_NUM];

static void Task_VT100(ULONG initial_data);


/*-----------------------------------------------------------------------------------------------------
  Создание задачи VT100.
  Можем создать не более VT100_TASKS_MAX_NUM задач

  \param void
-----------------------------------------------------------------------------------------------------*/
uint32_t Task_VT100_create(T_serial_io_driver* p_driver, int32_t *handle)
{
  UINT                err;
  T_serial_io_driver *p_drv;
  int32_t             indx = -1;

  if (handle != 0) *handle = indx;

  // Проверяем есть ли свободный слот для задачи
  for (uint32_t i=0; i < VT100_TASKS_MAX_NUM; i++)
  {
    if (vt100_allocations[i] == 0)
    {
      vt100_allocations[i] = 1;
      indx = i;
      // Назначаем функции вводы вывода задаче
      p_drv = p_driver;

      p_vt100_task_cbls[indx] = App_malloc(sizeof(T_VT100_task_cbl));
      if (p_vt100_task_cbls[indx] == NULL) goto err_exit_;
      p_vt100_task_cbls[indx]->mcbl.pdrv       = p_drv;
      sprintf(p_vt100_task_cbls[indx]->task_name,"VT100_%d",indx);
    }
    if (indx >= 0) break;
  }
  if (indx < 0)
  {
    return RES_ERROR;
  }

  // Инициализируем драйвер
  // Выделяется память драйвера, создается задача приема драйвера
  if (p_drv->_init(&p_drv->pdrvcbl, p_drv) != RES_OK)
  {
    goto err_exit_;
  }
  else
  {
    p_vt100_stacks[indx] = App_malloc(VT100_TASK_STACK_SIZE);
    if (p_vt100_stacks[indx] == NULL) goto err_exit_;

    err = tx_thread_create(
         &p_vt100_task_cbls[indx]->VT100_thread,
         (CHAR *)p_vt100_task_cbls[indx]->task_name,
         Task_VT100,
         (ULONG)(&(p_vt100_task_cbls[indx]->mcbl)),
         p_vt100_stacks[indx],
         VT100_TASK_STACK_SIZE,
         VT100_TASK_PRIO,
         VT100_TASK_PRIO,

         1,
         TX_AUTO_START
         );
    if (err != TX_SUCCESS) goto err_exit_;
  }

  if (handle != 0) *handle = indx;
  return RES_OK;


err_exit_:
  App_free(p_vt100_task_cbls[indx]);
  App_free(p_vt100_stacks[indx]);
  p_drv->_deinit(&p_drv->pdrvcbl);
  vt100_allocations[indx] = 0;
  return RES_ERROR;
}

/*-----------------------------------------------------------------------------------------------------


  \param alloc_indx

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Task_VT100_delete(int32_t *alloc_indx)
{
  int32_t indx;

  if (alloc_indx == 0) return RES_ERROR;
  indx =*alloc_indx;

  if (indx >= VT100_TASKS_MAX_NUM) return RES_ERROR;

  if (tx_thread_terminate(&(p_vt100_task_cbls[indx]->VT100_thread)) != TX_SUCCESS) return RES_ERROR;

  tx_thread_delete(&(p_vt100_task_cbls[indx]->VT100_thread));
  p_vt100_task_cbls[indx]->mcbl.pdrv->_deinit(&(p_vt100_task_cbls[indx]->mcbl.pdrv->pdrvcbl));

  App_free(p_vt100_task_cbls[indx]);
  App_free(p_vt100_stacks[indx]);
  vt100_allocations[indx] = 0;
  *alloc_indx = -1;
  return RES_OK;
}


/*-------------------------------------------------------------------------------------------------------------
   Задача монитора

   В initial_data содержится индекс задаче в массиве управляющих структур задач VT100
-------------------------------------------------------------------------------------------------------------*/
static void Task_VT100(ULONG arg)
{
  uint8_t    b;

  T_monitor_cbl   *monitor_cbl = (T_monitor_cbl *)(arg);

  tx_thread_identify()->environment =  (ULONG)(monitor_cbl);
  tx_thread_identify()->driver      =  (ULONG)(monitor_cbl->pdrv);

  GET_MCBL;


  time_delay(2000);


  // Очищаем экран
  MPRINTF(VT100_CLEAR_AND_HOME);

  Goto_main_menu();

  do
  {
    if (WAIT_CHAR(&b,1000) == RES_OK)
    {
      if (b != 0)
      {
        if ((b == 0x1B) && (mcbl->Monitor_func != Edit_func))
        {
          MPRINTF(VT100_CLEAR_AND_HOME);
          //Entry_check();
          Goto_main_menu();
        }
        else
        {
          if (mcbl->Monitor_func) mcbl->Monitor_func(b);  // Обработчик нажатий главного цикла
        }
      }
    }


    if (mcbl->menu_trace[mcbl->menu_nesting] == &MENU_MAIN)
    {
      unsigned int uid[3];
      uid[0] = HAL_GetUIDw0();
      uid[1] = HAL_GetUIDw1();
      uid[2] = HAL_GetUIDw2();
      VT100_set_cursor_pos(21, 0);
      MPRINTF("UID         : %08X %08X %08X", uid[0], uid[1], uid[2]);
      VT100_set_cursor_pos(22, 0);
      MPRINTF("Firmware ver: %s",SOFTWARE_VERSION);
      VT100_set_cursor_pos(23, 0);
      MPRINTF("Compile time: %s",__DATE__" "__TIME__);
      VT100_set_cursor_pos(24, 0);
      uint32_t  t;
      Get_system_ticks(&t);
      t = (t * 1000ul) / TX_TIMER_TICKS_PER_SECOND;
      MPRINTF("Up time     : %d ms",t);
    }
  }while (1);
}

/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
void Set_monitor_func(void (*func)(unsigned char))
{
  GET_MCBL;

  mcbl->Monitor_func = func;
}



/*-------------------------------------------------------------------------------------------------------------
  Вывести на экран текущее меню
-------------------------------------------------------------------------------------------------------------*/
void Display_menu(void)
{
  GET_MCBL;

  MPRINTF(VT100_CLEAR_AND_HOME);

  if (mcbl->menu_trace[mcbl->menu_nesting] == 0) return;

  // Вывод заголовка меню
  VT100_send_str_to_pos((uint8_t *)mcbl->menu_trace[mcbl->menu_nesting]->menu_header, 1, Find_str_center((uint8_t *)mcbl->menu_trace[mcbl->menu_nesting]->menu_header));
  VT100_send_str_to_pos(DASH_LINE, 2, 0);
  // Вывод строки содержимого меню
  VT100_send_str_to_pos((uint8_t *)mcbl->menu_trace[mcbl->menu_nesting]->menu_body, 3, 0);
  MPRINTF("\r\n");
  MPRINTF(DASH_LINE);

}
/*-------------------------------------------------------------------------------------------------------------
  Поиск в текущем меню пункта вызываемого передаваемым кодом
  Параметры:
    b - код команды вазывающей пункт меню
  Возвращает:
    Указатель на соответствующий пункт в текущем меню
-------------------------------------------------------------------------------------------------------------*/
int32_t Lookup_menu_item(T_VT100_Menu_item **item, uint8_t b)
{
  int16_t           i;
  GET_MCBL;

  if (isalpha(b) != 0) b = toupper(b);

  i = 0;
  do
  {
    *item = (T_VT100_Menu_item *)mcbl->menu_trace[mcbl->menu_nesting]->menu_items + i;
    if ((*item)->but == b) return (1);
    if ((*item)->but == 0) break;
    i++;
  }while (1);

  return (0);
}


/*----------------------------------------------------------------------------
 *      Line Editor
 *---------------------------------------------------------------------------*/
static int Get_string(char *lp, int n)
{
  int  cnt = 0;
  char c;
  GET_MCBL;

  do
  {
    if (WAIT_CHAR((unsigned char *)&c,  1000) == RES_OK)
    {
      switch (c)
      {
      case VT100_CNTLQ:                    // ignore Control S/Q
      case VT100_CNTLS:
        break;

      case VT100_BCKSP:
      case VT100_DEL:
        if (cnt == 0)
        {
          break;
        }
        cnt--;                             // decrement count
        lp--;                              // and line VOID*
                                           // echo backspace
        MPRINTF("\x008 \x008");
        break;
      case VT100_ESC:
        *lp = 0;                           // ESC - stop editing line
        return (RES_ERROR);
      default:
        MPRINTF("*");
        *lp = c;                           // echo and store character
        lp++;                              // increment line VOID*
        cnt++;                             // and count
        break;
      }
    }
  }while (cnt < n - 1 && c != 0x0d);       // check limit and line feed
  *lp = 0;                                 // mark end of string
  return (RES_OK);
}

/*-------------------------------------------------------------------------------------------------------------
  Ввод кода для доступа к специальному меню
-------------------------------------------------------------------------------------------------------------*/
int Enter_special_code(void)
{
  char str[32];
  GET_MCBL;

  if (mcbl->g_access_to_spec_menu != 0)
  {
    return RES_OK;
  }
  MPRINTF(VT100_CLEAR_AND_HOME"Enter access code>");
  if (Get_string(str, 31) == RES_OK)
  {
    if (strcmp(str, "4321\r") == 0)
    {
      mcbl->g_access_to_spec_menu = 1;
      return RES_OK;
    }
  }

  return RES_ERROR;
}
/*-------------------------------------------------------------------------------------------------------------
  Поиск пункта меню по коду вызова (в текущем меню)
  и выполнение соответствующей ему функции
  Параметры:
    b - код символа введенного с клавиатуры и вазывающего пункт меню

-------------------------------------------------------------------------------------------------------------*/
void Menu_press_key_handler(uint8_t b)
{
  T_VT100_Menu_item *menu_item;
  GET_MCBL;

  // Ищем запись в списке которой соответствует заданный символ в переменной b
  if (Lookup_menu_item(&menu_item, b) != 0)
  {
    // Нашли соответствующий пункт меню
    if (menu_item->psubmenu != 0)
    {
      // Если присутствует субменю, то вывести его

      if ((T_VT100_Menu *)menu_item->psubmenu == &MENU_SPEC)
      {
        if (Enter_special_code() != RES_OK)
        {
          Display_menu();
        }
      }

      mcbl->menu_nesting++;
      mcbl->menu_trace[mcbl->menu_nesting] = (T_VT100_Menu *)menu_item->psubmenu;

      Display_menu();
      // Если есть функция у пункта меню, то передать ей обработчик нажатий в главном цикле и выполнить функцию.
      if (menu_item->func != 0)
      {
        mcbl->Monitor_func = (T_menu_func)(menu_item->func); // Установить обработчик нажатий главного цикла на функцию из пункта меню
        menu_item->func(0);                                  // Выполнить саму функцию меню
      }
    }
    else
    {
      if (menu_item->func == 0)
      {
        // Если нет ни субменю ни функции, значит это пункт возврата в предыдущее меню
        // Управление остается в главном цикле у обработчика по умолчанию
        Return_to_prev_menu();
        Display_menu();
      }
      else
      {
        // Если у пункта нет своего меню, то перейти очистить экран и перейти к выполению  функции выбранного пункта
        MPRINTF(VT100_CLEAR_AND_HOME);
        menu_item->func(0);
        // Управление возвращается в главный цикл обработчику по умолчанию
        Display_menu();
      }
    }

  }
}


/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
void Goto_main_menu(void)
{
  GET_MCBL;

  mcbl->menu_nesting = 0;
  mcbl->menu_trace[mcbl->menu_nesting] = (T_VT100_Menu *)&MENU_MAIN;
  Display_menu();
  mcbl->Monitor_func = Menu_press_key_handler; // Назначение функции
}

/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
void Return_to_prev_menu(void)
{
  GET_MCBL;

  if (mcbl->menu_nesting > 0)
  {
    mcbl->menu_trace[mcbl->menu_nesting] = 0;
    mcbl->menu_nesting--;
  }
  mcbl->Monitor_func = Menu_press_key_handler; // Назначение функции
}



/*-------------------------------------------------------------------------------------------------------------
  Очистка экрана монитора
-------------------------------------------------------------------------------------------------------------*/
void VT100_clr_screen(void)
{
  GET_MCBL;
  MPRINTF(VT100_CLEAR_AND_HOME);
}

/*-------------------------------------------------------------------------------------------------------------
     Установка курсора в заданную позицию
     Счет столбцов и строк начинается с нуля
-------------------------------------------------------------------------------------------------------------*/
void VT100_set_cursor_pos(uint8_t row, uint8_t col)
{
  GET_MCBL;
  MPRINTF("\033[%.2d;%.2dH", row, col);
}

/*-------------------------------------------------------------------------------------------------------------
     Вывод строки в заданную позицию
-------------------------------------------------------------------------------------------------------------*/
void VT100_send_str_to_pos(uint8_t *str, uint8_t row, uint8_t col)
{
  GET_MCBL;
  MPRINTF("\033[%.2d;%.2dH", row, col);
  SEND_BUF(str, strlen((char *)str));
}

/*-------------------------------------------------------------------------------------------------------------
    Находим позицию начала строки для расположения ее по центру экрана
-------------------------------------------------------------------------------------------------------------*/
uint8_t Find_str_center(uint8_t *str)
{
  int16_t l = 0;
  while (*(str + l) != 0) l++; // Находим длину строки
  return (COLCOUNT - l) / 2;
}

static uint8_t BCD2ToBYTE(uint8_t val)
{
  uint32_t tmp;
  tmp =((val & 0xF0)>> 4) * 10;
  return(uint8_t)(tmp +(val & 0x0F));
}
/*-----------------------------------------------------------------------------------------------------
  Установка даты и времени
-----------------------------------------------------------------------------------------------------*/
void Do_date_time_set(uint8_t keycode)
{
  unsigned char      i, k, b;
  uint8_t            buf[EDSTLEN];
  volatile struct tm          rt_time;
  GET_MCBL;

  MPRINTF(VT100_CLEAR_AND_HOME);

  VT100_send_str_to_pos("SYSTEM TIME SETTING", 1, 30);
  VT100_send_str_to_pos("\033[5C <M> - Display main menu, <Esc> - Exit \r\n", 2, 10);
  VT100_send_str_to_pos("Print in form [YY.MM.DD HH.MM.SS]:  .  .     :  :  ", SCR_ITEMS_VERT_OFFS, 1);

  mcbl->beg_pos = 38;
  k = 0;
  mcbl->curr_pos = mcbl->beg_pos;
  VT100_set_cursor_pos(SCR_ITEMS_VERT_OFFS, mcbl->curr_pos);
  MPRINTF((char *)VT100_CURSOR_ON);

  for (i = 0; i < EDSTLEN; i++) buf[i] = 0;

  do
  {
    if (WAIT_CHAR(&b, 200) == RES_OK)
    {
      switch (b)
      {
      case VT100_BCKSP:  // Back Space
        if (mcbl->curr_pos > mcbl->beg_pos)
        {
          mcbl->curr_pos--;
          k--;
          switch (k)
          {
          case 2:
          case 5:
          case 8:
          case 11:
          case 14:
            k--;
            mcbl->curr_pos--;
            break;
          }

          VT100_set_cursor_pos(SCR_ITEMS_VERT_OFFS, mcbl->curr_pos);
          MPRINTF((char *)" ");
          VT100_set_cursor_pos(SCR_ITEMS_VERT_OFFS, mcbl->curr_pos);
          buf[k] = 0;
        }
        break;
      case VT100_DEL:  // DEL
        mcbl->curr_pos = mcbl->beg_pos;
        k = 0;
        for (i = 0; i < EDSTLEN; i++) buf[i] = 0;
        VT100_set_cursor_pos(SCR_ITEMS_VERT_OFFS, mcbl->beg_pos);
        MPRINTF((char *)"  .  .        :  :  ");
        VT100_set_cursor_pos(SCR_ITEMS_VERT_OFFS, mcbl->beg_pos);
        break;
      case VT100_ESC:  // ESC
        MPRINTF((char *)VT100_CURSOR_OFF);
        return;
      case 'M':  //
      case 'm':  //
        MPRINTF((char *)VT100_CURSOR_OFF);
        return;

      case VT100_CR:  // Enter
        MPRINTF((char *)VT100_CURSOR_OFF);

        rt_time.tm_year  = BCD2ToBYTE((buf[0] << 4)+ buf[1]) - 1900;
        rt_time.tm_mon   = BCD2ToBYTE((buf[3] << 4)+ buf[4]) - 1;
        rt_time.tm_mday  = BCD2ToBYTE((buf[6] << 4)+ buf[7]);
        rt_time.tm_hour  = BCD2ToBYTE((buf[9] << 4)+ buf[10]);
        rt_time.tm_min   = BCD2ToBYTE((buf[12] << 4)+ buf[13]);
        rt_time.tm_sec   = BCD2ToBYTE((buf[15] << 4)+ buf[16]);
        //RTC_set_system_DateTime(&rt_time);
        return;
      default:
        if (isdigit(b))
        {
          if (k < EDSTLEN)
          {
            uint8_t str[2];
            str[0] = b;
            str[1] = 0;
            MPRINTF((char *)str);
            buf[k] = b - 0x30;
            mcbl->curr_pos++;
            k++;
            switch (k)
            {
            case 2:
            case 5:
            case 8:
            case 11:
            case 14:
              k++;
              mcbl->curr_pos++;
              break;
            }
            VT100_set_cursor_pos(SCR_ITEMS_VERT_OFFS, mcbl->curr_pos);
          }
        }
        break;

      } // switch
    }
  }while (1);
}

/*------------------------------------------------------------------------------
 Вывод дампа области памяти


 \param addr       - выводимый начальный адрес дампа
 \param buf        - указатель на память
 \param buf_len    - количество байт
 \param sym_in_str - количество выводимых байт в строке дампа

 \return int32_t
 ------------------------------------------------------------------------------*/
void Mon_print_dump(uint32_t addr, void *buf, uint32_t buf_len, uint8_t sym_in_str)
{

  uint32_t   i;
  uint32_t   scnt;
  uint8_t    *pbuf;
  GET_MCBL;

  pbuf = (uint8_t *)buf;
  scnt = 0;
  for (i = 0; i < buf_len; i++)
  {
    if (scnt == 0)
    {
      MPRINTF("%08X: ", addr);
    }

    MPRINTF("%02X ", pbuf[ i ]);

    addr++;
    scnt++;
    if (scnt >= sym_in_str)
    {
      scnt = 0;
      MPRINTF("\r\n");
    }
  }

  if (scnt != 0)
  {
    MPRINTF("\r\n");
  }
}

/*-----------------------------------------------------------------------------------------------------
  Ввод строки

  \param buf      - буффер для вводимых символов
  \param buf_len  - размер буфера c учетом нулевого байта
  \param row      - строка в которой будет производится ввод
  \param instr    - строка с начальным значением

  \return int32_t - RES_OK если ввод состоялся
-----------------------------------------------------------------------------------------------------*/
int32_t Edit_string_in_pos(char *buf, int buf_len, int row, char *instr)
{

  int   indx = 0;
  uint8_t b  = 0;
  int   res;
  uint8_t bs_seq[] = { VT100_BCKSP, ' ', VT100_BCKSP, 0 };
  GET_MCBL;

  indx = 0;
  VT100_set_cursor_pos(row, 0);
  MPRINTF(VT100_CLL_FM_CRSR);
  MPRINTF(">");

  if (instr != 0)
  {
    indx = strlen(instr);
    if (indx >= (buf_len-1)) indx = buf_len-1;
    SEND_BUF(instr, indx);
    for (uint32_t n=0; n < indx; n++)
    {
      buf[n] = instr[n];
    }
  }

  do
  {
    if (WAIT_CHAR(&b,  10000) != RES_OK)
    {
      res = RES_ERROR;
      goto exit_;
    };

    if (b == VT100_BCKSP)
    {
      if (indx > 0)
      {
        indx--;
        SEND_BUF(bs_seq, sizeof(bs_seq));
      }
    }
    else if (b == VT100_ESC)
    {
      res = RES_ERROR;
      goto exit_;
    }
    else if (b != VT100_CR && b != VT100_LF && b != 0)
    {
      SEND_BUF(&b, 1);
      buf[indx] = b;           /* String[i] value set to alpha */
      indx++;
      if (indx >= buf_len)
      {
        res = RES_ERROR;
        goto exit_;
      };
    }
  }while ((b != VT100_CR) && (indx < COL));

  res = RES_OK;
  buf[indx] = 0;                     /* End of string set to NUL */
exit_:

  VT100_set_cursor_pos(row, 0);
  MPRINTF(VT100_CLL_FM_CRSR);

  return (res);
}

/*-----------------------------------------------------------------------------------------------------
  Редактирование строки

  \param buf          - буфер для редактирования строки
  \param buf_len      - размер буфера для редактирования не включая завершающий 0
  \param instr        - начальное значение строки

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
int32_t Edit_string(char *buf, uint32_t buf_len, char *instr)
{

  int       indx = 0;
  uint8_t   b;
  int       res;
  uint8_t   bs_seq[] = { VT100_BCKSP, ' ', VT100_BCKSP, 0 };
  GET_MCBL;

  indx = 0;
  MPRINTF(">");

  if (instr != 0)
  {
    indx = strlen(instr);
    if (indx >= (buf_len-1)) indx = buf_len-1;
    SEND_BUF(instr, indx);
    for (uint32_t n=0; n < indx; n++)
    {
      buf[n] = instr[n];
    }
  }

  do
  {
    if (WAIT_CHAR(&b,  10000) != RES_OK)
    {
      res = RES_ERROR;
      goto exit_;
    };

    if (b == VT100_BCKSP)
    {
      if (indx > 0)
      {
        indx--;
        SEND_BUF(bs_seq, sizeof(bs_seq));
      }
    }
    else if (b == VT100_ESC)
    {
      res = RES_ERROR;
      goto exit_;
    }
    else if (b != VT100_CR && b != VT100_LF && b != 0)
    {
      if (indx < (buf_len-1))
      {
        SEND_BUF(&b, 1);
        buf[indx] = b;
        indx++;
      };
    }
  }while ((b != VT100_CR) && (indx < COL));

  res = RES_OK;
  buf[indx] = 0;
exit_:


  return (res);
}


/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
void Edit_uinteger_val(uint32_t row, uint32_t *value, uint32_t minv, uint32_t maxv)
{
  char   str[32];
  char   buf[32];
  uint32_t tmpv;
  sprintf(str, "%d",*value);
  if (Edit_string_in_pos(buf, 31, row, str) == RES_OK)
  {
    if (sscanf(buf, "%d",&tmpv) == 1)
    {
      if (tmpv > maxv) tmpv = maxv;
      if (tmpv < minv) tmpv = minv;
      *value = tmpv;
    }
  }
}
/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
void Edit_integer_val(uint32_t row, int32_t *value, int32_t minv, int32_t maxv)
{
  char   str[32];
  char   buf[32];
  int32_t tmpv;
  sprintf(str, "%d",*value);
  if (Edit_string_in_pos(buf, 31, row, str) == RES_OK)
  {
    if (sscanf(buf, "%d",&tmpv) == 1)
    {
      if (tmpv > maxv) tmpv = maxv;
      if (tmpv < minv) tmpv = minv;
      *value = tmpv;
    }
  }
}
/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
void Edit_float_val(uint32_t row, float *value, float minv, float maxv)
{
  char   str[32];
  char   buf[32];
  float tmpv;
  sprintf(str, "%f",*value);
  if (Edit_string_in_pos(buf, 31, row, str) == RES_OK)
  {
    if (sscanf(buf, "%f",&tmpv) == 1)
    {
      if (tmpv > maxv) tmpv = maxv;
      if (tmpv < minv) tmpv = minv;
      *value = tmpv;
    }
  }
}

/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
static void Do_show_event_log(uint8_t keycode)
{
  AppLogg_monitor_output();
}


/*-----------------------------------------------------------------------------------------------------


  \param keycode
-----------------------------------------------------------------------------------------------------*/
static void Do_FileX_block_rw_test(uint8_t keycode)
{
  File_Read_Write_test();
}

/*-----------------------------------------------------------------------------------------------------


  \param keycode
-----------------------------------------------------------------------------------------------------*/
static void Do_FileX_test(uint8_t keycode)
{
  FileX_test();
}

/*------------------------------------------------------------------------------
 Сброс системы
 ------------------------------------------------------------------------------*/
static void Do_Reset(uint8_t keycode)
{
  Reset_system();
}


