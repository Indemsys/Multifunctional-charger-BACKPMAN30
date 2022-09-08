// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2021-11-15
// 16:41:18
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"

extern FDCAN_HandleTypeDef    hfdcan;

TX_THREAD                     can_thread;
#pragma data_alignment=8
uint8_t                       thread_CAN_stack[THREAD_CAN_STACK_SIZE] @ "DTCM";


static void Thread_CAN(ULONG initial_input);

T_can_stat                 can_stat; // Структура для сбора диагностики по принимаемым пакетам
T_can_land_cmd             g_can_cmd;
/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Thread_CAN_create(void)
{
  tx_thread_create(&can_thread, "CAN", Thread_CAN,
    0,
    (void *)thread_CAN_stack, // stack_start
    THREAD_CAN_STACK_SIZE,   // stack_size
    THREAD_CAN_PRIORITY,     // priority. Numerical priority of thread. Legal values range from 0 through (TX_MAX_PRIORITES-1), where a value of 0 represents the highest priority.
    THREAD_CAN_PRIORITY,     // preempt_threshold. Highest priority level (0 through (TX_MAX_PRIORITIES-1)) of disabled preemption. Only priorities higher than this level are allowed to preempt this thread. This value must be less than or equal to the specified priority. A value equal to the thread priority disables preemption-threshold.
    TX_NO_TIME_SLICE,
    TX_AUTO_START);
}

/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
uint32_t CAN_send_packet(uint32_t canid, uint8_t *data, uint8_t len, uint8_t rtr)
{
  FDCAN_TxHeaderTypeDef    TxHeader;

  TxHeader.Identifier          = canid;
  TxHeader.IdType              = FDCAN_EXTENDED_ID;
  if (rtr)
  {
    TxHeader.TxFrameType         = FDCAN_REMOTE_FRAME;
  }
  else
  {
    TxHeader.TxFrameType         = FDCAN_DATA_FRAME;
  }
  switch (len)
  {
  case 0:
    TxHeader.DataLength          = FDCAN_DLC_BYTES_0;
    break;
  case 1:
    TxHeader.DataLength          = FDCAN_DLC_BYTES_1;
    break;
  case 2:
    TxHeader.DataLength          = FDCAN_DLC_BYTES_2;
    break;
  case 3:
    TxHeader.DataLength          = FDCAN_DLC_BYTES_3;
    break;
  case 4:
    TxHeader.DataLength          = FDCAN_DLC_BYTES_4;
    break;
  case 5:
    TxHeader.DataLength          = FDCAN_DLC_BYTES_5;
    break;
  case 6:
    TxHeader.DataLength          = FDCAN_DLC_BYTES_6;
    break;
  case 7:
    TxHeader.DataLength          = FDCAN_DLC_BYTES_7;
    break;
  case 8:
  default:
    TxHeader.DataLength          = FDCAN_DLC_BYTES_8;
    break;
  }
  TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
  TxHeader.BitRateSwitch       = FDCAN_BRS_OFF;
  TxHeader.FDFormat            = FDCAN_CLASSIC_CAN;
  TxHeader.TxEventFifoControl  = FDCAN_NO_TX_EVENTS;
  TxHeader.MessageMarker       = 0;

  if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan,&TxHeader,data) != HAL_OK)
  {
    return RES_ERROR;
  }

  return RES_OK;
}


/*-----------------------------------------------------------------------------------------------------
  Функция для диагностики и сбора статистики

  \param rx_cbl
  \param rx_data
-----------------------------------------------------------------------------------------------------*/
static void Diagnostic_manager(FDCAN_RxHeaderTypeDef  *rx_cbl, uint8_t *rx_data)
{
  can_stat.received_msg_cnt++;

  uint32_t len = rx_cbl->DataLength >> 16;
  if (len > 8) len = 8;
  uint32_t n = can_stat.msg_buf_pos;

  can_stat.msg_buf[n].pid = rx_cbl->Identifier;
  can_stat.msg_buf[n].len = len;


  memset(can_stat.msg_buf[n].msg, 0, 8);
  memcpy(can_stat.msg_buf[n].msg, rx_data, len);
  n++;
  if (n >= CAN_MSG_BUF_LEN) n = 0;
  can_stat.msg_buf_pos = n;
}

/*-----------------------------------------------------------------------------------------------------
  Функция реализованная для тестирования работы CAN интерфеса в сети системы SB200M

  \param void
-----------------------------------------------------------------------------------------------------*/
void Test_CAN_communication(void)
{
  FDCAN_RxHeaderTypeDef    rx_cbl;
  uint8_t                  rx_data[8];
  uint32_t                 k = 0;
  T_sys_timestump          t1;
  T_sys_timestump          t2;

  Get_hw_timestump(&t1);
  while (1)
  {
    if (Wait_CAN_message(10) == RES_OK)
    {
      HAL_FDCAN_GetRxMessage(&hfdcan, FDCAN_RX_FIFO0,&rx_cbl, rx_data);

      // Запись статистики
      Diagnostic_manager(&rx_cbl,rx_data);

      // Периодическая отправка тестового пакета на LED дисплей с кодом символа
      Get_hw_timestump(&t2);
      if (Hw_timestump_diff64_us(&t1,&t2) > 250000)
      {
        memcpy(&t1,&t2, sizeof(t1));
        rx_data[0]= 1;
        rx_data[1]= k;
        rx_data[2]= 2;
        CAN_send_packet(0x1E02FFFF,rx_data, 3, 0);
        k++;
        if (k >= 10) k = 0;
      }
    }
  }

}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void CAN_Commands_manager(void)
{
  FDCAN_RxHeaderTypeDef    rx_cbl;
  uint8_t                  rx_data[8];


  while (1)
  {
    if (Wait_CAN_message(10) == RES_OK)
    {
      HAL_FDCAN_GetRxMessage(&hfdcan, FDCAN_RX_FIFO0,&rx_cbl, rx_data);

      // Запись статистики по принятым пакетам
      Diagnostic_manager(&rx_cbl,rx_data);

      Set_output_can_active_blink(OUTP_LED_CAN);

      switch (rx_cbl.Identifier)
      {
      case REQUEST_STATE_TO_ALL:
        break;
      default:
        break;

      }

    }
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param initial_input
-----------------------------------------------------------------------------------------------------*/
static void Thread_CAN(ULONG initial_input)
{
  BACKPMAN3_CAN_init();
  CAN_Commands_manager();
}

