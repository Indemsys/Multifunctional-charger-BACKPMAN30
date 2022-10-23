// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2021-11-11
// 12:24:10
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"
#include   "stm32h7xx_hal_fdcan.h"

FDCAN_HandleTypeDef           hfdcan;
FDCAN_FilterTypeDef           sFilterConfig;

static TX_EVENT_FLAGS_GROUP   can_flag;

#define    CAN_MSG_RECEIVED   BIT(0)

static void _HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs);




/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void BACKPMAN3_CAN_init(void)
{

  tx_event_flags_create(&can_flag, "CAN_rx");

  //  Тактирование модуля CAN от 60 МГц
  //
  //   Bit time configuration:
  //
  //   Bit time parameter         | Nominal
  //   ---------------------------|--------------
  //   fdcan_ker_ck               | 10  (60/6)
  //   Time_quantum (tq)          | 111.11  ns
  //   Synchronization_segment    | 1  tq
  //   Propagation_segment        | 7 tq
  //   Phase_segment_1            | 5  tq
  //   Phase_segment_2            | 5  tq
  //   Synchronization_Jump_width | 4  tq
  //   Bit_length                 | 40 tq = 2 µs
  //   Bit_rate                   | 0.555555 MBit/s


  hfdcan.Instance                  = FDCAN1;
  hfdcan.Init.FrameFormat          = FDCAN_FRAME_CLASSIC;
  hfdcan.Init.Mode                 = FDCAN_MODE_NORMAL;
  hfdcan.Init.AutoRetransmission   = ENABLE;
  hfdcan.Init.TransmitPause        = DISABLE;
  hfdcan.Init.ProtocolException    = ENABLE;
  hfdcan.Init.NominalPrescaler     = 6;  /* tq = NominalPrescaler x (1/fdcan_ker_ck) */
  hfdcan.Init.NominalSyncJumpWidth = 4;
  hfdcan.Init.NominalTimeSeg1      = 12; /* NominalTimeSeg1 = Propagation_segment + Phase_segment_1 */
  hfdcan.Init.NominalTimeSeg2      = 5;

  hfdcan.Init.DataPrescaler        = 6;
  hfdcan.Init.DataSyncJumpWidth    = 4;
  hfdcan.Init.DataTimeSeg1         = 12; /* DataTimeSeg1 = Propagation_segment + Phase_segment_1 */
  hfdcan.Init.DataTimeSeg2         = 5;

  hfdcan.Init.MessageRAMOffset     = 0;
  hfdcan.Init.StdFiltersNbr        = 1; // Применяем один стандартный 11-битный фильтр. Допускается до 128 фильтров
  hfdcan.Init.ExtFiltersNbr        = 1; // Применяем один расширенный 29-битный фильтр. Допускается до 64 фильтров
  hfdcan.Init.RxFifo0ElmtsNbr      = 2;
  hfdcan.Init.RxFifo0ElmtSize      = FDCAN_DATA_BYTES_8;
  hfdcan.Init.RxFifo1ElmtsNbr      = 0;
  hfdcan.Init.RxBuffersNbr         = 0;
  hfdcan.Init.TxEventsNbr          = 0;
  hfdcan.Init.TxBuffersNbr         = 0;
  hfdcan.Init.TxFifoQueueElmtsNbr  = 32;
  hfdcan.Init.TxFifoQueueMode      = FDCAN_TX_FIFO_OPERATION;
  hfdcan.Init.TxElmtSize           = FDCAN_DATA_BYTES_8;
  HAL_FDCAN_Init(&hfdcan);


  // Конфигурируем фильт для стандартного 11-битного идентификатора
  sFilterConfig.IdType             = FDCAN_STANDARD_ID;
  sFilterConfig.FilterIndex        = 0;
  sFilterConfig.FilterType         = FDCAN_FILTER_MASK;  // Определяем значение полей как: FilterID1 = filter, FilterID2 = mask
  sFilterConfig.FilterConfig       = FDCAN_FILTER_TO_RXFIFO0;
  sFilterConfig.FilterID1          = 0;
  sFilterConfig.FilterID2          = 0;
  HAL_FDCAN_ConfigFilter(&hfdcan,&sFilterConfig);


  // Конфигурируем фильт для расширенного 29-битного идентификатора

  sFilterConfig.IdType             = FDCAN_EXTENDED_ID;
  sFilterConfig.FilterIndex        = 0;
  sFilterConfig.FilterType         = FDCAN_FILTER_MASK; // Определяем значение полей как: FilterID1 = filter, FilterID2 = mask
  sFilterConfig.FilterConfig       = FDCAN_FILTER_TO_RXFIFO0;
  sFilterConfig.FilterID1          = 1;          // Фильтр. Биты фильтра должны совпадать с битами принятого идентификатора CAN сообщения если эти быты незамаскированы в регитре маски
  sFilterConfig.FilterID2          = 0x1FFFFFFF; // Маска. 1 - означает необходимость соответствия бита идентификатора биту фильтра, 0 - означает отсутствие проверки соответсвия бита
  HAL_FDCAN_ConfigFilter(&hfdcan,&sFilterConfig);

  /* Configure global filter to reject all non-matching frames */
  HAL_FDCAN_ConfigGlobalFilter(&hfdcan, FDCAN_REJECT, FDCAN_REJECT, FDCAN_REJECT_REMOTE, FDCAN_REJECT_REMOTE);

  /* Configure Rx FIFO 0 watermark to 2 */
  HAL_FDCAN_ConfigFifoWatermark(&hfdcan, FDCAN_CFG_RX_FIFO0, 2);

  HAL_FDCAN_RegisterRxFifo0Callback(&hfdcan, _HAL_FDCAN_RxFifo0Callback);
  /* Activate Rx FIFO 0 watermark notification */
  HAL_FDCAN_ActivateNotification(&hfdcan, FDCAN_IT_RX_FIFO0_WATERMARK, 0);

  HAL_FDCAN_Start(&hfdcan);

}

/*-----------------------------------------------------------------------------------------------------


  \param fdcanHandle
-----------------------------------------------------------------------------------------------------*/
void HAL_FDCAN_MspInit(FDCAN_HandleTypeDef *fdcanHandle)
{
  if (fdcanHandle->Instance == FDCAN1)
  {
    __HAL_RCC_FDCAN_CLK_ENABLE();

    HAL_NVIC_SetPriority(FDCAN1_IT0_IRQn, 0, 1);
    HAL_NVIC_SetPriority(FDCAN1_IT1_IRQn, 0, 1);
    HAL_NVIC_SetPriority(FDCAN_CAL_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(FDCAN1_IT0_IRQn);
    HAL_NVIC_EnableIRQ(FDCAN1_IT1_IRQn);
    HAL_NVIC_EnableIRQ(FDCAN_CAL_IRQn);
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param fdcanHandle
-----------------------------------------------------------------------------------------------------*/
void HAL_FDCAN_MspDeInit(FDCAN_HandleTypeDef *fdcanHandle)
{

  if (fdcanHandle->Instance == FDCAN1)
  {
    __HAL_RCC_FDCAN_CLK_DISABLE();

    HAL_NVIC_DisableIRQ(FDCAN1_IT0_IRQn);
    HAL_NVIC_DisableIRQ(FDCAN1_IT1_IRQn);
    HAL_NVIC_DisableIRQ(FDCAN_CAL_IRQn);
  }
}


void FDCAN1_IT0_IRQHandler(void)
{
  HAL_FDCAN_IRQHandler(&hfdcan);
}


void FDCAN1_IT1_IRQHandler(void)
{
  HAL_FDCAN_IRQHandler(&hfdcan);
}

/*-----------------------------------------------------------------------------------------------------


  \param hfdcan
  \param RxFifo0ITs
-----------------------------------------------------------------------------------------------------*/
static void _HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
  if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_WATERMARK) != RESET)
  {
    tx_event_flags_set(&can_flag, CAN_MSG_RECEIVED, TX_OR);
  }
}


/*-----------------------------------------------------------------------------------------------------


  \param timeout_ms

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t  Wait_CAN_message(uint32_t timeout_ms)
{
  UINT      res;
  ULONG     actual_flags;

  res =  tx_event_flags_get(&can_flag, CAN_MSG_RECEIVED, TX_OR_CLEAR,&actual_flags,  MS_TO_TICKS(timeout_ms));
  if (res == TX_SUCCESS)
  {
    return RES_OK;
  }
  return RES_ERROR;
}

