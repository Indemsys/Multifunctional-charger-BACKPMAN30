#ifndef INPUTS_CONTROLLER_H
  #define INPUTS_CONTROLLER_H

  #define IO_TASK_PERIOD     1000 // Период вызова задачи определения состояния входов в мксек

  #define ADC_PREC           65535.0f
  #define VREF               3.3f

  #define INTV_SCALE        (82.0f/1000.0f)
  #define EXIN_SCALE        (82.0f/2000.0f)
  #define AIN_SCALE         (VREF/(ADC_PREC))
  #define TMCS1100A2_SCALE  (0.1f)              // Чувствительность сенсора тока TMCS1100A2 = 0.100 V/A
  #define INA240A1_SCALE    (0.02f)              // Чувствительность сенсора тока INA240A1D  = 20V/V, на шунте 0.001 Ом чувствительность будет равна 0.02 V/A

// Структура для обработчика подавления дребезга
typedef struct
{
    int8_t    curr;
    int8_t    prev;
    uint32_t  cnt;
    uint8_t   init;

} T_bncf;

  #define GEN_SW     0
  #define ESC_SW     1

// Структура алгоритма определения состояния сигналов
typedef struct
{
    uint8_t            itype;    // Тип  сигнала. 0 - простой контакт бистабильный, 1 - контакт в цепи безопасности с 3-я состояниями
    uint32_t           *p_smpl1; // Указатель на результат сэмплирования напряжения на контакте с более высоким напряжением
    uint32_t           *p_smpl2; // Указатель на результат сэмплирования напряжения на контакте с более низким напряжением
    uint16_t           lbound;   // Граница между логическим 0 и 1 на входе
    uint32_t           l0_time;  // Время устоявшегося состояния для фиксации низкого уровня сигнала
    uint32_t           l1_time;  // Время устоявшегося состояния для фиксации высокого уровня сигнала
    uint32_t           lu_time;  // Время устоявшегося состояния для фиксации неопределенного уровня сигнала
    int8_t             *val;     // Указатель на переменную для сохранения вычисленного состояния входа
    int8_t             *val_prev; // Указатель на переменную для сохранения предыдущего состояния входа
    int8_t             *flag;    // Указатель на флаг переменной. Флаг не равный нулю указывает на произошедшее изменение состояния переменной
    T_bncf             pbncf;    // Структура для алгоритма фильтрации дребезга
} T_input_cbl;


  #define ANIN1V_BOUND          ((uint16_t)(1.0f/AIN_SCALE)) //


  #define   GEN_BNC_L  (1000/IO_TASK_PERIOD)    // Количество тактов для отработки антидребезга обычного датчика при переходе в состояние 0
  #define   GEN_BNC_H  (1000/IO_TASK_PERIOD)    // Количество тактов для отработки антидребезга обычного датчика при переходе в состояние 1



typedef struct
{
    int8_t enc_sw;          //
    int8_t enc_a;           //
    int8_t enc_b;           //

} T_backpman3_inputs;

typedef struct
{
    int8_t f_enc_sw;
    int8_t f_enc_a;
    int8_t f_enc_b;

} T_backpman3_inputs_flags;

typedef struct
{
    float  ain1;
    float  ain2;
    float  ain3;
    float  ain4;

    float  vref165;

    float  acc_i;
    float  psrc_i;
    float  load_i;

    float  acc_v;
    float  psrc_v;
    float  load_v;
    float  sys_v;
    float  ref_v;

    float  cpu_temp;
    float  t_sens;

    float  psrc_pwr;
    float  acc_pwr;
    float  loss;
    float  efficiency;

    float  flt_acc_i  ;
    float  flt_psrc_i ;
    float  flt_load_i ;
    float  flt_acc_v  ;
    float  flt_psrc_v ;
    float  flt_load_v ;
    float  flt_sys_v  ;
    float  flt_ref_v  ;

} T_fp_measurement_results;



extern uint32_t                  inp_DSENS;
extern uint32_t                  inp_LSENS;

extern T_backpman3_inputs          inp;
extern T_backpman3_inputs          inp_prev;
extern T_backpman3_inputs_flags    inp_flags;

extern T_fp_measurement_results  fp_meas;

void      Inputs_processing(void);

#endif // INPUTS_PROCESSING_H



