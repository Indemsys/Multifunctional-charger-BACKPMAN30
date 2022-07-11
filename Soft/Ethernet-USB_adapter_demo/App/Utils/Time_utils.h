#ifndef APP_TIME_UTILS_H
  #define APP_TIME_UTILS_H

  #define  SEC_TO_TICKS(x)  (x * TX_TIMER_TICKS_PER_SECOND)
  #define  MS_TO_TICKS(x)  (((x * TX_TIMER_TICKS_PER_SECOND) / 1000U) + 1U)
  #define  TICKS_TO_SEC(x) ((float)x/(float)TX_TIMER_TICKS_PER_SECOND)

typedef struct
{
    uint32_t usec;
    uint32_t sec;

} TIME_STRUCT;


typedef struct
{
    uint32_t cycles;
    uint32_t ticks;

} T_sys_timestump;

uint32_t  time_delay(uint32_t ms);
uint32_t  Wait_ms(uint32_t ms);
uint32_t  Get_system_ticks(uint32_t *v);
void      Get_hw_timestump(T_sys_timestump *pst);
uint64_t  Hw_timestump_diff64_us(T_sys_timestump *p_begin, T_sys_timestump *p_end);
uint32_t  Time_elapsed_sec(T_sys_timestump *p_time);
uint32_t  Time_elapsed_msec(T_sys_timestump *p_time);
void      Timestump_convert_to_sec_usec(T_sys_timestump *p_timestump, uint32_t *sec, uint32_t *usec);

#endif // APP_TIME_UTILS_H



