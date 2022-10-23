#ifndef IO_TASKS_H
  #define IO_TASKS_H

#define  HMI_SCREEN_PWR_CONTROL         1
#define  HMI_SCREEN_NET_INFO            2
#define  HMI_SCREEN_SD_CARD_INFO        3
#define  HMI_SCREEN_CALIBRATE           4
#define  HMI_SCREEN_MENU                5

#define  HMI_MENU_INDX_PWR_CONTROL      0
#define  HMI_MENU_INDX_SD_CARD_INFO     1
#define  HMI_MENU_INDX_NET_INFO         2
#define  HMI_MENU_INDX_CALIBRATE        3
#define  HMI_MENU_INDX_RESET_FAULTS     4

#define  HMU_MENU_ITEMS_COUNT      5


#define  MIN_VOIUT (1.265f)
#define  MAX_VOIUT (32.774f)
#define  STEP_V    ((MAX_VOIUT - MIN_VOIUT)/65536.0f)

#define  STEP_SET  (0.1f)
#define  MIN_V_SET (1.3f)
#define  MAX_V_SET (32.7f)
#define  STEPS_NUM ((int32_t)((MAX_V_SET-MIN_V_SET)/STEP_SET)-1)



typedef struct
{
  volatile uint32_t hmi_mode;
  volatile int32_t  enc_cnt;
  volatile int32_t  curr_menu_item;

} T_hmi_cbl;

extern  uint32_t                     cpu_id[];


void      Thread_APP_create(void);
uint32_t  Get_measured_val(float *p_val, float *res);
uint32_t  Get_app_mutex(uint32_t wait_cycles);
void      Put_app_mutex(void);
uint32_t  Get_new_HMI_mode(T_hmi_cbl **hcbl);
uint32_t  Send_flag_to_app(uint32_t flag);

void Calibrate_acc_current_offset(void);
void Calibrate_psrc_current_offset(void);
void Calibrate_load_current_offset(void);


#endif // IO_TASKS_H



