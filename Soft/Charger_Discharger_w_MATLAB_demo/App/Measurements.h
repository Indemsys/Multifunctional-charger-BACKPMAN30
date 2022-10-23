#ifndef MEASUREMENTS_H
  #define MEASUREMENTS_H

typedef struct
{
  float  psrc_v;
  float  psrc_i;
  float  sys_v;
  float  acc_v;
  float  acc_i;
  float  load_v;
  float  load_i;
  float  temper;
  double charge;

} T_measurement_results;

void     Measurements_Init(void);
uint32_t Measurements_fp_conversion(void);
uint32_t Get_measurements_results(T_measurement_results *p_res);


#endif // MEASUREMENTS_H



