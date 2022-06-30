#ifndef CAN_THREAD_H
  #define CAN_THREAD_H


#define CAN_MSG_BUF_LEN 100

typedef struct
{
  uint32_t pid;
  uint32_t len;
  uint8_t  msg[8];

} T_can_msg;


// Структура для сбора диагностики по принимаемым пакетам
typedef struct
{
  uint32_t   received_msg_cnt;
  uint32_t   transmitted_msg_cnt;

  T_can_msg  msg_buf[CAN_MSG_BUF_LEN];
  uint32_t   msg_buf_pos;

} T_can_stat;





void     Thread_CAN_create(void);
uint32_t CAN_send_packet(uint32_t canid, uint8_t *data, uint8_t len, uint8_t rtr);

#endif // CAN_THREAD_H



