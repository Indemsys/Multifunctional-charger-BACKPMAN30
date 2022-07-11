#ifndef FILTERS_H
# define FILTERS_H

//=======================================================================*
//  Fixed width word size data types:                                     *
//    int8_T, int16_T, int32_T     - signed 8, 16, or 32 bit integers     *
//    uint8_T, uint16_T, uint32_T  - unsigned 8, 16, or 32 bit integers   *
//    real32_T, real64_T           - 32 and 64 bit floating point numbers *
// =======================================================================
typedef signed char int8_T;
typedef unsigned char uint8_T;
typedef short int16_T;
typedef unsigned short uint16_T;
typedef int int32_T;
typedef unsigned int uint32_T;
typedef float real32_T;
typedef double real64_T;

//===========================================================================*
//  Generic type definitions: boolean_T, char_T, byte_T, int_T, uint_T,       *
//                            real_T, time_T, ulong_T.                        *
// ===========================================================================
typedef double real_T;
typedef double time_T;
typedef unsigned char boolean_T;
typedef int int_T;
typedef unsigned int uint_T;
typedef unsigned long ulong_T;
typedef char char_T;
typedef unsigned char uchar_T;
typedef char_T byte_T;

//=======================================================================*
//  Min and Max:                                                          *
//    int8_T, int16_T, int32_T     - signed 8, 16, or 32 bit integers     *
//    uint8_T, uint16_T, uint32_T  - unsigned 8, 16, or 32 bit integers   *
// =======================================================================
#define MAX_int8_T                     ((int8_T)(127))
#define MIN_int8_T                     ((int8_T)(-128))
#define MAX_uint8_T                    ((uint8_T)(255U))
#define MAX_int16_T                    ((int16_T)(32767))
#define MIN_int16_T                    ((int16_T)(-32768))
#define MAX_uint16_T                   ((uint16_T)(65535U))
#define MAX_int32_T                    ((int32_T)(2147483647))
#define MIN_int32_T                    ((int32_T)(-2147483647-1))
#define MAX_uint32_T                   ((uint32_T)(0xFFFFFFFFU))


typedef struct
{
    uint8_t  en;
    uint8_t  smpl_cnt;
    uint16_t arr[3];
    uint16_t output;

} T_median_filter_uint16_decim;

typedef struct
{
    uint8_t  en;
    uint8_t  smpl_cnt;
    uint16_t arr[3];

} T_median_filter_uint16;

typedef struct
{
    uint8_t  en;
    uint8_t  smpl_cnt;
    uint32_t arr[3];

} T_median_filter_uint32;

typedef struct
{
    uint8_t  en;
    uint8_t  smpl_cnt;
    int32_t  arr[3];

} T_median_filter_int32;


# define RUN_AVERAGE_FILTER_8 (8)          // Длина фильтра скользящего среднего
typedef struct
{
    uint8_t  en;
    uint32_t head;
    float    acc;
    float    arr[RUN_AVERAGE_FILTER_8];

} T_run_average_float_8;


typedef struct
{
    uint8_t  en;
    uint32_t head;
    float    acc;
    float    *arr;
    uint32_t len;

} T_run_average_float_N;


typedef struct
{
    uint8_t  en;
    uint32_t head;
    int32_t  acc;
    int32_t  *arr;
    int32_t  len;

} T_run_average_int32_N;



typedef struct
{
    uint8_t  en;
    uint32_t head;
    uint32_t  acc;
    uint32_t  *arr;
    int32_t  len;
} T_run_average_uint32_N;

# define RUN_AVERAGE_FILTER_4 (4)          // Длина фильтра скользящего среднего
typedef struct
{
    uint8_t  en;
    uint32_t head;
    int16_t  acc;
    int16_t  arr[RUN_AVERAGE_FILTER_4];

}T_run_average_int16_4;

typedef struct
{
    uint8_t  en;
    uint32_t head;
    int32_t  acc;
    int32_t  arr[RUN_AVERAGE_FILTER_4];

}T_run_average_int32_4;



# define RUN_AVERAGE_FILTER_32 (32)          // Длина фильтра скользящего среднего
typedef struct
{
    uint8_t   en;
    uint32_t  head;
    uint32_t  acc;
    uint16_t  arr[RUN_AVERAGE_FILTER_32];

}T_run_average_uint16_32;


# define RUN_AVERAGE_FILTER_20 (20)          // Длина фильтра скользящего среднего
typedef struct
{
    uint8_t   en;
    uint32_t  head;
    uint32_t  acc;
    uint16_t  arr[RUN_AVERAGE_FILTER_20];

}T_run_average_uint32_20;



typedef struct
{
    uint8_t   en;
    uint32_t  head;
    int32_t   acc;
    int16_t   arr[RUN_AVERAGE_FILTER_32];

}T_run_average_int16_32;


typedef struct
{
    float states[2]; /* '<S1>/Filter_HP_10' */
}
T_hp10_filter1_dw;


typedef struct
{
    float states[2]; /* '<S1>/Filter_HP_10' */
}
T_hp02_filter1_dw;


/* Block signals and states (auto storage) for system '<Root>' */
typedef struct
{
    float  Delay11_DSTATE;               /* '<S1>/Delay11' */
    float  Delay21_DSTATE;               /* '<S1>/Delay21' */
    float  Delay12_DSTATE;               /* '<S1>/Delay12' */
}
T_eliptic_filter1_dw;

typedef struct
{
    float  Delay11_DSTATE;             /* '<S1>/Delay11' */
    float  Delay21_DSTATE;             /* '<S1>/Delay21' */
    float  Delay12_DSTATE;             /* '<S1>/Delay12' */
    float  Delay22_DSTATE;             /* '<S1>/Delay22' */
    float  Delay13_DSTATE;             /* '<S1>/Delay13' */
}
T_eliptic_110Hz_filter_dw;



/* Block signals and states (auto storage) for system '<Root>' */
typedef struct {
    int16_t BodyDelay17_DSTATE;          /* '<S1>/BodyDelay17' */
    int16_t BodyDelay16_DSTATE;          /* '<S1>/BodyDelay16' */
    int16_t BodyDelay15_DSTATE;          /* '<S1>/BodyDelay15' */
    int16_t BodyDelay14_DSTATE;          /* '<S1>/BodyDelay14' */
    int16_t BodyDelay13_DSTATE;          /* '<S1>/BodyDelay13' */
    int16_t BodyDelay12_DSTATE;          /* '<S1>/BodyDelay12' */
    int16_t BodyDelay11_DSTATE;          /* '<S1>/BodyDelay11' */
    int16_t BodyDelay10_DSTATE;          /* '<S1>/BodyDelay10' */
    int16_t BodyDelay9_DSTATE;           /* '<S1>/BodyDelay9' */
    int16_t BodyDelay8_DSTATE;           /* '<S1>/BodyDelay8' */
    int16_t BodyDelay7_DSTATE;           /* '<S1>/BodyDelay7' */
    int16_t BodyDelay6_DSTATE;           /* '<S1>/BodyDelay6' */
    int16_t BodyDelay5_DSTATE;           /* '<S1>/BodyDelay5' */
    int16_t BodyDelay4_DSTATE;           /* '<S1>/BodyDelay4' */
    int16_t BodyDelay3_DSTATE;           /* '<S1>/BodyDelay3' */
    int16_t BodyDelay2_DSTATE;           /* '<S1>/BodyDelay2' */
    int16_t BodyDelay18_DSTATE;          /* '<S1>/BodyDelay18' */
} T_fir_filter;


/* Block signals and states (default storage) for system '<Root>' */
typedef struct {
    float Delay11_DSTATE;             /* '<S1>/Delay11' */
    float Delay21_DSTATE;             /* '<S1>/Delay21' */
    float Delay12_DSTATE;             /* '<S1>/Delay12' */
    float Delay22_DSTATE;             /* '<S1>/Delay22' */
} T_eliptic_5hz_filter;

typedef struct {
  float Delay11_DSTATE;             /* '<S1>/Delay11' */
  float Delay21_DSTATE;             /* '<S1>/Delay21' */
  float Delay12_DSTATE;             /* '<S1>/Delay12' */
  float Delay22_DSTATE;             /* '<S1>/Delay22' */
  float Delay13_DSTATE;             /* '<S1>/Delay13' */
  float Delay23_DSTATE;             /* '<S1>/Delay23' */
  float Delay14_DSTATE;             /* '<S1>/Delay14' */
  float Delay24_DSTATE;             /* '<S1>/Delay24' */
} T_eliptic_50hz_filter;

typedef struct {
  int32_t Delay11_DSTATE;              /* '<S1>/Delay11' */
  int32_t Delay21_DSTATE;              /* '<S1>/Delay21' */
  int32_t Delay12_DSTATE;              /* '<S1>/Delay12' */
} T_fixlp_100_200_30db;


typedef struct
{
  float state[4];/* '<S1>/Digital Filter' */
}
T_LPF_Ch_60_200_36;

typedef struct
{
  float state[4];/* '<S1>/Digital Filter' */
}
T_LPF_But_25_200_33;

typedef struct
{
  float state[4];/* '<S1>/Digital Filter' */
}
T_LPF_Elip_100_200_30;

typedef struct
{
  float state[2]; /* '<S1>/Digital Filter' */
}
T_LPF_Elip_5_200_70;

typedef struct
{
   int32_t  k;    // Степеь коэффициента фильтра - n. Коэффициент выражается в виде 2^n , т.е. является степенью двойки для того чтобы можно было применять сдвиги вместо делений и умножений
   int32_t  v1;   // Сохраненное значение
   int8_t   init; // Флаг инициализации.  Если 0, то фильтрация начинается от текущего входного значения, иначе от нуля

}  T_exp_filter;

typedef struct
{
  real32_T state[2];
}
T_LPF_But_100_31250;

typedef struct
{
  float state[2];
}
T_MaxFlat_1_1000_cbl;


uint16_t MedianFilter_3uint16_decim(uint16_t inp, T_median_filter_uint16_decim *fltr, uint8_t decimation);
int16_t  MedianFilter_3uint16(uint16_t inp, T_median_filter_uint16 *fltr);
int16_t  MedianFilter_3int16(int16_t inp, T_median_filter_uint16 *fltr);
uint32_t MedianFilter_3uint32(uint32_t inp, T_median_filter_uint32 *fltr);
int32_t  MedianFilter_3int32(int32_t inp, T_median_filter_int32 *fltr);
float    RunAverageFilter_float_8(float inp, T_run_average_float_8 *fltr);
int16_t  RunAverageFilter_int16_4(int16_t inp, T_run_average_int16_4 *fltr);
int32_t  RunAverageFilter_int32_4(int32_t inp, T_run_average_int32_4 *fltr);
float    RunAverageFilter_float_N(float inp, T_run_average_float_N *fltr);
int32_t  RunAverageFilter_int32_N(int32_t inp, T_run_average_int32_N *fltr);
int32_t  RunAverageFilter_uint32_N(uint32_t inp, T_run_average_uint32_N *fltr);
uint16_t RunAverageFilter_uint16_32(int16_t inp, T_run_average_uint16_32 *fltr);
int16_t  RunAverageFilter_int16_32(int16_t inp, T_run_average_int16_32 *fltr);
int32_t  RunAverageFilter_uint32_20(uint32_t inp, T_run_average_uint32_20 *fltr);

float    LPF_Elip_100_200_35(T_eliptic_filter1_dw *rtDW, int16_t Input);
float    LPF_Elip_110_150_48(T_eliptic_110Hz_filter_dw *rtDW, int16_t Input);
int16_t  LPF_FIR_200_400_20_int16(T_fir_filter *rtDW, int16_t Input);
int16_t  IIR_HP_10_filter(T_hp10_filter1_dw *rtDW, int16_t Input);
int16_t  IIR_HP_02_filter(T_hp02_filter1_dw *rtDW, int16_t Input);

float    LPF_Elip_5_100_124(T_eliptic_5hz_filter *rtDW, float input);
float    LPF_Elip_50_200_62(T_eliptic_50hz_filter *rtDW, float input);
int16_t  LPF_Elip_100_200_30_int16(T_fixlp_100_200_30db *rtDW, int16_t input);
float    LPF_Cheb_60_200_36_step(T_LPF_Ch_60_200_36 *rtDW,  float input);
float    LPF_But_25_200_33_step(T_LPF_But_25_200_33 *rtDW, float input);
float    Elip_100_200_30_output(T_LPF_Elip_100_200_30 *rtDW, float input);
float    Elip_5_200_70_output(T_LPF_Elip_5_200_70 *rtDW, float input);

int32_t  Exponential_filter(T_exp_filter *flt, int32_t input );

float    LPF_But_100_31250_step(T_LPF_But_100_31250 *rtDW, float input);
float    LPF_MaxFlat_1_1000_step(T_MaxFlat_1_1000_cbl *fcbl, float input);

#endif // FILTERS_H



