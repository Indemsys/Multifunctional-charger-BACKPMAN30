#ifndef DCDC_CONTROLLER_H
  #define DCDC_CONTROLLER_H


typedef struct
{
  uint8_t dcdc_enable;

} T_power_controller_state;


extern T_power_controller_state  pwr_cbl;
extern uint16_t                  g_dac_data;


void DCDC_toggle_state(void);
void DAC_proc(uint16_t dac_data);

#endif // DCDC_CONTROLLER_H



