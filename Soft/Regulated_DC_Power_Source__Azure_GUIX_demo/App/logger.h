#ifndef __APP_LOGGER
  #define __App_LOGGER


  #define     EVENT_LOG_SIZE         256 // Должно быть степенью 2
  #define     EVENT_LOG_DISPLAY_ROW  32  // Количество строк лога выводимых на экран при автообновлении


  #define     EVNT_LOG_FNAME_SZ      64

  #define     LOG_STR_MAX_SZ         256
  #define     FILE_LOG_MSG_CNT       64

// Определения статуса записей для логгера приложения
  #define     SEVERITY_RED             0
  #define     SEVERITY_GREEN           1
  #define     SEVERITY_YELLOW          2
  #define     SEVERITY_DEFAULT         4



// Структура хранения записи лога
typedef struct
{
    T_sys_timestump  time;
    char             msg[LOG_STR_MAX_SZ+1];
    char             func_name[EVNT_LOG_FNAME_SZ+1];
    uint32_t         line_num;
    uint32_t         severity;
} T_app_log_record;


uint32_t    AppLogg_init(void);

void         LOG(const char *str, const char *name, unsigned int line_num, unsigned int severity);
void         LOG1(const char *str, const char *name, unsigned int line_num, unsigned int severity, uint32_t v1);
void         LOG2(const char *str, const char *name, unsigned int line_num, unsigned int severity, uint32_t v1, uint32_t v2);
void         LOG3(const char *str, const char *name, unsigned int line_num, unsigned int severity, uint32_t v1, uint32_t v2, uint32_t v3);
void         LOGs(const char *name, unsigned int line_num, unsigned int severity, const char *fmt_ptr, ...);
void         AppLogg_monitor_output(void);
int32_t      AppLog_get_tail_record(T_app_log_record *rec);

#define      APPLOG(...)    LOGs(__FUNCTION__, __LINE__, SEVERITY_RED, ##__VA_ARGS__);

#endif
