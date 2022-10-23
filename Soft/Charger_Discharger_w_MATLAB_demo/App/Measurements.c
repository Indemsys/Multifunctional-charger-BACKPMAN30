﻿// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2022-09-12
// 11:09:55 
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"

static T_exp_filter_fp flt_cbl_acc_i;
static T_exp_filter_fp flt_cbl_psrc_i;
static T_exp_filter_fp flt_cbl_load_i;
static T_exp_filter_fp flt_cbl_acc_v;
static T_exp_filter_fp flt_cbl_psrc_v;
static T_exp_filter_fp flt_cbl_load_v;
static T_exp_filter_fp flt_cbl_sys_v;
static T_exp_filter_fp flt_cbl_ref_v;


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Measurements_Init(void)
{
  flt_cbl_acc_i.alpha  = 1 / 128.0f;
  flt_cbl_psrc_i.alpha = 1 / 128.0f;
  flt_cbl_load_i.alpha = 1 / 128.0f;
  flt_cbl_acc_v.alpha  = 1 / 128.0f;
  flt_cbl_psrc_v.alpha = 1 / 128.0f;
  flt_cbl_load_v.alpha = 1 / 128.0f;
  flt_cbl_sys_v.alpha  = 1 / 128.0f;
  flt_cbl_ref_v.alpha  = 1 / 128.0f;
}

/*-----------------------------------------------------------------------------------------------------
  Функция вызывается каждые 1000 мкс

  \param void
-----------------------------------------------------------------------------------------------------*/
uint32_t Measurements_fp_conversion(void)
{
  float psrc_v;
  float acc_v;
  float load_v;
  float sys_v;
  float ref_v;
  float acc_i;
  float psrc_i;
  float load_i;


  if (Get_app_mutex(10) != RES_OK)  return RES_ERROR;

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


  fp_meas.flt_acc_i_uncal  = Exponential_filter_fp(&flt_cbl_acc_i  , fp_meas.acc_i);
  fp_meas.flt_psrc_i_uncal = Exponential_filter_fp(&flt_cbl_psrc_i , fp_meas.psrc_i);
  fp_meas.flt_load_i_uncal = Exponential_filter_fp(&flt_cbl_load_i , fp_meas.load_i);


  acc_i   = fp_meas.flt_acc_i_uncal  - wvar.acc_current_offset;
  psrc_i  = fp_meas.flt_psrc_i_uncal - wvar.psrc_current_offset;
  load_i  = fp_meas.flt_load_i_uncal - wvar.load_current_offset;

  psrc_v  = Exponential_filter_fp(&flt_cbl_psrc_v , fp_meas.psrc_v);
  acc_v   = Exponential_filter_fp(&flt_cbl_acc_v  , fp_meas.acc_v);
  load_v  = Exponential_filter_fp(&flt_cbl_load_v , fp_meas.load_v);
  sys_v   = Exponential_filter_fp(&flt_cbl_sys_v  , fp_meas.sys_v);
  ref_v   = Exponential_filter_fp(&flt_cbl_ref_v  , fp_meas.ref_v);

  // Устраняем ошибку измерения инструментальных усилителей при низком измеряемом напряжении
  if (psrc_v < 0.2f)  psrc_v = 0.0f;
  if (acc_v < 0.2f)   acc_v = 0.0f;
  if (load_v < 0.2f)  load_v = 0.0f;

  // Устраняем ошибку измерения малых токов вызванную неустранимыми смещениями измерителей тока
  if (fabs(psrc_i) < 0.02f)  psrc_i  = 0.0f;
  if (fabs(acc_i) < 0.02f)   acc_i   = 0.0f;
  if (fabs(load_i) < 0.1f)   load_i  = 0.0f;

  fp_meas.flt_psrc_i       = psrc_i;
  fp_meas.flt_load_i       = load_i;

  fp_meas.flt_psrc_v       = psrc_v;
  fp_meas.flt_acc_v        = acc_v;
  fp_meas.flt_load_v       = load_v;
  fp_meas.flt_sys_v        = sys_v;
  fp_meas.flt_ref_v        = ref_v;


  // Здесь обнуляем ток если он реально не может течь в данном напряавлении при данном состоянии ключей
  // Таким образом пытаемся снизить ошибку смещения при измерении тока
  if (Get_EN_CHARGER()==0)
  {
    if ((Get_ASW_R()==0) && (Get_ASW_F()==0))
    {
      acc_i =0;
    }
    else if ((Get_ASW_R()==1) && (Get_ASW_F()==0))
    {
      if (acc_i > 0) acc_i = 0;
    }
    else if ((Get_ASW_R()==0) && (Get_ASW_F()==1))
    {
      if (acc_i < 0) acc_i = 0;
    }
  }
  fp_meas.flt_acc_i        = acc_i;


  if (Get_EN_CHARGER() || Get_ASW_R() || Get_ASW_F())
  {
    fp_meas.charge          += (double)(-acc_i) * ((double)ADC_RESULTS_PERIOD_US / (1000000.0 * 60.0 * 60.0));
  }

  //  Если включена нагрузка то учитывает ток нагрузки при расчете щзаряда аккумулятора
  if (Get_LSW_F())
  {
    fp_meas.charge          += (double)(-load_i) * ((double)ADC_RESULTS_PERIOD_US / (1000000.0 * 60.0 * 60.0));
  }

  if ((Get_PSW_F()==0) )
  {
    fp_meas.control_pwr = fp_meas.flt_psrc_i * fp_meas.flt_psrc_v;
  }

  fp_meas.psrc_pwr         = (fp_meas.flt_psrc_i * fp_meas.flt_psrc_v) - fp_meas.control_pwr;


  if (fp_meas.psrc_pwr < 0) fp_meas.psrc_pwr = 0;

  fp_meas.acc_pwr          = -fp_meas.flt_acc_i * fp_meas.flt_acc_v;

  if ((fp_meas.psrc_pwr > fp_meas.acc_pwr) && (fp_meas.acc_pwr > 0))
  {
    fp_meas.charge_loss             = fp_meas.psrc_pwr - fp_meas.acc_pwr;
  }
  else
  {
    fp_meas.charge_loss            = 0.0f;
  }

  if ((fp_meas.acc_pwr > 0) && (fp_meas.psrc_pwr != 0))
  {
    fp_meas.charge_efficiency     =(fp_meas.acc_pwr * 100.0f) / fp_meas.psrc_pwr;
  }
  else
  {
    fp_meas.charge_efficiency     = 0.0f;
  }


  float R0 = 10000.0f;
  float T0 = 298.15f; // 25 C в кельвинах
  float B  = 3435.0f;
  float RC = R0 * expf(-B / T0);
  float v  = (float)adcs.smpl_TSENS   * AIN_SCALE;
  float r  =(VREF-v) / (v / R0);
  float t  = B / logf(r / RC)- 273.15f;
  fp_meas.t_sens = t;

  if (t > DCDC_OVERHEAT_BOUNDARY)
  {
    DCDC_emergency_shutdown(FAULT_OVERHEAT);
  }

  if (Get_EN_CHARGER()==1)
  {
    if (pwr_cbl.dcdc_on_timer < 0xFFFFFFFF) pwr_cbl.dcdc_on_timer++;

    // Сигнал DCDC_PGOOD возникает приблизительно через 3 мс после появления сигнала EN_CHARGER включающего DCDC
    if ((Get_DCDC_PGOOD()==0) && (pwr_cbl.dcdc_on_timer>10))
    {
      DCDC_emergency_shutdown(FAULT_DCDC_FAULT);
    }
  }
  else
  {
    pwr_cbl.dcdc_on_timer = 0;
  }


  Put_app_mutex();
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param res

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Get_measurements_results(T_measurement_results *p_res)
{
  if (Get_app_mutex(10) != RES_OK)  return RES_ERROR;

  p_res->psrc_v = fp_meas.flt_psrc_v;
  p_res->psrc_i = fp_meas.flt_psrc_i;
  p_res->sys_v  = fp_meas.flt_sys_v;
  p_res->acc_v  = fp_meas.flt_acc_v;
  p_res->acc_i  = fp_meas.flt_acc_i;
  p_res->load_v = fp_meas.flt_load_v;
  p_res->load_i = fp_meas.flt_load_i;
  p_res->temper = fp_meas.t_sens;
  p_res->charge = fp_meas.charge;
  Put_app_mutex();
  return RES_OK;
}

