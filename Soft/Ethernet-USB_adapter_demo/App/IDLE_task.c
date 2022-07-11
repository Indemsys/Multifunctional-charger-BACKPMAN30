#include   "App.h"

TX_THREAD                     idle_thread;
#pragma data_alignment=8
uint8_t                       thread_idle_stack[THREAD_IDLE_STACK_SIZE];


uint64_t                      ref_time;             // Калибровочная константа предназначенная для измерения нагрузки микропроцессора
volatile uint32_t             g_cpu_usage;
volatile float                g_cpu_usage_fp;

static   uint8_t              sffs_test_request;
static   uint8_t              sffs_test_delete;
/*-------------------------------------------------------------------------------------------------------------
  Фоновая задача.
  Измеряет загруженность процессора
-------------------------------------------------------------------------------------------------------------*/
static void Thread_idle(ULONG initial_input)
{
  uint64_t  t;
  uint64_t  dt;

  for (;;)
  {
    t = Measure_reference_time_interval(REF_TIME_INTERVAL);

    if (t < ref_time)
    {
      dt = 0;
    }
    else
    {
      dt = t - ref_time;
    }
    g_cpu_usage =(1000ull * dt) / ref_time;
    g_cpu_usage_fp = (float)dt*100.0f/(float)ref_time;

    if (sffs_test_request != 0)
    {
      Thread_STfs_test_create(0);
      sffs_test_request = 0;
    }

    if (sffs_test_delete != 0)
    {
      Thread_STfs_test_delete();
      sffs_test_delete = 0;
    }
    //    if (request_save_to_NV_mem)
    //    {
    //      request_save_to_NV_mem =0;
    //      Save_Params_to_NV_store();
    //    }
    //    Wdog_refresh();

  }
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Thread_idle_create(void)
{
  tx_thread_create(&idle_thread, "IDLE", Thread_idle,
    0,
    (void *)thread_idle_stack, // stack_start
    THREAD_IDLE_STACK_SIZE,   // stack_size
    THREAD_IDLE_PRIORITY,     // priority. Numerical priority of thread. Legal values range from 0 through (TX_MAX_PRIORITES-1), where a value of 0 represents the highest priority.
    THREAD_IDLE_PRIORITY,     // preempt_threshold. Highest priority level (0 through (TX_MAX_PRIORITIES-1)) of disabled preemption. Only priorities higher than this level are allowed to preempt this thread. This value must be less than or equal to the specified priority. A value equal to the thread priority disables preemption-threshold.
    TX_NO_TIME_SLICE,
    TX_AUTO_START);
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void  Send_cmd_execute_sffs_test(void)
{
  sffs_test_request = 1;
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void  Send_cmd_delete_STfs_test(void)
{
  sffs_test_delete = 1;
}
