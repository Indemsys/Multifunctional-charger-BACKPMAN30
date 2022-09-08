// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2022-03-29
// 16:25:18
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"

static  TX_EVENT_FLAGS_GROUP   spi4_flag;
#define SPI4_TX_DONE   BIT(0)

static  TX_EVENT_FLAGS_GROUP   spi1_flag;
#define SPI1_TX_DONE   BIT(0)

/*-----------------------------------------------------------------------------------------------------
  Инициализация SPI4 в качестве интерфеса к TFT дисплея

  Тактирование SPI4 осуществляется от частоты 120 МГц
  Периферийный модуль SPI4 подключен к шине APB2 (120 МГц)

  Конфигурация для дисплея должна быть:
  - MSB transmitted first
  - CPHA = 0
  - CPOL = 0
  - Скорость работы SPI допустима до 62 МГц

  \param void
-----------------------------------------------------------------------------------------------------*/
void SPI_TFT_init(void)
{
  uint32_t          tmp;
  volatile uint32_t vreg;

  RCC->APB2ENR |= RCC_APB2ENR_SPI4EN;
  vreg = RCC->APB2ENR; // Обратное чтение чтобы выполнить обязательную задержку


  // Настраиваем на частоту 60 МГц
  tmp = 0
       + LSHIFT(0, 28) // MBR[2:0]     | master baud rate
                       //                000: SPI master clock/2
                       //                001: SPI master clock/4
                       //                010: SPI master clock/8
                       //                011: SPI master clock/16
                       //                100: SPI master clock/32
                       //                101: SPI master clock/64
                       //                110: SPI master clock/128
                       //                111: SPI master clock/256
       + LSHIFT(0, 22) // CRCEN        | hardware CRC computation enable
       + LSHIFT(0, 16) // CRCSIZE[4:0] | length of CRC frame to be transacted and compared
       + LSHIFT(0, 15) // TXDMAEN      | Tx DMA stream enable
       + LSHIFT(0, 14) // RXDMAEN      | Rx DMA stream enable
       + LSHIFT(0, 11) // UDRDET[1:0]  | detection of underrun condition at slave transmitter
       + LSHIFT(0,  9) // UDRCFG[1:0]  | behavior of slave transmitter at underrun condition
       + LSHIFT(7,  5) // FTHLV[3:0]   | FIFO threshold level
                       //                 0000: 1-data
                       //                  ...
                       //                 1111: 16-data
       + LSHIFT(7,  0) // DSIZE[4:0]   | number of bits in at single SPI data frame
                       //                00011: 4-bits
                       //                00100: 5-bits
                       //                00101: 6-bits
                       //                00110: 7-bits
                       //                00111: 8-bits
                       //                 ...
                       //                11111: 32-bits
  ;
  SPI4->CFG1 = tmp;

  SPI4->CR1 |= BIT(12); // SSI = 1
  //
  tmp = 0
       + LSHIFT(1, 31) // AFCNTR    | alternate function GPIOs control. 1: the peripheral keeps always control of all associated GPIOs
       + LSHIFT(0, 30) // SSOM      | SS output management in master mode.
       + LSHIFT(0, 29) // SSOE      | SS output enable. 0: SS output is disabled. 1: SS output is enabled
       + LSHIFT(0, 28) // SSIOP     | SS input/output polarity. 0: low level is active for SS signal. 1: high level is active for SS signal
       + LSHIFT(1, 26) // SSM       | software management of SS signal input. 0: SS input value is determined by the SS PAD. 1: SS input value is determined by the SSI bit
       + LSHIFT(0, 25) // CPOL      | clock polarity. 0: SCK signal is at 0 when idle. 1: SCK signal is at 1 when idle
       + LSHIFT(0, 24) // CPHA      | clock phase. 0: the first clock transition is the first data capture edge. 1: the second clock transition is the first data capture edge
       + LSHIFT(0, 23) // LSBFRST   | data frame format. 0: MSB transmitted first. 1: LSB transmitted first
       + LSHIFT(1, 22) // MASTER    | SPI master. 0: SPI Slave. 1: SPI Master
       + LSHIFT(0, 19) // SP[2:0]   | Serial protocol. 000: SPI Motorola. 001: SPI TI
       + LSHIFT(1, 17) // COMM[1:0] | SPI communication mode. 00: full-duplex. 01: simplex transmitter. 10: simplex receiver. 11: half-duplex
       + LSHIFT(0, 15) // IOSWP     | swap functionality of MISO and MOSI pins. 0: no swap. 1: MOSI and MISO are swapped
       + LSHIFT(0,  4) // MIDI[3:0] | master Inter-Data Idleness. 0000: no delay. 0001: 1 clock cycle period delay. ... 1111: 15 clock cycle periods delay
       + LSHIFT(0,  0) // MSSI[3:0] | master SS idleness. 0000: no extra delay. 0001: 1 clock cycle period delay added. ... 1111: 15 clock cycle periods delay added
  ;
  SPI4->CFG2 = tmp;

  DMA1_from_SPI4_init();

  tx_event_flags_create(&spi4_flag, "SPI4_tx");

  NVIC_SetPriority(SPI4_IRQn, NVIC_EncodePriority(prioritygroup, SPI4_ISR_PRIO, 0));
  NVIC_EnableIRQ(SPI4_IRQn);

}

/*-----------------------------------------------------------------------------------------------------
  Обслуживание прерывания SPI4

  \param void
-----------------------------------------------------------------------------------------------------*/
void  SPI4_IRQHandler(void)
{
  uint32_t st = SPI4->SR;
  SPI4->IFCR = st; // Сбрасываем флаги прерываний

  if (st & BIT(3))
  {
  // Сигнализируем флагом об окончании передачи
    tx_event_flags_set(&spi4_flag, SPI4_TX_DONE, TX_OR);
    SPI4->IER  &= ~BIT(3);    // Запрещаем прерывание EOTIE
  }
}



/*-----------------------------------------------------------------------------------------------------
  Отправка блока данных по DMA через SPI на дисплей
  Отправлять допустимо не более 65535 байт

  Выключаем бит SPE в регистре SPI2S_CR1
  Записываем len в поле TSIZE[15:0] в регистре SPI_CR2
  Прерывания организуется по флагу TXC в регистре SPI2S_SR для этого разрешается прерывание EOTIE в регистре SPI2S_IER
  Включаем бит SPE в регистре SPI2S_CR1
  Стартуем передачу CSTART в регитсре SPI2S_CR1
  Конфигурируем DMA на передачу len байт
  Ждем события из прерывания TXC

  \param buf    - адрес в памяти откуда будут извлекаться данные для передачи
  \param len    - количество передаваемых слов
  \param bits   - количество бит в передаваемом слове
  \param no_inc - если не надо инкрементировать адарес памяти то ставим 1
-----------------------------------------------------------------------------------------------------*/
uint32_t SPI_TFT_send(volatile uint8_t *buf, uint16_t len, uint8_t bits, uint8_t en_inc)
{
  ULONG     actual_flags;
  uint32_t  res;

  SPI4->IFCR  = 0xFFFFFFFF;
  SPI4->CR1  &= ~BIT(0);    // Сбрасываем бит SPE (serial peripheral enable) чтобы иметь возможность конфигурировать другие регистры SPI4
  SPI4->CR2   = len;        // Записываем len в поле TSIZE[15:0] в регистре SPI_CR2

  DMA1_mem_to_SPI4_start(buf, len, bits, en_inc);
  SPI4->CFG1 |= BIT(15);   // TXDMAEN      | Tx DMA stream enable
  SPI4->IER   = BIT(3);    // Разрешается прерывание EOTIE
  SPI4->CR1  |= BIT(0);    // Устанавливаем бит SPE (serial peripheral enable)
  SPI4->CR1  |= BIT(9);    // Устанавливаем бит CSTART (master transfer start)

  res =  tx_event_flags_get(&spi4_flag, SPI4_TX_DONE, TX_OR_CLEAR,&actual_flags,  MS_TO_TICKS(50));

  SPI4->CFG1 &= ~BIT(15);  //  TXDMAEN      | Tx DMA stream disable

  if (res != TX_SUCCESS)
  {
    return RES_ERROR;
  }
  return RES_OK;
}


/*-----------------------------------------------------------------------------------------------------
  Инициализация SPI1 в качестве интерфеса к DAC80501

  Тактирование SPI1 осуществляется от частоты 60 МГц
  Периферийный модуль SPI1 подключен к шине APB2 (120 МГц)

  Конфигурация для DAC80501 должна быть:
  - MSB transmitted first
  - CPHA = 1
  - CPOL = 0
  - Скорость работы SPI допустима до 50 МГц

  \param void
-----------------------------------------------------------------------------------------------------*/
void SPI_DAC_init(void)
{
  uint32_t          tmp;
  volatile uint32_t vreg;

  RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
  vreg = RCC->APB2ENR; // Обратное чтение чтобы выполнить обязательную задержку


  // Настраиваем на частоту 60/64 МГц поскольку большую частоту не пропускает изолятор ADUM7441ARQZ-RL7
  // Длина посылки 24 бита
  tmp = 0
       + LSHIFT(5, 28) // MBR[2:0]     | master baud rate
                       //                000: SPI master clock/2
                       //                001: SPI master clock/4
                       //                010: SPI master clock/8
                       //                011: SPI master clock/16
                       //                100: SPI master clock/32
                       //                101: SPI master clock/64
                       //                110: SPI master clock/128
                       //                111: SPI master clock/256
       + LSHIFT(0, 22) // CRCEN        | hardware CRC computation enable
       + LSHIFT(0, 16) // CRCSIZE[4:0] | length of CRC frame to be transacted and compared
       + LSHIFT(0, 15) // TXDMAEN      | Tx DMA stream enable
       + LSHIFT(0, 14) // RXDMAEN      | Rx DMA stream enable
       + LSHIFT(0, 11) // UDRDET[1:0]  | detection of underrun condition at slave transmitter
       + LSHIFT(0,  9) // UDRCFG[1:0]  | behavior of slave transmitter at underrun condition
       + LSHIFT(7,  5) // FTHLV[3:0]   | FIFO threshold level
                       //                 0000: 1-data
                       //                  ...
                       //                 1111: 16-data
       + LSHIFT(23, 0) // DSIZE[4:0]   | number of bits in at single SPI data frame
                       //                00011: 4-bits
                       //                00100: 5-bits
                       //                00101: 6-bits
                       //                00110: 7-bits
                       //                00111: 8-bits
                       //                 ...
                       //                11111: 32-bits
  ;
  SPI1->CFG1 = tmp;

  SPI1->CR1 |= BIT(12); // SSI = 1
  SPI1->CR2  = 1;       // Записываем 1 в поле TSIZE[15:0] в регистре SPI_CR2. Посылаем всегда по однму слову

  //
  tmp = 0
       + LSHIFT(1, 31) // AFCNTR    | alternate function GPIOs control. 1: the peripheral keeps always control of all associated GPIOs
       + LSHIFT(0, 30) // SSOM      | SS output management in master mode.
       + LSHIFT(0, 29) // SSOE      | SS output enable. 0: SS output is disabled. 1: SS output is enabled
       + LSHIFT(0, 28) // SSIOP     | SS input/output polarity. 0: low level is active for SS signal. 1: high level is active for SS signal
       + LSHIFT(1, 26) // SSM       | software management of SS signal input. 0: SS input value is determined by the SS PAD. 1: SS input value is determined by the SSI bit
       + LSHIFT(0, 25) // CPOL      | clock polarity. 0: SCK signal is at 0 when idle. 1: SCK signal is at 1 when idle
       + LSHIFT(1, 24) // CPHA      | clock phase. 0: the first clock transition is the first data capture edge. 1: the second clock transition is the first data capture edge
       + LSHIFT(0, 23) // LSBFRST   | data frame format. 0: MSB transmitted first. 1: LSB transmitted first
       + LSHIFT(1, 22) // MASTER    | SPI master. 0: SPI Slave. 1: SPI Master
       + LSHIFT(0, 19) // SP[2:0]   | Serial protocol. 000: SPI Motorola. 001: SPI TI
       + LSHIFT(1, 17) // COMM[1:0] | SPI communication mode. 00: full-duplex. 01: simplex transmitter. 10: simplex receiver. 11: half-duplex
       + LSHIFT(0, 15) // IOSWP     | swap functionality of MISO and MOSI pins. 0: no swap. 1: MOSI and MISO are swapped
       + LSHIFT(0,  4) // MIDI[3:0] | master Inter-Data Idleness. 0000: no delay. 0001: 1 clock cycle period delay. ... 1111: 15 clock cycle periods delay
       + LSHIFT(0,  0) // MSSI[3:0] | master SS idleness. 0000: no extra delay. 0001: 1 clock cycle period delay added. ... 1111: 15 clock cycle periods delay added
  ;
  SPI1->CFG2 = tmp;

  tx_event_flags_create(&spi1_flag, "SPI1_tx");

  NVIC_SetPriority(SPI1_IRQn, NVIC_EncodePriority(prioritygroup, SPI4_ISR_PRIO, 0));
  NVIC_EnableIRQ(SPI1_IRQn);

}


/*-----------------------------------------------------------------------------------------------------
  Обслуживание прерывания SPI4

  \param void
-----------------------------------------------------------------------------------------------------*/
void  SPI1_IRQHandler(void)
{
  uint32_t st = SPI1->SR;
  SPI1->IFCR = st; // Сбрасываем флаги прерываний

  if (st & BIT(3))
  {
  // Сигнализируем флагом об окончании передачи
    tx_event_flags_set(&spi1_flag, SPI1_TX_DONE, TX_OR);
    SPI1->IER  &= ~BIT(3);    // Запрещаем прерывание EOTIE
  }
}

/*-----------------------------------------------------------------------------------------------------
  Отсылка

  \param data

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SPI_DAC_send(uint8_t cmd, uint16_t data)
{
  ULONG     actual_flags;
  uint32_t  res;
  uint8_t   *a;
  uint32_t  w;

  a = (uint8_t *)&w;
  a[0] = 0;
  a[2] = cmd;
  a[1] =(data >> 8) & 0xFF;
  a[0] = data & 0xFF;


  Set_DAC_CS(0);
  SPI1->IFCR  = 0xFFFFFFFF;
  SPI1->CR1  &= ~BIT(0);    // Сбрасываем бит SPE (serial peripheral enable) чтобы иметь возможность конфигурировать другие регистры SPI4
  SPI1->CR2   = 1;          // Записываем len в поле TSIZE[15:0] в регистре SPI_CR2
  SPI1->IER   = BIT(3);     // Разрешается прерывание EOTIE
  SPI1->CR1  |= BIT(0);     // Устанавливаем бит SPE (serial peripheral enable)

  SPI1->TXDR  = w;
  SPI1->CR1  |= BIT(9);     // Устанавливаем бит CSTART (master transfer start)

  res =  tx_event_flags_get(&spi1_flag, SPI1_TX_DONE, TX_OR_CLEAR,&actual_flags,  MS_TO_TICKS(50));
  Set_DAC_CS(1);

  if (res != TX_SUCCESS)
  {
    return RES_ERROR;
  }
  return RES_OK;


}

