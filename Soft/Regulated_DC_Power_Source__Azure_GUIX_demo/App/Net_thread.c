// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2021-11-07
// 15:40:27
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"



TX_THREAD                     net_thread;
#pragma data_alignment=8
uint8_t                       thread_net_stack[THREAD_NET_STACK_SIZE];

static TX_EVENT_FLAGS_GROUP   net_flag;

static void Thread_net(ULONG initial_input);

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Thread_net_create(void)
{
  tx_thread_create(&net_thread, "Net", Thread_net,
    0,
    (void *)thread_net_stack, // stack_start
    THREAD_NET_STACK_SIZE,   // stack_size
    THREAD_NET_PRIORITY,     // priority. Numerical priority of thread. Legal values range from 0 through (TX_MAX_PRIORITES-1), where a value of 0 represents the highest priority.
    THREAD_NET_PRIORITY,     // preempt_threshold. Highest priority level (0 through (TX_MAX_PRIORITIES-1)) of disabled preemption. Only priorities higher than this level are allowed to preempt this thread. This value must be less than or equal to the specified priority. A value equal to the thread priority disables preemption-threshold.
    TX_NO_TIME_SLICE,
    TX_AUTO_START);
}

/*-----------------------------------------------------------------------------------------------------


  \param voi
-----------------------------------------------------------------------------------------------------*/
static void Net_task_abnormal_stop(void)
{
  APPLOG("Net task stopped.");
  do
  {
    time_delay(100);
  } while (1);
}


/*-----------------------------------------------------------------------------------------------------

  \param msg

  \return UINT
-----------------------------------------------------------------------------------------------------*/
UINT  Send_event_to_Net_task(uint32_t event_flag)
{
  return  tx_event_flags_set(&net_flag, event_flag, TX_OR);
}
/*-----------------------------------------------------------------------------------------------------


  \param initial_input
-----------------------------------------------------------------------------------------------------*/
static void Thread_net(ULONG initial_input)
{
  UINT      res;
  ULONG     actual_flags;

  res = tx_event_flags_create(&net_flag, "Task_Net");
  if  (res != TX_SUCCESS)
  {
    APPLOG("Event creation error %d.", res);
    Net_task_abnormal_stop();
  }

  Init_Net();
  RNDIS_init_network_stack();

  do
  {
    // Ожидаем сообщения 10 мс
    res =  tx_event_flags_get(&net_flag, 0XFFFFFFFF, TX_OR_CLEAR,&actual_flags,  MS_TO_TICKS(10));
    if (res == TX_SUCCESS)
    {
     // if (actual_flags & EVT_MQTT_MSG)
     // {
     //   // Отработка сообщения от MQTT брокера
     //   MQTTMC_messages_processor();
     // }
    }
    else
    {
      RNDIS_interface_controller();
      //HTTP_server_controller();
    }

  } while (1);

}

