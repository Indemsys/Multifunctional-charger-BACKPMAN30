// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2020-11-10
// 18:58:24
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"


extern volatile ULONG    _tx_timer_system_clock;



/*-----------------------------------------------------------------------------------------------------


  \param time_ms

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t ms_to_ticks(uint32_t time_ms)
{
  return (((time_ms * TX_TIMER_TICKS_PER_SECOND) / 1000U)+ 1U);
}


/*-----------------------------------------------------------------------------------------------------


  \param ms

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t time_delay(uint32_t ms)
{
  return tx_thread_sleep(MS_TO_TICKS(ms));
}


/*-----------------------------------------------------------------------------------------------------


  \param ms

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Wait_ms(uint32_t ms)
{
  return time_delay(ms);
}


/*-----------------------------------------------------------------------------------------------------
  Зозвращает значение системного таймера (в количестве тиков)

  \param v
-----------------------------------------------------------------------------------------------------*/
uint32_t  Get_system_ticks(uint32_t *v)
{
  uint32_t t;
  t = tx_time_get();
  if (v != 0) *v = t;
  return t;
}

/*-----------------------------------------------------------------------------------------------------


  \param st
-----------------------------------------------------------------------------------------------------*/
void Get_hw_timestump(T_sys_timestump *pst)
{
  uint32_t scy;
  ULONG    scl1;
  ULONG    scl2;

  scl1 = _tx_timer_system_clock;
  scy  = SysTick->VAL;
  scl2 = _tx_timer_system_clock;
  if (scl1 != scl2)
  {
    pst->cycles  = SysTick->VAL;
    pst->ticks   = scl2;
  }
  else
  {
    pst->cycles  = scy;
    pst->ticks   = scl1;
  }
}


/*-----------------------------------------------------------------------------------------------------


  \param start_time_val
  \param stop_time_val

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Hw_timestump_diff32_us(T_sys_timestump *p_begin, T_sys_timestump *p_end)
{
  return (uint32_t)Hw_timestump_diff64_us(p_begin,p_end);
}

/*-----------------------------------------------------------------------------------------------------


  \param psart
  \param pend

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint64_t Hw_timestump_diff64_us(T_sys_timestump *p_begin, T_sys_timestump *p_end)
{
  uint64_t  val;
  int32_t   tmp;
  int32_t   uc_in_tick;
  uint32_t  c1;
  uint32_t  c2;
  uint32_t  load_val;

  load_val = SysTick->LOAD+1;  // Получаем делитель системной частоты для получения системного тика

  c1 = load_val - p_begin->cycles -1;
  c2 = load_val - p_end->cycles - 1;
  uc_in_tick = 1000000ul / TX_TIMER_TICKS_PER_SECOND;
  val = (uint64_t)(p_end->ticks - p_begin->ticks) * uc_in_tick;

  if (c2 >= c1)
  {
    tmp =(c2 - c1) / (SYSTEM_CLOCK / 1000000ul);
    val += (uint64_t)tmp;
  }
  else
  {
    tmp =(load_val -(c1 - c2)) / (SYSTEM_CLOCK / 1000000ul);
    val = val - (uint64_t)uc_in_tick +  (uint64_t)tmp;
  }
  return val;
}

/*-----------------------------------------------------------------------------------------------------


  \param p_time

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Time_elapsed_sec(T_sys_timestump *p_time)
{
  uint64_t secs;
  T_sys_timestump now;
  Get_hw_timestump(&now);
  secs = Hw_timestump_diff64_us(p_time, &now)/1000000ull;
  return secs;
}

/*-----------------------------------------------------------------------------------------------------


  \param p_time

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Time_elapsed_msec(T_sys_timestump *p_time)
{
  uint64_t msecs;
  T_sys_timestump now;
  Get_hw_timestump(&now);
  msecs = Hw_timestump_diff64_us(p_time, &now)/1000ull;
  return msecs;
}

/*-----------------------------------------------------------------------------------------------------
  Разница во времени в секундах
  Аргументы выражаются в количестве тиков

  \param start_time_val
  \param stop_time_val

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Time_diff_seconds(uint32_t start_time_val, uint32_t stop_time_val)
{
  return (stop_time_val - start_time_val) / TX_TIMER_TICKS_PER_SECOND;
}

/*-----------------------------------------------------------------------------------------------------


  \param p_time
  \param sec
  \param usec

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
void Timestump_convert_to_sec_usec(T_sys_timestump *p_timestump, uint32_t *sec, uint32_t *usec)
{
  *sec  = p_timestump->ticks/TX_TIMER_TICKS_PER_SECOND;
  *usec = (p_timestump->ticks % TX_TIMER_TICKS_PER_SECOND)*(1000000ul/TX_TIMER_TICKS_PER_SECOND) + p_timestump->cycles/(SYSTEM_CLOCK/1000000ul);
}

