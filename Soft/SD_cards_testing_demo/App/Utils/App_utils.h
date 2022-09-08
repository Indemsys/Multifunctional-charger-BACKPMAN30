#ifndef APP_UTILS_H
  #define APP_UTILS_H


uint16_t Get_CRC16_of_block(void *b, uint32_t len, uint16_t crc);
void     Reset_system(void);
uint64_t Measure_reference_time_interval(uint32_t time_delay_ms);
float    Get_float_val(volatile float *p_val);
void     Set_float_val(volatile float *p_val, float val);
void     Get_CPU_info(char *str);

#endif // APP_UTILS_H



