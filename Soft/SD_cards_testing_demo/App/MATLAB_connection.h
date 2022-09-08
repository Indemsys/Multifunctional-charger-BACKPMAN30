#ifndef MATLAB_CONNECTION_H
  #define MATLAB_CONNECTION_H


  #define MATLAB_CONN_PORT 3333

  #define MTLB_CMD_FILES_WRITE_READ_DEL_TEST      1
  #define MTLB_CMD_FILES_CONT_WRITE_TEST          2
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

uint32_t Thread_MATLAB_create(void);
uint32_t Thread_MATLAB_start(void);
uint32_t Thread_MATLAB_delete(void);
uint32_t MATLAB_connection_server_create(NX_IP *ip_ptr);
uint32_t MATLAB_connection_server_delete(void);
int      MATLAB_buf_send(const void *buf, unsigned int len);


#endif // MATLAB_CONNECTION_H



