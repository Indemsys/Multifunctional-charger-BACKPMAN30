// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2022-10-23
// 12:50:46
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"
#include   "nxd_ftp_client.h"

#define  CSV_FILE_NAME_PRFIX          "File_"

TX_THREAD                     ftp_client_thread @ "DTCM";

static void Thread_FTP_client(ULONG initial_input);
#pragma data_alignment=8
uint8_t                       thread_ftp_client_stack[THREAD_FTP_CLIENT_STACK_SIZE] @ "DTCM";

#define                       DATA_PACKET_MAX_SZ   1400
static char                   file_name[64];
static char                   data_buf[DATA_PACKET_MAX_SZ];

static FX_FILE               ftp_f;
NX_IP                        *ftp_client_ip_ptr;
static NX_FTP_CLIENT         ftp_client;

uint32_t                     ftp_client_last_error;

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Thread_FTP_client_create(void)
{
  if (wvar.en_ftp_client == 0) return;
  tx_thread_create(&ftp_client_thread, "FTP Client", Thread_FTP_client,
                   0,
                   (void *)thread_ftp_client_stack, // stack_start
                   THREAD_FTP_CLIENT_STACK_SIZE,    // stack_size
                   THREAD_FTP_CLIENT_PRIORITY,      // priority. Numerical priority of thread. Legal values range from 0 through (TX_MAX_PRIORITES-1), where a value of 0 represents the highest priority.
                   THREAD_FTP_CLIENT_PRIORITY,      // preempt_threshold. Highest priority level (0 through (TX_MAX_PRIORITIES-1)) of disabled preemption. Only priorities higher than this level are allowed to preempt this thread. This value must be less than or equal to the specified priority. A value equal to the thread priority disables preemption-threshold.
                   TX_NO_TIME_SLICE,
                   TX_AUTO_START);
}



/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static uint32_t  Send_file_to_FTP(void)
{
  uint32_t    status;
  ULONG       actual_size;
  ULONG       ftp_server_ip = 0;
  NX_PACKET  *packet_ptr;


  if (Str_to_IP_v4((char const *)wvar.ftp_client_server_adress,(uint8_t *)&ftp_server_ip) != RES_OK)
  {
    ftp_client_last_error = 1;
    return RES_ERROR;
  }

  status  = nx_ftp_client_create(&ftp_client, "FTP Client", ftp_client_ip_ptr, 2048,&net_packet_pool);
  if (status != NX_SUCCESS)
  {
    ftp_client_last_error = 2;
    return RES_ERROR;
  }

  // Ждем соединения 10 секунд
  status =  nx_ftp_client_connect(&ftp_client, ftp_server_ip, (CHAR *)wvar.ftp_client_server_login, (CHAR *)wvar.ftp_client_server_pass, ms_to_ticks(100));
  if (status != NX_SUCCESS)
  {
    ftp_client_last_error = 3;
    goto err0;
  }

  nx_ftp_client_file_delete(&ftp_client, file_name, ms_to_ticks(1000));
  status = nx_ftp_client_file_open(&ftp_client, file_name, NX_FTP_OPEN_FOR_WRITE, ms_to_ticks(1000));
  if (status != NX_SUCCESS)
  {
    ftp_client_last_error = 4;
    goto err0;
  }

  status  = fx_file_open(&fat_fs_media,&ftp_f, file_name,  FX_OPEN_FOR_READ);
  if (status != NX_SUCCESS)
  {
    ftp_client_last_error = 5;
    goto err1;
  }

  do
  {
    status = nx_packet_allocate(&net_packet_pool,&packet_ptr, NX_TCP_PACKET, ms_to_ticks(100));
    if (status != NX_SUCCESS)
    {
      ftp_client_last_error = 6;
      goto err2;
    }


    status = fx_file_read(&ftp_f, data_buf , DATA_PACKET_MAX_SZ,&actual_size);
    if (status != NX_SUCCESS)
    {
      ftp_client_last_error = 7;
      nx_packet_release(packet_ptr);
      goto err2;
    }

    memcpy(packet_ptr->nx_packet_prepend_ptr,data_buf ,  actual_size);
    packet_ptr->nx_packet_length = actual_size;
    packet_ptr->nx_packet_append_ptr = packet_ptr->nx_packet_prepend_ptr + actual_size;

    status = nx_ftp_client_file_write(&ftp_client, packet_ptr, ms_to_ticks(100)); // Функция освобождает пакет даже если передача не состоялась
    if (status != NX_SUCCESS)
    {
      ftp_client_last_error = 8;
      goto err2;
    }

  } while (actual_size == DATA_PACKET_MAX_SZ);

  fx_file_close(&ftp_f);
  nx_ftp_client_file_close(&ftp_client, ms_to_ticks(1000));
  nx_ftp_client_disconnect(&ftp_client, ms_to_ticks(1000));
  nx_ftp_client_delete(&ftp_client);

  return RES_OK;


err2:
  fx_file_close(&ftp_f);

err1:
  nx_ftp_client_file_close(&ftp_client, ms_to_ticks(1000));
  nx_ftp_client_disconnect(&ftp_client, ms_to_ticks(1000));

err0:
  nx_ftp_client_delete(&ftp_client);
  return RES_ERROR;
}


/*-----------------------------------------------------------------------------------------------------


  \param initial_input
-----------------------------------------------------------------------------------------------------*/
static void Thread_FTP_client(ULONG initial_input)
{
  uint32_t status;

  // Проверяем статус файловой системы на SD карте
  status  = Get_FS_init_res()->fx_media_open_result;
  if (status != FX_SUCCESS)
  {
    ftp_client_last_error = 10;
    return;
  }

  // Начинаем цикл записи данных в файл
  do
  {
    // Создаем файл на запись
    uint32_t  t;
    Get_system_ticks(&t);
    t =t / TX_TIMER_TICKS_PER_SECOND;
    sprintf(file_name, "%s%d.csv", CSV_FILE_NAME_PRFIX, t);

    status = Recreate_file_for_write(&ftp_f, file_name);
    if (status != FX_SUCCESS)
    {
      ftp_client_last_error = 11;
      return;
    }

    // Начинаем запись данных в файл
    do
    {
      ULONG actual_flags;
      status =  tx_event_flags_get(&adc_flag, LOG_ADC_RES, TX_OR_CLEAR, &actual_flags,  MS_TO_TICKS(10));
      if (status == TX_SUCCESS)
      {
        T_measurement_results    res;

        if (Get_measurements_results(&res) == RES_OK)
        {
          Get_system_ticks(&t);
          uint32_t sz = snprintf(data_buf, DATA_PACKET_MAX_SZ-1, "%08d, %0.5f,%0.5f,%0.5f,%0.5f,%0.5f,%0.5f,%0.5f,%0.5f,%e,%d\r\n",
                                 t,
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
          if (sz > 0) status = fx_file_write(&ftp_f, data_buf, sz);
          if (status != FX_SUCCESS)
          {
            ftp_client_last_error = 12;
            fx_file_close(&ftp_f);
            return;
          }
          // Если размер файла больше установленного, то передать файл на FTP сервер
          if (ftp_f.fx_file_current_file_size > wvar.max_scv_file_sz)
          {
            ftp_client_ip_ptr = 0;
            if (Is_RNDIS_network_active())
            {
              ftp_client_ip_ptr = rndis_ip_ptr;
            }
            else if (Is_ECM_Host_network_active())
            {
              ftp_client_ip_ptr = ecm_host_ip_ptr;
            }

            if (ftp_client_ip_ptr != 0)
            {
              // Закрываем файл перед передачей на FTP
              fx_media_flush(&fat_fs_media);
              status = fx_file_close(&ftp_f);
              if (status != FX_SUCCESS)
              {
                ftp_client_last_error = 13;
                return;
              }
              do
              {
                if (Send_file_to_FTP()==RES_OK) break;
                if ((ftp_client_last_error != 4) && (ftp_client_last_error != 3)) return;
                Wait_ms(1000);
              } while (1);
              fx_file_delete(&fat_fs_media, file_name);
              break;
            }
          }

        } // if Get_measurements_results Ok
      } // if tx_event_flags_get Ok

    }while (1);



  }while (1);



}

