// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2017-01-17
// 12:08:47
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"


/*-----------------------------------------------------------------------------------------------------


  \param inp
  \param fltr
  \param decimation - параметр прореживания. Необходим для фильтрации отсчетов берущихся реже чем основной период отсчетов АЦП

  \return uint16_t
-----------------------------------------------------------------------------------------------------*/
uint16_t MedianFilter_3uint16_decim(uint16_t inp, T_median_filter_uint16_decim *fltr, uint8_t decimation)
{
  fltr->smpl_cnt++;
  if (fltr->smpl_cnt >= decimation)
  {
    fltr->smpl_cnt = 0;
    if (fltr->en == 0)
    {
      fltr->arr[0] = inp;
      fltr->arr[1] = inp;
      fltr->arr[2] = inp;
      fltr->en = 1;
    }
    else
    {
      fltr->arr[2] = fltr->arr[1];
      fltr->arr[1] = fltr->arr[0];
      fltr->arr[0] = inp;
    }

    // Фильтрация медианным фильтром по выборке из 3-х
    if (fltr->arr[1] > fltr->arr[0])
    {
      if (fltr->arr[2] > fltr->arr[1])
      {
        fltr->output =  fltr->arr[1];
      }
      else if (fltr->arr[2] < fltr->arr[0])
      {
        fltr->output =   fltr->arr[0];
      }
      else
      {
        fltr->output =   fltr->arr[2];
      }
    }
    else
    {
      if (fltr->arr[0] < fltr->arr[2])
      {
        fltr->output =  fltr->arr[0];
      }
      else if (fltr->arr[1] > fltr->arr[2])
      {
        fltr->output =   fltr->arr[1];
      }
      else
      {
        fltr->output =   fltr->arr[2];
      }
    }
  }
  return fltr->output;

}


/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
int16_t MedianFilter_3uint16(uint16_t inp, T_median_filter_uint16 *fltr)
{
  if (fltr->en == 0)
  {
    fltr->arr[0] = inp;
    fltr->arr[1] = inp;
    fltr->arr[2] = inp;
    fltr->en = 1;
  }
  else
  {
    fltr->arr[2] = fltr->arr[1];
    fltr->arr[1] = fltr->arr[0];
    fltr->arr[0] = inp;
  }

  // Фильтрация медианным фильтром по выборке из 3-х
  if (fltr->arr[1] > fltr->arr[0])
  {
    if (fltr->arr[2] > fltr->arr[1]) return fltr->arr[1];
    else if (fltr->arr[2] < fltr->arr[0]) return fltr->arr[0];
    else return fltr->arr[2];
  }
  else
  {
    if (fltr->arr[0] < fltr->arr[2]) return fltr->arr[0];
    else if (fltr->arr[1] > fltr->arr[2]) return fltr->arr[1];
    else return fltr->arr[2];
  }
}

/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
int16_t MedianFilter_3int16(int16_t inp, T_median_filter_uint16 *fltr)
{
  if (fltr->en == 0)
  {
    fltr->arr[0] = inp;
    fltr->arr[1] = inp;
    fltr->arr[2] = inp;
    fltr->en = 1;
  }
  else
  {
    fltr->arr[2] = fltr->arr[1];
    fltr->arr[1] = fltr->arr[0];
    fltr->arr[0] = inp;
  }

  // Фильтрация медианным фильтром по выборке из 3-х
  if (fltr->arr[1] > fltr->arr[0])
  {
    if (fltr->arr[2] > fltr->arr[1]) return fltr->arr[1];
    else if (fltr->arr[2] < fltr->arr[0]) return fltr->arr[0];
    else return fltr->arr[2];
  }
  else
  {
    if (fltr->arr[0] < fltr->arr[2]) return fltr->arr[0];
    else if (fltr->arr[1] > fltr->arr[2]) return fltr->arr[1];
    else return fltr->arr[2];
  }
}
/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
uint32_t MedianFilter_3uint32(uint32_t inp, T_median_filter_uint32 *fltr)
{
  if (fltr->en == 0)
  {
    fltr->arr[0] = inp;
    fltr->arr[1] = inp;
    fltr->arr[2] = inp;
    fltr->en = 1;
  }
  else
  {
    fltr->arr[2] = fltr->arr[1];
    fltr->arr[1] = fltr->arr[0];
    fltr->arr[0] = inp;
  }

  // Фильтрация медианным фильтром по выборке из 3-х
  if (fltr->arr[1] > fltr->arr[0])
  {
    if (fltr->arr[2] > fltr->arr[1]) return fltr->arr[1];
    else if (fltr->arr[2] < fltr->arr[0]) return fltr->arr[0];
    else return fltr->arr[2];
  }
  else
  {
    if (fltr->arr[0] < fltr->arr[2]) return fltr->arr[0];
    else if (fltr->arr[1] > fltr->arr[2]) return fltr->arr[1];
    else return fltr->arr[2];
  }
}
/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
int32_t MedianFilter_3int32(int32_t inp, T_median_filter_int32 *fltr)
{
  if (fltr->en == 0)
  {
    fltr->arr[0] = inp;
    fltr->arr[1] = inp;
    fltr->arr[2] = inp;
    fltr->en = 1;
  }
  else
  {
    fltr->arr[2] = fltr->arr[1];
    fltr->arr[1] = fltr->arr[0];
    fltr->arr[0] = inp;
  }

  // Фильтрация медианным фильтром по выборке из 3-х
  if (fltr->arr[1] > fltr->arr[0])
  {
    if (fltr->arr[2] > fltr->arr[1]) return fltr->arr[1];
    else if (fltr->arr[2] < fltr->arr[0]) return fltr->arr[0];
    else return fltr->arr[2];
  }
  else
  {
    if (fltr->arr[0] < fltr->arr[2]) return fltr->arr[0];
    else if (fltr->arr[1] > fltr->arr[2]) return fltr->arr[1];
    else return fltr->arr[2];
  }
}

/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
float RunAverageFilter_float_8(float inp, T_run_average_float_8 *fltr)
{
  uint32_t i;
  if (fltr->en == 0)
  {
    for (i = 0; i < RUN_AVERAGE_FILTER_8; i++)
    {
      fltr->arr[i] = inp;
    }
    fltr->acc = inp * RUN_AVERAGE_FILTER_8;
    fltr->head = 0;
    fltr->en = 1;
    return inp;
  }
  else
  {
    uint32_t head =(fltr->head + 1) & (RUN_AVERAGE_FILTER_8 - 1);
    fltr->acc -= fltr->arr[head]; // Удаляем из аккумулятора последнее старое значение
    fltr->arr[head] = inp;        // Добавляем в память фильтра новое значение
    fltr->acc += inp;             // Добавляем в аккумулятор новое значение
    fltr->head = head;
    return (fltr->acc / (float)RUN_AVERAGE_FILTER_8);
  }
}

/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
float RunAverageFilter_float_N(float inp, T_run_average_float_N *fltr)
{
  uint32_t i;
  if (fltr->en == 0)
  {
    for (i = 0; i < fltr->len; i++)
    {
      fltr->arr[i] = inp;
    }
    fltr->acc = inp * fltr->len;
    fltr->head = 0;
    fltr->en = 1;
    return inp;
  }
  else
  {
    uint32_t head;

    head = fltr->head + 1;
    if (head >= fltr->len) head = 0;

    fltr->acc -= fltr->arr[head]; // Удаляем из аккумулятора последнее старое значение
    fltr->arr[head] = inp;        // Добавляем в память фильтра новое значение
    fltr->acc += inp;             // Добавляем в аккумулятор новое значение
    fltr->head = head;
    return (fltr->acc / (float)fltr->len);
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param inp
  \param fltr

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
int32_t RunAverageFilter_int32_N(int32_t inp, T_run_average_int32_N *fltr)
{
  uint32_t i;
  if (fltr->en == 0)
  {
    for (i = 0; i < fltr->len; i++)
    {
      fltr->arr[i] = inp;
    }
    fltr->acc = inp * fltr->len;
    fltr->head = 0;
    fltr->en = 1;
    return inp;
  }
  else
  {
    uint32_t head;

    head = fltr->head + 1;
    if (head >= fltr->len) head = 0;

    fltr->acc -= fltr->arr[head]; // Удаляем из аккумулятора последнее старое значение
    fltr->arr[head] = inp;        // Добавляем в память фильтра новое значение
    fltr->acc += inp;             // Добавляем в аккумулятор новое значение
    fltr->head = head;
    return (fltr->acc / fltr->len);
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param inp
  \param fltr

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
int32_t RunAverageFilter_uint32_N(uint32_t inp, T_run_average_uint32_N *fltr)
{
  uint32_t i;
  if (fltr->en == 0)
  {
    for (i = 0; i < fltr->len; i++)
    {
      fltr->arr[i] = inp;
    }
    fltr->acc = inp * fltr->len;
    fltr->head = 0;
    fltr->en = 1;
    return inp;
  }
  else
  {
    uint32_t head;

    head = fltr->head + 1;
    if (head >= fltr->len) head = 0;

    fltr->acc -= fltr->arr[head]; // Удаляем из аккумулятора последнее старое значение
    fltr->arr[head] = inp;        // Добавляем в память фильтра новое значение
    fltr->acc += inp;             // Добавляем в аккумулятор новое значение
    fltr->head = head;
    return (fltr->acc / fltr->len);
  }
}


/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
int16_t RunAverageFilter_int16_4(int16_t inp, T_run_average_int16_4 *fltr)
{
  uint32_t i;
  if (fltr->en == 0)
  {
    for (i = 0; i < RUN_AVERAGE_FILTER_4; i++)
    {
      fltr->arr[i] = inp;
    }
    fltr->acc = inp * RUN_AVERAGE_FILTER_4;
    fltr->head = 0;
    fltr->en = 1;
    return inp;
  }
  else
  {
    uint32_t head =(fltr->head + 1) & (RUN_AVERAGE_FILTER_4 - 1);
    fltr->acc -= fltr->arr[head]; // Удаляем из аккумулятора последнее старое значение
    fltr->arr[head] = inp;        // Добавляем в память фильтра новое значение
    fltr->acc += inp;             // Добавляем в аккумулятор новое значение
    fltr->head = head;
    return (fltr->acc / RUN_AVERAGE_FILTER_4);
  }
}



/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
uint16_t RunAverageFilter_uint16_32(int16_t inp, T_run_average_uint16_32 *fltr)
{
  uint32_t i;
  if (fltr->en == 0)
  {
    for (i = 0; i < RUN_AVERAGE_FILTER_32; i++)
    {
      fltr->arr[i] = inp;
    }
    fltr->acc = inp * RUN_AVERAGE_FILTER_32;
    fltr->head = 0;
    fltr->en = 1;
    return inp;
  }
  else
  {
    uint32_t head =(fltr->head + 1) & (RUN_AVERAGE_FILTER_32 - 1);
    fltr->acc -= fltr->arr[head]; // Удаляем из аккумулятора последнее старое значение
    fltr->arr[head] = inp;        // Добавляем в память фильтра новое значение
    fltr->acc += inp;             // Добавляем в аккумулятор новое значение
    fltr->head = head;
    return (uint16_t)(fltr->acc / RUN_AVERAGE_FILTER_32);
  }
}

/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
int16_t RunAverageFilter_int16_32(int16_t inp, T_run_average_int16_32 *fltr)
{
  uint32_t i;
  if (fltr->en == 0)
  {
    for (i = 0; i < RUN_AVERAGE_FILTER_32; i++)
    {
      fltr->arr[i] = inp;
    }
    fltr->acc = inp * RUN_AVERAGE_FILTER_32;
    fltr->head = 0;
    fltr->en = 1;
    return inp;
  }
  else
  {
    uint32_t head =(fltr->head + 1) & (RUN_AVERAGE_FILTER_32 - 1);
    fltr->acc -= fltr->arr[head]; // Удаляем из аккумулятора последнее старое значение
    fltr->arr[head] = inp;        // Добавляем в память фильтра новое значение
    fltr->acc += inp;             // Добавляем в аккумулятор новое значение
    fltr->head = head;
    return (int16_t)(fltr->acc / RUN_AVERAGE_FILTER_32);
  }
}

/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
int32_t RunAverageFilter_int32_4(int32_t inp, T_run_average_int32_4 *fltr)
{
  uint32_t i;
  if (fltr->en == 0)
  {
    for (i = 0; i < RUN_AVERAGE_FILTER_4; i++)
    {
      fltr->arr[i] = inp;
    }
    fltr->acc = inp * RUN_AVERAGE_FILTER_4;
    fltr->head = 0;
    fltr->en = 1;
    return inp;
  }
  else
  {
    uint32_t head =(fltr->head + 1) & (RUN_AVERAGE_FILTER_4 - 1);
    fltr->acc -= fltr->arr[head]; // Удаляем из аккумулятора последнее старое значение
    fltr->arr[head] = inp;        // Добавляем в память фильтра новое значение
    fltr->acc += inp;             // Добавляем в аккумулятор новое значение
    fltr->head = head;
    return (fltr->acc / RUN_AVERAGE_FILTER_4);
  }
}


/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
int32_t RunAverageFilter_uint32_20( T_run_average_uint32_20 *fltr, uint32_t inp)
{
  uint32_t i;
  if (fltr->en == 0)
  {
    for (i = 0; i < RUN_AVERAGE_FILTER_20; i++)
    {
      fltr->arr[i] = inp;
    }
    fltr->acc = inp * RUN_AVERAGE_FILTER_20;
    fltr->head = 0;
    fltr->en = 1;
    return inp;
  }
  else
  {
    uint32_t head;

    head = fltr->head + 1;
    if (head >= RUN_AVERAGE_FILTER_20) head = 0;

    fltr->acc -= fltr->arr[head]; // Удаляем из аккумулятора последнее старое значение
    fltr->arr[head] = inp;        // Добавляем в память фильтра новое значение
    fltr->acc += inp;             // Добавляем в аккумулятор новое значение
    fltr->head = head;
    return (fltr->acc / RUN_AVERAGE_FILTER_20);
  }
}


/*-----------------------------------------------------------------------------------------------------
  IIR фильтр высоких частот
  Пропускает с частоты 0.2 Гц
  Ослабление на частоте 0.02 Гц = 10дБ
  Убирает постоянную составляющую

  \param rtDW
  \param Input

  \return int16_t
-----------------------------------------------------------------------------------------------------*/
int16_t  IIR_HP_02_filter(T_hp02_filter1_dw *rtDW, int16_t Input)
{
  float  Output;
  float denAccum;

  /* S-Function (sdspbiquad): '<S1>/Filter' incorporates:
   *  Inport: '<Root>/Input'
   */
  denAccum =(0.999952853F * (float)Input - -0.999905765F * rtDW->states[0])- 0.0F * rtDW->states[1];

  /* Outport: '<Root>/Output' incorporates:
   *  S-Function (sdspbiquad): '<S1>/Filter'
   */
  Output =(denAccum + -rtDW->states[0])+ 0.0F * rtDW->states[1];

  /* S-Function (sdspbiquad): '<S1>/Filter' */
  rtDW->states[1] = rtDW->states[0];
  rtDW->states[0] = denAccum;
  return (int16_t)Output;
}


/*-----------------------------------------------------------------------------------------------------
  IIR фильтр высоких частот
  Пропускает с частоты 10 Гц
  Ослабление на частоте 1 Гц = 10дБ
  Убирает постоянную составляющую

  \param rtDW
  \param Input

  \return int16_t
-----------------------------------------------------------------------------------------------------*/
int16_t IIR_HP_10_filter(T_hp10_filter1_dw *rtDW, int16_t Input)
{
  float  Output;
  float denAccum;

  /* S-Function (sdspbiquad): '<S1>/Filter_HP_10' incorporates:
   *  Inport: '<Root>/Input'
   */
  denAccum =(0.997649372F * (float)Input - -0.995298684F * rtDW->states[0])- 0.0F * rtDW->states[1];

  /* Outport: '<Root>/Output' incorporates:
   *  S-Function (sdspbiquad): '<S1>/Filter_HP_10'
   */
  Output =(denAccum + -rtDW->states[0])+ 0.0F * rtDW->states[1];

  /* S-Function (sdspbiquad): '<S1>/Filter_HP_10' */
  rtDW->states[1] = rtDW->states[0];
  rtDW->states[0] = denAccum;
  return (int16_t)Output;
}

/*-----------------------------------------------------------------------------------------------------
 Целочисленный  FIR фильтр
 Сэмплирование 2000 Гц
 Полоса пропускания 0-100 Гц
 Полоса заграждения от 200 Гц -20дБ

 Сэмплирование 4000 Гц
 Полоса пропускания 0-200 Гц
 Полоса заграждения от 400 Гц -20дБ


 \param rtDW
 \param Input

 \return int32_t
-----------------------------------------------------------------------------------------------------*/
int16_t LPF_FIR_200_400_20_int16(T_fir_filter *rtDW, int16_t Input)
{
  int32_t Output;
  int16_T rtb_BodyDelay17;
  int32_T tmp;
  int32_T tmp_0;
  int32_T tmp_1;
  int32_T tmp_2;
  int32_T tmp_3;
  int32_T tmp_4;
  int32_T tmp_5;
  int32_T tmp_6;
  int32_T tmp_7;
  int32_T tmp_8;
  int32_T tmp_9;
  int32_T tmp_a;
  int32_T tmp_b;
  int32_T tmp_c;
  int32_T tmp_d;
  int32_T tmp_e;
  int32_T tmp_f;
  int32_T tmp_g;

  /* Delay: '<S1>/BodyDelay17' */
  rtb_BodyDelay17 = rtDW->BodyDelay17_DSTATE;

  /* Gain: '<S1>/b(17)' incorporates:
   *  Delay: '<S1>/BodyDelay17'
   */
  tmp = -212 * rtDW->BodyDelay17_DSTATE;

  /* Gain: '<S1>/b(16)' incorporates:
   *  Delay: '<S1>/BodyDelay16'
   */
  tmp_0 = 219 * rtDW->BodyDelay16_DSTATE;

  /* Gain: '<S1>/b(15)' incorporates:
   *  Delay: '<S1>/BodyDelay15'
   */
  tmp_1 = 944 * rtDW->BodyDelay15_DSTATE;

  /* Gain: '<S1>/b(14)' incorporates:
   *  Delay: '<S1>/BodyDelay14'
   */
  tmp_2 = 1889 * rtDW->BodyDelay14_DSTATE;

  /* Gain: '<S1>/b(13)' incorporates:
   *  Delay: '<S1>/BodyDelay13'
   */
  tmp_3 = 2932 * rtDW->BodyDelay13_DSTATE;

  /* Gain: '<S1>/b(12)' incorporates:
   *  Delay: '<S1>/BodyDelay12'
   */
  tmp_4 = 3915 * rtDW->BodyDelay12_DSTATE;

  /* Gain: '<S1>/b(11)' incorporates:
   *  Delay: '<S1>/BodyDelay11'
   */
  tmp_5 = 4681 * rtDW->BodyDelay11_DSTATE;

  /* Gain: '<S1>/b(10)' incorporates:
   *  Delay: '<S1>/BodyDelay10'
   */
  tmp_6 = 5099 * rtDW->BodyDelay10_DSTATE;

  /* Gain: '<S1>/b(9)' incorporates:
   *  Delay: '<S1>/BodyDelay9'
   */
  tmp_7 = 5099 * rtDW->BodyDelay9_DSTATE;

  /* Gain: '<S1>/b(8)' incorporates:
   *  Delay: '<S1>/BodyDelay8'
   */
  tmp_8 = 4681 * rtDW->BodyDelay8_DSTATE;

  /* Gain: '<S1>/b(7)' incorporates:
   *  Delay: '<S1>/BodyDelay7'
   */
  tmp_9 = 3915 * rtDW->BodyDelay7_DSTATE;

  /* Gain: '<S1>/b(6)' incorporates:
   *  Delay: '<S1>/BodyDelay6'
   */
  tmp_a = 2932 * rtDW->BodyDelay6_DSTATE;

  /* Gain: '<S1>/b(5)' incorporates:
   *  Delay: '<S1>/BodyDelay5'
   */
  tmp_b = 1889 * rtDW->BodyDelay5_DSTATE;

  /* Gain: '<S1>/b(4)' incorporates:
   *  Delay: '<S1>/BodyDelay4'
   */
  tmp_c = 944 * rtDW->BodyDelay4_DSTATE;

  /* Gain: '<S1>/b(3)' incorporates:
   *  Delay: '<S1>/BodyDelay3'
   */
  tmp_d = 219 * rtDW->BodyDelay3_DSTATE;

  /* Gain: '<S1>/b(2)' incorporates:
   *  Delay: '<S1>/BodyDelay2'
   */
  tmp_e = -212 * rtDW->BodyDelay2_DSTATE;

  /* Gain: '<S1>/b(1)' incorporates:
   *  Inport: '<Root>/Input'
   */
  tmp_f = -2033 * Input;

  /* Gain: '<S1>/b(18)' incorporates:
   *  Delay: '<S1>/BodyDelay18'
   */
  tmp_g = -2033 * rtDW->BodyDelay18_DSTATE;

  /* DataTypeConversion: '<S1>/ConvertOut' incorporates:
   *  Gain: '<S1>/b(1)'
   *  Gain: '<S1>/b(10)'
   *  Gain: '<S1>/b(11)'
   *  Gain: '<S1>/b(12)'
   *  Gain: '<S1>/b(13)'
   *  Gain: '<S1>/b(14)'
   *  Gain: '<S1>/b(15)'
   *  Gain: '<S1>/b(16)'
   *  Gain: '<S1>/b(17)'
   *  Gain: '<S1>/b(18)'
   *  Gain: '<S1>/b(2)'
   *  Gain: '<S1>/b(3)'
   *  Gain: '<S1>/b(4)'
   *  Gain: '<S1>/b(5)'
   *  Gain: '<S1>/b(6)'
   *  Gain: '<S1>/b(7)'
   *  Gain: '<S1>/b(8)'
   *  Gain: '<S1>/b(9)'
   *  Sum: '<S1>/BodyLSum10'
   *  Sum: '<S1>/BodyLSum11'
   *  Sum: '<S1>/BodyLSum12'
   *  Sum: '<S1>/BodyLSum13'
   *  Sum: '<S1>/BodyLSum14'
   *  Sum: '<S1>/BodyLSum15'
   *  Sum: '<S1>/BodyLSum16'
   *  Sum: '<S1>/BodyLSum17'
   *  Sum: '<S1>/BodyLSum18'
   *  Sum: '<S1>/BodyLSum5'
   *  Sum: '<S1>/BodyLSum6'
   *  Sum: '<S1>/BodyLSum7'
   *  Sum: '<S1>/BodyLSum8'
   *  Sum: '<S1>/BodyLSum9'
   */
  tmp =((((((((((((((((((tmp_b & 268435456) != 0 ? tmp_b | -268435456 : tmp_b &
                                    268435455)+((tmp_c & 268435456) != 0 ? tmp_c | -268435456 : tmp_c &
                                    268435455))+((tmp_a & 268435456) != 0 ? tmp_a | -268435456 :
                                  tmp_a & 268435455))+((tmp_9 & 268435456) != 0 ? tmp_9 | -268435456 : tmp_9
                                & 268435455))+((tmp_8 & 268435456) != 0 ? tmp_8 | -268435456 : tmp_8 &
                              268435455))+((tmp_7 & 268435456) != 0 ? tmp_7 |
                            -268435456 : tmp_7 & 268435455))+((tmp_6 & 268435456) != 0 ? tmp_6 |
                          -268435456 : tmp_6 & 268435455))+((tmp_5 & 268435456) != 0 ? tmp_5 |
                        -268435456 : tmp_5 & 268435455))+((tmp_4 & 268435456) != 0 ? tmp_4 |
                      -268435456 : tmp_4 & 268435455))+((tmp_3 & 268435456) != 0 ? tmp_3 |
                    -268435456 : tmp_3 & 268435455))+((tmp_2 & 268435456) != 0 ? tmp_2 |
                  -268435456 : tmp_2 & 268435455))+((tmp_1 & 268435456) != 0 ? tmp_1 |
                -268435456 : tmp_1 & 268435455))+((tmp_0 & 268435456) != 0 ? tmp_0 |
              -268435456 : tmp_0 & 268435455))+((tmp & 268435456) != 0 ? tmp |
            -268435456 : tmp & 268435455))+((tmp_d & 268435456) != 0 ? tmp_d |
          -268435456 : tmp_d & 268435455))+((tmp_e & 268435456) != 0 ? tmp_e
        | -268435456 : tmp_e & 268435455))+((tmp_f & 268435456) != 0 ?
       tmp_f | -268435456 : tmp_f & 268435455))+((tmp_g & 268435456) != 0 ?
       tmp_g | -268435456 : tmp_g & 268435455);

  /* Outport: '<Root>/Output' incorporates:
   *  DataTypeConversion: '<S1>/ConvertOut'
   */
  Output = (int16_T)(((((tmp & 16384) != 0) && ((tmp & 16383) != 0))+(tmp >> 15))+(((tmp & 32767) == 16384) && ((tmp & 32768) != 0)));

  /* Update for Delay: '<S1>/BodyDelay17' incorporates:
   *  Delay: '<S1>/BodyDelay16'
   */
  rtDW->BodyDelay17_DSTATE = rtDW->BodyDelay16_DSTATE;

  /* Update for Delay: '<S1>/BodyDelay16' incorporates:
   *  Delay: '<S1>/BodyDelay15'
   */
  rtDW->BodyDelay16_DSTATE = rtDW->BodyDelay15_DSTATE;

  /* Update for Delay: '<S1>/BodyDelay15' incorporates:
   *  Delay: '<S1>/BodyDelay14'
   */
  rtDW->BodyDelay15_DSTATE = rtDW->BodyDelay14_DSTATE;

  /* Update for Delay: '<S1>/BodyDelay14' incorporates:
   *  Delay: '<S1>/BodyDelay13'
   */
  rtDW->BodyDelay14_DSTATE = rtDW->BodyDelay13_DSTATE;

  /* Update for Delay: '<S1>/BodyDelay13' incorporates:
   *  Delay: '<S1>/BodyDelay12'
   */
  rtDW->BodyDelay13_DSTATE = rtDW->BodyDelay12_DSTATE;

  /* Update for Delay: '<S1>/BodyDelay12' incorporates:
   *  Delay: '<S1>/BodyDelay11'
   */
  rtDW->BodyDelay12_DSTATE = rtDW->BodyDelay11_DSTATE;

  /* Update for Delay: '<S1>/BodyDelay11' incorporates:
   *  Delay: '<S1>/BodyDelay10'
   */
  rtDW->BodyDelay11_DSTATE = rtDW->BodyDelay10_DSTATE;

  /* Update for Delay: '<S1>/BodyDelay10' incorporates:
   *  Delay: '<S1>/BodyDelay9'
   */
  rtDW->BodyDelay10_DSTATE = rtDW->BodyDelay9_DSTATE;

  /* Update for Delay: '<S1>/BodyDelay9' incorporates:
   *  Delay: '<S1>/BodyDelay8'
   */
  rtDW->BodyDelay9_DSTATE = rtDW->BodyDelay8_DSTATE;

  /* Update for Delay: '<S1>/BodyDelay8' incorporates:
   *  Delay: '<S1>/BodyDelay7'
   */
  rtDW->BodyDelay8_DSTATE = rtDW->BodyDelay7_DSTATE;

  /* Update for Delay: '<S1>/BodyDelay7' incorporates:
   *  Delay: '<S1>/BodyDelay6'
   */
  rtDW->BodyDelay7_DSTATE = rtDW->BodyDelay6_DSTATE;

  /* Update for Delay: '<S1>/BodyDelay6' incorporates:
   *  Delay: '<S1>/BodyDelay5'
   */
  rtDW->BodyDelay6_DSTATE = rtDW->BodyDelay5_DSTATE;

  /* Update for Delay: '<S1>/BodyDelay5' incorporates:
   *  Delay: '<S1>/BodyDelay4'
   */
  rtDW->BodyDelay5_DSTATE = rtDW->BodyDelay4_DSTATE;

  /* Update for Delay: '<S1>/BodyDelay4' incorporates:
   *  Delay: '<S1>/BodyDelay3'
   */
  rtDW->BodyDelay4_DSTATE = rtDW->BodyDelay3_DSTATE;

  /* Update for Delay: '<S1>/BodyDelay3' incorporates:
   *  Delay: '<S1>/BodyDelay2'
   */
  rtDW->BodyDelay3_DSTATE = rtDW->BodyDelay2_DSTATE;

  /* Update for Delay: '<S1>/BodyDelay2' incorporates:
   *  Update for Inport: '<Root>/Input'
   */
  rtDW->BodyDelay2_DSTATE = Input;

  /* Update for Delay: '<S1>/BodyDelay18' */
  rtDW->BodyDelay18_DSTATE = rtb_BodyDelay17;
  return Output;
}





/*-----------------------------------------------------------------------------------------------------
   Design Method Information
   Design Algorithm : ellip

   Design Options
   Match Exactly : passband
   Scale Norm    : no scaling
   SystemObject  : false

   Design Specifications
   Sample Rate     : N/A (normalized frequency)
   Response        : Lowpass
   Specification   : Fp,Fst,Ap,Ast
   Stopband Edge   : 0.1
   Passband Ripple : 1 dB
   Stopband Atten. : 30 dB
   Passband Edge   : 0.05

   Measurements
   Sample Rate      : N/A (normalized frequency)
   Passband Edge    : 0.05
   3-dB Point       : 0.054004
   6-dB Point       : 0.058488
   Stopband Edge    : 0.1
   Passband Ripple  : 1 dB
   Stopband Atten.  : 34.6411 dB
   Transition Width : 0.05

   Implementation Cost
   Number of Multipliers            : 6
   Number of Adders                 : 6
   Number of States                 : 3
   Multiplications per Input Sample : 6
   Additions per Input Sample       : 6
-----------------------------------------------------------------------------------------------------*/
int16_t LPF_Elip_100_200_30_int16(T_fixlp_100_200_30db *rtDW, int16_t input)
{
  int32_t rtb_Delay11;
  int32_t rtb_CastState1;
  int32_t rtb_CastState2;
  int32_t tmp;
  int64_t tmp_0;
  int16_t output;

  /* Delay: '<S1>/Delay11' */
  rtb_Delay11 = rtDW->Delay11_DSTATE;

  /* Gain: '<S1>/s(1)' incorporates:
   *  Inport: '<Root>/Input'
   */
  tmp_0 = 1779245417LL * input;

  /* Sum: '<S1>/SumA21' incorporates:
   *  Gain: '<S1>/s(1)'
   */
  tmp = (int32_t)(((((tmp_0 & 1048576ULL) != 0ULL) && ((tmp_0 & 1048575ULL) !=
            0ULL))+(tmp_0 >> 21))+(((tmp_0 & 2097151LL) == 1048576LL) && ((tmp_0 &
          2097152ULL) != 0ULL)));

  /* Gain: '<S1>/a(2)(1)' incorporates:
   *  Delay: '<S1>/Delay11'
   */
  tmp_0 = -2051007709LL * rtDW->Delay11_DSTATE;

  /* Sum: '<S1>/SumA21' incorporates:
   *  Gain: '<S1>/a(2)(1)'
   */
  rtb_CastState2 = (int32_t)(((((tmp_0 & 2147483648ULL) != 0ULL) && ((tmp_0 &
                  2147483647ULL) != 0ULL))+(tmp_0 >> 32))+(((tmp_0 & 4294967295LL) ==
                  2147483648LL) && ((tmp_0 & 4294967296ULL) != 0ULL)));

  /* Gain: '<S1>/a(3)(1)' incorporates:
   *  Delay: '<S1>/Delay21'
   */
  tmp_0 = 31346849LL * rtDW->Delay21_DSTATE;

  /* Sum: '<S1>/SumA31' incorporates:
   *  Gain: '<S1>/a(3)(1)'
   */
  rtb_CastState1 = (int32_t)(((((tmp_0 & 67108864ULL) != 0ULL) && ((tmp_0 &
                  67108863ULL) != 0ULL))+(tmp_0 >> 27))+(((tmp_0 & 134217727LL) ==
                  67108864LL) && ((tmp_0 & 134217728ULL) != 0ULL)));

  /* DataTypeConversion: '<S1>/CastState1' incorporates:
   *  Sum: '<S1>/SumA21'
   *  Sum: '<S1>/SumA31'
   */
  rtb_CastState1 =(((((((((((tmp & 2U) != 0U) && ((tmp & 1U) != 0U))+(tmp >>
                    2))+(((tmp & 3) == 2) && ((tmp & 4U) != 0U)))-(rtb_CastState2 >> 2))-
                  (((rtb_CastState2 & 2U) != 0U) && ((rtb_CastState2 & 1U)
                  != 0U)))-(((rtb_CastState2 & 3) == 2) && ((rtb_CastState2 & 4U) != 0U)))-
                  (rtb_CastState1 >> 2))-(((rtb_CastState1 & 2U) != 0U) &&
                  ((rtb_CastState1 & 1U) != 0U)))-(((rtb_CastState1 & 3) == 2) &&
                  ((rtb_CastState1 & 4U) != 0U)))<< 4;

  /* Gain: '<S1>/b(2)(1)' incorporates:
   *  Delay: '<S1>/Delay11'
   */
  tmp_0 = -1006482793LL * rtDW->Delay11_DSTATE;

  /* Sum: '<S1>/SumB21' incorporates:
   *  Gain: '<S1>/b(2)(1)'
   */
  tmp = (int32_t)(((((tmp_0 & 1073741824ULL) != 0ULL) && ((tmp_0 & 1073741823ULL)
          != 0ULL))+(tmp_0 >> 31))+(((tmp_0 & 2147483647LL) == 1073741824LL) &&
       ((tmp_0 & 2147483648ULL) != 0ULL)));

  /* Gain: '<S1>/s(2)' incorporates:
   *  Delay: '<S1>/Delay21'
   *  Sum: '<S1>/SumB21'
   *  Sum: '<S1>/SumB31'
   */
  tmp_0 =(((((((((((rtb_CastState1 & 8U) != 0U) && ((rtb_CastState1 & 7U) != 0U))
                  +(rtb_CastState1 >> 4))+(((rtb_CastState1 & 15) == 8) &&
                  ((rtb_CastState1 & 16U) != 0U)))+(tmp >> 2))+(((tmp & 2U) != 0U) &&
              ((tmp & 1U) != 0U)))+(((tmp & 3) == 2) && ((tmp & 4U) != 0U)))+
         (rtDW->Delay21_DSTATE >> 4))+(((rtDW->Delay21_DSTATE & 8U) != 0U) &&
         ((rtDW->Delay21_DSTATE & 7U) != 0U)))+(((rtDW->Delay21_DSTATE & 15)
         == 8) && ((rtDW->Delay21_DSTATE & 16U) != 0U))) * 1297035571LL;

  /* Sum: '<S1>/SumA22' incorporates:
   *  Gain: '<S1>/s(2)'
   */
  tmp = (int32_t)(((((tmp_0 & 2147483648ULL) != 0ULL) && ((tmp_0 & 2147483647ULL)
          != 0ULL))+(tmp_0 >> 32))+(((tmp_0 & 4294967295LL) == 2147483648LL) &&
       ((tmp_0 & 4294967296ULL) != 0ULL)));

  /* Gain: '<S1>/a(2)(2)' incorporates:
   *  Delay: '<S1>/Delay12'
   */
  tmp_0 = -246577029LL * rtDW->Delay12_DSTATE;

  /* Sum: '<S1>/SumA22' incorporates:
   *  Gain: '<S1>/a(2)(2)'
   */
  rtb_CastState2 = (int32_t)(((((tmp_0 & 536870912ULL) != 0ULL) && ((tmp_0 &
                  536870911ULL) != 0ULL))+(tmp_0 >> 30))+(((tmp_0 & 1073741823LL) ==
                  536870912LL) && ((tmp_0 & 1073741824ULL) != 0ULL)));

  /* DataTypeConversion: '<S1>/CastState2' incorporates:
   *  Sum: '<S1>/SumA22'
   */
  rtb_CastState2 =((((((((tmp & 2U) != 0U) && ((tmp & 1U) != 0U))+(tmp >> 2))
                  +(((tmp & 3) == 2) && ((tmp & 4U) != 0U)))-
                  (rtb_CastState2 >> 2))-(((rtb_CastState2 & 2U) != 0U) &&
                  ((rtb_CastState2 & 1U) != 0U)))-(((rtb_CastState2 & 3) == 2) &&
                  ((rtb_CastState2 & 4U) != 0U)))<< 4;

  /* DataTypeConversion: '<S1>/ConvertOut' incorporates:
   *  Delay: '<S1>/Delay12'
   *  Sum: '<S1>/SumB22'
   */
  tmp =(((((((rtb_CastState2 & 8U) != 0U) && ((rtb_CastState2 & 7U) != 0U))+
          (rtb_CastState2 >> 4))+(((rtb_CastState2 & 15) == 8) &&
          ((rtb_CastState2 & 16U) != 0U)))+(rtDW->Delay12_DSTATE >> 4))+
       (((rtDW->Delay12_DSTATE & 8U) != 0U) && ((rtDW->Delay12_DSTATE & 7U) !=
          0U)))+(((rtDW->Delay12_DSTATE & 15) == 8) && ((rtDW->Delay12_DSTATE &
        16U) != 0U));

  /* Outport: '<Root>/Output' incorporates:
   *  DataTypeConversion: '<S1>/ConvertOut'
   */
  output = (int16_t)(((((tmp & 1024U) != 0U) && ((tmp & 1023U) != 0U))+
          (tmp >> 11))+(((tmp & 2047) == 1024) && ((tmp & 2048U) != 0U)));

  /* Update for Delay: '<S1>/Delay11' */
  rtDW->Delay11_DSTATE = rtb_CastState1;

  /* Update for Delay: '<S1>/Delay21' */
  rtDW->Delay21_DSTATE = rtb_Delay11;

  /* Update for Delay: '<S1>/Delay12' */
  rtDW->Delay12_DSTATE = rtb_CastState2;

  return output;
}



/*-----------------------------------------------------------------------------------------------------
 IIR фильтр c плавающей запятой
 Сэмплирование 2000 Гц
 Полоса пропускания 0-100 Гц
 Полоса заграждения от 200 Гц -35дБ

 Сэмплирование 4000 Гц
 Полоса пропускания 0-200 Гц
 Полоса заграждения от 400 Гц -35дБ

 \param dw
 \param Input

 \return float
-----------------------------------------------------------------------------------------------------*/
float  LPF_Elip_100_200_35(T_eliptic_filter1_dw *rtDW, int16_t Input)
{
  float  Output;
  float  rtb_Delay11;
  float  rtb_SumA31;
  float  rtb_SumA22;

  /* Delay: '<S1>/Delay11' */
  rtb_Delay11 = rtDW->Delay11_DSTATE;

  /* Sum: '<S1>/SumA31' incorporates:
   *  Delay: '<S1>/Delay11'
   *  Delay: '<S1>/Delay21'
   *  Gain: '<S1>/a(2)(1)'
   *  Gain: '<S1>/a(3)(1)'
   *  Gain: '<S1>/s(1)'
   *  Inport: '<Root>/Input'
   *  Sum: '<S1>/SumA21'
   */
  rtb_SumA31 =(0.10367614775896072f * (float)Input - -1.7808578014373779f * rtDW->Delay11_DSTATE)- 0.87342262268066406f * rtDW->Delay21_DSTATE;

  /* Sum: '<S1>/SumA22' incorporates:
   *  Delay: '<S1>/Delay11'
   *  Delay: '<S1>/Delay12'
   *  Delay: '<S1>/Delay21'
   *  Gain: '<S1>/a(2)(2)'
   *  Gain: '<S1>/b(2)(1)'
   *  Gain: '<S1>/s(2)'
   *  Sum: '<S1>/SumB21'
   *  Sum: '<S1>/SumB31'
   */
  rtb_SumA22 =((-1.5203124284744263f * rtDW->Delay11_DSTATE + rtb_SumA31)+ rtDW->Delay21_DSTATE) * 0.14596366882324219f - -0.84315669536590576f * rtDW->Delay12_DSTATE;

  /* Outport: '<Root>/Output' incorporates:
   *  Delay: '<S1>/Delay12'
   *  Sum: '<S1>/SumB22'
   */
  Output = rtb_SumA22 + rtDW->Delay12_DSTATE;

  /* Update for Delay: '<S1>/Delay11' */
  rtDW->Delay11_DSTATE = rtb_SumA31;

  /* Update for Delay: '<S1>/Delay21' */
  rtDW->Delay21_DSTATE = rtb_Delay11;

  /* Update for Delay: '<S1>/Delay12' */
  rtDW->Delay12_DSTATE = rtb_SumA22;

  return Output;
}




/*-----------------------------------------------------------------------------------------------------
  Design Method Information
  Design Algorithm : ellip

  Design Specifications
  Sample Rate     : N/A (normalized frequency) (Fs=4000 Hz, Band = 2000 Hz)
  Response        : Lowpass
  Specification   : Fp,Fst,Ap,Ast
  Stopband Atten. : 100 dB
  Passband Edge   : 0.0025    (2000*0.0025 = 5   Hz)
  Stopband Edge   : 0.05      (2000*0.05   = 100 Hz)
  Passband Ripple : 0.1 dB

  Measurements
  Sample Rate      : N/A (normalized frequency)
  Passband Edge    : 0.0025
  3-dB Point       : 0.0030319
  6-dB Point       : 0.0032985
  Stopband Edge    : 0.05
  Passband Ripple  : 0.1 dB
  Stopband Atten.  : 123.928 dB
  Transition Width : 0.0475

  Implementation Cost
  Number of Multipliers            : 8
  Number of Adders                 : 8
  Number of States                 : 4
  Multiplications per Input Sample : 8
  Additions per Input Sample       : 8

  \param rtDW
  \param Input

  \return float
-----------------------------------------------------------------------------------------------------*/
float LPF_Elip_5_100_124(T_eliptic_5hz_filter *rtDW, float input)
{
  float    output;
  float    rtb_Delay11;
  float    rtb_SumA31;
  float    rtb_Delay12;
  float    rtb_SumA32;

  /* Delay: '<S1>/Delay11' */
  rtb_Delay11 = rtDW->Delay11_DSTATE;

  /* Sum: '<S1>/SumA31' incorporates:
   *  Delay: '<S1>/Delay11'
   *  Delay: '<S1>/Delay21'
   *  Gain: '<S1>/a(2)(1)'
   *  Gain: '<S1>/a(3)(1)'
   *  Gain: '<S1>/s(1)'
   *  Inport: '<Root>/Input'
   *  Sum: '<S1>/SumA21'
   */
  rtb_SumA31 =(0.00289748725F * input - -1.98998952F * rtDW->Delay11_DSTATE)
              - 0.990027845F * rtDW->Delay21_DSTATE;

  /* Delay: '<S1>/Delay12' */
  rtb_Delay12 = rtDW->Delay12_DSTATE;

  /* Sum: '<S1>/SumA32' incorporates:
   *  Delay: '<S1>/Delay11'
   *  Delay: '<S1>/Delay12'
   *  Delay: '<S1>/Delay21'
   *  Delay: '<S1>/Delay22'
   *  Gain: '<S1>/a(2)(2)'
   *  Gain: '<S1>/a(3)(2)'
   *  Gain: '<S1>/b(2)(1)'
   *  Gain: '<S1>/s(2)'
   *  Sum: '<S1>/SumA22'
   *  Sum: '<S1>/SumB21'
   *  Sum: '<S1>/SumB31'
   */
  rtb_SumA32 =(((-1.83785069F * rtDW->Delay11_DSTATE + rtb_SumA31)+
              rtDW->Delay21_DSTATE) * 0.000228880672F - -1.9957844F *
              rtDW->Delay12_DSTATE)- 0.995866299F * rtDW->Delay22_DSTATE;

  /* Outport: '<Root>/Output' incorporates:
   *  Delay: '<S1>/Delay12'
   *  Delay: '<S1>/Delay22'
   *  Gain: '<S1>/b(2)(2)'
   *  Sum: '<S1>/SumB22'
   *  Sum: '<S1>/SumB32'
   */
  output =(-1.97118759F * rtDW->Delay12_DSTATE + rtb_SumA32)+
          rtDW->Delay22_DSTATE;

  /* Update for Delay: '<S1>/Delay11' */
  rtDW->Delay11_DSTATE = rtb_SumA31;

  /* Update for Delay: '<S1>/Delay21' */
  rtDW->Delay21_DSTATE = rtb_Delay11;

  /* Update for Delay: '<S1>/Delay12' */
  rtDW->Delay12_DSTATE = rtb_SumA32;

  /* Update for Delay: '<S1>/Delay22' */
  rtDW->Delay22_DSTATE = rtb_Delay12;

  return output;
}



/*-----------------------------------------------------------------------------------------------------
  Design Method Information
  Design Algorithm : ellip

  Design Specifications
  Sample Rate     : N/A (normalized frequency)
  Response        : Lowpass
  Specification   : Fp,Fst,Ap,Ast
  Passband Edge   : 0.025   (2000*0.025 = 50   Hz)
  Stopband Atten. : 60 dB
  Passband Ripple : 0.1 dB
  Stopband Edge   : 0.03

  Measurements
  Sample Rate      : N/A (normalized frequency)
  Passband Edge    : 0.025
  3-dB Point       : 0.025656
  6-dB Point       : 0.025951
  Stopband Edge    : 0.03
  Passband Ripple  : 0.099999 dB
  Stopband Atten.  : 62.3257 dB
  Transition Width : 0.005

  Implementation Cost
  Number of Multipliers            : 16
  Number of Adders                 : 16
  Number of States                 : 8
  Multiplications per Input Sample : 16
  Additions per Input Sample       : 16


  \param rtDW
  \param input
-----------------------------------------------------------------------------------------------------*/
float LPF_Elip_50_200_62(T_eliptic_50hz_filter *rtDW, float input)
{
  float output;
  float rtb_Delay11;
  float rtb_SumA31;
  float rtb_Delay12;
  float rtb_SumA32;
  float rtb_Delay13;
  float rtb_SumA33;
  float rtb_Delay14;
  float rtb_SumA34;

  /* Delay: '<S1>/Delay11' */
  rtb_Delay11 = rtDW->Delay11_DSTATE;

  /* Sum: '<S1>/SumA31' incorporates:
   *  Delay: '<S1>/Delay11'
   *  Delay: '<S1>/Delay21'
   *  Gain: '<S1>/a(2)(1)'
   *  Gain: '<S1>/a(3)(1)'
   *  Gain: '<S1>/s(1)'
   *  Inport: '<Root>/Input'
   *  Sum: '<S1>/SumA21'
   */
  rtb_SumA31 =((float)(0.69226826656010187f * input)- (float)
              (-1.9761124048698842f * rtDW->Delay11_DSTATE))- (float)
              (0.98163301141454706f * rtDW->Delay21_DSTATE);

  /* Delay: '<S1>/Delay12' */
  rtb_Delay12 = rtDW->Delay12_DSTATE;

  /* Sum: '<S1>/SumA32' incorporates:
   *  Delay: '<S1>/Delay11'
   *  Delay: '<S1>/Delay12'
   *  Delay: '<S1>/Delay21'
   *  Delay: '<S1>/Delay22'
   *  Gain: '<S1>/a(2)(2)'
   *  Gain: '<S1>/a(3)(2)'
   *  Gain: '<S1>/b(2)(1)'
   *  Gain: '<S1>/s(2)'
   *  Sum: '<S1>/SumA22'
   *  Sum: '<S1>/SumB21'
   *  Sum: '<S1>/SumB31'
   */
  rtb_SumA32 =((float)((((float)(-1.988974352325221f * rtDW->Delay11_DSTATE)
              + rtb_SumA31)+ rtDW->Delay21_DSTATE) * 0.496516852859398f)- (float)
              (-1.9560729795624432f * rtDW->Delay12_DSTATE))- (float)
              (0.959731647368088f * rtDW->Delay22_DSTATE);

  /* Delay: '<S1>/Delay13' */
  rtb_Delay13 = rtDW->Delay13_DSTATE;

  /* Sum: '<S1>/SumA33' incorporates:
   *  Delay: '<S1>/Delay12'
   *  Delay: '<S1>/Delay13'
   *  Delay: '<S1>/Delay22'
   *  Delay: '<S1>/Delay23'
   *  Gain: '<S1>/a(2)(3)'
   *  Gain: '<S1>/a(3)(3)'
   *  Gain: '<S1>/b(2)(2)'
   *  Gain: '<S1>/s(3)'
   *  Sum: '<S1>/SumA23'
   *  Sum: '<S1>/SumB22'
   *  Sum: '<S1>/SumB32'
   */
  rtb_SumA33 =((float)((((float)(-1.9800002201204845f *
              rtDW->Delay12_DSTATE)+ rtb_SumA32)+ rtDW->Delay22_DSTATE) *
              0.18616132581589989f)- (float)(-1.934721006989478f * rtDW->Delay13_DSTATE))
              - (float)(0.93628549053110655f * rtDW->Delay23_DSTATE);

  /* Delay: '<S1>/Delay14' */
  rtb_Delay14 = rtDW->Delay14_DSTATE;

  /* Sum: '<S1>/SumA34' incorporates:
   *  Delay: '<S1>/Delay13'
   *  Delay: '<S1>/Delay14'
   *  Delay: '<S1>/Delay23'
   *  Delay: '<S1>/Delay24'
   *  Gain: '<S1>/a(2)(4)'
   *  Gain: '<S1>/a(3)(4)'
   *  Gain: '<S1>/b(2)(3)'
   *  Gain: '<S1>/s(4)'
   *  Sum: '<S1>/SumA24'
   *  Sum: '<S1>/SumB23'
   *  Sum: '<S1>/SumB33'
   */
  rtb_SumA34 =((float)((((float)(-1.8662887071540415f *
              rtDW->Delay13_DSTATE)+ rtb_SumA33)+ rtDW->Delay23_DSTATE) *
              0.011664751902121611f)- (float)(-1.9886143976670656f * rtDW->Delay14_DSTATE))
              - (float)(0.99499887322020075f * rtDW->Delay24_DSTATE);

  /* Outport: '<Root>/Output' incorporates:
   *  Delay: '<S1>/Delay14'
   *  Delay: '<S1>/Delay24'
   *  Gain: '<S1>/b(2)(4)'
   *  Sum: '<S1>/SumB24'
   *  Sum: '<S1>/SumB34'
   */
  output =((float)(-1.9909377675031565f * rtDW->Delay14_DSTATE)+
          rtb_SumA34)+ rtDW->Delay24_DSTATE;

  /* Update for Delay: '<S1>/Delay11' */
  rtDW->Delay11_DSTATE = rtb_SumA31;

  /* Update for Delay: '<S1>/Delay21' */
  rtDW->Delay21_DSTATE = rtb_Delay11;

  /* Update for Delay: '<S1>/Delay12' */
  rtDW->Delay12_DSTATE = rtb_SumA32;

  /* Update for Delay: '<S1>/Delay22' */
  rtDW->Delay22_DSTATE = rtb_Delay12;

  /* Update for Delay: '<S1>/Delay13' */
  rtDW->Delay13_DSTATE = rtb_SumA33;

  /* Update for Delay: '<S1>/Delay23' */
  rtDW->Delay23_DSTATE = rtb_Delay13;

  /* Update for Delay: '<S1>/Delay14' */
  rtDW->Delay14_DSTATE = rtb_SumA34;

  /* Update for Delay: '<S1>/Delay24' */
  rtDW->Delay24_DSTATE = rtb_Delay14;

  return output;
}


/*-----------------------------------------------------------------------------------------------------
 IIR фильтр c плавающей запятой

 Сэмплирование 4000 Гц
 Полоса пропускания 0...110 Гц
 Полоса заграждения от 150 Гц -48дБ


  \param rtDW
  \param Input
-----------------------------------------------------------------------------------------------------*/
float LPF_Elip_110_150_48(T_eliptic_110Hz_filter_dw *rtDW, int16_t Input)
{
  float  Output;
  float  rtb_Delay11;
  float  rtb_SumA31;
  float  rtb_Delay12;
  float  rtb_SumA32;
  float  rtb_SumA23;

  /* Delay: '<S1>/Delay11' */
  rtb_Delay11 = rtDW->Delay11_DSTATE;

  /* Sum: '<S1>/SumA31' incorporates:
   *  Delay: '<S1>/Delay11'
   *  Delay: '<S1>/Delay21'
   *  Gain: '<S1>/a(2)(1)'
   *  Gain: '<S1>/a(3)(1)'
   *  Gain: '<S1>/s(1)'
   *  Inport: '<Root>/Input'
   *  Sum: '<S1>/SumA21'
   */
  rtb_SumA31 =(0.17559959F * (float)Input - -1.95003033F * rtDW->Delay11_DSTATE)- 0.979401946F * rtDW->Delay21_DSTATE;

  /* Delay: '<S1>/Delay12' */
  rtb_Delay12 = rtDW->Delay12_DSTATE;

  /* Sum: '<S1>/SumA32' incorporates:
   *  Delay: '<S1>/Delay11'
   *  Delay: '<S1>/Delay12'
   *  Delay: '<S1>/Delay21'
   *  Delay: '<S1>/Delay22'
   *  Gain: '<S1>/a(2)(2)'
   *  Gain: '<S1>/a(3)(2)'
   *  Gain: '<S1>/b(2)(1)'
   *  Gain: '<S1>/s(2)'
   *  Sum: '<S1>/SumA22'
   *  Sum: '<S1>/SumB21'
   *  Sum: '<S1>/SumB31'
   */
  rtb_SumA32 =(((-1.94088F * rtDW->Delay11_DSTATE + rtb_SumA31)+ rtDW->Delay21_DSTATE) * 0.12542823F - -1.90927088F * rtDW->Delay12_DSTATE)- 0.924921453F * rtDW->Delay22_DSTATE;

  /* Sum: '<S1>/SumA23' incorporates:
   *  Delay: '<S1>/Delay12'
   *  Delay: '<S1>/Delay13'
   *  Delay: '<S1>/Delay22'
   *  Gain: '<S1>/a(2)(3)'
   *  Gain: '<S1>/b(2)(2)'
   *  Gain: '<S1>/s(3)'
   *  Sum: '<S1>/SumB22'
   *  Sum: '<S1>/SumB32'
   */
  rtb_SumA23 =((-1.87522316F * rtDW->Delay12_DSTATE + rtb_SumA32)+ rtDW->Delay22_DSTATE) * 0.0840368271F - -0.940594F * rtDW->Delay13_DSTATE;

  /* Outport: '<Root>/Output' incorporates:
   *  Delay: '<S1>/Delay13'
   *  Sum: '<S1>/SumB23'
   */
  Output = rtb_SumA23 + rtDW->Delay13_DSTATE;

  /* Update for Delay: '<S1>/Delay11' */
  rtDW->Delay11_DSTATE = rtb_SumA31;

  /* Update for Delay: '<S1>/Delay21' */
  rtDW->Delay21_DSTATE = rtb_Delay11;

  /* Update for Delay: '<S1>/Delay12' */
  rtDW->Delay12_DSTATE = rtb_SumA32;

  /* Update for Delay: '<S1>/Delay22' */
  rtDW->Delay22_DSTATE = rtb_Delay12;

  /* Update for Delay: '<S1>/Delay13' */
  rtDW->Delay13_DSTATE = rtb_SumA23;

  return Output;
}






/*-----------------------------------------------------------------------------------------------------
  Discrete-Time IIR Filter (real)
  -------------------------------
  Filter Structure    : Direct-Form II, Second-Order Sections
  Number of Sections  : 2
  Stable              : Yes
  Linear Phase        : No

  Design Method Information
  Design Algorithm : cheby1

  Design Options
  Match Exactly : passband
  Scale Norm    : no scaling
  SystemObject  : false

  Design Specifications
  Sample Rate     : N/A (normalized frequency)
  Response        : Lowpass
  Specification   : Fp,Fst,Ap,Ast
  Passband Edge   : 0.03
  Passband Ripple : 1 dB
  Stopband Edge   : 0.1
  Stopband Atten. : 36 dB

  Measurements
  Sample Rate      : N/A (normalized frequency)
  Passband Edge    : 0.03
  3-dB Point       : 0.032841
  6-dB Point       : 0.03618
  Stopband Edge    : 0.1
  Passband Ripple  : 1 dB
  Stopband Atten.  : 37.1451 dB
  Transition Width : 0.07

  Implementation Cost
  Number of Multipliers            : 6
  Number of Adders                 : 6
  Number of States                 : 3
  Multiplications per Input Sample : 6
  Additions per Input Sample       : 6

-----------------------------------------------------------------------------------------------------*/
float LPF_Cheb_60_200_36_step(T_LPF_Ch_60_200_36 *rtDW,  float input)
{

  float    numAccum;
  float    denAccum;
  float    output;

  /* S-Function (sdspbiquad): '<S1>/Digital Filter' incorporates:
   *  Inport: '<Root>/In1'
   */
  denAccum =(0.00215605111F * input - -1.94592643F * rtDW->state[0])- 0.954550683F * rtDW->state[1];
  numAccum =(2.0F * rtDW->state[0] + denAccum)+ rtDW->state[1];
  rtDW->state[1] = rtDW->state[0];
  rtDW->state[0] = denAccum;
  denAccum =(0.0227737632F * numAccum - -0.954452455F * rtDW->state[2])- 0.0F * rtDW->state[3];

  /* Outport: '<Root>/Out1' incorporates:
   *  S-Function (sdspbiquad): '<S1>/Digital Filter'
   */
  output =(denAccum + rtDW->state[2])+ 0.0F * rtDW->state[3];

  /* S-Function (sdspbiquad): '<S1>/Digital Filter' */
  rtDW->state[3] = rtDW->state[2];
  rtDW->state[2] = denAccum;

  return output;
}




/*-----------------------------------------------------------------------------------------------------
  Discrete-Time IIR Filter (real)
  -------------------------------
  Filter Structure    : Direct-Form II, Second-Order Sections
  Number of Sections  : 2
  Stable              : Yes
  Linear Phase        : No

  Design Method Information
  Design Algorithm : butter

  Design Options
  Match Exactly : passband
  Scale Norm    : no scaling
  SystemObject  : false

  Design Specifications
  Sample Rate     : N/A (normalized frequency)
  Response        : Lowpass
  Specification   : Fp,Fst,Ap,Ast
  Passband Edge   : 0.0125
  Passband Ripple : 1 dB
  Stopband Edge   : 0.1
  Stopband Atten. : 46 dB

  Measurements
  Sample Rate      : N/A (normalized frequency)
  Passband Edge    : 0.0125
  3-dB Point       : 0.015656
  6-dB Point       : 0.0188
  Stopband Edge    : 0.1
  Passband Ripple  : 1 dB
  Stopband Atten.  : 48.5294 dB
  Transition Width : 0.0875

  Implementation Cost
  Number of Multipliers            : 6
  Number of Adders                 : 6
  Number of States                 : 3
  Multiplications per Input Sample : 6
  Additions per Input Sample       : 6
-----------------------------------------------------------------------------------------------------*/
float LPF_But_25_200_33_step(T_LPF_But_25_200_33 *rtDW, float input)
{
  float     numAccum;
  float     denAccum;
  float     output;

  /* S-Function (sdspbiquad): '<S1>/Digital Filter' incorporates:
   *  Inport: '<Root>/In1'
   */
  denAccum =(0.000590160605F * input - -1.94965386F * rtDW->state[0])- 0.952014446F * rtDW->state[1];
  numAccum =(2.0F * rtDW->state[0] +denAccum)+ rtDW->state[1];
  rtDW->state[1] =rtDW->state[0];
  rtDW->state[0] = denAccum;
  denAccum =(0.0240069311F * numAccum - -0.951986134F * rtDW->state[2])- 0.0F * rtDW->state[3];

  /* Outport: '<Root>/Out1' incorporates:
   *  S-Function (sdspbiquad): '<S1>/Digital Filter'
   */
  output =(denAccum +rtDW->state[2])+ 0.0F * rtDW->state[3];

  /* S-Function (sdspbiquad): '<S1>/Digital Filter' */
  rtDW->state[3] =rtDW->state[2];
  rtDW->state[2] = denAccum;

  return output;
}



/*-----------------------------------------------------------------------------------------------------
  Discrete-Time IIR Filter (real)
  -------------------------------
  Filter Structure    : Direct-Form II, Second-Order Sections
  Number of Sections  : 2
  Stable              : Yes
  Linear Phase        : No

  Design Method Information
  Design Algorithm : ellip

  Design Options
  Match Exactly : both
  Scale Norm    : no scaling
  SystemObject  : false

  Design Specifications
  Sample Rate     : N/A (normalized frequency)
  Response        : Lowpass
  Specification   : Fp,Fst,Ap,Ast
  Stopband Edge   : 0.1
  Stopband Atten. : 30 dB
  Passband Ripple : 1 dB
  Passband Edge   : 0.05

  Measurements
  Sample Rate      : N/A (normalized frequency)
  Passband Edge    : 0.05
  3-dB Point       : 0.053726
  6-dB Point       : 0.057817
  Stopband Edge    : 0.1
  Passband Ripple  : 1 dB
  Stopband Atten.  : 30 dB
  Transition Width : 0.05

  Implementation Cost
  Number of Multipliers            : 6
  Number of Adders                 : 6
  Number of States                 : 3
  Multiplications per Input Sample : 6
  Additions per Input Sample       : 6
-----------------------------------------------------------------------------------------------------*/
float Elip_100_200_30_output(T_LPF_Elip_100_200_30 *rtDW, float input)
{
  float output;
  float denAccum;
  float rtb_DigitalFilter;

  /* S-Function (sdspbiquad): '<S1>/Digital Filter' incorporates:
   *  Inport: '<Root>/In1'
   */
  denAccum = (0.146843821F * input - -1.91353607F * rtDW->state[0]) - 0.937777579F * rtDW->state[1];
  rtb_DigitalFilter = (-1.90762627F * rtDW->state[0] + denAccum) + rtDW->state[1];
  rtDW->state[1] = rtDW->state[0];
  rtDW->state[0] = denAccum;
  denAccum = (0.075382024F * rtb_DigitalFilter - -0.915638804F * rtDW->state[2]) - 0.0F * rtDW->state[3];

  /* Outport: '<Root>/Out1' incorporates:
   *  S-Function (sdspbiquad): '<S1>/Digital Filter'
   */
  output = (denAccum + rtDW->state[2]) + 0.0F * rtDW->state[3];

  /* S-Function (sdspbiquad): '<S1>/Digital Filter' */
  rtDW->state[3] =
    rtDW->state[2];
  rtDW->state[2] = denAccum;

  return output;
}


/*-----------------------------------------------------------------------------------------------------
  Discrete-Time IIR Filter (real)
  -------------------------------
  Filter Structure    : Direct-Form II, Second-Order Sections
  Number of Sections  : 1
  Stable              : Yes
  Linear Phase        : No

  Design Method Information
  Design Algorithm : ellip

  Design Options
  Match Exactly : passband
  Scale Norm    : no scaling
  SystemObject  : false

  Design Specifications
  Sample Rate     : N/A (normalized frequency)
  Response        : Lowpass
  Specification   : Fp,Fst,Ap,Ast
  Stopband Edge   : 0.1
  Stopband Atten. : 70 dB
  Passband Ripple : 1 dB
  Passband Edge   : 0.0025

  Measurements
  Sample Rate      : N/A (normalized frequency)
  Passband Edge    : 0.0025
  3-dB Point       : 0.0030438
  6-dB Point       : 0.003709
  Stopband Edge    : 0.1
  Passband Ripple  : 1 dB
  Stopband Atten.  : 70.3963 dB
  Transition Width : 0.0975

  Implementation Cost
  Number of Multipliers            : 4
  Number of Adders                 : 4
  Number of States                 : 2
  Multiplications per Input Sample : 4
  Additions per Input Sample       : 4
-----------------------------------------------------------------------------------------------------*/
float Elip_5_200_70_output(T_LPF_Elip_5_200_70 *rtDW, float input)
{
  float output;
  float denAccum;

  /* S-Function (sdspbiquad): '<S1>/Digital Filter' incorporates:
   *  Inport: '<Root>/In1'
   */
  denAccum = (0.00031591274588030208F * input - -1.9913492452797295F * rtDW->state[0]) - 0.9914169717526421F * rtDW->state[1];

  /* Outport: '<Root>/Out1' incorporates:
   *  S-Function (sdspbiquad): '<S1>/Digital Filter'
   */
  output = (-1.808930524940737F * rtDW->state[0] + denAccum) + rtDW->state[1];

  /* S-Function (sdspbiquad): '<S1>/Digital Filter' */
  rtDW->state[1] = rtDW->state[0];
  rtDW->state[0] = denAccum;
  return output;
}



/*-----------------------------------------------------------------------------------------------------
  Экспоненциальный целочисленный фильтр

  \param flt
  \param input

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
int32_t Exponential_filter(T_exp_filter *flt, int32_t input )
{
  int32_t outp;
  if (flt->init == 0)
  {
    flt->v1 = input >> flt->k;
    flt->init = 1;
  }

  outp = input + flt->v1;
  flt->v1 = outp - (outp >> flt->k);

  return outp >> flt->k;
}

/*-----------------------------------------------------------------------------------------------------
  Экспоненциальный целочисленный фильтр

  \param flt
  \param input

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
float Exponential_filter_fp(T_exp_filter_fp *flt, float input )
{
  float outp;
  if (flt->init == 0)
  {
    flt->v1 = input;
    flt->init = 1;
  }

  outp = input + flt->v1;
  flt->v1 = outp - (outp * flt->alpha);

  return outp * flt->alpha;
}

/*-----------------------------------------------------------------------------------------------------
//    Discrete-Time IIR Filter (real)
//    -------------------------------
//    Filter Structure    : Direct-Form II, Second-Order Sections
//    Number of Sections  : 1
//    Stable              : Yes
//    Linear Phase        : No
//    Arithmetic          : single
//
//    Design Method Information
//    Design Algorithm : butter
//
//    Design Options
//    Scale Norm   : no scaling
//    SystemObject : false
//
//    Design Specifications
//    Sample Rate   : N/A (normalized frequency)
//    Response      : Lowpass
//    Specification : N,F3dB
//    3-dB Point    : 0.0064
//    Filter Order  : 2
//
//    Measurements
//    Sample Rate      : N/A (normalized frequency)
//    Passband Edge    : Unknown
//    3-dB Point       : 0.0064
//    6-dB Point       : 0.0084228
//    Stopband Edge    : Unknown
//    Passband Ripple  : Unknown
//    Stopband Atten.  : Unknown
//    Transition Width : Unknown
//
//    Implementation Cost
//    Number of Multipliers            : 4
//    Number of Adders                 : 4
//    Number of States                 : 2
//    Multiplications per Input Sample : 4
//    Additions per Input Sample       : 4
//
//    При частоте дискретизации 31250 Гц полоса пропускания по границе 3 dB равна 100 Гц
//    На частоте 131 Гц подавление в два раза
-----------------------------------------------------------------------------------------------------*/
float LPF_But_100_31250_step(T_LPF_But_100_31250 *rtDW, float input)
{
  float output;
  float tmp_state;

  tmp_state = (9.96447707E-5F * input - -1.97156739F * rtDW->state[0]) - 0.971966F * rtDW->state[1];
  output = (2.0F * rtDW->state[0] + tmp_state) + rtDW->state[1];
  rtDW->state[1] = rtDW->state[0];
  rtDW->state[0] = tmp_state;
  return output;
}

/*-----------------------------------------------------------------------------------------------------
//   Discrete-Time IIR Filter (real)
//   -------------------------------
//   Filter Structure    : Direct-Form II, Second-Order Sections
//   Number of Sections  : 1
//   Stable              : Yes
//   Linear Phase        : No
//   Arithmetic          : single
//
//   Design Method Information
//   Design Algorithm : butter
//
//   Design Options
//   Scale Norm   : no scaling
//   SystemObject : false
//
//   Design Specifications
//   Sample Rate   : N/A (normalized frequency)
//   Response      : Lowpass
//   Specification : Nb,Na,F3dB
//   NumOrder      : 2
//   DenOrder      : 2
//   3-dB Point    : 0.002
//
//   Measurements
//   Sample Rate      : N/A (normalized frequency)
//   Passband Edge    : Unknown
//   3-dB Point       : 0.002
//   6-dB Point       : 0.0026324
//   Stopband Edge    : Unknown
//   Passband Ripple  : Unknown
//   Stopband Atten.  : Unknown
//   Transition Width : Unknown
//
//   Implementation Cost
//   Number of Multipliers            : 4
//   Number of Adders                 : 4
//   Number of States                 : 2
//   Multiplications per Input Sample : 4
//   Additions per Input Sample       : 4
//
//
//   Lowpass Maximally flat  При частоте дискретизации 1000 Гц полоса пропускания 1 Гц. На частоте 10 Гц затухание 40 dB
-----------------------------------------------------------------------------------------------------*/
float LPF_MaxFlat_1_1000_step(T_MaxFlat_1_1000_cbl *fcbl, float input)
{
  float output;
  float tmp_state;

  tmp_state = (9.825917E-6F * input - -1.99111426F * fcbl->state[0]) - 0.991153598F * fcbl->state[1];
  output = (2.0F * fcbl->state[0] + tmp_state) + fcbl->state[1];
  fcbl->state[1] = fcbl->state[0];
  fcbl->state[0] = tmp_state;
  return output;
}
