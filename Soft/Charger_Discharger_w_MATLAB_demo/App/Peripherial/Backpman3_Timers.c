﻿// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2022-02-15
// 18:01:44 
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"


uint32_t  dcdc_duty_cycle_last;
uint32_t  dcdc_duty_cycle;
uint32_t  dcdc_compare_val;
uint32_t  adc_trig_compare_val;
/*-----------------------------------------------------------------------------------------------------
  Инициализируем таймер HRTIM1 для генерации PWM для DC/DC преобразователя напряжения управляющего внешним исполнительным устройством
  и для генерации сигнала тригера ADC

  HRTIM1 подключен к шине APB2 с частотой тактирования 120 МГц
  Тактирование выполняем от CPU clock (480 MHz)


  Генерируем PWM на частоте 100 Кгц, период 10 мкс
  Сигналы триггера ADC выдаем с той же частотой, но на каждом сигнале триггера оцифровывается только один канал АЦП
  Полный цикл оцифровки всех каналов АЦП производится за 5 сигналов триггера т.е. с периодом 50 мкс

  Импульсы на выходах формируются с помощью компаратора 1
  Триггер для ADC формируются с помощью компаратора 3 так чтобы он приходился на середину выходных импульсов и был как можно дальше от их фронтов

  \param void
-----------------------------------------------------------------------------------------------------*/
void HRTIM1_init(void)
{
  uint32_t  tmp;

  RCC->CFGR |= BIT(14); // Устаноавливаем бит HRTIMSEL - тактирование от CPU clock (480 MHz)

  RCC->APB2ENR |= BIT(29); // Разрешаем тактирование HRTIM

  HRTIM1->sMasterRegs.MCR = 0; // Выключаем все таймеры, чтобы иметь возможность их конфигурировать

  HRTIM1->sCommonRegs.CR2 = BIT(9); // TARST: Timer A counter software reset

  // Программируем регистр TIMxCR канала A таймера HRTIM1
  // Используем только таймер A для управления сигналами SLND_T и SLND_B
  // Включаем режим работы с мертвым временнем в районе 50 нс
  // Делитель частоты = 1
  // Включаем режим обновления регистров по событию перезагрузки таймера.

  tmp = 0
       + LSHIFT(0, 28) // UPDGAT[3:0]  | These bits define how the update occurs relatively to the burst DMA transaction and the external update request on update enable inputs 1 to 3
       + LSHIFT(1, 27) // PREEN        | 0: Preload disabled: the write access is directly done into the active register. 1: Preload enabled: the write access is done into the preload register
       + LSHIFT(0, 25) // DACSYNC[1:0] |
                       //                00: No DAC trigger generated
                       //                01: Trigger generated on hrtim_dac_trg1
                       //                10: Trigger generated on hrtim_dac_trg2
                       //                11: Trigger generated on hrtim_dac_trg3
       + LSHIFT(0, 24) // MSTU         | 0: Update by master timer disabled. 1: Update by master timer enabled
       + LSHIFT(0, 23) // TEU          | 0: Update by timer E disabled. 1: Update by timer E enabled
       + LSHIFT(0, 22) // TDU          | 0: Update by timer D disabled. 1: Update by timer D enabled
       + LSHIFT(0, 21) // TCU          | 0: Update by timer C disabled. 1: Update by timer C enabled
       + LSHIFT(0, 20) // TBU          | 0: Update by timer B disabled. 1: Update by timer B enabled
       + LSHIFT(0, 19) // TAU          | 0: Update by timer A disabled. 1: Update by timer A enabled
       + LSHIFT(1, 18) // TxRSTU       | 0: Update by timer x reset / roll-over disabled. 1: Update by timer x reset / roll-over enabled
       + LSHIFT(0, 17) // TxREPU       | 0: Update on repetition disabled. 1: Update on repetition enabled
       + LSHIFT(0, 14) // DELCMP4[1:0] | This bitfield defines whether the compare register is behaving in standard mode (compare match issued as soon as counter equal compare), or in auto-delayed mode
                       //                00: CMP4 register is always active (standard compare mode)
                       //                01: CMP4 value is recomputed and is active following a capture 2 event
                       //                10: CMP4 value is recomputed and is active following a capture 2 event, or is recomputed and active after Compare 1 match (timeout function if capture 2 event is missing)
                       //                11: CMP4 value is recomputed and is active following a capture event, or is recomputed and active after Compare 3 match (timeout function if capture event is missing)
       + LSHIFT(0, 12) // DELCMP2[1:0] | This bitfield defines whether the compare register is behaving in standard mode (compare match issued as soon as counter equal compare), or in auto-delayed mode
                       //                00: CMP2 register is always active (standard compare mode)
                       //                01: CMP2 value is recomputed and is active following a capture 1 event
                       //                10: CMP2 value is recomputed and is active following a capture 1 event, or is recomputed and active after Compare 1 match (timeout function if capture event is missing)
                       //                11: CMP2 value is recomputed and is active following a capture 1 event, or is recomputed and active after Compare 3 match (timeout function if capture event is missing)
       + LSHIFT(0, 11) // SYNCSTRT     | This bit defines the Timer x behavior following the synchronization event:
                       //                0: No effect on Timer
                       //                1: A synchronization input event starts the Timer
       + LSHIFT(0, 10) // SYNCRST      | This bit defines the Timer x behavior following the synchronization event:
                       //                0: No effect on Timer
                       //                1: A synchronization input event resets the Timer
       + LSHIFT(0,  6) // PSHPLL       | 0: Push-Pull mode disabled. 1: Push-Pull mode enabled
       + LSHIFT(0,  5) // HALF         | 0: Half mode disabled, 1: Half mode enabled
       + LSHIFT(0,  4) // RETRIG       | This bit defines the counter behavior in single shot mode.
                       //                0: The timer is not re-triggerable: a counter reset is done if the counter is stopped (period elapsed in single-shot mode or counter stopped in continuous mode)
                       //                1: The timer is re-triggerable: a counter reset is done whatever the counter state.
       + LSHIFT(1,  3) // CONT         | This bit defines the timer operating mode.
                       //                0: The timer operates in single-shot mode and stops when it reaches TIMxPER value
                       //                1: The timer operates in continuous mode and rolls over to zero when it reaches TIMxPER value
       + LSHIFT(5,  0) // CKPSCx[2:0]  | These bits define the master timer clock prescaler ratio.
                       //                101: fCOUNTER = fHRTIM
                       //                110: fCOUNTER = fHRTIM / 2
                       //                111: fCOUNTER = fHRTIM / 4
  ;
  HRTIM1->sTimerxRegs[0].TIMxCR = tmp;


  HRTIM1->sTimerxRegs[0].PERxR = HRTIM1_PER;
  HRTIM1->sTimerxRegs[0].CNTxR = 0;

  HRTIM1->sTimerxRegs[0].REPxR = 0; // Счетчик повторений не используем. Триггер на ADC срабатывать должен на каждом периоде PWM.

  // Программируем работу выхода 1 канала A  таймера HRTIM1
  // Канал 2 программировать не надо, поскольку он в режиме с мертвым временем определяется работой выхода 1
  // Используем для управления скважностью компаратор CMP1
  HRTIM1->sTimerxRegs[0].SETx1R = BIT(2); // Bit 2 PER: Timer x Period. Timer A Period event forces the output to its active state.
  HRTIM1->sTimerxRegs[0].RSTx1R = BIT(3); // Bit 3 CMP1: Timer x Compare 1 Timer A compare 1 event forces the output to its inactive state


  // Программирование регистра конфигурации выходов канала A  таймера HRTIM1
  // Включаем функцию мертвого времени
  tmp = 0
       + LSHIFT(0, 23) // DIDL2       | Output 2 Deadtime upon burst mode Idle entry. This bit can delay the idle mode entry by forcing a deadtime insertion before switching the outputs to their idle state.
       + LSHIFT(0, 22) // CHP2        | Output 2 Chopper enable
       + LSHIFT(0, 20) // FAULT2[1:0] | These bits select the output 2 state after a fault event. 00: No action. 01: Active. 10: Inactive. 11: High-Z
       + LSHIFT(1, 19) // IDLES2      | This bit selects the output 2 idle state. 0: Inactive. 1: Active
       + LSHIFT(0, 18) // IDLEM2      | This bit selects the output 2 idle mode. 0: No action. 1: The output is in idle state when requested by the burst mode controller.
       + LSHIFT(0, 17) // POL2        | This bit selects the output 2 polarity. 0: positive polarity (output active high). 1: negative polarity (output active low)
       + LSHIFT(0, 10) // DLYPRT[2:0] | These bits define the source and outputs on which the delayed protection schemes are applied.
       + LSHIFT(0,  9) // DLYPRTEN    | This bit enables the delayed protection scheme. 0: No action. 1: Delayed protection is enabled, as per DLYPRT[2:0] bits
       + LSHIFT(1,  8) // DTEN        | This bit enables the deadtime insertion on output 1 and output 2. 0: Output 1 and output 2 signals are independent. 1: Deadtime is inserted between output 1 and output 2 (reference signal is output 1 signal generator)
       + LSHIFT(0,  7) // DIDL1       | Output 1 Deadtime upon burst mode Idle entry. This bit can delay the idle mode entry by forcing a deadtime insertion before switching the outputs to their idle state.
       + LSHIFT(0,  6) // CHP1        | Output 1 Chopper enable
       + LSHIFT(0,  4) // FAULT1[1:0] | These bits select the output 1 state after a fault event. 00: No action. 01: Active. 10: Inactive. 11: High-Z
       + LSHIFT(0,  3) // IDLES1      | This bit selects the output 1 idle state. 0: Inactive. 1: Active
       + LSHIFT(0,  2) // IDLEM1      | This bit selects the output 1 idle mode. 0: No action. 1: The output is in idle state when requested by the burst mode controller.
       + LSHIFT(0,  1) // POL1        | This bit selects the output 1 polarity. 0: positive polarity (output active high). 1: negative polarity (output active low)
  ;
  HRTIM1->sTimerxRegs[0].OUTxR = tmp;


  // Программирование регистра мертвого времени для блока выходов канала A таймера HRTIM1
  // Частота тактирования счетчика мертвого времени равна частоте тактирования таймера
  tmp = 0
       + LSHIFT(1, 31) // DTFLKx     | This write-once bit prevents the deadtime (sign and value) to be modified, if enabled.
       + LSHIFT(1, 30) // DTFSLKx    | This write-once bit prevents the sign of falling deadtime to be modified, if enabled.
       + LSHIFT(0, 25) // SDTFx      | 0: Positive deadtime on falling edge. 1: Negative deadtime on falling edge
       + LSHIFT(HRTIM1_DEADT_VAL, 16) // DTFx[8:0]  | This register holds the value of the deadtime following a falling edge of reference PWM signal. tDTF = DTFx[8:0] x tDTG
       + LSHIFT(1, 15) // DTRLKx     | This write-once bit prevents the deadtime (sign and value) to be modified, if enabled
       + LSHIFT(1, 14) // DTRSLKx    | This write-once bit prevents the sign of deadtime to be modified, if enabled
       + LSHIFT(3, 10) // DTPRSC[2:0]| This register holds the value of the deadtime clock prescaler.
                       //               011: tDTG= tHRTIM
                       //               100: tDTG= tHRTIM x 2
                       //               101: tDTG= tHRTIM x 4
                       //               110: tDTG= tHRTIM x 8
                       //               111: tDTG= tHRTIM x 16
       + LSHIFT(0,  9) // SDTRx      | 0: Positive deadtime on rising edge. 1: Negative deadtime on rising edge
       + LSHIFT(HRTIM1_DEADT_VAL,  0) // DTRx[8:0]  | This register holds the value of the deadtime following a rising edge of reference PWM signal. tDTR = DTRx[8:0] x tDTG
  ;
  HRTIM1->sTimerxRegs[0].DTxR = tmp;

  dcdc_duty_cycle_last = 0;

  // Закоментировано, поскольку работы выходов здесь не разрешаем
  //HRTIM1->sCommonRegs.OENR = BIT(0)+ BIT(1); // Bit 0 TA1OEN: Timer A Output 1 (HRTIM_CHA1) Enable. Bit 1 TA2OEN: Timer A Output 2 Enable

  // Назначаем сигналы запускающие сигнал триггера ADC_TRG_HRTIM1_ADCTRG1
  //HRTIM1->sCommonRegs.ADC1R = BIT(14); // Бит AD1TARST. ADC trigger 1 on Timer A Reset and counter roll-over. This bit enables the generation of an ADC Trigger upon Timer A reset and roll-over event, on ADC Trigger 1 output.
  HRTIM1->sCommonRegs.ADC1R = BIT(11); // Бит ADC1TAC3. ADC trigger 1 on Timer A Compare 3. This bit enables the generation of an ADC Trigger upon Timer A Compare 3 event, on ADC Trigger 1 output (hrtim_adc_trg1).

  HRTIM1->sTimerxRegs[0].TIMxDIER = 0; // Прерывания не используем

  // Программируем компаратор формирующий импулься на выходах
  dcdc_compare_val = HRTIM1_PER;                    // Установка в это значение приводит к переходу в низкое состояние сигнала на выходе Timer A Output 1
  HRTIM1->sTimerxRegs[0].CMP1xR = dcdc_compare_val; // Загружаем начальное значение в регистр компаратора 1 канала A, который определяет коэффициента заполнения PWM

  // Программируем компаратор CMP2xR формирующий триггер для ADC
  if (dcdc_compare_val >=  (HRTIM1_PER / 2))
  {
    adc_trig_compare_val = dcdc_compare_val / 2;
  }
  else
  {
    adc_trig_compare_val = dcdc_compare_val +(HRTIM1_PER - dcdc_compare_val) / 2;
  }
  HRTIM1->sTimerxRegs[0].CMP3xR = adc_trig_compare_val;


  // Запускаем работу PWM
  HRTIM1->sMasterRegs.MCR = BIT(17); // Bit 17 TACEN: Timer A counter enable


}


/*-----------------------------------------------------------------------------------------------------
  Установка коэффициента заполнения ШИМ сигнала DCDC преобразователя

  \param val - задаваемая величина. 0 - отсутствие ШИМ. 100 - максимальный коэффициент заполнения
-----------------------------------------------------------------------------------------------------*/
void Set_DCDC_PWM_percent(uint32_t val)
{
  if (val >= 100) val = 99;

  if (val == 0)
  {
    // Запрещаем работу выходов
    HRTIM1->sCommonRegs.ODISR = BIT(0)+ BIT(1); // Bit 0 TA1OEN: Timer A Output 1 (HRTIM_CHA1) Disable. Bit 1 TA2OEN: Timer A Output 2 Disable
    dcdc_duty_cycle_last  = 0;
    dcdc_compare_val = HRTIM1_PER;              // Установка в это значение приводит к переходу в низкое состояние сигнала на выходе Timer A Output 1
    HRTIM1->sTimerxRegs[0].CMP1xR = dcdc_compare_val;
    return;
  }

  // Перепрограммируем компаратор формирующий импулься на выходах
  dcdc_compare_val =((uint32_t)val * HRTIM1_PER) / 100ul;
  HRTIM1->sTimerxRegs[0].CMP1xR = dcdc_compare_val;

  // Перепрограммируем компаратор CMP2xR формирующий триггер для ADC
  if (dcdc_compare_val >=  (HRTIM1_PER / 2))
  {
    adc_trig_compare_val = dcdc_compare_val / 2;
  }
  else
  {
    adc_trig_compare_val = dcdc_compare_val +(HRTIM1_PER - dcdc_compare_val) / 2;
  }
  HRTIM1->sTimerxRegs[0].CMP3xR = adc_trig_compare_val;


  if (dcdc_duty_cycle_last == 0)
  {
    // Разрешаем работу выходов если она была перед этим запрещена
    HRTIM1->sCommonRegs.OENR = BIT(0)+ BIT(1); // Bit 0 TA1OEN: Timer A Output 1 (HRTIM_CHA1) Enable. Bit 1 TA2OEN: Timer A Output 2 Enable
  }
  dcdc_duty_cycle_last  = val;

}

/*-----------------------------------------------------------------------------------------------------
  Таймер используется для формирования каналом CH3 сигнала синхронизации DCDC преобразователя LTC3789EGN зарядника
  Микросхема LTC3789EGN может синхронизировать свою частоту в пределах 200...600 кГц

  Таймер находится на шине APB2 и тактируется частотой 240 МГц
  Наибольший коэфициент деления = 1200
  Нименьший  = 400

  \param void
-----------------------------------------------------------------------------------------------------*/
void TIM1_init(void)
{

  //RCC->CFGR |= BIT(15);    // Bit 15 TIMPRE: Timers clocks prescaler selection

  RCC->APB2ENR |= BIT(0);   //Bit 0 TIM1EN: TIM1 peripheral clock enable

  TIM1->CR1 = 0
             + LSHIFT(0, 11) // UIFREMAP: UIF status bit remapping. 0: No remapping. UIF status bit is not copied to TIMx_CNT register bit 31.
             + LSHIFT(0,  8) // CKD[1:0]: Clock division. This bit-field indicates the division ratio between the timer clock (CK_INT) frequency and the dead-time and sampling clock
             + LSHIFT(1,  7) // ARPE: Auto-reload preload enable. 0: TIMx_ARR register is not buffered
             + LSHIFT(0,  5) // CMS[1:0]: Center-aligned mode selection. 00: Edge-aligned mode. The counter counts up or down depending on the direction bit (DIR).
             + LSHIFT(0,  4) // DIR: Direction. 0: Counter used as upcounter
             + LSHIFT(0,  3) // OPM: One pulse mode. 0: Counter is not stopped at update event
             + LSHIFT(0,  2) // URS: Update request source. This bit is set and cleared by software to select the UEV event sources.
             + LSHIFT(0,  1) // UDIS: Update disable. This bit is set and cleared by software to enable/disable UEV event generation.
             + LSHIFT(0,  0) // CEN: Counter enable
  ;

  //
  TIM1->CR2 = 0
             + LSHIFT(0, 20) // MMS2[3:0]: Master mode selection 2. 0000: Reset
             + LSHIFT(0, 18) // OIS6: Output Idle state 6 (OC6 output)
             + LSHIFT(0, 16) // OIS5: Output Idle state 5 (OC5 output)
             + LSHIFT(0, 14) // OIS4: Output Idle state 4 (OC4 output)
             + LSHIFT(0, 13) // OIS3N: Output Idle state 3 (OC3N output)
             + LSHIFT(0, 12) // OIS3: Output Idle state 3 (OC3 output)
             + LSHIFT(0, 11) // OIS2N: Output Idle state 2 (OC2N output)
             + LSHIFT(0, 10) // OIS2: Output Idle state 2 (OC2 output)
             + LSHIFT(0,  9) // OIS1N: Output Idle state 1 (OC1N output)
             + LSHIFT(0,  8) // OIS1: Output Idle state
             + LSHIFT(0,  7) // TI1S: TI1 selection. 0: The TIMx_CH1 pin is connected to TI1 input
             + LSHIFT(0,  4) // MMS[2:0]: Master mode selection. 000: Reset
             + LSHIFT(0,  3) // CCDS: Capture/compare DMA selection. 0: CCx DMA request sent when CCx event occurs
             + LSHIFT(0,  2) // CCUS: Capture/compare control update selection. 0: When capture/compare control bits are preloaded (CCPC=1), they are updated by setting the COMG bit only
             + LSHIFT(0,  0) // CCPC: Capture/compare preloaded control. 0: CCxE, CCxNE and OCxM bits are not preloaded
  ;

  TIM1->DIER = 0;        // Запрещаем все прерывания
  TIM1->SR = 0xFFFFFFFF; // Сбрасываем все запросы прерываний



  TIM1->CCMR1 = 0; // Конфигурирование CH1 CH3
  TIM1->CCMR2 = 0  // Конфигурирование CH3 CH4
               + LSHIFT(0, 24) // OC4M[3]   Output Compare 4 mode
               + LSHIFT(0, 16) // OC3M[3]   Output Compare 3 mode
               + LSHIFT(0, 15) // OC4CE Output Compare 4 clear enable. 0: OC1Ref is not affected by the ETRF input
               + LSHIFT(0, 12) // OC4M[2:0] Output Compare 4 mode. 0000: Frozen
               + LSHIFT(0, 11) // OC4PE Output Compare 4 preload enable. 0: Preload register on TIMx_CCR1 disabled. TIMx_CCR1 can be written at anytime, the new value is taken in account immediately.
               + LSHIFT(0, 10) // OC4FE Output Compare 4 fast enable. 0: CC4 behaves normally depending on counter and CCR1 values even when the trigger is ON.
               + LSHIFT(0,  8) // CC4S[1:0] 00: CC4 channel is configured as output
               + LSHIFT(0,  7) // OC3CE Output Compare 3 clear enable. 0: OC1Ref is not affected by the ETRF input
               + LSHIFT(0,  4) // OC3M[2:0] Output Compare 3 mode. 0110: PWM mode 1
               + LSHIFT(0,  3) // OC3PE Output Compare 3 preload enable. 0: Preload register on TIMx_CCR1 disabled. TIMx_CCR1 can be written at anytime, the new value is taken in account immediately.
               + LSHIFT(0,  2) // OC3FE Output Compare 3 fast enable. 0: CC3 behaves normally depending on counter and CCR1 values even when the trigger is ON.
               + LSHIFT(0,  0) // CC3S[1:0]. 00: CC3 channel is configured as output
  ;
  TIM1->CCMR3 = 0; // Конфигурирование CH5 CH6

  TIM1->CNT = 0;
  TIM1->PSC = 0;         // Предделитель входной частоты таймера не используем

  TIM1_set_CH3_freq(TIM1_MIN_FREQ_KHZ);
  TIM1_set_CH3_mode(1);

  TIM1->CCER |= BIT(8);  // CC3E: Capture/Compare 3 output enable
  TIM1->BDTR |= BIT(15); // MOE: Main output enable. 1: OC and OCN outputs are enabled if their respective enable bits are set (CCxE, CCxNE in TIMx_CCER register).

  TIM1->CR1  |= BIT(0); // Запускаем таймер
}


/*-----------------------------------------------------------------------------------------------------
  Установка работы выхода канала 3 таймера TIM1
  Значение v:
  0 - на выходе 0
  1 - на выходе 1
  2 - на выходе меандр сигнала компаратора

  \param v
-----------------------------------------------------------------------------------------------------*/
void TIM1_set_CH3_mode(uint8_t v)
{
  uint32_t tmp;
  switch (v)
  {
  case 0:
    tmp = TIM1->CCMR2;
    tmp &= ~LSHIFT(7,  4);
    tmp |= LSHIFT(2,  4);
    TIM1->CCMR2 = tmp;
    pwr_cbl.dcdc_mode = 0;
    break;
  case 1:
    tmp = TIM1->CCMR2;
    tmp &= ~LSHIFT(7,  4);
    tmp |= LSHIFT(1,  4);
    TIM1->CCMR2 = tmp;
    pwr_cbl.dcdc_mode = 1;
    break;
  case 2:
    tmp = TIM1->CCMR2;
    tmp &= ~LSHIFT(7,  4);
    tmp |= LSHIFT(6,  4);
    TIM1->CCMR2 = tmp;
    pwr_cbl.dcdc_mode = 2;
    break;
  }
}

/*-----------------------------------------------------------------------------------------------------
  Установка частоты в килогерцах на выходе канала 3 таймера TIM1

  \param v
-----------------------------------------------------------------------------------------------------*/
void TIM1_set_CH3_freq(uint32_t freq_khz)
{
  uint32_t arr;

  if (freq_khz < TIM1_MIN_FREQ_KHZ) freq_khz = TIM1_MIN_FREQ_KHZ;
  if (freq_khz > TIM1_MAX_FREQ_KHZ) freq_khz = TIM1_MAX_FREQ_KHZ;

  pwr_cbl.dcdc_sync_freq = freq_khz;

  arr =(TIM1_CLOCK_KHZ / freq_khz);
  TIM1->ARR = arr;
  TIM1->CCR3 = arr / 2;
}

