// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2022-03-02
// 10:42:06
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"
#include   "freemaster.h"

static TX_MUTEX               io_mutex;

TX_THREAD                     io_thread;
#pragma data_alignment=8
uint8_t                       thread_io_stack[THREAD_IO_STACK_SIZE];


volatile int32_t             g_encoder_counter;
static uint8_t               enc_a;
static uint8_t               enc_a_prev;
static uint8_t               curr_enc_a_smpl;
static uint8_t               prev_enc_a_smpl;
static uint32_t              enc_a_cnt;

static uint8_t               enc_b;
static uint8_t               curr_enc_b_smpl;
static uint8_t               prev_enc_b_smpl;
static uint32_t              enc_b_cnt;

static T_MaxFlat_1_1000_cbl  flt_cbl_acc_i  ;
static T_MaxFlat_1_1000_cbl  flt_cbl_psrc_i ;
static T_MaxFlat_1_1000_cbl  flt_cbl_load_i ;
static T_MaxFlat_1_1000_cbl  flt_cbl_acc_v  ;
static T_MaxFlat_1_1000_cbl  flt_cbl_psrc_v ;
static T_MaxFlat_1_1000_cbl  flt_cbl_load_v ;
static T_MaxFlat_1_1000_cbl  flt_cbl_sys_v  ;
static T_MaxFlat_1_1000_cbl  flt_cbl_ref_v  ;


uint8_t                      zero_calibr_en;
T_run_average_uint32_N       flt_zero_curr;


static void Thread_IO(ULONG initial_input);
static void Measurements_fp_conversion(void);
/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Thread_IO_create(void)
{
  Outputs_mutex_create();
  tx_mutex_create(&io_mutex, "IO", TX_INHERIT);
  tx_thread_create(&io_thread, "I/O", Thread_IO,
                   0,
                   (void *)thread_io_stack, // stack_start
                   THREAD_IO_STACK_SIZE,    // stack_size
                   THREAD_IO_PRIORITY,      // priority. Numerical priority of thread. Legal values range from 0 through (TX_MAX_PRIORITES-1), where a value of 0 represents the highest priority.
                   THREAD_IO_PRIORITY,      // preempt_threshold. Highest priority level (0 through (TX_MAX_PRIORITIES-1)) of disabled preemption. Only priorities higher than this level are allowed to preempt this thread. This value must be less than or equal to the specified priority. A value equal to the thread priority disables preemption-threshold.
                   TX_NO_TIME_SLICE,
                   TX_AUTO_START);
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void Application_function(void)
{
  static   uint8_t init=0;
  static   int32_t prev_encoder_counter;
  int32_t          tmp;

  if (init == 0)
  {
    init = 1;
    Set_DCDC_MODE(1); // Режим mode=1 препятствует появлению реверсного тока в DCDC  преобразователе
    prev_encoder_counter = g_encoder_counter;
  }


  if (inp_flags.f_enc_sw)
  {
    inp_flags.f_enc_sw = 0;
    if (inp.enc_sw == 0)
    {
      DCDC_toggle_state();
    }
  }

  tmp = g_encoder_counter;
  if (prev_encoder_counter != tmp)
  {
    int32_t delta = -(0x10000 / 205) * (tmp - prev_encoder_counter); // Знак устанавливаем чтобы вращение по часовой стрелке энкодера увеличивало напряжение

    if  ((g_dac_data + delta) > 0xFFFF)
    {
      g_dac_data = 0xFFFF;
    }
    else if  ((g_dac_data + delta) < 0)
    {
      g_dac_data = 0;
    }
    else
    {
      g_dac_data += delta;
    }
    prev_encoder_counter = tmp;
  }
  DAC_proc(g_dac_data);

}

/*-----------------------------------------------------------------------------------------------------


  \param initial_input
-----------------------------------------------------------------------------------------------------*/
static void Thread_IO(ULONG initial_input)
{
  ULONG     actual_flags;
  uint32_t  res;

  while (1)
  {
    res =  tx_event_flags_get(&adc_flag, ADC_RES_READY, TX_OR_CLEAR,&actual_flags,  MS_TO_TICKS(10));
    if (res == TX_SUCCESS)
    {
      ITM_EVENT8(1,1);

      // Время выполнения блока - 37 мкс без оптимизации

      Inputs_processing();
      Measurements_fp_conversion();
      Outputs_state_automat();
      Application_function();
      Take_app_state_snapshot();
      FMSTR_Recorder(0);

      ITM_EVENT8(1,0);

    } // tx_event_flags_get


  }
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void Measurements_fp_conversion(void)
{

  if (tx_mutex_get(&io_mutex, 10) != TX_SUCCESS)  return;

  fp_meas.ain1       = (float)adcs.smpl_AIN1    * AIN_SCALE / EXIN_SCALE;
  fp_meas.ain2       = (float)adcs.smpl_AIN2    * AIN_SCALE / EXIN_SCALE;
  fp_meas.ain3       = (float)adcs.smpl_AIN3    * AIN_SCALE / EXIN_SCALE;
  fp_meas.ain4       = (float)adcs.smpl_AIN4    * AIN_SCALE / EXIN_SCALE;

  fp_meas.vref165    = (float)adcs.smpl_VREF165 * AIN_SCALE;

  fp_meas.acc_i      =(((float)adcs.smpl_ACC_I   * AIN_SCALE)- fp_meas.vref165) / TMCS1100A2_SCALE;
  fp_meas.psrc_i     =(((float)adcs.smpl_PSRC_I  * AIN_SCALE)- fp_meas.vref165) / TMCS1100A2_SCALE;
  fp_meas.load_i     = (float)adcs.smpl_LOAD_I  * AIN_SCALE / INA240A1_SCALE;

  fp_meas.acc_v      = (float)adcs.smpl_ACC_V   * AIN_SCALE / INTV_SCALE;
  fp_meas.psrc_v     = (float)adcs.smpl_PSRC_V  * AIN_SCALE / INTV_SCALE;
  fp_meas.load_v     = (float)adcs.smpl_LOAD_V  * AIN_SCALE / INTV_SCALE;
  fp_meas.sys_v      = (float)adcs.smpl_SYS_V   * AIN_SCALE / INTV_SCALE;
  fp_meas.ref_v      = (float)adcs.smpl_VREF    * AIN_SCALE;

  fp_meas.cpu_temp   =((110.0f-30.0f) * ((float)adcs.smpl_TERM - TS_CAL1)) / (TS_CAL2 - TS_CAL1)+ 30.0f;


  fp_meas.flt_acc_i  = LPF_MaxFlat_1_1000_step(&flt_cbl_acc_i  , fp_meas.acc_i );
  fp_meas.flt_psrc_i = LPF_MaxFlat_1_1000_step(&flt_cbl_psrc_i , fp_meas.psrc_i);
  fp_meas.flt_load_i = LPF_MaxFlat_1_1000_step(&flt_cbl_load_i , fp_meas.load_i);
  fp_meas.flt_acc_v  = LPF_MaxFlat_1_1000_step(&flt_cbl_acc_v  , fp_meas.acc_v );
  fp_meas.flt_psrc_v = LPF_MaxFlat_1_1000_step(&flt_cbl_psrc_v , fp_meas.psrc_v);
  fp_meas.flt_load_v = LPF_MaxFlat_1_1000_step(&flt_cbl_load_v , fp_meas.load_v);
  fp_meas.flt_sys_v  = LPF_MaxFlat_1_1000_step(&flt_cbl_sys_v  , fp_meas.sys_v );
  fp_meas.flt_ref_v  = LPF_MaxFlat_1_1000_step(&flt_cbl_ref_v  , fp_meas.ref_v );

  fp_meas.psrc_pwr   = fp_meas.flt_psrc_i * fp_meas.flt_psrc_v;
  fp_meas.acc_pwr    = -fp_meas.flt_acc_i * fp_meas.flt_acc_v;
  fp_meas.loss       = fp_meas.psrc_pwr - fp_meas.acc_pwr;
  fp_meas.efficiency =(fp_meas.acc_pwr * 100.0f) / fp_meas.psrc_pwr;


  float R0 = 10000.0f;
  float T0 = 298.15f; // 25 C в кельвинах
  float B  = 3435.0f;
  float RC = R0 * expf(-B / T0);
  float v  = (float)adcs.smpl_TSENS   * AIN_SCALE;
  float r  =(VREF-v) / (v / R0);
  float t  = B / logf(r / RC)- 273.15f;
  fp_meas.t_sens = t;

  tx_mutex_put(&io_mutex);
}


/*-----------------------------------------------------------------------------------------------------


  \param p_val

  \return float
-----------------------------------------------------------------------------------------------------*/
uint32_t Get_measured_val(float *p_val, float *res)
{
  if (tx_mutex_get(&io_mutex, 10) != TX_SUCCESS)  return RES_ERROR;

  memcpy(res, p_val,  sizeof(res));

  tx_mutex_put(&io_mutex);
  return RES_OK;
}


/*-----------------------------------------------------------------------------------------------------
  Процедура обработки сигналов энкодера
  Вызывается с частотой в 20 КГц (каждые 50 мкс)

  Самый ктороткий импульс у энкодера зафиксирован длительностью 300 мкс

  \param void
-----------------------------------------------------------------------------------------------------*/
void Encoder_proc(void)
{

  curr_enc_a_smpl = Get_smpl_enc_a();
  curr_enc_b_smpl = Get_smpl_enc_b();

  if (curr_enc_a_smpl == 0)
  {
    if (prev_enc_a_smpl != 0)
    {
      enc_a_cnt = 0;
    }
    else
    {
      enc_a_cnt++;
    }
  }
  else
  {
    if (prev_enc_a_smpl == 0)
    {
      enc_a_cnt = 0;
    }
    else
    {
      enc_a_cnt++;
    }
  }

  if (curr_enc_b_smpl == 0)
  {
    if (prev_enc_b_smpl != 0)
    {
      enc_b_cnt = 0;
    }
    else
    {
      enc_b_cnt++;
    }
  }
  else
  {
    if (prev_enc_b_smpl == 0)
    {
      enc_b_cnt = 0;
    }
    else
    {
      enc_b_cnt++;
    }
  }
  if (enc_b_cnt > 0)
  {
    enc_b = curr_enc_b_smpl;
  }

  prev_enc_a_smpl = curr_enc_a_smpl;
  prev_enc_b_smpl = curr_enc_b_smpl;


  // Здесь определяем и считаем импульсы
  if (enc_a_cnt > 0)
  {
    enc_a = curr_enc_a_smpl;
    if ((enc_a == 1) && (enc_a_prev == 0))
    {
      if (enc_b == 1)
      {
        g_encoder_counter--;
      }
      else
      {
        g_encoder_counter++;
      }
    }
    enc_a_prev = enc_a;
  }

}

