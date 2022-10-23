// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2022-03-02
// 10:42:06
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"
#include   "freemaster.h"

static TX_MUTEX              app_mutex;

TX_THREAD                    app_thread;
#pragma data_alignment=8
uint8_t                      thread_app_stack[THREAD_APP_STACK_SIZE] @ "DTCM";

uint8_t                      zero_calibr_en;
T_run_average_uint32_N       flt_zero_curr;

T_hmi_cbl                    hmi_cbl;

uint32_t                     cpu_id[4];

static void Thread_APP(ULONG initial_input);

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Thread_APP_create(void)
{
  Outputs_mutex_create();
  tx_mutex_create(&app_mutex, "APP", TX_INHERIT);
  tx_thread_create(&app_thread, "APP", Thread_APP,
                   0,
                   (void *)thread_app_stack, // stack_start
                   THREAD_APP_STACK_SIZE,    // stack_size
                   THREAD_APP_PRIORITY,      // priority. Numerical priority of thread. Legal values range from 0 through (TX_MAX_PRIORITES-1), where a value of 0 represents the highest priority.
                   THREAD_APP_PRIORITY,      // preempt_threshold. Highest priority level (0 through (TX_MAX_PRIORITIES-1)) of disabled preemption. Only priorities higher than this level are allowed to preempt this thread. This value must be less than or equal to the specified priority. A value equal to the thread priority disables preemption-threshold.
                   TX_NO_TIME_SLICE,
                   TX_AUTO_START);
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void DCDC_init(void)
{
  Set_DCDC_MODE(1); // Режим mode=1 препятствует появлению реверсного тока в DCDC  преобразователе
  DCDC_reset_fault();
  pwr_cbl.dac_steps = 1;
  pwr_cbl.dac_data  = Get_DAC_value_from_steps(pwr_cbl.dac_steps);
  DAC_proc(pwr_cbl.dac_data);

  // Вычисляем код минимального напряжения на входе зарядника
  dcdc_min_in_v = (uint32_t)(MIN_CHARGER_INPUT_V * INTV_SCALE / AIN_SCALE);
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void HMI_init(void)
{
  hmi_cbl.hmi_mode      = HMI_SCREEN_PWR_CONTROL;
  hmi_cbl.enc_cnt       = Get_encoder_counter();
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void Controlled_voltage_source_mode(void)
{

  // Если контроль ведется из MATLAB, то не выполнять команды от энкодера по управлению DCDC
  if (pwr_cbl.matlab_control == 0)
  {


    int32_t   delta = Get_encoder_counter_delta(&hmi_cbl.enc_cnt);
    if (delta != 0)
    {
      if  ((pwr_cbl.dac_steps + delta) > STEPS_NUM)
      {
        pwr_cbl.dac_steps = STEPS_NUM;
      }
      else if  ((pwr_cbl.dac_steps + delta) < 1)
      {
        pwr_cbl.dac_steps = 1;
      }
      else
      {
        pwr_cbl.dac_steps += delta;
      }

      pwr_cbl.dac_data = Get_DAC_value_from_steps(pwr_cbl.dac_steps);

    }
    DAC_proc(pwr_cbl.dac_data);


    if (Get_switch_press_signal())
    {
      DCDC_toggle_state();
    }

  }




  if (Get_switch_long_press_signal())
  {
    hmi_cbl.curr_menu_item = HMI_MENU_INDX_PWR_CONTROL;
    hmi_cbl.hmi_mode       = HMI_SCREEN_MENU;
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void Net_info_mode(void)
{
  if (Get_switch_press_signal())
  {
    hmi_cbl.curr_menu_item = HMI_MENU_INDX_NET_INFO;
    hmi_cbl.hmi_mode       = HMI_SCREEN_MENU;
  }
  if (Get_switch_long_press_signal())
  {
    if (wvar.usb_mode == 0) wvar.usb_mode = 1;
    else wvar.usb_mode = 0;
    Req_to_save_settings();
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void SD_card_info_mode(void)
{
  if (Get_switch_press_signal() || Get_switch_long_press_signal())
  {
    hmi_cbl.curr_menu_item = HMI_MENU_INDX_SD_CARD_INFO;
    hmi_cbl.hmi_mode       = HMI_SCREEN_MENU;
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Calibrate_acc_current_offset(void)
{
  if (Get_app_mutex(10) != RES_OK)  return ;
  wvar.acc_current_offset  = fp_meas.flt_acc_i_uncal;
  Put_app_mutex();
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Calibrate_psrc_current_offset(void)
{
  if (Get_app_mutex(10) != RES_OK)  return ;
  wvar.psrc_current_offset = fp_meas.flt_psrc_i_uncal;
  Put_app_mutex();
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Calibrate_load_current_offset(void)
{
  if (Get_app_mutex(10) != RES_OK)  return ;
  wvar.load_current_offset = fp_meas.flt_load_i_uncal;
  Put_app_mutex();
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void Calibrate_mode(void)
{
  if (Get_switch_press_signal())
  {

    // Переносим текущие токи в калибровочные константы
    Calibrate_acc_current_offset();
    Calibrate_psrc_current_offset();
    Calibrate_load_current_offset();


    Req_to_save_settings();
  }

  if (Get_switch_long_press_signal())
  {
    hmi_cbl.curr_menu_item = HMI_MENU_INDX_CALIBRATE;
    hmi_cbl.hmi_mode       = HMI_SCREEN_MENU;
  }
}

/*-----------------------------------------------------------------------------------------------------
  Обслуживание работы меню

  \param void
-----------------------------------------------------------------------------------------------------*/
static void Menu_mode(void)
{
  int32_t n     = hmi_cbl.curr_menu_item;
  int32_t delta = Get_encoder_counter_delta(&hmi_cbl.enc_cnt);

  if (delta != 0)
  {
    if (delta > 0)
    {
      if (n > 0) n--;
    }
    else
    {
      if (n < (HMU_MENU_ITEMS_COUNT-1)) n++;
    }
    hmi_cbl.curr_menu_item = n;
  }

  if (Get_switch_press_signal())
  {
    switch (n)
    {
    case HMI_MENU_INDX_PWR_CONTROL:
      hmi_cbl.hmi_mode       = HMI_SCREEN_PWR_CONTROL;
      break;
    case HMI_MENU_INDX_SD_CARD_INFO:
      hmi_cbl.hmi_mode       = HMI_SCREEN_SD_CARD_INFO;
      break;
    case HMI_MENU_INDX_NET_INFO:
      hmi_cbl.hmi_mode       = HMI_SCREEN_NET_INFO;
      break;
    case HMI_MENU_INDX_CALIBRATE:
      hmi_cbl.hmi_mode       = HMI_SCREEN_CALIBRATE;
      break;
    case HMI_MENU_INDX_RESET_FAULTS:
      DCDC_reset_fault();
      hmi_cbl.hmi_mode       = HMI_SCREEN_PWR_CONTROL;
      break;
    }
  }

}

/*-----------------------------------------------------------------------------------------------------
   Выполнение процедур для того или иного режима выбранного на дисплее

  \param void
-----------------------------------------------------------------------------------------------------*/
static void HMI_control(void)
{

  switch (hmi_cbl.hmi_mode)
  {
  case HMI_SCREEN_PWR_CONTROL:
    Controlled_voltage_source_mode();
    break;

  case HMI_SCREEN_NET_INFO:
    Net_info_mode();
    break;

  case HMI_SCREEN_SD_CARD_INFO:
    SD_card_info_mode();
    break;

  case HMI_SCREEN_CALIBRATE:
    Calibrate_mode();
    break;

  case HMI_SCREEN_MENU:
    Menu_mode();
    break;
  }


}


/*-----------------------------------------------------------------------------------------------------


  \param wait_cycles

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Get_app_mutex(uint32_t wait_cycles)
{
  if (tx_mutex_get(&app_mutex, 10) != TX_SUCCESS)  return RES_ERROR;
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Put_app_mutex(void)
{
  tx_mutex_put(&app_mutex);
}

/*-----------------------------------------------------------------------------------------------------


  \param p_val

  \return float
-----------------------------------------------------------------------------------------------------*/
uint32_t Get_measured_val(float *p_val, float *res)
{
  if (Get_app_mutex(10) != RES_OK)  return RES_ERROR;

  memcpy(res, p_val,  sizeof(res));

  Put_app_mutex();
  return RES_OK;
}


/*-----------------------------------------------------------------------------------------------------

  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Get_new_HMI_mode(T_hmi_cbl **hcbl)
{
  static uint32_t prev_hmi_screen = 0;

  if (hmi_cbl.hmi_mode != prev_hmi_screen)
  {
    prev_hmi_screen = hmi_cbl.hmi_mode;

    *hcbl =&hmi_cbl;
    return hmi_cbl.hmi_mode;
  }
  else
  {
    return 0;
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param flag

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Send_flag_to_app(uint32_t flag)
{
  return  tx_event_flags_set(&adc_flag, flag, TX_OR);
}

/*-----------------------------------------------------------------------------------------------------


  \param initial_input
-----------------------------------------------------------------------------------------------------*/
static void Thread_APP(ULONG initial_input)
{
  ULONG     actual_flags;
  uint32_t  status;

  DCDC_init();
  Measurements_Init();
  HMI_init();

  while (1)
  {
    // Флаг ADC_RES_READY взводится каждые 1000 мкс
    status =  tx_event_flags_get(&adc_flag, ADC_SYSV_TOO_LOW || ADC_RES_READY, TX_OR_CLEAR, &actual_flags,  MS_TO_TICKS(10));
    if (status == TX_SUCCESS)
    {
      //ITM_EVENT8(1,1);
      if (actual_flags & ADC_SYSV_TOO_LOW)
      {
        DCDC_emergency_shutdown(FAULT_TOO_LOW_SYSV);
      }

      // Время выполнения блока - 37 мкс без оптимизации
      if (actual_flags & ADC_RES_READY)
      {
        Inputs_processing();
        Measurements_fp_conversion();
        Outputs_state_automat();
        HMI_control();
        FMSTR_Recorder(0);
        tx_event_flags_set(&adc_flag, LOG_ADC_RES, TX_OR);
      }

      //ITM_EVENT8(1,0);

    } // tx_event_flags_get


  }
}

