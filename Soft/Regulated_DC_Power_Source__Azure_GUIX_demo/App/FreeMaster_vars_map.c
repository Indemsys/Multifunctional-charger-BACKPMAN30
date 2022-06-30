#include "App.h"
#include "freemaster_cfg.h"
#include "freemaster.h"
#include "freemaster_tsa.h"


FMSTR_TSA_TABLE_BEGIN(app_vars)


FMSTR_TSA_RW_VAR(g_cpu_usage                            ,FMSTR_TSA_UINT32)
FMSTR_TSA_RW_VAR(g_cpu_usage_fp                         ,FMSTR_TSA_FLOAT)

FMSTR_TSA_RW_VAR(fp_meas.ain1                           ,FMSTR_TSA_FLOAT)
FMSTR_TSA_RW_VAR(fp_meas.ain2                           ,FMSTR_TSA_FLOAT)
FMSTR_TSA_RW_VAR(fp_meas.ain3                           ,FMSTR_TSA_FLOAT)
FMSTR_TSA_RW_VAR(fp_meas.ain4                           ,FMSTR_TSA_FLOAT)
FMSTR_TSA_RW_VAR(fp_meas.vref165                        ,FMSTR_TSA_FLOAT)
FMSTR_TSA_RW_VAR(fp_meas.acc_i                          ,FMSTR_TSA_FLOAT)
FMSTR_TSA_RW_VAR(fp_meas.psrc_i                         ,FMSTR_TSA_FLOAT)
FMSTR_TSA_RW_VAR(fp_meas.load_i                         ,FMSTR_TSA_FLOAT)
FMSTR_TSA_RW_VAR(fp_meas.acc_v                          ,FMSTR_TSA_FLOAT)
FMSTR_TSA_RW_VAR(fp_meas.psrc_v                         ,FMSTR_TSA_FLOAT)
FMSTR_TSA_RW_VAR(fp_meas.load_v                         ,FMSTR_TSA_FLOAT)
FMSTR_TSA_RW_VAR(fp_meas.sys_v                          ,FMSTR_TSA_FLOAT)
FMSTR_TSA_RW_VAR(fp_meas.ref_v                          ,FMSTR_TSA_FLOAT)
FMSTR_TSA_RW_VAR(fp_meas.cpu_temp                       ,FMSTR_TSA_FLOAT)
FMSTR_TSA_RW_VAR(fp_meas.t_sens                         ,FMSTR_TSA_FLOAT)
FMSTR_TSA_RW_VAR(fp_meas.psrc_pwr                       ,FMSTR_TSA_FLOAT)
FMSTR_TSA_RW_VAR(fp_meas.acc_pwr                        ,FMSTR_TSA_FLOAT)
FMSTR_TSA_RW_VAR(fp_meas.loss                           ,FMSTR_TSA_FLOAT)
FMSTR_TSA_RW_VAR(fp_meas.efficiency                     ,FMSTR_TSA_FLOAT)

FMSTR_TSA_RW_VAR(adcs.smpl_ACC_I                        ,FMSTR_TSA_UINT32)
FMSTR_TSA_RW_VAR(adcs.smpl_PSRC_I                       ,FMSTR_TSA_UINT32)
FMSTR_TSA_RW_VAR(adcs.smpl_LOAD_V                       ,FMSTR_TSA_UINT32)
FMSTR_TSA_RW_VAR(adcs.smpl_AIN2                         ,FMSTR_TSA_UINT32)
FMSTR_TSA_RW_VAR(adcs.smpl_VREF165                      ,FMSTR_TSA_UINT32)
FMSTR_TSA_RW_VAR(adcs.smpl_ACC_V                        ,FMSTR_TSA_UINT32)
FMSTR_TSA_RW_VAR(adcs.smpl_SYS_V                        ,FMSTR_TSA_UINT32)
FMSTR_TSA_RW_VAR(adcs.smpl_AIN3                         ,FMSTR_TSA_UINT32)
FMSTR_TSA_RW_VAR(adcs.smpl_AIN1                         ,FMSTR_TSA_UINT32)
FMSTR_TSA_RW_VAR(adcs.smpl_AIN4                         ,FMSTR_TSA_UINT32)
FMSTR_TSA_RW_VAR(adcs.smpl_TSENS                        ,FMSTR_TSA_UINT32)
FMSTR_TSA_RW_VAR(adcs.smpl_PSRC_V                       ,FMSTR_TSA_UINT32)
FMSTR_TSA_RW_VAR(adcs.smpl_LOAD_I                       ,FMSTR_TSA_UINT32)
FMSTR_TSA_RW_VAR(adcs.smpl_TERM                         ,FMSTR_TSA_UINT32)
FMSTR_TSA_RW_VAR(adcs.smpl_VREF                         ,FMSTR_TSA_UINT32)

FMSTR_TSA_RW_VAR(pst.OUT1                               ,FMSTR_TSA_UINT8)
FMSTR_TSA_RW_VAR(pst.OUT2                               ,FMSTR_TSA_UINT8)
FMSTR_TSA_RW_VAR(pst.OUT3                               ,FMSTR_TSA_UINT8)
FMSTR_TSA_RW_VAR(pst.OUT4                               ,FMSTR_TSA_UINT8)
FMSTR_TSA_RW_VAR(pst.LED_green                          ,FMSTR_TSA_UINT8)
FMSTR_TSA_RW_VAR(pst.LED_red                            ,FMSTR_TSA_UINT8)
FMSTR_TSA_RW_VAR(pst.ASW_R                              ,FMSTR_TSA_UINT8)
FMSTR_TSA_RW_VAR(pst.ASW_F                              ,FMSTR_TSA_UINT8)
FMSTR_TSA_RW_VAR(pst.PSW_R                              ,FMSTR_TSA_UINT8)
FMSTR_TSA_RW_VAR(pst.PSW_F                              ,FMSTR_TSA_UINT8)
FMSTR_TSA_RW_VAR(pst.LSW_F                              ,FMSTR_TSA_UINT8)
FMSTR_TSA_RW_VAR(pst.OLED_RES                           ,FMSTR_TSA_UINT8)
FMSTR_TSA_RW_VAR(pst.OLEDV                              ,FMSTR_TSA_UINT8)
FMSTR_TSA_RW_VAR(pst.OLED_DC                            ,FMSTR_TSA_UINT8)
FMSTR_TSA_RW_VAR(pst.OLED_CS                            ,FMSTR_TSA_UINT8)
FMSTR_TSA_RW_VAR(pst.EN_CHARGER                         ,FMSTR_TSA_UINT8)
FMSTR_TSA_RW_VAR(pst.DCDC_MODE                          ,FMSTR_TSA_UINT8)



FMSTR_TSA_TABLE_END();

