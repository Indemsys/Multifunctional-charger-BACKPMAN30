#ifndef DCDC_CONTROLLER_H
  #define DCDC_CONTROLLER_H


#define  DCDC_OVERHEAT_BOUNDARY  (90.0f)
#define  MIN_CHARGER_INPUT_V     (20.0f) // Минимальное напряжение при ктором может еще работать зарядник

#define  DCDC_NO_FAULT      0
#define  FAULT_OVERHEAT     1
#define  FAULT_OVERLOAD     2
#define  FAULT_DCDC_FAULT   3
#define  FAULT_TOO_LOW_SYSV 4


typedef struct
{
  uint32_t  fault;
  uint16_t  dac_data;
  uint16_t  dac_steps;
  uint32_t  dcdc_sync_freq;
  uint8_t   dcdc_mode;

  uint32_t  dcdc_on_timer;
  uint8_t   control_state;
  uint8_t   matlab_control;

} T_power_controller_state;


extern T_power_controller_state    pwr_cbl;
extern uint32_t                    dcdc_min_in_v;  // Кодированное в формате ADC значение минимального напряжения на входе зарядника для его нормальной работы


void     DCDC_toggle_state(void);
void     DAC_proc(uint16_t dac_data);
uint16_t Get_DAC_value(void);
uint16_t Get_DAC_value_from_steps(uint16_t steps);
float    Get_voltage_from_value(uint16_t val);
void     DCDC_emergency_shutdown(uint32_t fault);
uint8_t  Get_DCDC_MODE(void);
void     DCDC_reset_fault(void);
void     Set_DCDC_MODE(uint32_t val);
#endif // DCDC_CONTROLLER_H



