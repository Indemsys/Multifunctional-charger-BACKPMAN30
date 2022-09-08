#ifndef BACKPMAN3_TIMERS_H
  #define BACKPMAN3_TIMERS_H


#define  PWM_FREQ           100000ul  // Частота генерации PWM на выходе канала A таймера HRTIM1
#define  PWM_DEAD_TIME      (50ull)   // Величина мертвого времени между импульсами PWM в нс

#define  HRTIM1_CLK         CPU_CLK   // Тактирование таймера HRTIM1 выполняем от CPU clock (480 MHz)

#define  HRTIM1_PER         (HRTIM1_CLK/PWM_FREQ)  // Значение регистра периода в канале A таймера HRTIM1
                                                   // Минимальное значение = 3, максимальное значение = 0xFFDF

#define  HRTIM1_DEADT_VAL   (HRTIM1_CLK*PWM_DEAD_TIME/1000000000ull) // Величина загрузаемая в регистр мертвого времени


void HRTIM1_init(void);

void Set_DCDC_PWM_percent(uint32_t val);

#endif // BACKPMAN3_TIMERS_H



