#ifndef ENCODER_HANDLER_H
  #define ENCODER_HANDLER_H


#define ENC_SW_PRESSED            1
#define ENC_SW_LONG_PRESSED       2
#define ENC_SW_MAX_LONG_PRESSED   3

typedef struct
{
    uint8_t               curr_enc_a_smpl;
    uint8_t               curr_enc_b_smpl;
    uint8_t               curr_enc_sw_smpl;

    uint8_t               enc_a_smpl_prev;
    uint8_t               enc_b_smpl_prev;
    uint8_t               enc_sw_smpl_prev;
    uint32_t              A_cnt;
    uint32_t              B_cnt;
    uint32_t              sw_cnt;
    uint32_t              prev_sw_cnt;
    uint8_t               A_state;
    uint8_t               B_state;
    uint8_t               sw_state;
    uint8_t               prev_B_state;
    uint8_t               prev_A_state;
    uint8_t               prev_sw_state;
    uint8_t               A_rising_edge;
    uint8_t               B_rising_edge;
    volatile uint32_t     encoder_counter;
    volatile uint32_t     sw_release_sig;
} T_enc_cbl;


extern T_enc_cbl        enc_cbl;


void      Manual_encoder_processing(void);
uint32_t  Get_switch_press_signal(void);
uint32_t  Get_switch_long_press_signal(void);
int32_t   Get_encoder_counter(void);
int32_t   Get_encoder_counter_delta(volatile int32_t  *prev_cnt);

#endif // ENCODER_HANDLER_H



