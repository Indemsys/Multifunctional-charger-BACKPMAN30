// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2019.10.28
// 17:32:29
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"

extern NX_IP         *mqtt_ip_ptr;

#define MAX_MQTT_PAYLOAD_LEN  1024
static  char          g_mqtt_topic_buffer[NXD_MQTT_MAX_TOPIC_NAME_LENGTH];
static  char          g_mqtt_message_buffer[NXD_MQTT_MAX_MESSAGE_LENGTH];

// Структура автомата состояний приемника потока пакетов
typedef struct
{
    uint8_t  pack_header;       // Заголовочный байт пакета
    uint32_t pack_num;          // Номер ожидаемого пакета
    uint16_t pack_payload_sz;   // Количество байт полезныз данных текущего пакета
    uint32_t data_cnt;          // Счетчик количества принятых байт
    uint32_t data_sz;           // Количество байт которое необходимо принять
    uint32_t out_data_sz;       // Количество байт данных после декомпрессии
    uint32_t packet_pos;        // Индекс в буфкрк пакета с которого начинаются полезные данные
    uint8_t  unpack_from_mem;   // Флаг сжатого потока, который следует распаковывать прямо из памяти
    uint8_t  unpack_from_file;  // Флаг сжатого потока, который следует сохранять в файл и распаковывать из файла
    uint8_t  stream_to_file;    // Флаг потока сохраняемого в файл
    uint8_t  file_transfer;     // Флаг указывающий что поток предназначен для записи файла на карту
    uint8_t  *data_buf;         // Указатель на буфер приема данныых
    uint8_t  *data_ptr;         // Указатель на текущую позицию в буфере приема данных
    FX_FILE  data_file;         // Файл для сохранения данных
    uint8_t  err;               // Ошибка во время приема данных
    uint8_t  last_err;          // Последняя ошибка
    uint32_t err_cnt;           // Счетчик ошибок
    uint8_t  f_done;            // Флаг успешного завершения приема
    uint8_t *out_data_buf;      // Буфер с распакованными данными
} T_decompress_stream_sm;

static T_decompress_stream_sm  msm;

static uint32_t  cmd_opcode;


static uint32_t  reques_to_send_records_list;
static uint32_t  last_state_req_t;
static uint32_t  last_msg_time;
static uint32_t  mqtt_msg_counter;



/*-----------------------------------------------------------------------------------------------------
  Подписываемся на топики:
    - топик составленный из номера CPU  Пример: 5301766735393735407843535454227D
    - топик MQTT_BROADCAST_TOPIC        Пример: brodcast

-----------------------------------------------------------------------------------------------------*/
static void _MQTTMC_subscribe_default_topics(void)
{
  UINT       res;
  uint32_t   err_cnt;

  err_cnt = 0;

  res = nxd_mqtt_client_subscribe(&mqtt_client, cpu_id_str, strlen(cpu_id_str), MQTT_QOS1);
  if (res != NXD_MQTT_SUCCESS)
  {
    APPLOG("Failed to subscribe topic %s. Error 0x%05X", cpu_id_str, res);
    err_cnt++;
  }

  res = nxd_mqtt_client_subscribe(&mqtt_client, MQTT_SUBSCR_TOPIC_BROADCAST, strlen(MQTT_SUBSCR_TOPIC_BROADCAST), MQTT_QOS1);
  if (res != NXD_MQTT_SUCCESS)
  {
    APPLOG("Failed to subscribe topic '%s'. Error 0x%05X", MQTT_SUBSCR_TOPIC_BROADCAST, res);
    err_cnt++;
  }
  if (err_cnt == 0)
  {
    APPLOG("All subscription to MQTT broker done sucessfully");
  }
  else
  {
    Send_flag_to_app(APP_DO_SYSTEM_RESTART);
  }
}

/*-----------------------------------------------------------------------------------------------------
  Отписываемся и снова подписываемся чтобы остановить поток пакетов неудачно обработанного стрима
  !!!  Тестирование показало что поток от этого не останавливается. !!!

  \param void
-----------------------------------------------------------------------------------------------------*/
static uint32_t _MQTTMC_reset_subscribe(void)
{
  UINT       res;
  res = nxd_mqtt_client_unsubscribe(&mqtt_client, cpu_id_str, strlen(cpu_id_str));
  if (res == NXD_MQTT_SUCCESS)
  {
    res = nxd_mqtt_client_subscribe(&mqtt_client, cpu_id_str, strlen(cpu_id_str), MQTT_QOS1);
  }
  return res;
}

/*-----------------------------------------------------------------------------------------------------
  Переслать данные из файла переданного в виде хэндлера в топик MQTT брокеру

  Структура посылаемых пакетов

  Первый пакет:
  Смещение     Данные
  0            [X]              - 1 байт. Заголовок пакета. Содержит информационные флаги
                                      Флаги в заголовке пакетов в потоке
                                        FIRST_PACKET_FLAG           = BIT(0)   Флаг обозначающий первый пакет в потоке
                                        LAST_PACKET_FLAG            = BIT(1)   Флаг обозначающий последний пакет в потоке
                                        FILE_STREAM_FLAG            = BIT(4)   Флаг обозначающий поток предназначенный для записи в файл
                                        LONG_COMPRESSED_STREAM_FLAG = BIT(5)   Флаг обозначающий длинный сжатый поток, который надо распаковывать в файл
                                        COMPRESSED_STREAM_FLAG      = BIT(6)   Флаг обозначающий сжатый поток
                                        STREAM_PACKET_FLAG          = BIT(7)   Обязательный флаг в заголовке пакета

  1            [0..15]          - 16 байт уникального идентификатора устройства
  17           [N][N]           - 2 байта содержащие количество байт в пакете с данными
  19           [L][L][L][L]     - 4 байта содержащие номер пакета
  23           [P][P][P][P]     - 4 байта содержащие полный размер данных которые должны быть переданы

                                 Следующие байты не формируются в данной функции, а уже находятся в передаваемом блоке
                                 Компрессор упаковывает данные блоками по размеру не превышающими величину MAX_COMPRESSIBLE_BLOCK_SIZE
  27           [O][O][O][O]     - 4 байта содержащие размер первого сжатого блока
  31           [K][K][K][K]     - 4 байта содержащие размер распакованного первого блока
               либо
  27           [F]              - 1 байта длины имени файла включая завершающий ноль
  28           [0..F-1]         - байты имени файла

  Последующие пакеты:
  Смещение     Данные
  0            [X]              - 1 байт. Заголовок пакета. Содержит информационные флаги
                                      Флаги в заголовке пакетов в потоке
                                      STREAM_PACKET_FLAG      0x80  - Обязательный флаг в заголовке каждого пакета потока
                                      COMPRESSED_STREAM_FLAG  0x40  - Флаг обозначающий поток со сжатыми данными
                                      FILE_STREAM_FLAG        0x20  - Флаг обозначабщий поток предназначенный для записи в файл
                                      FIRST_PACKET_FLAG       0x01  - Флаг обозначающий первый пакет в потоке
                                      LAST_PACKET_FLAG        0x02  - Флаг обозначающий последний пакет в потоке
  1            [0..15]          - 16 байт уникального идентификатора устройства
  17           [N][N]           - 2 байта содержащие количество байт в пакете с данными
  19           [L][L][L][L]     - 4 байта содержащие номер пакета


  \param file_handler   - управляющая структура файла
  \param compress_flag  - флаг указывающий сжатый или не сжатый файл передаем

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t _MQTTMC_send_file_by_handler(FX_FILE *file_handler, uint8_t compress_flag)
{
  uint32_t sz = file_handler->fx_file_current_file_size;
  uint32_t blsz;
  uint32_t bl_num = 0;
  uint32_t header_len;
  uint32_t res;
  uint32_t err = 0;
  ULONG  actual_size;

  do
  {
    blsz = sz;
    if (blsz > MAX_MQTT_PAYLOAD_LEN) blsz = MAX_MQTT_PAYLOAD_LEN;

    // Передаем заголовок пакета
    g_mqtt_message_buffer[0] = compress_flag | STREAM_PACKET_FLAG; // Флаги потока сжатых данных

    memcpy(&g_mqtt_message_buffer[1],&cpu_id,16);            // Записываем уникальный идентификатор устройства

    g_mqtt_message_buffer[17] =  blsz & 0xFF;                // Размер блока данных
    g_mqtt_message_buffer[18] =(blsz >> 8) & 0xFF;

    g_mqtt_message_buffer[19] = bl_num & 0xFF;               // Номер блока данных
    g_mqtt_message_buffer[20] =(bl_num >> 8) & 0xFF;
    g_mqtt_message_buffer[21] =(bl_num >> 16) & 0xFF;
    g_mqtt_message_buffer[22] =(bl_num >> 24) & 0xFF;

    // В первом пакете передаем также размер сжатых данных, это ускорит процесс распаковки на приемной строне
    if (bl_num == 0)
    {
      uint32_t compr_sz = file_handler->fx_file_current_file_size;
      g_mqtt_message_buffer[23] = compr_sz & 0xFF;           // Размер сжатых данных
      g_mqtt_message_buffer[24] =(compr_sz >> 8) & 0xFF;
      g_mqtt_message_buffer[25] =(compr_sz >> 16) & 0xFF;
      g_mqtt_message_buffer[26] =(compr_sz >> 24) & 0xFF;

      header_len = 27;
      g_mqtt_message_buffer[0] |= FIRST_PACKET_FLAG;
    }
    else
    {
      header_len = 23;
    }

    res = fx_file_read(file_handler,&g_mqtt_message_buffer[header_len],blsz,&actual_size);
    if (res != FX_SUCCESS)
    {
      APPLOG("Error %d", res);
      err = 1;
      break;
    }
    bl_num++;
    sz = sz  - blsz;
    if (sz == 0)
    {
      g_mqtt_message_buffer[0] |= LAST_PACKET_FLAG; // Обозначаем этим битом последний блок
    }

    res = nxd_mqtt_client_publish(&mqtt_client, g_mqtt_topic_buffer, strlen(g_mqtt_topic_buffer), g_mqtt_message_buffer, blsz + header_len, MQTT_NO_RETAIN, MQTT_QOS1, ms_to_ticks(2000));
    if (res != NXD_MQTT_SUCCESS)
    {
      APPLOG("Error 0x%05X", res);
      err = 1;
      break;
    }

  } while (sz > 0);


  if (err == 0) return RES_OK;
  return RES_ERROR;
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void _MQTTMC_reset_stream(void)
{
  // Если перед получением пакета уже проводился прием стрима, то освободить его
  if (msm.data_file.fx_file_id == FX_FILE_ID)
  {
    fx_file_close(&msm.data_file);
    fx_media_flush(&fat_fs_media);
    fx_file_delete(&fat_fs_media,msm.data_file.fx_file_name);
    memset(&msm.data_file, 0, sizeof(FX_FILE));
    if (msm.file_transfer)
    {
      msm.last_err = MQTT_STREAM_ERROR_BROKEN_STREAM;
      msm.err_cnt++;
      if (_MQTTMC_reset_subscribe() != NXD_MQTT_SUCCESS) Send_flag_to_app(APP_DO_SYSTEM_RESTART);
      MQTTMC_Send_File_transfer_Ack(msm.last_err); // Сообщаем об ошибке передачи файла
    }
  }
  if (msm.data_buf != NULL)
  {
    App_free(msm.data_buf);
    msm.data_buf = NULL;
  }

  msm.pack_num          = 0;
  msm.data_cnt          = 0;
  msm.unpack_from_mem   = 0;
  msm.unpack_from_file  = 0;
  msm.stream_to_file    = 0;
  msm.file_transfer     = 0;
  msm.err               = 0;  // Приход первого пакета сбрасывает предыдущие ошибки приема
}


/*-----------------------------------------------------------------------------------------------------
  Обработка потока бинарных пакетов обозначаемых  флагом STREAM_PACKET_FLAG

  Входные данные находятся в буфере mqtt_message_buffer

  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t _MQTTMC_binary_stream_receiving(void)
{
  uint32_t    pnum;
  int32_t     res;

  msm.f_done = 0;
  msm.pack_header = g_mqtt_message_buffer[0];

  if (msm.pack_header & BRAKE_STREAM_FLAG)
  {
    _MQTTMC_reset_stream();
    return RES_OK;
  }

  memcpy(&msm.pack_payload_sz,&g_mqtt_message_buffer[17], 2);  // Получаем длину пакета
  memcpy(&pnum,&g_mqtt_message_buffer[19], 4);                 // Получаем номер пакета

  if (msm.pack_header & FIRST_PACKET_FLAG)
  {
    // Обработка первого пакета нового стрима
    _MQTTMC_reset_stream();

    memcpy(&msm.data_sz,&g_mqtt_message_buffer[23], 4);

    if (msm.pack_header & LONG_COMPRESSED_STREAM_FLAG)
    {
      msm.unpack_from_file = 1;   // Поблочно сжатый длинный поток сначала сохраняется в файл, а потом распаковывается из него.
      msm.out_data_sz      = 0;   // Для поблочно сжатого длиного потока мы не знаем окончательный размер несжатых данных
      msm.stream_to_file   = 1;
      res =  Recreate_file_for_write(&msm.data_file, COMPRESSED_STREAM_FILE_NAME);  // Создаем временный файл из которого будет проводится распаковка
      if (res != FX_SUCCESS)
      {
        msm.err  = MQTT_STREAM_ERROR_FILE_CREATION;
        goto EXIT_ON_ERROR; // Ошибка. Не удалось создать файл
      }
      msm.packet_pos = 27;
    }
    else if (msm.pack_header & SHORT_COMPRESSED_STREAM_FLAG)
    {
      msm.unpack_from_mem  = 1;
      memcpy(&msm.out_data_sz,&g_mqtt_message_buffer[31], 4); // Для сжатого потока не превышающего MAX_COMPRESSIBLE_BLOCK_SIZE байт здесь будет размер несжатых данных
      // Выделяем память для буфера приема входящих пакетов
      msm.data_buf = App_malloc_pending(msm.data_sz, 10);
      if (msm.data_buf == NULL)
      {
        msm.err  = MQTT_STREAM_ERROR_ALLOC_ERROR;
        goto EXIT_ON_ERROR; // Ошибка. Не удалось выделить память
      }
      msm.data_ptr = msm.data_buf;
      msm.packet_pos = 27;
    }
    else if (msm.pack_header & FILE_STREAM_FLAG)
    {
      uint8_t file_name_len;
      char    *file_name =&g_mqtt_message_buffer[28]; // Извлекаем имя файла

      msm.stream_to_file = 1;
      msm.file_transfer  = 1;

      res =  Recreate_file_for_write(&msm.data_file,file_name);
      if (res != FX_SUCCESS)
      {
        msm.err  = MQTT_STREAM_ERROR_FILE_CREATION;
        goto EXIT_ON_ERROR; // Ошибка. Не удалось создать файл
      }
      file_name_len =*(uint8_t *)&g_mqtt_message_buffer[27];
      msm.packet_pos = 28 + file_name_len;
    }
    else
    {
      msm.out_data_sz = msm.data_sz;
    }
  }
  else if (msm.err != 0)
  {
    // Если уже была зафиксирована ошибка в приеме, то последующие пакеты игнорируем до приема пакета начала нового стрима
    goto EXIT_ON_ERROR;
  }
  else
  {
    msm.packet_pos = 23;
  }

  // Дополнительные проверки
  if (msm.data_sz == 0)
  {
    msm.err  = MQTT_STREAM_ERROR_ZERO_SZ;
    goto EXIT_ON_ERROR; // Ошибка. Размер данных не может равняться 0
  }
  if (msm.pack_num != pnum)
  {
    msm.err  = MQTT_STREAM_ERROR_SEQUENCE;
    goto EXIT_ON_ERROR; // Ошибка. Не соответствует номер пакета
  }
  if ((msm.pack_payload_sz == 0) || (msm.pack_payload_sz > MAX_MQTT_PAYLOAD_LEN))
  {
    msm.err  = MQTT_STREAM_ERROR_PAYLOAD_SZ;
    goto EXIT_ON_ERROR; // Ошибка. Неправильная длина пакета
  }

  // Читаем данные из пакета в место назначения
  if (msm.stream_to_file == 1)
  {
    // Запись в файл
    res = fx_file_write(&msm.data_file,&g_mqtt_message_buffer[msm.packet_pos], msm.pack_payload_sz);
    if (res != FX_SUCCESS)
    {
      msm.err  = MQTT_STREAM_ERROR_FS_WRITE_ERR;
      goto EXIT_ON_ERROR; // Ошибка. Не удалось записать в файл
    }
  }
  else
  {
    // Запись в память
    memcpy(msm.data_ptr,&g_mqtt_message_buffer[msm.packet_pos], msm.pack_payload_sz);
    msm.data_ptr += msm.pack_payload_sz;
  }

  if (msm.pack_header & LAST_PACKET_FLAG)
  {
    if (msm.stream_to_file == 1)
    {
      res = fx_file_close(&msm.data_file);
      fx_media_flush(&fat_fs_media);
      if (msm.file_transfer)
      {
        MQTTMC_Send_File_transfer_Ack(res); // Передаем сообщение о завершения приема файла
        APPLOG("MQTT file %s receiving result: %d", msm.data_file.fx_file_name,  res);
      }
      memset(&msm.data_file, 0, sizeof(FX_FILE));
    }
    msm.f_done = 1;
  }


  msm.pack_num++; // Увеличиваем номер ожидаемого пакета
  return RES_OK;

EXIT_ON_ERROR:

  if (msm.data_buf != NULL) App_free(msm.data_buf);
  msm.data_buf = NULL;
  msm.data_ptr = 0;

  if (msm.file_transfer)
  {
    if (_MQTTMC_reset_subscribe() != NXD_MQTT_SUCCESS) Send_flag_to_app(APP_DO_SYSTEM_RESTART);
    MQTTMC_Send_File_transfer_Ack(msm.err); // Сообщаем об ошибке передачи файла
  }

  if (msm.data_file.fx_file_id == FX_FILE_ID)
  {
    fx_file_close(&msm.data_file);
    fx_media_flush(&fat_fs_media);
    memset(&msm.data_file, 0, sizeof(FX_FILE));
  }

  msm.last_err = msm.err;
  msm.err_cnt++;
  return RES_ERROR;
}

/*-----------------------------------------------------------------------------------------------------
  Распаковка если надо и размещение принятых данных из стрима в память для последующей десериализации


  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t _MQTTMC_decompress_stream(void)
{
  // Если пакет слишком большой для распаковки, то игнорируем его
  if (msm.out_data_sz > (MAX_COMPRESSIBLE_BLOCK_SIZE-1))
  {
    APPLOG("Error. To long MQTT packet. size=%d, max=%d", msm.out_data_sz, MAX_COMPRESSIBLE_BLOCK_SIZE-1);
    goto EXIT_ON_ERROR;
  }

  if (msm.unpack_from_file)
  {
    // Распаковываем файл в файл
    if (Decompress_file_to_file(MQTTMC_COMPRESSION_ALG, COMPRESSED_STREAM_FILE_NAME,UNCOMPESSED_STREAM_FILE_NAME) != RES_OK)
    {
      APPLOG("Error. MQTT large file don't decompressed.");
      goto EXIT_ON_ERROR;
    }
  }
  else if (msm.unpack_from_mem)
  {
    // Выделяем буфер для распаковки данных в память c учетом дополнительного нуля для окончания строки
    msm.out_data_buf = App_malloc_pending(msm.out_data_sz+1, 10);
    if (msm.out_data_buf == NULL)
    {
      APPLOG("Error. No memory for MQTT packet. size=%d", msm.out_data_sz);
      goto EXIT_ON_ERROR;
    }

    //RTT_printf(0,"Allocated out buf. Size %d  Addr:%08X\r\n", msm.out_data_sz+1, (uint32_t)msm.out_data_buf);
    if (msm.stream_to_file)
    {
      // Распаковываем файл в память
      if (Decompress_file_to_mem(MQTTMC_COMPRESSION_ALG,COMPRESSED_STREAM_FILE_NAME, msm.out_data_buf,msm.out_data_sz) != RES_OK)
      {
        APPLOG("Error. MQTT file don't decompressed.");
        goto EXIT_ON_ERROR;
      }
    }
    else
    {
      // Распаковываем буфер из памяти в память
      if (Decompress_mqtt_mem_to_mem(MQTTMC_COMPRESSION_ALG, msm.data_buf, msm.data_sz, msm.out_data_buf,msm.out_data_sz) != msm.out_data_sz)
      {
        APPLOG("Error. MQTT packet don't decompressed.");
        goto EXIT_ON_ERROR;
      }
    }
    msm.out_data_buf[msm.out_data_sz] = 0; // Установливаем символ конца строки JSON
    App_free(msm.data_buf);
    //RTT_printf(0,"Deallocated in buf %08X\r\n", (uint32_t)msm.data_buf);
    msm.data_buf = NULL;
    msm.data_ptr = NULL;
  }
  else
  {
    App_free(msm.out_data_buf);
    //RTT_printf(0,"Deallocated out buf %08X\r\n", (uint32_t)msm.out_data_buf);
    // Если данные не сжаты, то просто копируем указатели
    msm.out_data_buf = msm.data_buf;
    msm.out_data_sz = msm.data_sz;
    msm.data_buf = NULL;
    msm.data_ptr = NULL;
  }

  return RES_OK;

EXIT_ON_ERROR:

  msm.last_err = 100;
  msm.err_cnt++;
  //RTT_printf(0,"Error %d. Total errors %d\r\n", msm.last_err, msm.err_cnt);

  App_free(msm.out_data_buf);
  //RTT_printf(0,"Deallocated out buf %08X\r\n", (uint32_t)msm.out_data_buf);
  msm.out_data_buf = NULL;


  App_free(msm.data_buf);
  //RTT_printf(0,"Deallocated in buf %08X\r\n", (uint32_t)msm.data_buf);

  msm.data_buf = NULL;
  msm.data_ptr = NULL;
  return RES_ERROR;
}

/*-----------------------------------------------------------------------------------------------------
  Функция получает имя файла,
  проверяет размер файла и если он больше размера пакета то сжисает файл и пересылает сжатый файл по заданному топику
  Если файл короткий то он пересылается без зжатия.

  \param filename
  \param topic

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t _MQTTMC_Send_file_to_topic(char *input_filename, char *topic, uint8_t compression_flag)
{
  uint32_t res;
  uint8_t  err = 1;
  FX_FILE  input_file;
  FX_FILE  compressed_file;
  uint32_t fsize;
  char     *compressed_file_name = 0;

  input_file.fx_file_id = 0;
  compressed_file.fx_file_id = 0;

  APPLOG("Sending to topic %s", topic);


  // Открываем файл и узнаем его размер
  res = fx_file_open(&fat_fs_media,&input_file, input_filename,  FX_OPEN_FOR_READ);
  if (res != FX_SUCCESS) return RES_ERROR;

  strncpy(g_mqtt_topic_buffer, topic, NXD_MQTT_MAX_TOPIC_NAME_LENGTH-1);

  fsize = input_file.fx_file_current_file_size;
  if (fsize > MAX_MQTT_PAYLOAD_LEN)
  {
    if (compression_flag == ENABLE_COMPRESSION)
    {
      do
      {
        // Если размер файла превышает размер пакета MQTT, то передавать его по протоколу со сжатием

        // Создаем выходной файл в который будут записанно сжатое содержимое входного файла
        err = 1;
        compressed_file_name = App_malloc_pending(MQTT_MAX_FILE_NAME_LEN+1, 10);
        if (compressed_file_name == NULL)
        {
          APPLOG("Insufficient memory");
          break;
        }

        uint32_t name_len = strlen(input_filename);
        if (name_len > (MQTT_MAX_FILE_NAME_LEN - strlen(MQTT_PACK_FILE_EXT)))
        {
          APPLOG("Fale name len too big");
          break;
        }
        strcpy(compressed_file_name,input_filename);
        strcat(compressed_file_name,MQTT_PACK_FILE_EXT);

        res =  Recreate_file_for_write(&compressed_file, compressed_file_name);
        if (res != TX_SUCCESS)
        {
          APPLOG("Error 0x%04X", res);
          break;
        }

        if (Compress_file_to_file_by_handler(MQTTMC_COMPRESSION_ALG,&input_file,&compressed_file) != RES_OK) break;

        if (_MQTTMC_send_file_by_handler(&compressed_file, LONG_COMPRESSED_STREAM_FLAG) != RES_OK) break;

        err = 0;
      } while (0);
    }
    else if (compression_flag == DISABLE_COMPRESSION)
    {
      err = 1;
      if (_MQTTMC_send_file_by_handler(&input_file, UNCOMPRESSED_STREAM) == RES_OK) err=0;
    }
  }
  else
  {
    ULONG   actual_size;
    do
    {
      // Если размер файла не превышает размер пакета MQTT, то передавать его как есть
      res = fx_file_read(&input_file,g_mqtt_message_buffer, fsize,&actual_size);
      if (res != FX_SUCCESS)
      {
        APPLOG("Error 0x%04X", res);
        break;
      }
      res = nxd_mqtt_client_publish(&mqtt_client, g_mqtt_topic_buffer, strlen(g_mqtt_topic_buffer), g_mqtt_message_buffer, fsize, MQTT_NO_RETAIN, MQTT_QOS1, ms_to_ticks(2000));
      if (res != NX_SUCCESS)
      {
        APPLOG("Error 0x%05X", res);
        break;
      }
      err = 0;
    } while (0);
  }

  if (compressed_file.fx_file_id == FX_FILE_ID)
  {
    fx_file_close(&compressed_file);
    fx_file_delete(&fat_fs_media,compressed_file_name);
  }

  App_free(compressed_file_name);
  if (input_file.fx_file_id == FX_FILE_ID) fx_file_close(&input_file);

  if (err == 0) return RES_OK;
  return RES_ERROR;
}

/*-----------------------------------------------------------------------------------------------------
  Функция отсылки сжатого файла в заданный топик
  Используется для пересылки сжатого списка файлов

  \param input_filename
  \param topic

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t _MQTTMC_Send_compressed_file_to_topic(char *input_filename, char *topic)
{
  uint32_t res;
  FX_FILE  input_file;

  APPLOG("Sending to topic %s", topic);

  // Открываем файл и узнаем его размер
  res = fx_file_open(&fat_fs_media,&input_file, input_filename,  FX_OPEN_FOR_READ);
  if (res != FX_SUCCESS) return RES_ERROR;

  strncpy(g_mqtt_topic_buffer, topic, NXD_MQTT_MAX_TOPIC_NAME_LENGTH-1);

  res = _MQTTMC_send_file_by_handler(&input_file, LONG_COMPRESSED_STREAM_FLAG);
  fx_file_close(&input_file);

  return res;
}


/*-----------------------------------------------------------------------------------------------------
  Посылка подтверждения выпонения команды

  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t MQTTMC_Send_Ack(uint32_t ack_code)
{
  uint32_t  res = RES_ERROR;
  char     *json_str = 0;
  uint32_t  json_str_sz;

  if (Serialze_Ack_message(&json_str,&json_str_sz, ack_code, "Ack_code") == RES_OK)
  {
    res = nxd_mqtt_client_publish(&mqtt_client, MQTT_PUBLISH_TOPIC_ACK, strlen(MQTT_PUBLISH_TOPIC_ACK), json_str, json_str_sz, MQTT_NO_RETAIN, MQTT_QOS1, ms_to_ticks(2000));
    if (res != NX_SUCCESS)
    {
      APPLOG("Error 0x%05X", res);
    }
    else
    {
      res = RES_OK;
    }
  }
  if (json_str != 0) App_free(json_str);
  return res;
}

/*-----------------------------------------------------------------------------------------------------


  \param ack_code

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t MQTTMC_Send_File_transfer_Ack(uint32_t ack_code)
{
  uint32_t  res = RES_ERROR;
  char     *json_str = 0;
  uint32_t  json_str_sz;

  if (Serialze_Ack_message(&json_str,&json_str_sz, ack_code, "File_transfer_result") == RES_OK)
  {
    res = nxd_mqtt_client_publish(&mqtt_client, MQTT_PUBLISH_TOPIC_ACK, strlen(MQTT_PUBLISH_TOPIC_ACK), json_str, json_str_sz, MQTT_NO_RETAIN, MQTT_QOS1, ms_to_ticks(2000));
    if (res != NX_SUCCESS)
    {
      APPLOG("Error 0x%05X", res);
    }
    else
    {
      res = RES_OK;
    }
  }
  if (json_str != 0) App_free(json_str);
  return res;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t MQTTMC_Send_Device_State(void)
{
  uint32_t  res = RES_ERROR;
  char     *json_str = NULL;
  uint32_t  json_str_sz;

  res = Serialze_device_state_to_mem(&json_str,&json_str_sz);
  if (res == RES_OK)
  {
    res = nxd_mqtt_client_publish(&mqtt_client, MQTT_PUBLISH_TOPIC_STATE, strlen(MQTT_PUBLISH_TOPIC_STATE), json_str, json_str_sz, MQTT_NO_RETAIN, MQTT_QOS0, ms_to_ticks(2000));
    if (res != NX_SUCCESS)
    {
      APPLOG("Error 0x%05X", res);
      res = RES_ERROR;
    }
    else
    {
      res = RES_OK;
    }
  }
  else
  {
    APPLOG("Error %d", res);
    res = RES_ERROR;
  }
  App_free(json_str);
  return res;
}

/*-----------------------------------------------------------------------------------------------------
  Выслать схему параметров и их значения

  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t MQTTMC_Send_params_schema(void)
{
  uint32_t res = RES_ERROR;
  res = Serialze_settings_schema_to_JSON_file(PARAMS_SCHEMA_FILE_NAME, JSON_ENSURE_ASCII | JSON_COMPACT);
  if (res == RES_OK)
  {
    res = _MQTTMC_Send_file_to_topic(PARAMS_SCHEMA_FILE_NAME, MQTT_PUBLISH_TOPIC_SCHEMA, ENABLE_COMPRESSION);
    if (res != RES_OK)
    {
      APPLOG("Error 0x%05X", res);
    }
    fx_file_delete(&fat_fs_media,PARAMS_SCHEMA_FILE_NAME);
  }
  else
  {
    APPLOG("Error 0x%05X", res);
  }
  return res;
}

/*-----------------------------------------------------------------------------------------------------



  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t MQTTMC_Send_params_values(void)
{
  uint32_t res = RES_ERROR;
  res = Serialze_settings_to_JSON_file(PARAMS_VALUES_FILE_NAME, JSON_ENSURE_ASCII | JSON_COMPACT);
  if (res == RES_OK)
  {
    res = _MQTTMC_Send_file_to_topic(PARAMS_VALUES_FILE_NAME, MQTT_PUBLISH_TOPIC_VALUES, ENABLE_COMPRESSION);
    if (res != RES_OK)
    {
      APPLOG("Error 0x%05X", res);
    }
    fx_file_delete(&fat_fs_media,PARAMS_VALUES_FILE_NAME);
  }
  else
  {
    APPLOG("Error 0x%05X", res);
  }
  return res;
}


/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
void MQTT_client_controller(void)
{

  if (Is_RNDIS_network_active()) // RNDIS соединение имеет приоритет
  {
    Net_mqtt_client_create(rndis_ip_ptr);
  }
  else
  {
    if ((mqtt_ip_ptr == rndis_ip_ptr) && (rndis_ip_ptr != 0))
    {
      Net_MQTT_client_delete();
    }
  }


  if (Is_ECM_Host_network_active())
  {
    Net_mqtt_client_create(ecm_host_ip_ptr);
  }
  else
  {
    if ((mqtt_ip_ptr == ecm_host_ip_ptr) && (ecm_host_ip_ptr != 0))
    {
      Net_MQTT_client_delete();
    }
  }



  // Если клиент инициализирован, но не подключен, то предпринимать попытки подключения
  if (Is_mqtt_client_created() == 1)
  {
    // Каждые 2 сек повторять попытки присоединения к брокеру если еще не присоеденены
    if ((Is_mqtt_client_connected() == 0) && ((tx_time_get()- Get_last_mqtt_connection_time()) > ms_to_ticks(2000)))
    {
      // Делаем попытку присоедениться
      if (Net_mqtt_client_connect() == RES_OK)
      {
        // Регистрируем процедуру получения сообщений от брокера при удачном присоединении
        // nxd_mqtt_client_receive_notify_set(&mqtt_client, _MQTTMC_notify_func);
        _MQTTMC_subscribe_default_topics();

        // После подключения сразу посылаем схему параметров и их значения
        MQTTMC_Send_params_schema();
      }
    }
  }

}


/*-----------------------------------------------------------------------------------------------------
  Установка переменной cmd_opcode вызовет последующее исполнение команды

  \param opcode
-----------------------------------------------------------------------------------------------------*/
void MQTTMC_set_opcode(uint32_t opcode)
{
  cmd_opcode = opcode;
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void MQTTMC_Set_request_send_records_list(void)
{
  reques_to_send_records_list = 1;
}

/*-----------------------------------------------------------------------------------------------------
  Вызывается из Task_Net->MQTTMC_process_messages->_MQTTMC_execute_command

  Команда выполняется если установлено ненулевое и корректное значение переменной cmd_opcode
  После выполнения переменная cmd_opcode обнуляется

  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t _MQTTMC_execute_command(void)
{
  uint32_t res = RES_OK;

  switch (cmd_opcode)
  {
  case OPCODE_RESET:
    APPLOG("MQTT command: RESET");
    MQTTMC_Send_Ack(RES_OK);
    Wait_ms(200);
    Reset_system();
    break;
  case OPCODE_RESET_TO_DEFAULT:
    APPLOG("MQTT command: RESET_TO_DEFAULT");
    MQTTMC_Send_Ack(RES_OK);
    Wait_ms(200);
    Return_def_params();
    Reset_system();
    break;
  case OPCODE_REQUEST_SCHEMA:
    APPLOG("MQTT command: REQUEST_SCHEMA");
    res = MQTTMC_Send_params_schema();
    MQTTMC_Send_Ack(res);
    break;
  case OPCODE_REQUEST_PARAMETERS:
    APPLOG("MQTT command: REQUEST_PARAMETERS");
    res = MQTTMC_Send_params_values();
    MQTTMC_Send_Ack(res);
    break;

  case OPCODE_GET_LOG_FILE:
    _MQTTMC_Send_file_to_topic(LOG_FILE_PATH,MQTT_PUBLISH_TOPIC_LOG,DISABLE_COMPRESSION);
    MQTTMC_Send_Ack(RES_OK);
    break;
  case OPCODE_RESET_LOG_FILE:
    Req_to_reset_log_file();
    MQTTMC_Send_Ack(RES_OK);
    break;

  case OPCODE_REQUEST_STATE_INFO:
    // Состояние посылаем не чаще одного раза в секунду чтобы не зафлудить канал
    {
      uint32_t t;
      t = tx_time_get();
      if ((t - last_state_req_t) > SEC_TO_TICKS(1))
      {
        res = MQTTMC_Send_Device_State();
        last_state_req_t = t;
      }
    }
    break;
  default:
    break;
  }

  if (reques_to_send_records_list)
  {
    reques_to_send_records_list = 0;
    res = _MQTTMC_Send_compressed_file_to_topic(PACKED_FILES_LIST_FILE_NAME, MQTT_PUBLISH_TOPIC_FILES);
  }

  cmd_opcode= 0;
  return res;
}

/*-----------------------------------------------------------------------------------------------------
  Возвращаем 1 если сессия управления по MQTT считается активной.
  Сессия считается активной если последнее сообщение пришло не позже 1 мин назад.

  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Is_mqtt_session_active(void)
{
  uint32_t  curr_time;

  if (mqtt_msg_counter == 0) return 0;

  Get_system_ticks(&curr_time);
  if (Time_diff_seconds(last_msg_time, curr_time) < 1 * 60)
  {
    return 1;
  }
  return 0;
}

/*-----------------------------------------------------------------------------------------------------
  Отработка сообщения от MQTT брокера
  Вызывается из задачи Task_Net->MQTTMC_process_messages

  \param void
-----------------------------------------------------------------------------------------------------*/
void MQTTMC_messages_processor(void)
{
  uint32_t    message_length = 0;
  uint32_t    topic_length;
  UINT        res;

  do
  {
    // Получаем сообщение
    res = nxd_mqtt_client_message_get(&mqtt_client, (UCHAR *)g_mqtt_topic_buffer, NXD_MQTT_MAX_TOPIC_NAME_LENGTH,&topic_length, (UCHAR *)g_mqtt_message_buffer, NXD_MQTT_MAX_MESSAGE_LENGTH,&message_length);
    if (res == NXD_MQTT_SUCCESS)
    {
      //RTT_printf(0,"Packet %02d %03d\r\n",topic_length,message_length);
      g_mqtt_topic_buffer[topic_length] = 0;
      g_mqtt_message_buffer[message_length] = 0;
      if (strcmp(g_mqtt_topic_buffer, cpu_id_str) == 0)
      {
        if (message_length > 0)
        {
          // JSON пакеты от бинарных пакетов отделяем по признаку бита STREAM_PACKET_FLAG, который в JSON посылках не может присутствовать
          if (g_mqtt_message_buffer[0] & STREAM_PACKET_FLAG)
          {
            // Обработка потока бинарных пакетов
            if (_MQTTMC_binary_stream_receiving() == RES_OK)
            {
              if ((msm.f_done == 1) && ((msm.unpack_from_file == 1) || (msm.unpack_from_mem == 1)))
              {
                msm.f_done = 0;
                if (_MQTTMC_decompress_stream() == RES_OK)
                {
                  JSON_Deser_and_Exec_command((char *)msm.out_data_buf, msm.unpack_from_file);
                  App_free(msm.out_data_buf);
                  msm.out_data_buf = NULL;
                }
              }
            }
          }
          else
          {
            JSON_Deser_and_Exec_command(g_mqtt_message_buffer, 0);
          }
        }

      }
      else if (strcmp(g_mqtt_topic_buffer, MQTT_SUBSCR_TOPIC_BROADCAST) == 0)
      {

      }

      _MQTTMC_execute_command();

      // Фиксируем время прихода сообщения
      mqtt_msg_counter++;
      Get_system_ticks(&last_msg_time);
    }
    else
    {
      //if (res != NXD_MQTT_NO_MESSAGE) RTT_printf(0,"Packet error %d\r\n",res);
      break;
    }

  } while (1);


}


