#ifndef MATLAB_CONNECTION_H
  #define MATLAB_CONNECTION_H


  #define MATLAB_CONN_PORT 3333

  #define MTLB_CMD_FILES_WRITE_READ_DEL_TEST      1
  #define MTLB_CMD_CONT_WRITE_TO_FILE_TEST        2
  #define MTLB_CMD_PWR_SRC_CONTROL                3
  #define MTLB_CMD_STOP                           0xFF
  #define MTLB_ANSWER_OK                          0
  #define MTLB_ANSWER_ERROR                       1

__packed typedef struct
{
  uint32_t cmd_id;
  uint32_t sz;
  uint8_t  en_del_flag;
  uint8_t  en_read_flag;
  uint16_t crc;
}
T_matlab_cmd1;


__packed typedef struct
{
  uint32_t cmd_id;
  uint32_t block_sz;
  uint32_t block_num;
  uint16_t crc;
}
T_matlab_cmd2;


__packed typedef struct
{
  uint32_t cmd_id;
  struct
  {
    uint8_t PSW_R:1;
    uint8_t PSW_F:1;
    uint8_t ASW_R:1;
    uint8_t ASW_F:1;
    uint8_t  MODE:1;
    uint8_t EN_CH:1;
    uint8_t LSW_F:1;
    uint8_t C0   :1;
  } control_bits;
    struct
  {
    uint8_t Set_PSRC_i_offset :1;
    uint8_t Set_Accum_i_offset:1;
    uint8_t Set_Load_i_offset :1;
    uint8_t C0                :1;
    uint8_t C1                :1;
    uint8_t C2                :1;
    uint8_t C3                :1;
    uint8_t C4                :1;
  } command_bits;
  uint16_t dac_val;
  uint16_t crc;
}
T_matlab_cmd3;

extern uint32_t         matlab_time_step;
extern uint32_t         matlab_time_step_min;
extern uint32_t         matlab_time_step_max;


uint32_t Thread_MATLAB_create(void);
uint32_t Thread_MATLAB_start(void);
uint32_t Thread_MATLAB_delete(void);
uint32_t Thread_MATLAB_terminate(void);
uint32_t MATLAB_connection_server_create(NX_IP *ip_ptr);
uint32_t MATLAB_connection_server_delete(void);
int      MATLAB_buf_send(const void *buf, unsigned int len);


#endif // MATLAB_CONNECTION_H



