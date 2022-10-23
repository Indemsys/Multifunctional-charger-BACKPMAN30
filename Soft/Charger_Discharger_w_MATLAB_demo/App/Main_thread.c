#include "App.h"



char                 cpu_id_str[25];


uint8_t              request_save_to_NV_mem;
uint8_t              request_to_reset_log;
uint8_t              g_BSD_initialised;

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
  TIM1_init();

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

  Thread_APP_create();


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
  Thread_FTP_client_create();


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


    // Управление светодиодами энкодера
    if (pwr_cbl.fault != DCDC_NO_FAULT)
    {
      Set_output_blink_3(OUTP_LED_RED);
      Set_output_off(OUTP_LED_GREEN);
    }
    else
    {
      if (Get_EN_CHARGER())
      {
        Set_output_on(OUTP_LED_GREEN);
      }
      else
      {
        Set_output_off(OUTP_LED_GREEN);
      }
      if (Get_LSW_F())
      {
        Set_output_on(OUTP_LED_RED);
      }
      else
      {
        Set_output_off(OUTP_LED_RED);
      }
    }



    // Задержка на один тик RTOS, для обеспечения периодичности выполнения цикла с частото 1000 Гц
    tx_thread_sleep(1);
  }
}


