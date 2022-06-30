#include "App.h"
#include "freemaster_cfg.h"
#include "freemaster.h"
#include "freemaster_tsa.h"



FMSTR_TSA_TABLE_LIST_BEGIN()

FMSTR_TSA_TABLE(wvar_vars)
FMSTR_TSA_TABLE(app_vars)

FMSTR_TSA_TABLE_LIST_END()

T_serial_io_driver           *frm_drv;


#define FM_PIPE_RX_BUF_SIZE 64
#define FM_PIPE_TX_BUF_SIZE 1024
#define FM_PIPE_MAX_STR_LEN 512

#define FM_PIPE_PORT_NUM    0
#define FM_PIPE_CALLBACK    0
#define FM_PIPE_TYPE        FMSTR_PIPE_TYPE_ANSI_TERMINAL
FMSTR_ADDR                  pipeRxBuff;
FMSTR_PIPE_SIZE             pipeRxSize;
FMSTR_ADDR                  pipeTxBuff;
FMSTR_PIPE_SIZE             pipeTxSize;

FMSTR_HPIPE                 fm_pipe = NULL;
T_app_log_record            *p_log_rec;
char                        *log_str;
uint8_t                     f_unsent_record;

static  uint8_t             task_freemaster_created;
static uint8_t              *p_freemaster_stack;
static TX_THREAD            *p_freemaster_thread;

T_fm_pins_state             pst;
T_fm_pins_state             pst_prev;


static void Thread_freemaster(ULONG initial_data);

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t FreeMaster_task_create(ULONG initial_data)
{
  UINT                err;
  T_serial_io_driver *p_drv;

  if (task_freemaster_created != 0) return RES_ERROR;

  // Инициализируем драйвер
  // Выделяется память драйвера, создается задача приема драйвера
  p_drv = (T_serial_io_driver *)initial_data;
  if (p_drv->_init(&p_drv->pdrvcbl, p_drv) != RES_OK) return RES_ERROR;

  p_freemaster_stack = App_malloc(THREAD_FREEMASTER_STACK_SIZE);
  if (p_freemaster_stack == NULL) goto err_exit_;

  p_freemaster_thread = App_malloc(sizeof(TX_THREAD));
  if (p_freemaster_thread == NULL) goto err_exit_;

  err = tx_thread_create(
       p_freemaster_thread,
       (CHAR *)"FreeMaster",
       Thread_freemaster,
       (ULONG)p_drv,
       p_freemaster_stack,
       THREAD_FREEMASTER_STACK_SIZE,
       THREAD_FREEMASTER_PRIORITY,
       THREAD_FREEMASTER_PRIORITY,
       1,
       TX_AUTO_START
       );

  if (err != TX_SUCCESS) return RES_ERROR;
  task_freemaster_created = 1;
  return RES_OK;

err_exit_:

  App_free(p_freemaster_thread);
  App_free(p_freemaster_stack);
  return RES_ERROR;
}


/*-----------------------------------------------------------------------------------------------------
  Функция для управления пинами из FreeMaster

  \param void
-----------------------------------------------------------------------------------------------------*/
static void Pins_control(void)
{
  if (memcmp(&pst,&pst_prev, sizeof(T_fm_pins_state))!= 0)
  {
    Set_OUT1_level ( pst.OUT1      );
    Set_OUT2_level ( pst.OUT2      );
    Set_OUT3_level ( pst.OUT3      );
    Set_OUT4_level ( pst.OUT4      );
    Set_LED_green  ( pst.LED_green );
    Set_LED_red    ( pst.LED_red   );
    Set_ASW_R      ( pst.ASW_R     );
    Set_ASW_F      ( pst.ASW_F     );
    Set_PSW_R      ( pst.PSW_R     );
    Set_PSW_F      ( pst.PSW_F     );
    Set_LSW_F      ( pst.LSW_F     );
    Set_OLED_RES   ( pst.OLED_RES  );
    Set_OLEDV      ( pst.OLEDV     );
    Set_OLED_DC    ( pst.OLED_DC   );
    Set_OLED_CS    ( pst.OLED_CS   );
    Set_EN_CHARGER ( pst.EN_CHARGER);
    Set_DCDC_MODE  ( pst.DCDC_MODE );

    pst_prev = pst;
  }

}

/*-----------------------------------------------------------------------------------------------------
  Обработка пользовательских комманд поступающих движку FreeMaster

 \param app_command

 \return uint8_t
-----------------------------------------------------------------------------------------------------*/
static uint8_t Freemaster_Command_Manager(uint16_t app_command)
{
  uint8_t  res;
  uint32_t len;
  res = 0;
  uint8_t  *dbuf;

  // Получаем указатель на буфер с данными команды
  dbuf = FMSTR_GetAppCmdData(&len);

  switch (app_command)
  {
  case FMCMD_CHECK_LOG_PIPE:
    FMSTR_PipePuts(fm_pipe, "Log pipe checked.\r\n");
    break;
  case FMCMD_START_STFS_TEST:
    Send_cmd_execute_sffs_test();
    break;
  case FMCMD_STOP_STFS_TEST:
    Send_cmd_delete_STfs_test();
    break;
  case FMCMD_DUMP_SECTOR:
    if (len >= 4)
    {
      uint32_t val;
      memcpy(&val, dbuf, 4);
      Thread_STfs_test_create(val | BIT(31));
    }
    break;
  case FMCMD_SAVE_WVARS:
    Req_to_save_settings();
    break;


  case FMCMD_RESET_DEVICE:
    Reset_system();
    break;
  default:
    res = 0;
    break;
  }


  return res;
}


/*-----------------------------------------------------------------------------------------------------


  \param str
  \param max_str_len
  \param p_log_rec
-----------------------------------------------------------------------------------------------------*/
void Format_log_string(char *str, uint32_t max_str_len, T_app_log_record  *p_log_rec)
{
  uint32_t sec, usec;
  Timestump_convert_to_sec_usec(&p_log_rec->time,&sec,&usec);

  if (p_log_rec->line_num != 0)
  {
    snprintf(str, max_str_len, "%06d.%06d %s (%s %d)\n\r",
      sec, usec,
      p_log_rec->msg,
      p_log_rec->func_name,
      p_log_rec->line_num);
  }
  else
  {
    snprintf(str, max_str_len, "%06d.%06d %s\n\r",
      sec, usec,
      p_log_rec->msg);
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Freemaster_send_log_to_pipe(void)
{
  if (f_unsent_record != 0)
  {
    if (FMSTR_PipePuts(fm_pipe, log_str) != FMSTR_TRUE) return;
    f_unsent_record = 0;
  }

  while (AppLog_get_tail_record(p_log_rec) != 0)
  {
    Format_log_string(log_str, FM_PIPE_MAX_STR_LEN, p_log_rec);
    if (FMSTR_PipePuts(fm_pipe, log_str) != FMSTR_TRUE)
    {
      f_unsent_record = 1;
      return;
    }
  }

}

/*-------------------------------------------------------------------------------------------------------------
  Цикл движка FreeMaster
-------------------------------------------------------------------------------------------------------------*/
static void Thread_freemaster(ULONG initial_data)
{
  uint16_t app_command;
  uint8_t  res;

  tx_thread_identify()->driver =  initial_data;

  // Получаем указатель на драйвер последовательного интерфейса
  frm_drv = (T_serial_io_driver *)(tx_thread_identify()->driver);

  if (!FMSTR_Init())
  {
    return;
  }

  memset(&pst, 0, sizeof(T_fm_pins_state));
  memset(&pst_prev, 0, sizeof(T_fm_pins_state));


  pipeRxSize = FM_PIPE_RX_BUF_SIZE;
  pipeRxBuff = App_malloc(pipeRxSize);
  pipeTxSize = FM_PIPE_TX_BUF_SIZE;
  pipeTxBuff = App_malloc(pipeTxSize);
  p_log_rec  = App_malloc(sizeof(T_app_log_record));
  log_str    = App_malloc(FM_PIPE_MAX_STR_LEN);
  if ((pipeRxBuff != NULL) && (pipeTxBuff != NULL) && (p_log_rec != NULL) && (log_str != NULL))
  {
    fm_pipe = FMSTR_PipeOpen(FM_PIPE_PORT_NUM, FM_PIPE_CALLBACK,  pipeRxBuff, pipeRxSize, pipeTxBuff, pipeTxSize, FM_PIPE_TYPE, "SysLog");
  }

  while (1)
  {
    app_command = FMSTR_GetAppCmd();

    if (app_command != FMSTR_APPCMDRESULT_NOCMD)
    {
      res = Freemaster_Command_Manager(app_command);
      FMSTR_AppCmdAck(res);
    }
    FMSTR_Poll();
    Pins_control();
    if (fm_pipe != NULL)
    {
      Freemaster_send_log_to_pipe();
    }
  }
}


