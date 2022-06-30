#ifndef _APP_H
#define _APP_H

typedef struct
{
  uint32_t  cpu_usage;
  float     ain1      ;
  float     ain2      ;
  float     ain3      ;
  float     ain4      ;
  float     vref165   ;
  float     acc_i     ;
  float     psrc_i    ;
  float     load_i    ;
  float     acc_v     ;
  float     psrc_v    ;
  float     load_v    ;
  float     sys_v     ;
  float     ref_v     ;
  float     cpu_temp  ;
  float     t_sens    ;

} T_app_state;


void Req_to_save_settings(void);
void Req_to_reset_log_file(void);
void Thread_main(ULONG initial_input);
void Copy_app_state(T_app_state *app_state_copy);
void Take_app_state_snapshot(void);

#endif // APP_H
