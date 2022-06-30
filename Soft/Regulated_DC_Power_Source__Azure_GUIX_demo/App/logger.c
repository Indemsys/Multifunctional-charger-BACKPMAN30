#include "App.h"


T_app_log_record app_log[EVENT_LOG_SIZE];

typedef struct
{
  // Переменные лога событий
  volatile int32_t event_log_head;
  volatile int32_t event_log_tail;

  unsigned int sync_err;
  unsigned int overl_err;

} T_app_log_cbl;


static T_app_log_cbl log_cbl;
static TX_MUTEX log_mutex;

/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
uint32_t AppLogg_init(void)
{
  if (tx_mutex_create(&log_mutex, "LOG", TX_INHERIT) != TX_SUCCESS)
  { return RES_ERROR; }
  return RES_OK;
}

/*------------------------------------------------------------------------------
  Запись сообщения в таблицу лога
  Функция реентерабельная для мультипоточной среды


 \param str         : сообщение
 \param func_name   : имя функции
 \param line_num    : номер строки
 \param severity    : важность сообщения
 ------------------------------------------------------------------------------*/
static void _applog_write(char *str, const char *func_name, unsigned int line_num, unsigned int severity)
{
  int32_t head;
  int32_t tail;
#ifdef APP_TO_RTT_LOG
  char            rtt_log_str[LOG_STR_MAX_SZ + 1];
#endif


  if (tx_mutex_get(&log_mutex, 1) == TX_SUCCESS)
  {
    head = log_cbl.event_log_head;
    Get_hw_timestump(&app_log[head].time);
    strncpy(app_log[head].msg, str, LOG_STR_MAX_SZ - 1);
    strncpy(app_log[head].func_name, func_name, EVNT_LOG_FNAME_SZ - 1);
    app_log[head].line_num = line_num;
    app_log[head].severity = severity;
    // Сдвигаем указатель головы лога
    head++;
    if (head >= EVENT_LOG_SIZE)
    { head = 0; }
    log_cbl.event_log_head = head;

    tail = log_cbl.event_log_tail;
    // Если голова достигла хвоста, то сдвигает указатель хвоста
    if (head == tail)
    {
      tail++;
      if (tail >= EVENT_LOG_SIZE)
      { tail = 0; }
      log_cbl.event_log_tail = tail;
      log_cbl.overl_err++;
    }


#ifdef APP_TO_RTT_LOG
      {
        snprintf(rtt_log_str, LOG_STR_MAX_SZ, "%06d.%06d %s (%s %d)\r\n",
                 app_log[head].time.sec, app_log[head].time.usec,
                 app_log[head].msg,
                 app_log[head].func_name,
                 app_log[head].line_num);

        SEGGER_RTT_WriteString(0, rtt_log_str);
      }
#endif
    tx_mutex_put(&log_mutex);
  }
  else
  {
    log_cbl.sync_err++;
  }
}

/*------------------------------------------------------------------------------



 \param str
 \param name
 \param line_num
 \param severity
 ------------------------------------------------------------------------------*/
void LOG(const char *str, const char *name, unsigned int line_num, unsigned int severity)
{
  char *s;
  s = App_malloc(LOG_STR_MAX_SZ);
  if (s != NULL)
  {
    snprintf(s, LOG_STR_MAX_SZ - 1, "%s", str);
    _applog_write(s, name, line_num, severity);
    App_free(s);
  }
}

/*------------------------------------------------------------------------------



 \param str
 \param name
 \param line_num
 \param severity
 \param v1
 ------------------------------------------------------------------------------*/
void LOG1(const char *str, const char *name, unsigned int line_num, unsigned int severity, uint32_t v1)
{
  char *s;
  s = App_malloc(LOG_STR_MAX_SZ);
  if (s != NULL)
  {
    snprintf(s, LOG_STR_MAX_SZ - 1, str, v1);
    _applog_write(s, name, line_num, severity);
    App_free(s);
  }
}

/*------------------------------------------------------------------------------



 \param str
 \param name
 \param line_num
 \param severity
 \param v1
 \param v2
 ------------------------------------------------------------------------------*/
void LOG2(const char *str, const char *name, unsigned int line_num, unsigned int severity, uint32_t v1, uint32_t v2)
{
  char *s;
  s = App_malloc(LOG_STR_MAX_SZ);
  if (s != NULL)
  {
    snprintf(s, LOG_STR_MAX_SZ - 1, str, v1, v2);
    _applog_write(s, name, line_num, severity);
    App_free(s);
  }
}

/*------------------------------------------------------------------------------



 \param str
 \param name
 \param line_num
 \param severity
 \param v1
 \param v2
 \param v3
 ------------------------------------------------------------------------------*/
void LOG3(const char *str, const char *name, unsigned int line_num, unsigned int severity, uint32_t v1, uint32_t v2, uint32_t v3)
{
  char *s;
  s = App_malloc(LOG_STR_MAX_SZ);
  if (s != NULL)
  {
    snprintf(s, LOG_STR_MAX_SZ - 1, str, v1, v2, v3);
    _applog_write(s, name, line_num, severity);
    App_free(s);
  }
}


/*------------------------------------------------------------------------------



 \param name
 \param line_num
 \param severity
 \param fmt_ptr
 ------------------------------------------------------------------------------*/
void LOGs(const char *name, unsigned int line_num, unsigned int severity, const char *fmt_ptr, ...)
{
  char *s;
  va_list ap;

  s = App_malloc(LOG_STR_MAX_SZ);
  if (s != NULL)
  {
    va_start(ap, fmt_ptr);
    vsnprintf(s, LOG_STR_MAX_SZ - 1, (char *) fmt_ptr, ap);
    va_end(ap);
    _applog_write(s, name, line_num, severity);
    App_free(s);
  }
}

/*------------------------------------------------------------------------------
  Получить структуру записи лога от хвоста
  Возвращает 0 если записей в логе нет

 \param rec

 \return int
 ------------------------------------------------------------------------------*/
int32_t AppLog_get_tail_record(T_app_log_record *rec)
{
  int32_t res = 0;
  uint32_t tail;
  tx_mutex_get(&log_mutex, 10);

  tail = log_cbl.event_log_tail;
  if (log_cbl.event_log_head != tail)
  {
    memcpy(rec, &app_log[log_cbl.event_log_tail], sizeof(T_app_log_record));
    log_cbl.event_log_tail++;
    if (log_cbl.event_log_tail >= EVENT_LOG_SIZE)
    { log_cbl.event_log_tail = 0; }
    res = 1;
  }

  tx_mutex_put(&log_mutex);
  return res;
}

/*-----------------------------------------------------------------------------------------------------
  Вывод всех строк лога от хвоста до головы и продолжение вывода при появлении новых строк

  \param mcbl
-----------------------------------------------------------------------------------------------------*/
void AppLogg_monitor_output(void)
{
  uint32_t idx;
  uint8_t b;

  GET_MCBL
  MPRINTF(VT100_CLEAR_AND_HOME);
  MPRINTF("Events log. <R> - exit. (Overfl.n.= %d, Skipped n. = %d)\n\r", log_cbl.overl_err, log_cbl.sync_err);
  MPRINTF("--------------------------------------------\n\r");
  idx = log_cbl.event_log_tail;

  do
  {
    if (WAIT_CHAR(&b, 200) == RES_OK)
    {
      switch (b)
      {
        case 'R':
        case 'r':
        case VT100_ESC:
          return;
        default:
          return;
      }
    }
    while (idx != log_cbl.event_log_head)
    {
      uint32_t sec, usec;
      Timestump_convert_to_sec_usec(&app_log[idx].time, &sec, &usec);
      if (app_log[idx].line_num != 0)
      {
        MPRINTF(VT100_CLL_FM_CRSR"%06d.%06d %s (%s %d)\n\r",
                sec, usec,
                app_log[idx].msg,
                app_log[idx].func_name,
                app_log[idx].line_num);
      }
      else
      {
        MPRINTF(VT100_CLL_FM_CRSR"%06d.%06d %s\n\r",
                sec, usec,
                app_log[idx].msg);
      }
      idx++;
      if (idx >= EVENT_LOG_SIZE)
      { idx = 0; }
    }

  } while (1);
}



