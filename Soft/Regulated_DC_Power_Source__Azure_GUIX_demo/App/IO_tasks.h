#ifndef IO_TASKS_H
  #define IO_TASKS_H


extern volatile int32_t g_encoder_counter;

void      Thread_IO_create(void);
uint32_t  Get_measured_val(float *p_val, float *res);
void      Encoder_proc(void);

#endif // IO_TASKS_H



