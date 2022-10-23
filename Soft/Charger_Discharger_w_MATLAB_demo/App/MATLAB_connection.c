// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2022-08-19
// 11:37:03
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"
#include   "tx_thread.h"


#define  FILE_NAME_PRFIX          "File"
#define  MAX_TMP_STR_SZ           256
#define  DEFAULT_FILE_BUF_SIZE    32768


NET_TCP_SERVER    *matlab_tcp_server_ptr;
T_tcpsrv_setup     matlab_tcp_setup;


static void TCP_new_connection_callback(NET_TCP_SERVER *server_ptr);
static void TCP_receive_data_callback(NET_TCP_SERVER *server_ptr, NX_PACKET *packet_ptr);
static void TCP_connection_end_callback(NET_TCP_SERVER *server_ptr);

uint8_t          matlab_server_created;
uint8_t          matlab_connected;
uint32_t         matlab_cmd_id;
T_matlab_cmd1    matlab_cmd1;
T_matlab_cmd2    matlab_cmd2;
T_matlab_cmd3    matlab_cmd3;

uint32_t         matlab_time_step     = 10000;
uint32_t         matlab_time_step_min = 10000;
uint32_t         matlab_time_step_max = 10000;

static uint32_t  old_priority;
static uint8_t   new_cmd3;

static TX_MUTEX    matlab_mutex  @ "DTCM";
static TX_THREAD   matlab_thread @ "DTCM";
#pragma data_alignment=8
uint8_t     thread_matlab_stack[THREAD_MATLAB_STACK_SIZE] @ "DTCM";

uint8_t         *tmp_buf;
FX_FILE         *f;

static void Thread_MATLAB(ULONG initial_input);
static void Thread_entry_exit_notify(TX_THREAD *thread_ptr, UINT condition);

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
uint32_t Thread_MATLAB_create(void)
{
  uint32_t status;

  if (wvar.en_matlab != BINARY_YES) return RES_ERROR;

  if (matlab_thread.tx_thread_id == TX_THREAD_ID) return RES_ERROR;


  status= tx_thread_create(&matlab_thread, "MATLAB", Thread_MATLAB,
                           0,
                           (void *)thread_matlab_stack, // stack_start
                           THREAD_MATLAB_STACK_SIZE,    // stack_size
                           THREAD_MATLAB_PRIORITY,      // priority. Numerical priority of thread. Legal values range from 0 through (TX_MAX_PRIORITES-1), where a value of 0 represents the highest priority.
                           THREAD_MATLAB_PRIORITY,      // preempt_threshold. Highest priority level (0 through (TX_MAX_PRIORITIES-1)) of disabled preemption. Only priorities higher than this level are allowed to preempt this thread. This value must be less than or equal to the specified priority. A value equal to the thread priority disables preemption-threshold.
                           TX_NO_TIME_SLICE,
                           TX_DONT_START);

  if (status != TX_SUCCESS)
  {
    return RES_ERROR;
  }
  tx_thread_entry_exit_notify(&matlab_thread,Thread_entry_exit_notify);
  APPLOG("MATLAB thread created");
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------
  Функция вызывается при выполнении tx_thread_terminate
  Поэтому в этой функции нельзя вызывать tx_thread_terminate иначе возникнет бесконечная рекурсия

  \param thread_ptr
  \param condition
-----------------------------------------------------------------------------------------------------*/
static void Thread_entry_exit_notify(TX_THREAD *thread_ptr, UINT condition)
{
  if (condition == TX_THREAD_EXIT)
  {
    Thread_MATLAB_delete();
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Thread_MATLAB_start(void)
{
  return tx_thread_resume(&matlab_thread);
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Thread_MATLAB_terminate(void)
{
  uint32_t status;
  if (matlab_thread.tx_thread_id == TX_THREAD_ID)
  {
    status =  tx_thread_terminate(&matlab_thread);
  }
  else
  {
    status =  TX_SUCCESS;
  }
  APPLOG("MATLAB theread terminating status=%d", status);
  return status;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Thread_MATLAB_delete(void)
{
  uint32_t status = TX_SUCCESS;
  if (matlab_thread.tx_thread_id == TX_THREAD_ID)
  {
    if (pwr_cbl.matlab_control)
    {
      DCDC_emergency_shutdown(DCDC_NO_FAULT);
      pwr_cbl.matlab_control = 0;
    }

    status = tx_thread_delete(&matlab_thread);
  }

  if (tmp_buf != 0)
  {
    App_free(tmp_buf);
    tmp_buf = 0;
  }
  if (f != 0)
  {
    App_free(f);
    f = 0;
  }


  APPLOG("MATLAB theread deleting status=%d", status);
  return status;
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
uint32_t  MATLAB_connection_server_create(NX_IP *ip_ptr)
{
  uint32_t status;

  if (wvar.en_matlab != BINARY_YES) return RES_ERROR;

  if (tx_mutex_create(&matlab_mutex, "MATLAB", TX_INHERIT) != TX_SUCCESS) return RES_ERROR;


  if (matlab_server_created) return RES_OK;

  matlab_tcp_setup.ip_ptr                 = ip_ptr;
  matlab_tcp_setup.server_port            = MATLAB_CONN_PORT;
  matlab_tcp_setup.stack_size             = 2048;
  matlab_tcp_setup.en_options_negotiation = 0;
  matlab_tcp_setup.activity_timeout_val   = 0xFFFFFFFF; // Не контролируем таймаут поступления данных в канал от клиента
  matlab_tcp_setup.packet_pool_ptr        =&net_packet_pool;
  matlab_tcp_setup.cbl_New_connection     = TCP_new_connection_callback;
  matlab_tcp_setup.cbl_Receive_data       = TCP_receive_data_callback;
  matlab_tcp_setup.cbl_Connection_end     = TCP_connection_end_callback;

  status = Net_tcp_server_create(&matlab_tcp_server_ptr,&matlab_tcp_setup, "MATLAB srv");
  if (status == NX_SUCCESS)
  {
    APPLOG("MATLAB server creating result: %04X", status);
    matlab_server_created = 1;
    status = Net_tcp_server_start(matlab_tcp_server_ptr);
    APPLOG("MATLAB server start result: %04X", status);
    return  RES_OK;
  }
  else
  {
    APPLOG("MATLAB server creating result: %04X", status);
    return RES_ERROR;
  }
}


/*-----------------------------------------------------------------------------------------------------


  \param telnet_server_ptr

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t MATLAB_connection_server_delete(void)
{
  UINT status;

  if (matlab_server_created)
  {
    matlab_connected = 0;
    status = Net_tcp_server_delete(matlab_tcp_server_ptr);
    tx_mutex_delete(&matlab_mutex);

    APPLOG("MATLAB server deleting result: %04X", status);
    matlab_server_created = 0;
    if (status != NX_SUCCESS) return RES_ERROR;
    return RES_OK;
  }
  else
  {
    return RES_OK;
  }
}


/*-----------------------------------------------------------------------------------------------------


  \param buf
  \param len

  \return int
-----------------------------------------------------------------------------------------------------*/
int MATLAB_buf_send(const void *buf, unsigned int len)
{
  UINT                     res;
  NX_PACKET                *packet;

  res = nx_packet_allocate(&net_packet_pool,&packet, NX_TCP_PACKET, MS_TO_TICKS(10));
  if (res != NX_SUCCESS) return RES_ERROR;

  res = nx_packet_data_append(packet, (void *)buf, len,&net_packet_pool, MS_TO_TICKS(10));
  if (res != NX_SUCCESS)
  {
    nx_packet_release(packet);
    return RES_ERROR;
  }

  res = Net_tcp_server_packet_send(matlab_tcp_server_ptr,packet, MS_TO_TICKS(1000));
  if (res != NX_SUCCESS)
  {
    nx_packet_release(packet);
    return RES_ERROR;
  }

  return RES_OK;
}
/*-----------------------------------------------------------------------------------------------------


  \param telnet_server_ptr
-----------------------------------------------------------------------------------------------------*/
static void TCP_new_connection_callback(NET_TCP_SERVER *server_ptr)
{
  APPLOG("MATLAB connected");
  matlab_connected = 1;
}


/*-----------------------------------------------------------------------------------------------------
  Функция обработки принятого пакеты.
  Вызывается из контекста задачи примепа макетов сервера TCP


  \param telnet_server_ptr
  \param packet_ptr
-----------------------------------------------------------------------------------------------------*/
static void TCP_receive_data_callback(NET_TCP_SERVER *server_ptr, NX_PACKET *packet_ptr)
{
  uint16_t crc;
  uint16_t packet_crc;
  uint8_t  need_to_create_thread = 0;
  uint8_t *packet = packet_ptr->nx_packet_prepend_ptr;

  if (packet_ptr->nx_packet_length > (sizeof(matlab_cmd_id)+ sizeof(crc)))
  {
    memcpy(&matlab_cmd_id, packet, sizeof(matlab_cmd_id));

    switch (matlab_cmd_id)
    {
    case MTLB_CMD_FILES_WRITE_READ_DEL_TEST:
      if (packet_ptr->nx_packet_length == sizeof(T_matlab_cmd1))
      {
        memcpy(&packet_crc, packet + sizeof(T_matlab_cmd1) - 2, 2);
        crc = CRC16_matlab(packet, sizeof(T_matlab_cmd1)- 2);
        if (crc == packet_crc)
        {
          memcpy(&matlab_cmd1, packet, sizeof(T_matlab_cmd1));
          need_to_create_thread = 1;
        }
      }

      break;
    case MTLB_CMD_CONT_WRITE_TO_FILE_TEST:
      if (packet_ptr->nx_packet_length == sizeof(T_matlab_cmd2))
      {
        memcpy(&packet_crc, packet + sizeof(T_matlab_cmd2) - 2, 2);
        crc = CRC16_matlab(packet, sizeof(T_matlab_cmd2)- 2);
        if (crc == packet_crc)
        {
          memcpy(&matlab_cmd2, packet, sizeof(T_matlab_cmd2));
          need_to_create_thread = 1;
        }
      }
      break;
    case MTLB_CMD_PWR_SRC_CONTROL:
      if (packet_ptr->nx_packet_length == sizeof(T_matlab_cmd3))
      {
        memcpy(&packet_crc, packet + sizeof(T_matlab_cmd3) - 2, 2);
        crc = CRC16_matlab(packet, sizeof(T_matlab_cmd3)- 2);
        if (crc == packet_crc)
        {
          if (tx_mutex_get(&matlab_mutex, MS_TO_TICKS(2)) == TX_SUCCESS)
          {
            memcpy(&matlab_cmd3, packet, sizeof(T_matlab_cmd3));
            new_cmd3 = 1;
            tx_mutex_put(&matlab_mutex);
          }

          if (pwr_cbl.matlab_control == 0)
          {
            need_to_create_thread  = 1;
            pwr_cbl.matlab_control = 1;
          }
        }
      }
      break;
    }

  }

  if (need_to_create_thread)
  {
    if (Thread_MATLAB_terminate() == TX_SUCCESS)
    {
      if (Thread_MATLAB_delete() == TX_SUCCESS)
      {
        if (Thread_MATLAB_create() == RES_OK)
        {
          Thread_MATLAB_start();
        }
      }
    }
  }
  nx_packet_release(packet_ptr);
}

/*-----------------------------------------------------------------------------------------------------


  \param telnet_server_ptr
-----------------------------------------------------------------------------------------------------*/
static void TCP_connection_end_callback(NET_TCP_SERVER *server_ptr)
{
  matlab_connected = 0;
  if (Thread_MATLAB_terminate() == TX_SUCCESS) Thread_MATLAB_delete();
  APPLOG("MATLAB disconnected.");
}



/*-----------------------------------------------------------------------------------------------------
  Тест последовательной записи, чтения и стирания файлов

  \param void
-----------------------------------------------------------------------------------------------------*/
static uint32_t _Do_Files_write_read_delete_test(void)
{
  uint32_t           status;
  T_sys_timestump    t1, t2;
  char               *str;
  uint32_t           num = 0;
  uint32_t           open_t;
  uint32_t           write_t;
  uint32_t           close_t;
  uint32_t           delete_t;
  uint32_t           open_rd_t;
  uint32_t           read_t;
  uint8_t           *rd_file_buf;

  uint32_t file_buf_size = matlab_cmd1.sz;
  // Выделяем память для массивов

  if (matlab_cmd1.en_read_flag)
  {
    tmp_buf = App_malloc(2 * file_buf_size + MAX_TMP_STR_SZ);
  }
  else
  {
    tmp_buf = App_malloc(file_buf_size + MAX_TMP_STR_SZ);
  }
  if (tmp_buf == NULL)
  {
    APPLOG("Not enough memory for file buffer");
    return FX_NOT_ENOUGH_MEMORY;
  }

  str = (char *)&tmp_buf[file_buf_size];
  rd_file_buf =&tmp_buf[file_buf_size + MAX_TMP_STR_SZ];

  f = App_malloc(sizeof(FX_FILE));
  if (f == NULL)
  {
    App_free(tmp_buf);
    tmp_buf = 0;
    APPLOG("Not enough memory for file descriptor");
    return FX_NOT_ENOUGH_MEMORY;
  }


  // Заполняем буфер случайными числами
  num = 0;
  srand(num);
  uint8_t *bptr = (uint8_t *)tmp_buf;
  for (uint32_t i = 0; i < file_buf_size; i++)
  {
    *bptr = rand();
    bptr++;
  }


  tx_thread_priority_change(tx_thread_identify(), THREAD_HIGHER_PRIORITY,&old_priority);
  do
  {
    sprintf(str, "%s%d.BIN", FILE_NAME_PRFIX, num);

    // Открытие файла
    Get_hw_timestump(&t1);
    status = Recreate_file_for_write(f, (CHAR *)str);
    Get_hw_timestump(&t2);
    open_t = Hw_timestump_diff32_us(&t1,&t2);
    if (status != FX_SUCCESS)
    {
      APPLOG("Recreating file for write error %d.", status);
      goto err;
    }

    // Запись в файл
    Get_hw_timestump(&t1);
    status = fx_file_write(f, tmp_buf, file_buf_size);
    fx_media_flush(&fat_fs_media);
    Get_hw_timestump(&t2);
    write_t = Hw_timestump_diff32_us(&t1,&t2);
    if (status != FX_SUCCESS)
    {
      APPLOG("File write error %d.", status);
      goto err;
    }

    // Закрытие файла
    Get_hw_timestump(&t1);
    status = fx_file_close(f);
    Get_hw_timestump(&t2);
    close_t = Hw_timestump_diff32_us(&t1,&t2);
    if (status != FX_SUCCESS)
    {
      APPLOG("File close error %d.", status);
      goto err;
    }



    if (matlab_cmd1.en_read_flag)
    {
      ULONG actual_size;

      // Открытие файла на чтение
      Get_hw_timestump(&t1);
      status = fx_file_open(&fat_fs_media, f, (CHAR *)str,  FX_OPEN_FOR_READ);
      Get_hw_timestump(&t2);
      open_rd_t = Hw_timestump_diff32_us(&t1,&t2);
      if (status != FX_SUCCESS)
      {
        APPLOG("Open file for read error %d.", status);
        goto err;
      }

      memset(rd_file_buf,0,file_buf_size); // Очистка буфера
      // Чтение файла
      Get_hw_timestump(&t1);
      status = fx_file_read(f, rd_file_buf, file_buf_size,&actual_size);
      Get_hw_timestump(&t2);
      read_t = Hw_timestump_diff32_us(&t1,&t2);
      if (status != FX_SUCCESS)
      {
        APPLOG("File read error %d.", status);
        goto err;
      }

      // Сравнение с эталоном
      if (memcmp(rd_file_buf, tmp_buf, file_buf_size) != 0)
      {
        APPLOG("Readed file is different from the reference.");
        goto err;
      }

      // Закрытие файла
      status = fx_file_close(f);
      if (status != FX_SUCCESS)
      {
        APPLOG("File close error %d.", status);
        goto err;
      }
    }
    else
    {
      open_rd_t = 0;
      read_t    = 0;
    }


    if (matlab_cmd1.en_del_flag)
    {
      Get_hw_timestump(&t1);
      status = fx_file_delete(&fat_fs_media,(CHAR *)str);
      Get_hw_timestump(&t2);
      delete_t = Hw_timestump_diff32_us(&t1,&t2);
      if (status != FX_SUCCESS)
      {
        APPLOG("File delete error %d.", status);
        goto err;
      }
    }
    else
    {
      delete_t = 0;
    }

    // Передача результатов измерения в MATLAB
    uint32_t sz = snprintf(str, MAX_TMP_STR_SZ-1, "%d,%d,%d,%d,%d,%d\r\n",open_t,write_t,close_t,delete_t, open_rd_t, read_t);
    MATLAB_buf_send(str,sz);

    num++;
  }while (num < 0xFFFFFFFF);


  APPLOG("Test completed successfully");
  status = FX_SUCCESS;
err:

  App_free(tmp_buf);
  tmp_buf = 0;
  App_free(f);
  f = 0;

  tx_thread_priority_change(tx_thread_identify(), old_priority,&old_priority);

  return status;
}


/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t _Do_File_continuous_write_test(void)
{
  uint32_t           status;
  T_sys_timestump    t1, t2;
  char               *str;
  uint32_t           num = 0;
  uint32_t           write_t;

  uint32_t file_buf_size = matlab_cmd2.block_sz;

  tmp_buf = App_malloc(file_buf_size + MAX_TMP_STR_SZ);
  if (tmp_buf == NULL)
  {
    APPLOG("Not enough memory for file buffer");
    return FX_NOT_ENOUGH_MEMORY;
  }

  str = (char *)&tmp_buf[file_buf_size];

  f = App_malloc(sizeof(FX_FILE));
  if (f == NULL)
  {
    App_free(tmp_buf);
    tmp_buf = 0;
    APPLOG("Not enough memory for file descriptor");
    return FX_NOT_ENOUGH_MEMORY;
  }

  // Заполняем буфер случайными числами
  num = 0;
  srand(num);
  uint8_t *bptr = (uint8_t *)tmp_buf;
  for (uint32_t i = 0; i < file_buf_size; i++)
  {
    *bptr = rand();
    bptr++;
  }

  tx_thread_priority_change(tx_thread_identify(), THREAD_HIGHER_PRIORITY,&old_priority);

  sprintf(str, "Test.BIN");

  // Открытие файла
  status = Recreate_file_for_write(f, (CHAR *)str);
  if (status != FX_SUCCESS)
  {
    APPLOG("Recreating file for write error %d.", status);
    goto err;
  }


  do
  {

    // Запись в файл
    Get_hw_timestump(&t1);
    status = fx_file_write(f, tmp_buf, file_buf_size);
    Get_hw_timestump(&t2);
    write_t = Hw_timestump_diff32_us(&t1,&t2);
    if (status != FX_SUCCESS)
    {
      APPLOG("File write error %d.", status);
      goto err;
    }


    // Передача результатов измерения в MATLAB
    uint32_t sz = snprintf(str, MAX_TMP_STR_SZ-1, "%d\r\n",write_t);
    MATLAB_buf_send(str,sz);

    num++;
  }while (num < matlab_cmd2.block_num);


  // Закрытие файла
  fx_media_flush(&fat_fs_media);
  status = fx_file_close(f);
  if (status != FX_SUCCESS)
  {
    APPLOG("File close error %d.", status);
    goto err;
  }

  APPLOG("Test completed successfully");
  status = FX_SUCCESS;
err:

  App_free(tmp_buf);
  tmp_buf = 0;
  App_free(f);
  f = 0;

  tx_thread_priority_change(tx_thread_identify(), old_priority,&old_priority);

  return status;

}

/*-----------------------------------------------------------------------------------------------------
  Задача управления источником напряжения

  \param void
-----------------------------------------------------------------------------------------------------*/
void _Do_Power_source_control(void)
{
  char                    *str;
  T_measurement_results    res;
  T_sys_timestump          t1;
  T_sys_timestump          t2;
  uint8_t                  f_first;

  tmp_buf = App_malloc(MAX_TMP_STR_SZ);
  if (tmp_buf == NULL)
  {
    APPLOG("Not enough memory for string");
    return;
  }
  str = (char *)tmp_buf;

  matlab_time_step_min = 10000;
  matlab_time_step_max = 10000;
  f_first = 2;

  do
  {
    if (tx_mutex_get(&matlab_mutex, MS_TO_TICKS(2)) == TX_SUCCESS)
    {
      if (new_cmd3)
      {
        new_cmd3  = 0;
        // Выполняем команду

        DAC_proc(matlab_cmd3.dac_val);

        Set_PSW_R(matlab_cmd3.control_bits.PSW_R);
        Set_PSW_F(matlab_cmd3.control_bits.PSW_F);
        Set_ASW_R(matlab_cmd3.control_bits.ASW_R);
        Set_ASW_F(matlab_cmd3.control_bits.ASW_F);
        Set_DCDC_MODE(matlab_cmd3.control_bits.MODE);
        Set_EN_CHARGER(matlab_cmd3.control_bits.EN_CH);
        Set_LSW_F(matlab_cmd3.control_bits.LSW_F);

        if (matlab_cmd3.command_bits.Set_Accum_i_offset) Calibrate_acc_current_offset();
        if (matlab_cmd3.command_bits.Set_Load_i_offset) Calibrate_load_current_offset();
        if (matlab_cmd3.command_bits.Set_PSRC_i_offset) Calibrate_psrc_current_offset();
      }

      if (f_first)
      {
        Get_hw_timestump(&t1);
        f_first--;
      }
      else
      {
        Get_hw_timestump(&t2);
        matlab_time_step = (uint32_t)Hw_timestump_diff64_us(&t1,&t2);

        if (matlab_time_step < matlab_time_step_min) matlab_time_step_min = matlab_time_step;
        if (matlab_time_step > matlab_time_step_max) matlab_time_step_max = matlab_time_step;

        memcpy(&t1,&t2, sizeof(t1));
      }

      tx_mutex_put(&matlab_mutex);
    }

    if (Get_measurements_results(&res) == RES_OK)
    {
      // Передача результатов измерения в MATLAB
      uint32_t sz = snprintf(str, MAX_TMP_STR_SZ-1, "%0.5f,%0.5f,%0.5f,%0.5f,%0.5f,%0.5f,%0.5f,%0.5f,%e,%d\r\n",
                             res.psrc_v ,
                             res.psrc_i ,
                             res.sys_v  ,
                             res.acc_v  ,
                             res.acc_i  ,
                             res.load_v ,
                             res.load_i ,
                             res.temper ,
                             res.charge ,
                             pwr_cbl.fault
                            );
      MATLAB_buf_send(str,sz);
    }

    if (pwr_cbl.fault !=0)
    {
      break;
    }

    Wait_ms(9); // Периодичность работы задачи - 10 мс

  } while (1);

}

/*-----------------------------------------------------------------------------------------------------


  \param initial_input
-----------------------------------------------------------------------------------------------------*/
static void Thread_MATLAB(ULONG initial_input)
{
  APPLOG("MATLAB thread entered");

  // Ожидаем создание подсоединения
  while (matlab_connected == 0)
  {
    Wait_ms(10);
  }

  if (matlab_cmd_id != 0)
  {
    switch (matlab_cmd_id)
    {
    case MTLB_CMD_FILES_WRITE_READ_DEL_TEST:
      _Do_Files_write_read_delete_test();
      break;
    case MTLB_CMD_CONT_WRITE_TO_FILE_TEST:
      _Do_File_continuous_write_test();
      break;
    case MTLB_CMD_PWR_SRC_CONTROL:
      _Do_Power_source_control();
      break;
    }
    matlab_cmd_id = 0;
  }

  if (tmp_buf != 0)
  {
    App_free(tmp_buf);
    tmp_buf = 0;
  }
  if (f != 0)
  {
    App_free(f);
    f = 0;
  }

  APPLOG("MATLAB thread exited");
}

