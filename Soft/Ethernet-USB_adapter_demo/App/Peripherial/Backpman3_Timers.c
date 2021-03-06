// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
  if (dcdc_compare_val >=  (HRTIM1_PER/2) )
  {
    adc_trig_compare_val = dcdc_compare_val/2;
  }
  else
  {
    adc_trig_compare_val = dcdc_compare_val + (HRTIM1_PER - dcdc_compare_val)/2;
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
  if (dcdc_compare_val >=  (HRTIM1_PER/2) )
  {
    adc_trig_compare_val = dcdc_compare_val/2;
  }
  else
  {
    adc_trig_compare_val = dcdc_compare_val + (HRTIM1_PER - dcdc_compare_val)/2;
  }
  HRTIM1->sTimerxRegs[0].CMP3xR = adc_trig_compare_val;


  if (dcdc_duty_cycle_last == 0)
  {
    // Разрешаем работу выходов если она была перед этим запрещена
    HRTIM1->sCommonRegs.OENR = BIT(0)+ BIT(1); // Bit 0 TA1OEN: Timer A Output 1 (HRTIM_CHA1) Enable. Bit 1 TA2OEN: Timer A Output 2 Enable
  }
  dcdc_duty_cycle_last  = val;

}

