// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2021-11-11
// 10:35:14
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"


/*
  Все каналы ADC работают в single-ended mode
  In single-ended input mode, the analog voltage to be converted for channel “i” is the ifference between the external voltage VINP[i] (positive input) and VREF- (negative input).


  Название |  Канал    |  Канал  |   Канал  |
  сигнала  |  ADC1     |  ADC2   |   ADC3   |
-----------|-----------|---------|----------|
  VREF165  |  INP16    |         |          |
  AIN2     |  INP17    |         |          |
  AIN1     |  INP14    |  INP14  |          |
  AIN3     |  INP15    |  INP15  |          |
  ACC_I    |  INP3     |  INP3   |          |
  PSRC_I   |  INP7     |  INP7   |          |
  SYS_V    |  INP9     |  INP9   |          |
  AIN4     |  INP5     |  INP5   |          |
  TSENS    |  INP10    |  INP10  |  INP10   |
  ACC_V    |  INP4     |  INP4   |          |
  LOAD_V   |  INP8     |  INP8   |          |
  LOAD_I   |           |         |  INP0    |
  PSRC_V   |           |         |  INP1    |

  Последовательность преобразований:

             1            2           3          4           5         - номер преобразования

  ADC1 : | ACC_I    3 | PSRC_I 7 | LOAD_V 8 | AIN2   17 | VREF165 16 | - Название сигнала и номер канала
         |            |          |          |           |            |
  ADC2 : | ACC_V    4 | SYS_V  9 | AIN3  15 | AIN1   14 | AIN4     5 |
         |            |          |          |           |            |
  ADC3 : | TSENS   10 | PSRC_V 1 | LOAD_I 0 | TERM   18 | VREF    19 |


  */


T_adc adcs @ "DTCM";

typedef struct
{
    uint8_t  channel;
    uint8_t  sampling_time;
    uint32_t *p_var;
} T_ADC_ch_lst;

typedef struct
{
    ADC_TypeDef   *ADC;
    uint8_t       channels_num;
    T_ADC_ch_lst  ch_cfg[ADC_CH_CNT];
} T_ADC_ch_cfg;

#define SC  SMT_64_5_CYC // Длительность стадии сэмплирования

const T_ADC_ch_cfg  ADC_ch_cfg[3] =
{
  {ADC1, ADC_CH_CNT, {{ 3, SC ,&adcs.smpl_ACC_I   }, { 7, SC ,&adcs.smpl_PSRC_I }, { 8, SC ,&adcs.smpl_LOAD_V }, {17, SC ,&adcs.smpl_AIN2 }, {16, SC ,&adcs.smpl_VREF165 }}},
  {ADC2, ADC_CH_CNT, {{ 4, SC ,&adcs.smpl_ACC_V   }, { 9, SC ,&adcs.smpl_SYS_V  }, {15, SC ,&adcs.smpl_AIN3   }, {14, SC ,&adcs.smpl_AIN1 }, { 5, SC ,&adcs.smpl_AIN4    }}},
  {ADC3, ADC_CH_CNT, {{10, SC ,&adcs.smpl_TSENS   }, { 1, SC ,&adcs.smpl_PSRC_V }, { 0, SC ,&adcs.smpl_LOAD_I }, {18, SC ,&adcs.smpl_TERM }, {19, SC ,&adcs.smpl_VREF    }}},
};

uint8_t adc1_ch_pt = 0;
uint8_t adc2_ch_pt = 0;
uint8_t adc3_ch_pt = 0;


float  TS_CAL2;
float  TS_CAL1;


T_run_average_uint32_20    flt_ACC_I   @ "DTCM";
T_run_average_uint32_20    flt_PSRC_I  @ "DTCM";
T_run_average_uint32_20    flt_LOAD_V  @ "DTCM";
T_run_average_uint32_20    flt_AIN2    @ "DTCM";
T_run_average_uint32_20    flt_VREF165 @ "DTCM";
T_run_average_uint32_20    flt_ACC_V   @ "DTCM";
T_run_average_uint32_20    flt_SYS_V   @ "DTCM";
T_run_average_uint32_20    flt_AIN3    @ "DTCM";
T_run_average_uint32_20    flt_AIN1    @ "DTCM";
T_run_average_uint32_20    flt_AIN4    @ "DTCM";
T_run_average_uint32_20    flt_TSENS   @ "DTCM";
T_run_average_uint32_20    flt_PSRC_V  @ "DTCM";
T_run_average_uint32_20    flt_LOAD_I  @ "DTCM";
T_run_average_uint32_20    flt_TERM    @ "DTCM";
T_run_average_uint32_20    flt_VREF    @ "DTCM";



/*-----------------------------------------------------------------------------------------------------
  Функция программирующая регистры ADC в соответствии с конфигурацией обрабатываемых каналов
  Конфигурация каналов задается в массиве ADC_ch_cfg

  \param adc_id

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t ADC_set_channels(uint8_t adc_id)
{
  ADC_TypeDef   *ADC;
  uint32_t       i,n;

  ADC = ADC_ch_cfg[adc_id].ADC;

  n          = ADC_ch_cfg[adc_id].channels_num;
  ADC->SQR1  = n-1;
  ADC->SQR2  = 0;
  ADC->SQR3  = 0;
  ADC->SQR4  = 0;
  ADC->SMPR1 = 0;
  ADC->SMPR2 = 0;
  ADC->PCSEL = 0;

  if (n > 19) return RES_ERROR;

  for (i=0; i < n; i++)
  {
    uint8_t ch;
    uint8_t  st;
    ch = ADC_ch_cfg[adc_id].ch_cfg[i].channel;
    st = ADC_ch_cfg[adc_id].ch_cfg[i].sampling_time;
    if (ch > 19) return RES_ERROR;
    if (st > 7) return RES_ERROR;

    if (i < 4)
    {
      ADC->SQR1  |=  ch <<(6 * (i+1));
    }
    else if (i < 9)
    {
      ADC->SQR2  |=  ch <<(6 * (i-4));
    }
    else if (i < 14)
    {
      ADC->SQR3  |=  ch <<(6 * (i-9));
    }
    else
    {
      ADC->SQR4 |=  ch <<(6 * (i-14));
    }

    if (ch < 10)
    {
      ADC->SMPR1 |=  st <<(3 * ch);
    }
    else
    {
      ADC->SMPR2 |=  st <<(3 * (ch-10));
    }

    ADC->PCSEL |= BIT(ch);
  }
  return RES_OK;
}


/*-----------------------------------------------------------------------------------------------------
  Выключение ADC

  \param ADC

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t ADC_disable(ADC_TypeDef *ADC)
{

  if ((ADC->CR & A_CR_ADEN) == 0)
  {
    // Если модуль уже выключен, то выходим
    return RES_OK;
  }

  if (((ADC->CR & A_CR_ADSTART) == 0) && ((ADC->CR & A_CR_JADSTART) == 0))
  {
    // Если не идет преобразование, то запрещаем ADC
    if ((ADC->CR & A_CR_ADDIS) == 0)
    {
      ADC->CR |= A_CR_ADDIS; // Запрещаем ADC
      while (((ADC->CR & A_CR_ADDIS) != 0)) DELAY_1us; // Ждем пока не завершиться выполнение команды  ADDIS
    }
    ADC->CR = 0; // Выключаем ADC
    return RES_OK;
  }

  // Останавливаем регулярные преобразования
  if ((ADC->CR & A_CR_ADSTART) != 0)
  {
    // Идет регулярное преобразование
    // Ждем пока A_CR_ADDIS не установится в 0. Это будет означать завершение команды ADDIS
    while (((ADC->CR & A_CR_ADDIS) != 0)) DELAY_1us; // Ждем пока не завершиться выполнение команды  ADDIS

    ADC->CR |= A_CR_ADSTP; // Останавливаем регулярное преобразование
    while (((ADC->CR & A_CR_ADSTP) != 0)) DELAY_1us; // Ждем пока не завершиться выполнение команды ADSTP

  }

  // Останавливаем инжектированные преобразования
  if ((ADC->CR & A_CR_JADSTART) != 0)
  {
    // Идет инжектированное преобразование
    // Ждем пока A_CR_ADDIS не установится в 0. Это будет означать завершение команды ADDIS
    while (((ADC->CR & A_CR_ADDIS) != 0)) DELAY_1us; // Ждем пока не завершиться выполнение команды  ADDIS

    ADC->CR |= A_CR_JADSTP; // Останавливаем регулярное преобразование
    while (((ADC->CR & A_CR_JADSTP) != 0)) DELAY_1us; // Ждем пока не завершиться выполнение команды ADSTP
  }

  ADC->CR |= A_CR_ADDIS; // Запрещаем ADC
  while (((ADC->CR & A_CR_ADDIS) != 0)) DELAY_1us; // Ждем пока не завершиться выполнение команды  ADDIS

  ADC->CR = 0; // Выключаем ADC
  return RES_OK;

}

/*-----------------------------------------------------------------------------------------------------


  \param ADC

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t ADC_vreg_sw_on(ADC_TypeDef *ADC)
{
  uint32_t tmp;
  // Включем внутренние регуляторы напряжерия ADC и бустер
  //
  tmp = 0
       + LSHIFT(0, 31) // ADCAL        | 0: Calibration complete. 1: Write 1 to calibrate the ADC. Read at 1 means that a calibration in progress.
       + LSHIFT(0, 30) // ADCALDIF     | Differential mode for calibration
       + LSHIFT(0, 29) // DEEPPWD      | Deep-power-down enable
       + LSHIFT(1, 28) // ADVREGEN     | ADC voltage regulator enable
       + LSHIFT(0, 27) // LINCALRDYW6  | Linearity calibration ready Word 6
       + LSHIFT(0, 26) // LINCALRDYW5  | Linearity calibration ready Word 5
       + LSHIFT(0, 25) // LINCALRDYW4  | Linearity calibration ready Word 4
       + LSHIFT(0, 24) // LINCALRDYW3  | Linearity calibration ready Word 3
       + LSHIFT(0, 23) // LINCALRDYW2  | Linearity calibration ready Word 2
       + LSHIFT(0, 22) // LINCALRDYW1  | Linearity calibration ready Word 1
       + LSHIFT(0, 16) // ADCALLIN     | Linearity calibration
       + LSHIFT(3,  8) // BOOST[1:0]   | Boost mode control
                       //                 00: used when ADC clock ≤ 6.25 MHz
                       //                 01: used when 6.25 MHz < ADC clock frequency ≤ 12.5 MHz
                       //                 10: used when 12.5 MHz < ADC clock ≤ 25.0 MHz
                       //                 11: used when 25.0 MHz < ADC clock ≤ 50.0 MHz
       + LSHIFT(0,  5) // JADSTP       | ADC stop of injected conversion command
       + LSHIFT(0,  4) // ADSTP        | ADC stop of regular conversion command
       + LSHIFT(0,  3) // JADSTART     | ADC start of injected conversion
       + LSHIFT(0,  2) // ADSTART      | ADC start of regular conversion
       + LSHIFT(0,  1) // ADDIS        | ADC disable command
       + LSHIFT(0,  0) // ADEN         | ADC enable control
  ;

  ADC->CR = tmp;

  // Задержка пока не стабилизируется регулятор (max 10 мкс)
  DELAY_12us;
  // Если кто-то не включился выходим с ошибкой
  if ((ADC->CR & ADC_CR_ADVREGEN) == 0) return RES_ERROR;
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param ADC

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t ADC_enable(ADC_TypeDef *ADC)
{
  ADC->ISR = 0X7F;      // Сбрасываем все флаги
  ADC->CR |= A_CR_ADEN; // Разрешаем ADC
  while (((ADC->ISR & A_IS_LDORDY) == 0)) DELAY_1us; // Ждем пока не завершиться выполнение команды  ADEN
  ADC->ISR = 0x7F;      // Снова сбрасываем все флаги
  return RES_OK;
}
/*-----------------------------------------------------------------------------------------------------


  \param p_CCR

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t ADC_conf_clock(volatile uint32_t *p_CCR)
{
  uint32_t tmp;
  // Выбираем тактирование от  adc_sclk/4
  // Для чипов ревизии V сигнал adc_sclk - это сихронный сигнал тактирования sys_ck деленный на делители D1CPRE и  HPRE и равный AXI peripheral clocks = 240 МГц
  // После деления adc_sclk на 4 и затем на 2 внутри модуля ADC получаем 30 МГц.
  // Максимальная частота работы модуля ADC с разрешением 16 бит равна 36 МГц
  // Максимальная частота работы модуля ADC с разрешением 14 бит равна 50 МГц
  // Режим DUAL[4:0] = 00000: Independent mode

  tmp = 0
       + LSHIFT(1, 24) // VBATEN      | VBAT enable  (Only ADC3)
       + LSHIFT(1, 23) // TSEN        | Temperature sensor voltage enable (Only ADC3)
       + LSHIFT(1, 22) // VREFEN      | VREFINT enable (Only ADC3)
       + LSHIFT(0, 18) // PRESC[3:0]  | ADC prescaler. Делитель используется если CKMODE = 0
                       //               0000: input ADC clock not divided
                       //               0001: input ADC clock divided by 2
                       //               0010: input ADC clock divided by 4
                       //               0011: input ADC clock divided by 6
                       //               0100: input ADC clock divided by 8
                       //               0101: input ADC clock divided by 10
                       //               0110: input ADC clock divided by 12
                       //               0111: input ADC clock divided by 16
                       //               1000: input ADC clock divided by 32
                       //               1001: input ADC clock divided by 64
                       //               1010: input ADC clock divided by 128
                       //               1011: input ADC clock divided by 256
       + LSHIFT(3, 16) // CKMODE[1:0] | ADC clock mode
                       //               00: adc_ker_ck_input (Asynchronous clock mode)
                       //               01: adc_sclk/1       (Synchronous clock mode).
                       //               10: adc_sclk/2       (Synchronous clock mode)
                       //               11: adc_sclk/4       (Synchronous clock mode)
       + LSHIFT(0, 14) // DAMDF[1:0]  | Dual ADC Mode Data Format
                       //               00: Dual ADC mode without data packing (ADCx_CDR and ADCx_CDR2 registers not used).
                       //               01: Reserved.
                       //               10: Data formatting mode for 32 down to 10-bit resolution
                       //               11: Data formatting mode for 8-bit resolution
       + LSHIFT(0,  8) // DELAY[3:0]  | Delay between 2 sampling phases. These bits are used in dual interleaved modes. (Only ADC12)
       + LSHIFT(0,  0) // DUAL[4:0]   | Dual ADC mode selection (Only ADC12)
                       //               00000: Independent mode
                       //               00001: Combined regular simultaneous + injected simultaneous mode
                       //               00010: Combined regular simultaneous + alternate trigger mode
                       //               00011: Combined Interleaved mode + injected simultaneous mode
                       //               00100: Reserved.
                       //               00101: Injected simultaneous mode only
                       //               00110: Regular simultaneous mode only
                       //               00111: Interleaved mode only
                       //               01001: Alternate trigger mode only
  ;
  *p_CCR = tmp;
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param ADC
  \param trg_state
  \param trg_ch

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t ADC_config_hw_trigegr(ADC_TypeDef *ADC, uint8_t trg_state, uint8_t trg_ch)
{
  uint32_t tmp;
  tmp = ADC->CFGR;
  tmp &= ~(LSHIFT(0x3, 10) |  LSHIFT(0x1F,  5));
  tmp |=(LSHIFT(trg_state & 0x3, 10) | LSHIFT(trg_ch & 0x1F,  5));
  ADC->CFGR = tmp;
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param p_CCR

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t ADC_config(ADC_TypeDef *ADC)
{
  uint32_t tmp;
// Преобразования стартуют по сигналу (программному или аппаратному)
// Назначаем Single conversion mode (CONT=0) чтобы цепочка преобразований начиналась только по сигналу
// CONT    = 0 - Цепочка каналов обрабатывается один раз и следующая обработка произойдет по следующему сигналу
// DISCEN  = 0 - Discontinuous mode for regular channels disabled - все каналы в цепочке обрабатываются без пауз один за другим по сигналу
// DISCNUM - не используется в этом режиме
// Результат сохраняем в FIFO и используем DMA
  tmp = 0
       + LSHIFT(0, 31) // JQDIS        | 0: Injected Queue enabled, 1: Injected Queue disabled
       + LSHIFT(0, 26) // AWD1CH[4:0]  | Analog watchdog 1 channel selection
                       //                 00000: ADC analog input channel-0 monitored by AWD1
                       //                 00001: ADC analog input channel-1 monitored by AWD1
                       //                  ...
                       //                 10010: ADC analog input channel-19 monitored by AWD1
       + LSHIFT(0, 25) // JAUTO        | 0: Automatic injected group conversion disabled, 1: Automatic injected group conversion enabled
       + LSHIFT(0, 24) // JAWD1EN      | 0: Analog watchdog 1 disabled on injected channels, 1: Analog watchdog 1 enabled on injected channels
       + LSHIFT(0, 23) // AWD1EN       | 0: Analog watchdog 1 disabled on regular channels, 1: Analog watchdog 1 enabled on regular channels
       + LSHIFT(0, 22) // AWD1SGL      | 0: Analog watchdog 1 enabled on all channels, 1: Analog watchdog 1 enabled on a single channel
       + LSHIFT(0, 21) // JQM          | JSQR queue mode
                       //                 0:  The Queue is never empty and maintains the last written configuration into JSQR.
                       //                 1:  The Queue can be empty and when this occurs, the software and hardware
       + LSHIFT(0, 20) // JDISCEN      | 0: Discontinuous mode on injected channels disabled, 1: Discontinuous mode on injected channels enabled
       + LSHIFT(0, 17) // DISCNUM[2:0] | Discontinuous mode channel count
                       //                 000: 1 channel
                       //                 001: 2 channels
                       //                  ...
                       //                 111: 8 channels
       + LSHIFT(1, 16) // DISCEN       | 0: Discontinuous mode for regular channels disabled, 1: Discontinuous mode for regular channels enabled
       + LSHIFT(0, 14) // AUTDLY       | 0: Auto-delayed conversion mode off, 1: Auto-delayed conversion mode on
       + LSHIFT(0, 13) // CONT         | 0: Single conversion mode, 1: Continuous conversion mode
       + LSHIFT(0, 12) // OVRMOD       | Overrun Mode. 0: ADC_DR register is preserved with the old data when an overrun is detected. FIFO (8 data) enabled
       + LSHIFT(0, 10) // EXTEN[1:0]   | External trigger enable and polarity selection for regular channels
                       //                 00: Hardware trigger detection disabled (conversions can be launched by software)
                       //                 01: Hardware trigger detection on the rising edge
                       //                 10: Hardware trigger detection on the falling edge
                       //                 11: Hardware trigger detection on both the rising and falling edges
       + LSHIFT(0,  5)
                       // EXTSEL[4:0]  | External trigger selection for regular group
                       //                 00000: Event 0
                       //                 00001: Event 1
                       //                  ...
                       //                 11111: Event 31
       + LSHIFT(0,  2) // RES[2:0]     | Data resolution
                       //                 000: 16 bits
                       //                 101: 14 bits (for devices revision V)
                       //                 110: 12 bits (for devices revision V)
                       //                 011: 10 bits
                       //                 111: 8 bits (for devices revision V)
       + LSHIFT(3,  0) // DMNGT[1:0]   | Data Management configuration
                       //                 00: Regular conversion data stored in DR only
                       //                 01: DMA One Shot Mode selected
                       //                 10: DFSDM mode selected
                       //                 11: DMA Circular Mode selected
  ;
  ADC->CFGR = tmp;

// Оверсэмплинг не используем, результаты не сдвигаем
  tmp = 0
       + LSHIFT(0, 28) // LSHIFT[3:0] | Left shift factor
                       //                0000: No left shift
                       //                0001: Shift left 1-bit
                       //                0010: Shift left 2-bits
                       //                 ...
                       //                1111: Shift left 15-bits
       + LSHIFT(0, 16) // OSVR[9:0]   | Oversampling ratio
                       //                0: 1x (no oversampling)
                       //                1: 2x
                       //                2: 3x
                       //                 ...
                       //                1023: 1024x
       + LSHIFT(0, 14) // RSHIFT4     | Right-shift data after Offset 4 correction
       + LSHIFT(0, 13) // RSHIFT3     | Right-shift data after Offset 3 correction
       + LSHIFT(0, 12) // RSHIFT2     | Right-shift data after Offset 2 correction
       + LSHIFT(0, 11) // RSHIFT1     | Right-shift data after Offset 1 correction
       + LSHIFT(0, 10) // ROVSM       | Regular Oversampling mode
                       //                0: Continued mode: When injected conversions are triggered, the oversampling is temporary stopped and continued after the injection sequence (oversampling buffer is maintained during injected sequence)
                       //                1: Resumed mode: When injected conversions are triggered, the current oversampling is aborted and resumed from start after the injection sequence (oversampling buffer is zeroed by injected sequence start)
       + LSHIFT(0,  9) // TROVS       | Triggered Regular Oversampling
                       //                0: All oversampled conversions for a channel are done consecutively following a trigger
                       //                1: Each oversampled conversion for a channel needs a new trigger
       + LSHIFT(0,  5) // OVSS[3:0]   | Oversampling right shift
                       //                0000: No right shift
                       //                0001: Shift right 1-bit
                       //                0010: Shift right 2-bits
                       //                 ...
                       //                1011: Shift right 11-bits
       + LSHIFT(0,  1) // JOVSE       | 0: Injected Oversampling disabled, 1: Injected Oversampling enabled
       + LSHIFT(0,  0) // ROVSE       | 0: Regular Oversampling disabled, 1: Regular Oversampling enabled
  ;

  ADC->CFGR2 = tmp;
  return RES_OK;
}


/*-----------------------------------------------------------------------------------------------------


  \param ADC

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t ADC_calibrating_single_ended(ADC_TypeDef *ADC)
{
  ADC->CR =(ADC->CR & ~A_CR_ADCALDIF) | A_CR_ADCALLIN | A_CR_ADCAL;
  while ((ADC->CR & A_CR_ADCAL) != 0) DELAY_1us; // Ждем пока не завершиться калибровка
  return RES_OK;
}


/*-----------------------------------------------------------------------------------------------------
  Инициализация всех ADC

  Максимальная частота тактирования для STM32H753VIH6 ревизии V (BOOST = 11, Resolution <= 14 bits) равна 50 MHz
  Для Resolution = 16 bits частота должна быть снижена до 32 MHz

  Максимальное время выборки для худшего случая с выходным сопротивлением источника 15Ком не более 4 мкс

  adc_ker_ck_input = 60 MHz - здесь не используем
  adc_hclk = adc_sclk = AHB clock = 240 MHz
  adc_sclk делим на 4 , и далее делитель на 2. Итого тактирование ADC производится частотой 30 МГц

  \param void
-----------------------------------------------------------------------------------------------------*/
uint32_t ADCs_init(void)
{
  volatile uint32_t vreg;


  // Включаем тактирование всех ADC

  RCC->AHB1ENR |= RCC_AHB1ENR_ADC12EN;
  vreg = RCC->AHB1ENR; // Обратное чтение чтобы выполнить обязательную задержку

  RCC->AHB4ENR |= RCC_AHB4ENR_ADC3EN;
  vreg = RCC->AHB4ENR; // Обратное чтение чтобы выполнить обязательную задержку


  ADC_disable(ADC1);
  ADC_disable(ADC2);
  ADC_disable(ADC3);

  if (ADC_vreg_sw_on(ADC1) != RES_OK) return RES_ERROR;
  if (ADC_vreg_sw_on(ADC2) != RES_OK) return RES_ERROR;
  if (ADC_vreg_sw_on(ADC3) != RES_OK) return RES_ERROR;

  ADC_conf_clock(&(ADC12_COMMON->CCR));
  ADC_conf_clock(&(ADC3_COMMON->CCR));

  ADC_config(ADC1);
  ADC_config(ADC2);
  ADC_config(ADC3);

  ADC_calibrating_single_ended(ADC1);
  ADC_calibrating_single_ended(ADC2);
  ADC_calibrating_single_ended(ADC3);

  ADC_set_channels(ADC1_ID);
  ADC_set_channels(ADC2_ID);
  ADC_set_channels(ADC3_ID);

  ADC_config_hw_trigegr(ADC1, ADC_HW_TRG_RSE, ADC_TRG_HRTIM1_ADCTRG1);
  ADC_config_hw_trigegr(ADC2, ADC_HW_TRG_RSE, ADC_TRG_HRTIM1_ADCTRG1);
  ADC_config_hw_trigegr(ADC3, ADC_HW_TRG_RSE, ADC_TRG_HRTIM1_ADCTRG1);

  ADC_enable(ADC1);
  ADC_enable(ADC2);
  ADC_enable(ADC3);




//  Тестовая реализация варианта работы с использование прерываний от ADC
//
//  NVIC_SetPriority(ADC_IRQn, NVIC_EncodePriority(prioritygroup, ADC12_ISR_PRIO, 0));
//  NVIC_SetPriority(ADC3_IRQn, NVIC_EncodePriority(prioritygroup, ADC3_ISR_PRIO, 0));
//
//  NVIC_EnableIRQ(ADC_IRQn);
//  NVIC_EnableIRQ(ADC3_IRQn);
//
//  adc1_ch_pt = 0;
//  adc2_ch_pt = 0;
//  adc3_ch_pt = 0;
//
//  ADC1->IER = A_IE_EOSIE | A_IE_EOCIE;
//  ADC2->IER = A_IE_EOSIE | A_IE_EOCIE;
//  ADC3->IER = A_IE_EOSIE | A_IE_EOCIE;


  ADCs_software_start(ADC1);
  ADCs_software_start(ADC2);
  ADCs_software_start(ADC3);

  TS_CAL2 = (float)(*TEMPSENSOR_CAL2_ADDR);
  TS_CAL1 = (float)(*TEMPSENSOR_CAL1_ADDR);


  return RES_OK;
}


/*-----------------------------------------------------------------------------------------------------
  Запуск преобразований.
  Функция должна вызываться и в случае программного запуска и н старте в случае аппаратного запуска по триггеру


  \param void
-----------------------------------------------------------------------------------------------------*/
void ADCs_software_start(ADC_TypeDef *ADC)
{
  ADC->CR |= A_CR_ADSTART;
}

/*-----------------------------------------------------------------------------------------------------
  Обработчик прерываний ADC1 и ADC2
  Тестовая реализация при изучении работы программирования ADC
  Переносит данные после каждого преобразования в память


  \param void
-----------------------------------------------------------------------------------------------------*/
//void ADC_IRQHandler(void)
//{
//  uint32_t flags;
//
//  flags = ADC1->ISR;
//  if (flags & A_IS_EOS)
//  {
//    ADC1->ISR = A_IS_EOS;
//  }
//
//  if (flags & A_IS_EOC)
//  {
//    //ADC1->ISR = A_IS_EOC;  Сдесь сброс флага не нужен поскольку он будет сброшен после чтения регистра ADC1->DR
//    *(ADC_ch_cfg[0].ch_cfg[adc1_ch_pt].p_var)= ADC1->DR;
//    adc1_ch_pt++;
//    if (adc1_ch_pt >= ADC_CH_CNT) adc1_ch_pt=0;
//  }
//
//  if (flags & A_IS_OVR)
//  {
//    ADC1->ISR = A_IS_OVR;
//  }
//
//  flags = ADC2->ISR;
//  if (flags & A_IS_EOS)
//  {
//    // Извлекаем данные из FIFO
//    ADC2->ISR = A_IS_EOS;
//  }
//
//  if (flags & A_IS_EOC)
//  {
//    *(ADC_ch_cfg[1].ch_cfg[adc2_ch_pt].p_var)= ADC2->DR;
//    adc2_ch_pt++;
//    if (adc2_ch_pt >= ADC_CH_CNT) adc2_ch_pt=0;
//  }
//
//  if (flags & A_IS_OVR)
//  {
//    ADC2->ISR = A_IS_OVR;
//  }
//}


/*-----------------------------------------------------------------------------------------------------
  Обработчик прерываний ADC3


  \param void
-----------------------------------------------------------------------------------------------------*/
//void ADC3_IRQHandler(void)
//{
//  uint32_t flags;
//
//  flags = ADC3->ISR;
//
//  if (flags & A_IS_EOS)
//  {
//    ADC3->ISR = A_IS_EOS;
//  }
//
//  if (flags & A_IS_EOC)
//  {
//    *(ADC_ch_cfg[2].ch_cfg[adc3_ch_pt].p_var)= ADC3->DR;
//    adc3_ch_pt++;
//    if (adc3_ch_pt >= ADC_CH_CNT) adc3_ch_pt=0;
//  }
//
//  if (flags & A_IS_OVR)
//  {
//    ADC3->ISR = A_IS_OVR;
//  }
//
//}



