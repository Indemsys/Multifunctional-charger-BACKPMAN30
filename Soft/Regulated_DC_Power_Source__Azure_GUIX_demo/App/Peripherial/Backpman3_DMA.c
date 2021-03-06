// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2022-02-21
// 10:29:06 
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"

// Располагаем буферы приема отсчетов ADC из канала DMA в области памяти с отключенным кэшированием
uint32_t    adc1_buffer[2*ADC_CH_CNT] @ ".sram1";
uint32_t    adc2_buffer[2*ADC_CH_CNT] @ ".sram1";
uint32_t    adc3_buffer[2*ADC_CH_CNT] @ ".sram1";


uint8_t                adc1_bit;
uint8_t                adc2_bit;
uint8_t                adc3_bit;

TX_EVENT_FLAGS_GROUP   adc_flag;

/*-----------------------------------------------------------------------------------------------------
  DMA1 переносит данные между шинами AHB1, AHB4 и APB1, APB2

  На шине AHB1 находятся области памяти SRAM1, SRAM2, SRAM3
  На шине APB2 находятся ADC1, ADC2
  На шине AHB4 находятся ADC3


  Организуем циклическую пересылку (circular mode) из ADC в память


  \param void
-----------------------------------------------------------------------------------------------------*/
void DMA1_from_ADC_init(void)
{
  volatile uint32_t vreg;
  uint32_t          tmp;

  // Разрешаем работу DMA1 если она еще не разрешена
  RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
  vreg = RCC->AHB1ENR; // Обратное чтение чтобы выполнить обязательную задержку

  // Для передачи ADC1 в память используем канал 0
  // Для передачи ADC2 в память используем канал 1
  // Для передачи ADC3 в память используем канал 2
  //
  tmp = 0
       + LSHIFT(0, 23) // MBURST[1:0]| Memory burst transfer configuration
                       //              00: single transfer
                       //              01: INCR4 (incremental burst of 4 beats)
                       //              10: INCR8 (incremental burst of 8 beats)
                       //              11: INCR16 (incremental burst of 16 beats)
       + LSHIFT(0, 21) // PBURST[1:0]| Peripheral burst transfer configuration
                       //              00: single transfer
                       //              01: INCR4 (incremental burst of 4 beats)
                       //              10: INCR8 (incremental burst of 8 beats)
                       //              11: INCR16 (incremental burst of 16 beats)
       + LSHIFT(0, 20) // TRBUFF     | 0: bufferable transfers not enabled. 1: bufferable transfers enabled
       + LSHIFT(0, 19) // CT         | Current target (only in double-buffer mode)
                       //              0: current target memory is Memory 0 (addressed by the DMA_SxM0AR pointer)
                       //              1: current target memory is Memory 1 (addressed by the DMA_SxM1AR pointer)
       + LSHIFT(1, 18) // DBM        | Double-buffer mode. 0: no buffer switching at the end of transfer. 1: memory target switched at the end of the DMA transfer
       + LSHIFT(1, 16) // PL[1:0]    | Priority level. 00: low. 01: medium. 10: high. 11: very high
       + LSHIFT(0, 15) // PINCOS     | Peripheral increment offset size.
                       //              0: The offset size for the peripheral address calculation is linked to the PSIZE
                       //              1: The offset size for the peripheral address calculation is fixed to 4 (32-bit alignment).
       + LSHIFT(2, 13) // MSIZE[1:0] | Memory data size.          00: byte (8-bit). 01: half-word (16-bit). 10: word (32-bit).
       + LSHIFT(2, 11) // PSIZE[1:0] | Peripheral data size.      00: byte (8-bit). 01: half-word (16-bit). 10: word (32-bit).
       + LSHIFT(1, 10) // MINC       | Memory increment mode.     0: memory address pointer is fixed.  1: memory address pointer is incremented after each data transfer (increment is done according to MSIZE)
       + LSHIFT(0,  9) // PINC       | Peripheral increment mode. 0: peripheral address pointer fixed. 1: peripheral address pointer incremented after each data transfer (increment done according to PSIZE)
       + LSHIFT(1,  8) // CIRC       | Circular mode. 0: circular mode disabled. 1: circular mode enabled
       + LSHIFT(0,  6) // DIR[1:0]   | Data transfer direction.   00: peripheral-to-memory. 01: memory-to-peripheral. 10: memory-to-memory.
       + LSHIFT(0,  5) // PFCTRL     | Peripheral flow controller. 0: DMA is the flow controller. 1: The peripheral is the flow controller.
       + LSHIFT(1,  4) // TCIE       | Transfer complete interrupt enable
       + LSHIFT(0,  3) // HTIE       | Half transfer interrupt enable
       + LSHIFT(0,  2) // TEIE       | Transfer error interrupt enable
       + LSHIFT(0,  1) // DMEIE      | Direct mode error interrupt enable
       + LSHIFT(0,  0) // EN         | Stream enable
  ;

  DMA1_Stream0->CR = tmp;
  DMA1_Stream1->CR = tmp;
  DMA1_Stream2->CR = tmp;

  DMA1_Stream0->NDTR = ADC_CH_CNT;
  DMA1_Stream1->NDTR = ADC_CH_CNT;
  DMA1_Stream2->NDTR = ADC_CH_CNT;

  // Конфигурируем адрес периферии откуда данные извлекаются
  DMA1_Stream0->PAR  = (uint32_t)&(ADC1->DR);
  DMA1_Stream1->PAR  = (uint32_t)&(ADC2->DR);
  DMA1_Stream2->PAR  = (uint32_t)&(ADC3->DR);

  // Очищаем буферы приемники данных
  memset(adc1_buffer, 0, sizeof(adc1_buffer));
  memset(adc2_buffer, 0, sizeof(adc1_buffer));
  memset(adc3_buffer, 0, sizeof(adc1_buffer));

  // Используем circular mode. Т.е. после пакета пересылки DMA в один буфер происходит переключение на пересылку в другой буфер
  // Поэтому назначаем адреса для двух буферов какждому каналу
  DMA1_Stream0->M0AR = (uint32_t)adc1_buffer;
  DMA1_Stream1->M0AR = (uint32_t)adc2_buffer;
  DMA1_Stream2->M0AR = (uint32_t)adc3_buffer;

  DMA1_Stream0->M1AR = (uint32_t)&adc1_buffer[ADC_CH_CNT];
  DMA1_Stream1->M1AR = (uint32_t)&adc2_buffer[ADC_CH_CNT];
  DMA1_Stream2->M1AR = (uint32_t)&adc3_buffer[ADC_CH_CNT];


  // Конфигурируем модуль мультиплексора для передачи запросов DMA
  DMAMUX1_Channel0->CCR = DMA1_REQ_ADC1_DMA;
  DMAMUX1_Channel1->CCR = DMA1_REQ_ADC2_DMA;
  DMAMUX1_Channel2->CCR = DMA1_REQ_ADC3_DMA;

  tx_event_flags_create(&adc_flag, "ADC");

  // Конфигурируем прерывания каналов DMA
  NVIC_SetPriority(DMA1_Stream0_IRQn, NVIC_EncodePriority(prioritygroup, DMA_ADC_ISR_PRIO, 0));
  NVIC_SetPriority(DMA1_Stream1_IRQn, NVIC_EncodePriority(prioritygroup, DMA_ADC_ISR_PRIO, 0));
  NVIC_SetPriority(DMA1_Stream2_IRQn, NVIC_EncodePriority(prioritygroup, DMA_ADC_ISR_PRIO, 0));

  NVIC_EnableIRQ(DMA1_Stream0_IRQn);
  NVIC_EnableIRQ(DMA1_Stream1_IRQn);
  NVIC_EnableIRQ(DMA1_Stream2_IRQn);

  adc1_bit = 0;
  adc2_bit = 0;
  adc3_bit = 0;

  tmp |= BIT(0);
  // Стартуем DMA
  DMA1_Stream0->CR = tmp;
  DMA1_Stream1->CR = tmp;
  DMA1_Stream2->CR = tmp;

}

/*-----------------------------------------------------------------------------------------------------

  Инициализация DMA1 для переноса данных из памяти SRAM1 в SPI4

  Память SRAM1 находится на шине AHB1
  Периферийный модуль SPI4 подключен к шине APB2 (120 МГц)

  DMA1 переносит данные между шинами AHB1, AHB4 и APB1, APB2

  Используем канал 3 DMA и побайтную передачу
  Пересылки одноразовые memory-to-peripheral, без применения циклического переключения буферов
  Прерывания DMA не используем. Используется прерывание от SPI4 по флагу окончания передачи последнего байта
  Адрес памяти и количество данных назначем в функции старта пересылки


  \param void
-----------------------------------------------------------------------------------------------------*/
void DMA1_from_SPI4_init(void)
{
  volatile uint32_t vreg;
  uint32_t          tmp;

  // Разрешаем работу DMA1 если она еще не разрешена
  RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
  vreg = RCC->AHB1ENR; // Обратное чтение чтобы выполнить обязательную задержку

  tmp = 0
       + LSHIFT(0, 23) // MBURST[1:0]| Memory burst transfer configuration
                       //              00: single transfer
                       //              01: INCR4 (incremental burst of 4 beats)
                       //              10: INCR8 (incremental burst of 8 beats)
                       //              11: INCR16 (incremental burst of 16 beats)
       + LSHIFT(0, 21) // PBURST[1:0]| Peripheral burst transfer configuration
                       //              00: single transfer
                       //              01: INCR4 (incremental burst of 4 beats)
                       //              10: INCR8 (incremental burst of 8 beats)
                       //              11: INCR16 (incremental burst of 16 beats)
       + LSHIFT(0, 20) // TRBUFF     | 0: bufferable transfers not enabled. 1: bufferable transfers enabled
       + LSHIFT(0, 19) // CT         | Current target (only in double-buffer mode)
                       //              0: current target memory is Memory 0 (addressed by the DMA_SxM0AR pointer)
                       //              1: current target memory is Memory 1 (addressed by the DMA_SxM1AR pointer)
       + LSHIFT(1, 18) // DBM        | Double-buffer mode. 0: no buffer switching at the end of transfer. 1: memory target switched at the end of the DMA transfer
       + LSHIFT(0, 16) // PL[1:0]    | Priority level. 00: low. 01: medium. 10: high. 11: very high
       + LSHIFT(0, 15) // PINCOS     | Peripheral increment offset size.
                       //              0: The offset size for the peripheral address calculation is linked to the PSIZE
                       //              1: The offset size for the peripheral address calculation is fixed to 4 (32-bit alignment).
       + LSHIFT(0, 13) // MSIZE[1:0] | Memory data size.          00: byte (8-bit). 01: half-word (16-bit). 10: word (32-bit).
       + LSHIFT(0, 11) // PSIZE[1:0] | Peripheral data size.      00: byte (8-bit). 01: half-word (16-bit). 10: word (32-bit).
       + LSHIFT(1, 10) // MINC       | Memory increment mode.     0: memory address pointer is fixed.  1: memory address pointer is incremented after each data transfer (increment is done according to MSIZE)
       + LSHIFT(0,  9) // PINC       | Peripheral increment mode. 0: peripheral address pointer fixed. 1: peripheral address pointer incremented after each data transfer (increment done according to PSIZE)
       + LSHIFT(0,  8) // CIRC       | Circular mode. 0: circular mode disabled. 1: circular mode enabled
       + LSHIFT(1,  6) // DIR[1:0]   | Data transfer direction.   00: peripheral-to-memory. 01: memory-to-peripheral. 10: memory-to-memory.
       + LSHIFT(0,  5) // PFCTRL     | Peripheral flow controller. 0: DMA is the flow controller. 1: The peripheral is the flow controller.
       + LSHIFT(0,  4) // TCIE       | Transfer complete interrupt enable
       + LSHIFT(0,  3) // HTIE       | Half transfer interrupt enable
       + LSHIFT(0,  2) // TEIE       | Transfer error interrupt enable
       + LSHIFT(0,  1) // DMEIE      | Direct mode error interrupt enable
       + LSHIFT(0,  0) // EN         | Stream enable
  ;

  DMA1_Stream3->CR = tmp;

  // Конфигурируем адрес периферии куда данные посылаются
  DMA1_Stream3->PAR  = (uint32_t)&(SPI4->TXDR);


  // Конфигурируем модуль мультиплексора для передачи запросов DMA
  DMAMUX1_Channel3->CCR = DMA1_REQ_SPI4_TX_DMA;

}

/*-----------------------------------------------------------------------------------------------------
  Отсылка данных из области памяти в SPI4 по DMA

  \param mem_addr - адрес в памяти откуда будут извлекаться данные для передачи
  \param len      - количество передаваемых слов
  \param bits     - количество бит в передаваемом слове
  \param no_inc   - если не надо инкрементировать адарес памяти то ставим 1
-----------------------------------------------------------------------------------------------------*/
void DMA1_mem_to_SPI4_start(uint8_t *mem_addr, uint16_t len, uint8_t bits, uint8_t en_inc)
{
  uint32_t tmp;
  DMA1_Stream3->CR &= ~BIT(0); // Снимаем бит EN

  DMA1->LIFCR = BIT(27); // Сброс бита TCIF3

  tmp = 0
       + LSHIFT(0, 23) // MBURST[1:0]   | Memory burst transfer configuration
                       //                 00: single transfer
                       //                 01: INCR4 (incremental burst of 4 beats)
                       //                 10: INCR8 (incremental burst of 8 beats)
                       //                 11: INCR16 (incremental burst of 16 beats)
       + LSHIFT(0, 21) // PBURST[1:0]   | Peripheral burst transfer configuration
                       //                 00: single transfer
                       //                 01: INCR4 (incremental burst of 4 beats)
                       //                 10: INCR8 (incremental burst of 8 beats)
                       //                 11: INCR16 (incremental burst of 16 beats)
       + LSHIFT(0, 20) // TRBUFF        | 0: bufferable transfers not enabled. 1: bufferable transfers enabled
       + LSHIFT(0, 19) // CT            | Current target (only in double-buffer mode)
                       //                 0: current target memory is Memory 0 (addressed by the DMA_SxM0AR pointer)
                       //                 1: current target memory is Memory 1 (addressed by the DMA_SxM1AR pointer)
       + LSHIFT(0, 18) // DBM           | Double-buffer mode. 0: no buffer switching at the end of transfer. 1: memory target switched at the end of the DMA transfer
       + LSHIFT(0, 16) // PL[1:0]       | Priority level. 00: low. 01: medium. 10: high. 11: very high
       + LSHIFT(0, 15) // PINCOS        | Peripheral increment offset size.
                       //                 0: The offset size for the peripheral address calculation is linked to the PSIZE
                       //                 1: The offset size for the peripheral address calculation is fixed to 4 (32-bit alignment).
       + LSHIFT(bits, 13) // MSIZE[1:0] | Memory data size.          00: byte (8-bit). 01: half-word (16-bit). 10: word (32-bit).
       + LSHIFT(bits, 11) // PSIZE[1:0] | Peripheral data size.      00: byte (8-bit). 01: half-word (16-bit). 10: word (32-bit).
       + LSHIFT(en_inc, 10) // MINC     | Memory increment mode.     0: memory address pointer is fixed.  1: memory address pointer is incremented after each data transfer (increment is done according to MSIZE)
       + LSHIFT(0,  9) // PINC          | Peripheral increment mode. 0: peripheral address pointer fixed. 1: peripheral address pointer incremented after each data transfer (increment done according to PSIZE)
       + LSHIFT(0,  8) // CIRC          | Circular mode. 0: circular mode disabled. 1: circular mode enabled
       + LSHIFT(1,  6) // DIR[1:0]      | Data transfer direction.   00: peripheral-to-memory. 01: memory-to-peripheral. 10: memory-to-memory.
       + LSHIFT(0,  5) // PFCTRL        | Peripheral flow controller. 0: DMA is the flow controller. 1: The peripheral is the flow controller.
       + LSHIFT(0,  4) // TCIE          | Transfer complete interrupt enable
       + LSHIFT(0,  3) // HTIE          | Half transfer interrupt enable
       + LSHIFT(0,  2) // TEIE          | Transfer error interrupt enable
       + LSHIFT(0,  1) // DMEIE         | Direct mode error interrupt enable
       + LSHIFT(0,  0) // EN            | Stream enable
  ;

  DMA1_Stream3->CR = tmp;
  DMA1_Stream3->M0AR = (uint32_t)mem_addr;
  DMA1_Stream3->NDTR = len;

  DMA1_Stream3->CR |= BIT(0); // Уставливаем бит EN
}


/*-----------------------------------------------------------------------------------------------------
  Прерывание возникающее после выполнения пересылки по DMA всех выброк ADC1 выполненный в течении прохода по всем заданным каналам
  Частота повторения 20 Кгц (50 мкс)

  Длительность выполнения функции = 0.86 мкс без оптимизации

  \param void
-----------------------------------------------------------------------------------------------------*/
void DMA1_Stream0_IRQHandler(void)
{
  uint32_t tmp = DMA1_Stream1->CR;
  //ITM_EVENT8(1,1);
  DMA1->LIFCR = BIT(5);
  if (tmp & BIT(19))
  {
    adcs.smpl_ACC_I   = RunAverageFilter_uint32_20( adc1_buffer[0], &flt_ACC_I );
    adcs.smpl_PSRC_I  = RunAverageFilter_uint32_20( adc1_buffer[1], &flt_PSRC_I);
    adcs.smpl_LOAD_V  = RunAverageFilter_uint32_20( adc1_buffer[2], &flt_LOAD_V);
    adcs.smpl_AIN2    = RunAverageFilter_uint32_20( adc1_buffer[3], &flt_AIN2);
    adcs.smpl_VREF165 = RunAverageFilter_uint32_20( adc1_buffer[4], &flt_VREF165);
  }
  else
  {
    adcs.smpl_ACC_I   = RunAverageFilter_uint32_20( adc1_buffer[0+ADC_CH_CNT], &flt_ACC_I );
    adcs.smpl_PSRC_I  = RunAverageFilter_uint32_20( adc1_buffer[1+ADC_CH_CNT], &flt_PSRC_I);
    adcs.smpl_LOAD_V  = RunAverageFilter_uint32_20( adc1_buffer[2+ADC_CH_CNT], &flt_LOAD_V);
    adcs.smpl_AIN2    = RunAverageFilter_uint32_20( adc1_buffer[3+ADC_CH_CNT], &flt_AIN2);
    adcs.smpl_VREF165 = RunAverageFilter_uint32_20( adc1_buffer[4+ADC_CH_CNT], &flt_VREF165);
  }

  Encoder_proc();
  //ITM_EVENT8(1,0);
}



/*-----------------------------------------------------------------------------------------------------
  Прерывание возникающее после выполнения пересылки по DMA всех выброк ADC1 выполненный в течении прохода по всем заданным каналам
  Частота повторения 20 Кгц (50 мкс)

  Длительность выполнения функции = 0.58 мкс без оптимизации

  \param void
-----------------------------------------------------------------------------------------------------*/
void DMA1_Stream1_IRQHandler(void)
{
  uint32_t tmp = DMA1_Stream1->CR;
  //ITM_EVENT8(2,1);
  DMA1->LIFCR = BIT(11);
  if (tmp & BIT(19))
  {
    adcs.smpl_ACC_V = RunAverageFilter_uint32_20( adc2_buffer[0], &flt_ACC_V );
    adcs.smpl_SYS_V = RunAverageFilter_uint32_20( adc2_buffer[1], &flt_SYS_V);
    adcs.smpl_AIN3  = RunAverageFilter_uint32_20( adc2_buffer[2], &flt_AIN3);
    adcs.smpl_AIN1  = RunAverageFilter_uint32_20( adc2_buffer[3], &flt_AIN1);
    adcs.smpl_AIN4  = RunAverageFilter_uint32_20( adc2_buffer[4], &flt_AIN4);
  }
  else
  {
    adcs.smpl_ACC_V = RunAverageFilter_uint32_20( adc2_buffer[0+ADC_CH_CNT], &flt_ACC_V );
    adcs.smpl_SYS_V = RunAverageFilter_uint32_20( adc2_buffer[1+ADC_CH_CNT], &flt_SYS_V);
    adcs.smpl_AIN3  = RunAverageFilter_uint32_20( adc2_buffer[2+ADC_CH_CNT], &flt_AIN3);
    adcs.smpl_AIN1  = RunAverageFilter_uint32_20( adc2_buffer[3+ADC_CH_CNT], &flt_AIN1);
    adcs.smpl_AIN4  = RunAverageFilter_uint32_20( adc2_buffer[4+ADC_CH_CNT], &flt_AIN4);
  }
  //ITM_EVENT8(2,0);
}


/*-----------------------------------------------------------------------------------------------------
  Прерывание возникающее после выполнения пересылки по DMA всех выброк ADC1 выполненный в течении прохода по всем заданным каналам
  Частота повторения 20 Кгц (50 мкс)

  Длительность выполнения функции = 0.58 мкс без оптимизации

  Это прерывание возникает последним в цепочке прерываний обслуживающих АЦП

  \param void
-----------------------------------------------------------------------------------------------------*/
void DMA1_Stream2_IRQHandler(void)
{
  static uint32_t div = 0;
  uint32_t tmp = DMA1_Stream2->CR;
  //ITM_EVENT8(3,1);
  DMA1->LIFCR = BIT(21);
  if (tmp & BIT(19))
  {
    adcs.smpl_TSENS   = RunAverageFilter_uint32_20( adc3_buffer[0], &flt_TSENS);
    adcs.smpl_PSRC_V  = RunAverageFilter_uint32_20( adc3_buffer[1], &flt_PSRC_V );
    adcs.smpl_LOAD_I  = RunAverageFilter_uint32_20( adc3_buffer[2], &flt_LOAD_I );
    adcs.smpl_TERM    = RunAverageFilter_uint32_20( adc3_buffer[3], &flt_TERM );
    adcs.smpl_VREF    = RunAverageFilter_uint32_20( adc3_buffer[4], &flt_VREF);
  }
  else
  {
    adcs.smpl_TSENS   = RunAverageFilter_uint32_20( adc3_buffer[0+ADC_CH_CNT], &flt_TSENS);
    adcs.smpl_PSRC_V  = RunAverageFilter_uint32_20( adc3_buffer[1+ADC_CH_CNT], &flt_PSRC_V );
    adcs.smpl_LOAD_I  = RunAverageFilter_uint32_20( adc3_buffer[2+ADC_CH_CNT], &flt_LOAD_I );
    adcs.smpl_TERM    = RunAverageFilter_uint32_20( adc3_buffer[3+ADC_CH_CNT], &flt_TERM );
    adcs.smpl_VREF    = RunAverageFilter_uint32_20( adc3_buffer[4+ADC_CH_CNT], &flt_VREF);
  }

  div++;
  if (div >= 20)
  {
    div = 0;
    tx_event_flags_set(&adc_flag, ADC_RES_READY, TX_OR); // Длительность функции 0.31 мкс без оптимизации
  }

  //ITM_EVENT8(3,0);
}

