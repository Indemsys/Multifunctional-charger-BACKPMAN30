#ifndef BACKPMAN3_TIMERS_H
  #define BACKPMAN3_TIMERS_H


#define  PWM_FREQ           100000ul  // Частота генерации PWM на выходе канала A таймера HRTIM1
#define  PWM_DEAD_TIME      (50ull)   // Величина мертвого времени между импульсами PWM в нс

#define  HRTIM1_CLK         CPU_CLK   // Тактирование таймера HRTIM1 выполняем от CPU clock (480 MHz)

#define  HRTIM1_PER         (HRTIM1_CLK/PWM_FREQ)  // Значение регистра периода в канале A таймера HRTIM1
                                                   // Минимальное значение = 3, максимальное значение = 0xFFDF

#define  HRTIM1_DEADT_VAL   (HRTIM1_CLK*PWM_DEAD_TIME/1000000000ull) // Величина загрузаемая в регистр мертвого времени


#define  TIM1_CLOCK_KHZ     (240000)
#define  TIM1_MIN_FREQ_KHZ  (200)
#define  TIM1_MAX_FREQ_KHZ  (600)

#define  TIM1_MIN_FREQ_ARR  (TIM1_CLOCK_KHZ/TIM1_MIN_FREQ_KHZ)
#define  TIM1_MAX_FREQ_ARR  (TIM1_CLOCK_KHZ/TIM1_MAX_FREQ_KHZ)


void HRTIM1_init(void);
void Set_DCDC_PWM_percent(uint32_t val);
void TIM1_init(void);
void TIM1_set_CH3_mode(uint8_t v);
void TIM1_set_CH3_freq(uint32_t freq_khz);


#endif // BACKPMAN3_TIMERS_H



