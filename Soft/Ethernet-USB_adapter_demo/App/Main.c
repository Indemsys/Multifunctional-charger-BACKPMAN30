#include "App.h"

WVAR_TYPE    wvar;

#pragma data_alignment=8
uint8_t                 thread_main_stack[THREAD_MAIN_STACK_SIZE] @ "DTCM";
TX_THREAD               main_thread;


/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
int main(void)
{
  SCB_EnableICache();
  SCB_EnableDCache();
  BACKPMAN3_Memory_init(); // Объявляем область используемую DMA ADC как некэшируемую

  HAL_Init();
  SystemClock_Config();
  PeriphCommonClock_Config();

  Backpman3_init_pins();


  //RTC_Init();

  tx_kernel_enter();
  while (1)
  {
  }

}

/*-----------------------------------------------------------------------------------------------------


  \param first_unused_memory
-----------------------------------------------------------------------------------------------------*/
VOID tx_application_define(VOID *first_unused_memory)
{
  App_create_mem_pools(first_unused_memory);

  json_set_alloc_funcs(App_malloc, App_free); // Назначаем парсеру JSON функции работы с динамической памятью


  tx_thread_create(&main_thread, "Main", Thread_main,
    0,
    (void *)thread_main_stack, // stack_start
    THREAD_MAIN_STACK_SIZE,    // stack_size

    THREAD_MAIN_PRIORITY,     // priority.
                              // Numerical priority of thread.
                              // Legal values range from 0 through (TX_MAX_PRIORITIES-1), where a value of 0 represents the highest priority.

    THREAD_MAIN_PRIORITY,     // preempt_threshold.
                              // Highest priority level (0 through (TX_MAX_PRIORITIES-1)) of disabled preemption.
                              // Only priorities higher than this level are allowed to preempt this thread.
                              // This value must be less than or equal to the specified priority.
                              // A value equal to the thread priority disables preemption-threshold.
    TX_NO_TIME_SLICE,
    TX_AUTO_START);
}



