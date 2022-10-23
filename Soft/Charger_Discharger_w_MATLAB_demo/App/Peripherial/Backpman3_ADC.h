#ifndef BACKPMAN3_ADC_H
  #define BACKPMAN3_ADC_H



#define          ADC_PERIOD_US                50    // Период ADC в мкc
#define          ADC_RESULTS_PERIOD_US        1000  // Период выдачи результатов ADC в мкс

#define          ADC_CH_CNT                5 //  Количество каналов АЦП обрабатываемых за один цикл сканирования


#define          ADC1_ID                   0
#define          ADC2_ID                   1
#define          ADC3_ID                   2


#define SMT_1_5_CYC     0 //  1.5   ADC clock cycles
#define SMT_2_5_CYC     1 //  2.5   ADC clock cycles
#define SMT_8_5_CYC     2 //  8.5   ADC clock cycles
#define SMT_16_5_CYC    3 //  16.5  ADC clock cycles
#define SMT_32_5_CYC    4 //  32.5  ADC clock cycles
#define SMT_64_5_CYC    5 //  64.5  ADC clock cycles
#define SMT_387_5_CYC   6 //  387.5 ADC clock cycles
#define SMT_810_5_CYC   7 //  810.5 ADC clock cycles



//Назначение номеро аппаратных тригеров для ADC

#define          ADC_TRG_TIM1_OC1          0
#define          ADC_TRG_TIM1_OC2          1
#define          ADC_TRG_TIM1_OC3          2
#define          ADC_TRG_TIM2_OC2          3
#define          ADC_TRG_TIM3_TRGO         4
#define          ADC_TRG_TIM4_OC4          5
#define          ADC_TRG_EXTI11            6
#define          ADC_TRG_TIM8_TRGO         7
#define          ADC_TRG_TIM8_TRGO2        8
#define          ADC_TRG_TIM1_TRGO         9
#define          ADC_TRG_TIM1_TRGO2        10
#define          ADC_TRG_TIM2_TRGO         11
#define          ADC_TRG_TIM4_TRGO         12
#define          ADC_TRG_TIM6_TRGO         13
#define          ADC_TRG_TIM15_TRGO        14
#define          ADC_TRG_TIM3_OC4          15
#define          ADC_TRG_HRTIM1_ADCTRG1    16
#define          ADC_TRG_HRTIM1_ADCTRG3    17
#define          ADC_TRG_LPTIM1_OUT        18
#define          ADC_TRG_LPTIM2_OUT        19
#define          ADC_TRG_LPTIM3_OUT        20


#define          ADC_HW_TRG_DIS            0  //  00: Hardware trigger detection disabled (conversions can be launched by software)
#define          ADC_HW_TRG_RSE            1  //  01: Hardware trigger detection on the rising edge
#define          ADC_HW_TRG_FLE            2  //  10: Hardware trigger detection on the falling edge
#define          ADC_HW_TRG_RFE            3  //  11: Hardware trigger detection on both the rising and falling edges





#define A_CR_ADCAL          BIT(31)
#define A_CR_ADCALDIF       BIT(30)
#define A_CR_DEEPPWD        BIT(29)
#define A_CR_ADVREGEN       BIT(28)
#define A_CR_LINCALRDYW6    BIT(27)
#define A_CR_LINCALRDYW5    BIT(26)
#define A_CR_LINCALRDYW4    BIT(25)
#define A_CR_LINCALRDYW3    BIT(24)
#define A_CR_LINCALRDYW2    BIT(23)
#define A_CR_LINCALRDYW1    BIT(22)
#define A_CR_ADCALLIN       BIT(16)
#define A_CR_BOOST          BIT( 8)
#define A_CR_JADSTP         BIT( 5)
#define A_CR_ADSTP          BIT( 4)
#define A_CR_JADSTART       BIT( 3)
#define A_CR_ADSTART        BIT( 2)
#define A_CR_ADDIS          BIT( 1)
#define A_CR_ADEN           BIT( 0)


#define A_IS_LDORDY         BIT(12) // ADC LDO output voltage ready bit
#define A_IS_JQOVF          BIT(10) // Injected context queue overflow
#define A_IS_AWD3           BIT( 9) // Analog watchdog 3 flag
#define A_IS_AWD2           BIT( 8) // Analog watchdog 2 flag
#define A_IS_AWD1           BIT( 7) // Analog watchdog 1 flag
#define A_IS_JEOS           BIT( 6) // Injected channel end of sequence flag
#define A_IS_JEOC           BIT( 5) // Injected channel end of conversion flag
#define A_IS_OVR            BIT( 4) // ADC overrun
#define A_IS_EOS            BIT( 3) // End of regular sequence flag
#define A_IS_EOC            BIT( 2) // End of conversion flag
#define A_IS_EOSMP          BIT( 1) // End of sampling flag
#define A_IS_ADRDY          BIT( 0) // ADC ready


#define A_IE_JQOVFIE        BIT(10) // Enable interrupt : Injected context queue overflow
#define A_IE_AWD3IE         BIT( 9) // Enable interrupt : Analog watchdog 3 flag
#define A_IE_AWD2IE         BIT( 8) // Enable interrupt : Analog watchdog 2 flag
#define A_IE_AWD1IE         BIT( 7) // Enable interrupt : Analog watchdog 1 flag
#define A_IE_JEOSIE         BIT( 6) // Enable interrupt : Injected channel end of sequence flag
#define A_IE_JEOCIE         BIT( 5) // Enable interrupt : Injected channel end of conversion flag
#define A_IE_OVRIE          BIT( 4) // Enable interrupt : ADC overrun
#define A_IE_EOSIE          BIT( 3) // Enable interrupt : End of regular sequence flag
#define A_IE_EOCIE          BIT( 2) // Enable interrupt : End of conversion flag
#define A_IE_EOSMPIE        BIT( 1) // Enable interrupt : End of sampling flag
#define A_IE_ADRDYIE        BIT( 0) // Enable interrupt : ADC ready

typedef struct
{
  uint32_t smpl_ACC_I   ;
  uint32_t smpl_PSRC_I  ;
  uint32_t smpl_LOAD_V  ;
  uint32_t smpl_AIN2    ;
  uint32_t smpl_VREF165 ;

  uint32_t smpl_ACC_V   ;
  uint32_t smpl_SYS_V   ;
  uint32_t smpl_AIN3    ;
  uint32_t smpl_AIN1    ;
  uint32_t smpl_AIN4    ;

  uint32_t smpl_TSENS   ;
  uint32_t smpl_PSRC_V  ;
  uint32_t smpl_LOAD_I  ;
  uint32_t smpl_TERM    ;
  uint32_t smpl_VREF    ;
} T_adc;


extern T_adc adcs;

uint32_t  ADCs_init(void);
uint32_t  ADC_disable(ADC_TypeDef *ADC);
void      ADCs_software_start(ADC_TypeDef *ADC);

void      ADC_IRQHandler(void);
void      ADC3_IRQHandler(void);

#endif // BACKPMAN3_ADC_H



