#include "App.h"


char                 cpu_id_str[25];


uint8_t              request_save_to_NV_mem;
uint8_t              request_to_reset_log;

T_app_state          app_state;


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
   app_state.ain1     =   fp_meas.ain1     ;
   app_state.ain2     =   fp_meas.ain2     ;
   app_state.ain3     =   fp_meas.ain3     ;
   app_state.ain4     =   fp_meas.ain4     ;
   app_state.vref165  =   fp_meas.vref165  ;
   app_state.acc_i    =   fp_meas.acc_i    ;
   app_state.psrc_i   =   fp_meas.psrc_i   ;
   app_state.load_i   =   fp_meas.load_i   ;
   app_state.acc_v    =   fp_meas.acc_v    ;
   app_state.psrc_v   =   fp_meas.psrc_v   ;
   app_state.load_v   =   fp_meas.load_v   ;
   app_state.sys_v    =   fp_meas.sys_v    ;
   app_state.ref_v    =   fp_meas.ref_v    ;
   app_state.cpu_temp =   fp_meas.cpu_temp ;
   app_state.t_sens   =   fp_meas.t_sens   ;
}


/*-----------------------------------------------------------------------------------------------------


  \param initial_input
-----------------------------------------------------------------------------------------------------*/
void Thread_main(ULONG initial_input)
{
  T_stfs_info     stfs_info;

  // ???????????? HRTIM1 ?????????????????? ?????? ?????? ???????????????????? ???????????????????? DC/DC ?????????????????????? ?? ?????????????? ???????????????? ?????? ?????????????? ???????????????????????????? ??????
  // ?????? ???????????????? ?????????????????????? ?????? ?????????? ?????????????? ???????????????????????? ?? ?????????????????????? ?????????????? ???????????? ???????????????????????? DMA
  ADCs_init();
  DMA1_from_ADC_init();
  HRTIM1_init();

  // ?????????????????? ?????????????????????? ?????????? ?????????????? ???????????????????????? ?????? ???????????????? ???????????? ?????? ?????????????????????? ???????????????????????? ?????????????????????? ???????????????????? ?? ????????
  ref_time = Measure_reference_time_interval(REF_TIME_INTERVAL);


  Get_CPU_info(cpu_id_str);


  //Open_FileX_media();

  AppLogg_init();
  APPLOG("Main thread started");

  Thread_idle_create();

  STfs_init(INT_FLASH_BANK2,&stfs_info);

  Restore_NV_parameters();

  Thread_IO_create();

  App_USBX_Device_Init();

  if (wvar.vcom_mode == 0)
  {
    Task_VT100_create(Mnsdrv_get_usbfs_vcom0_driver(),0);
  }
  else
  {
    FreeMaster_task_create((ULONG)Mnsdrv_get_usbfs_vcom0_driver());
  }

  //Thread_net_create();
  Thread_CAN_create();
  Thread_HMI_create();

  for (;;)
  {


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

    // ???????????????? ???? ???????? ?????? RTOS, ?????? ?????????????????????? ?????????????????????????? ???????????????????? ?????????? ?? ?????????????? 1000 ????
    tx_thread_sleep(1);

  }
}


