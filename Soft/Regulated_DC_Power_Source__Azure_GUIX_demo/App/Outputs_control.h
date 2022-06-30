#ifndef OUTPUTS_CONTROL_H
#define OUTPUTS_CONTROL_H

#define OUTP_OUT1           0
#define OUTP_OUT2           1
#define OUTP_OUT3           2
#define OUTP_OUT4           3
#define OUTP_LED_GREEN      4
#define OUTP_LED_RED        5
#define OUTP_LCD_LIGHT      6
#define OUTP_LED_CAN        7

#define OUTS_NUM            8


int32_t Outputs_mutex_create(void);

void    Set_output_blink(uint32_t out_num);
void    Set_output_on(uint32_t out_num);
void    Set_output_off(uint32_t out_num);
void    Set_output_blink_undef(uint32_t out_num);
void    Set_output_blink_3(uint32_t out_num);
void    Set_output_off_blink_3(uint32_t out_num);
void    Set_output_can_active_blink(uint32_t out_num);

void    Outputs_state_automat(void);
void    Outputs_set_pattern(const int32_t *pttn, uint32_t n, uint32_t period);

#endif // OUTPUTS_CONTROL_H



