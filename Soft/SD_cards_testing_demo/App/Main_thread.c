#include "App.h"



char                 cpu_id_str[25];


uint8_t              request_save_to_NV_mem;
uint8_t              request_to_reset_log;
uint8_t              g_BSD_initialised;

T_app_state          app_state;
uint8_t              g_usb_mode;

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Req_to_save_settings(void)
{
  request_save_to_NV_mem = 1;
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Req_to_reset_log_file(void)
{
  request_to_reset_log  = 1;
}

/*-----------------------------------------------------------------------------------------------------


  \param app_state_copy
-----------------------------------------------------------------------------------------------------*/
void Copy_app_state(T_app_state *app_state_copy)
{
  memcpy(app_state_copy,&app_state, sizeof(T_app_state));
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Take_app_state_snapshot(void)
{
  app_state.ain1     =   fp_meas.ain1;
  app_state.ain2     =   fp_meas.ain2;
  app_state.ain3     =   fp_meas.ain3;
  app_state.ain4     =   fp_meas.ain4;
  app_state.vref165  =   fp_meas.vref165;
  app_state.acc_i    =   fp_meas.acc_i;
  app_state.psrc_i   =   fp_meas.psrc_i;
  app_state.load_i   =   fp_meas.load_i;
  app_state.acc_v    =   fp_meas.acc_v;
  app_state.psrc_v   =   fp_meas.psrc_v;
  app_state.load_v   =   fp_meas.load_v;
  app_state.sys_v    =   fp_meas.sys_v;
  app_state.ref_v    =   fp_meas.ref_v;
  app_state.cpu_temp =   fp_meas.cpu_temp;
  app_state.t_sens   =   fp_meas.t_sens;
}


/*-----------------------------------------------------------------------------------------------------


  \param initial_input
-----------------------------------------------------------------------------------------------------*/
void Thread_main(ULONG initial_input)
{
  T_stfs_info     stfs_info;

  // Таймер HRTIM1 формирует ШИМ для управления понижающим DC/DC конвертером и сигналы триггера для запуска преобразований АЦП
  // Для переноса результатов АЦП после полного сканирования в назначенную область памяти используется DMA
  ADCs_init();
  DMA1_from_ADC_init();
  HRTIM1_init();

  // Выполняем контрольный замер времени одновременно эта задержка служит для обеспечения установления референсных напряжений в чипе
  ref_time = Measure_reference_time_interval(REF_TIME_INTERVAL);


  Get_CPU_info(cpu_id_str);


  Init_exFAT();

  AppLogg_init();
  APPLOG("Main thread started");

  Thread_idle_create();

  Log_SD_card_FS_init_state();

  STfs_init(INT_FLASH_BANK2,&stfs_info);

  Restore_NV_parameters();
  g_usb_mode = wvar.usb_mode;

  Thread_IO_create();


  App_USBX_Init();

  if (g_usb_mode == USB_MODE_DEVICE)
  {
    Thread_net_create();
    App_USBX_Device_Init();
    Task_VT100_create(Mnsdrv_get_usbfs_vcom0_driver(),0);
  }
  else
  {
    Thread_net_create();
    USB_host_cdc_ecm_init();
  }


  Thread_CAN_create();
  Thread_HMI_create();
  Thread_FreeMaster_create();


  for (;;)
  {
    static uint32_t mem_log_div = 100;
    if (request_save_to_NV_mem)
    {
      request_save_to_NV_mem = 0;
      if (Save_Params_to_STfs() == RES_OK)
      {
        APPLOG("Parameters to STfs were saved successfully.");
      }
      else
      {
        APPLOG("Error saving parameters to STfs.");
      }
    }

    mem_log_div--;
    if (mem_log_div == 0)
    {
      Mem_man_log_statistic();
      mem_log_div = 300000;
    }

    // Задержка на один тик RTOS, для обеспечения периодичности выполнения цикла с частото 1000 Гц
    tx_thread_sleep(1);
  }
}


