// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2019.08.22
// 11:49:41
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"
#include   "Net_Telnet.h"

extern NX_TELNET_SERVER           rndis_telnet_server;

#define   RNDIS_TELNET_SERVER_STACK_SIZE  2048
#pragma data_alignment=8
uint8_t   rndis_telnet_server_stack_memory[RNDIS_TELNET_SERVER_STACK_SIZE];


static int Mn_rndis_telnet_drv_init(void **pcbl, void *pdrv);
static int Mn_rndis_telnet_drv_send_buf(const void *buf, unsigned int len);
static int Mn_rndis_telnet_drv_wait_ch(unsigned char *b, int timeout);
static int Mn_rndis_telnet_drv_printf(const char  *fmt_ptr, ...);
static int Mn_rndis_telnet_drv_deinit(void **pcbl);


T_serial_io_driver mon_rndis_telnet_drv_driver =
{
  MN_DRIVER_MARK,
  MN_RNDIS_TELNET_DRIVER,
  Mn_rndis_telnet_drv_init,
  Mn_rndis_telnet_drv_send_buf,
  Mn_rndis_telnet_drv_wait_ch,
  Mn_rndis_telnet_drv_printf,
  Mn_rndis_telnet_drv_deinit,
  0,
};


int32_t rndis_telnet_vt100_alloc_indx = -1;

#define   RNDIS_TELNET_LOGICAL_CONNECTION  0


#define   MB_TELNET_READ_REQUEST BIT(0)
#define   MB_TELNET_READ_DONE    BIT(1)

#define   TELNET_BUFFER_MAX_LENGTH 512
#define   TELNET_DRV_STR_SZ 512
#define   IN_BUF_QUANTITY   2           // Количество приемных буферов

// Ведем прием циклически в N приемных буферов
typedef struct
{
    uint32_t  len;  // Длина пакета
    uint8_t   buff[NX_TELNET_SERVER_PACKET_PAYLOAD]; // Буфер с пакетов

} T_telnet_rx_pack_cbl;


typedef struct
{
    char                   str[TELNET_DRV_STR_SZ];
    TX_EVENT_FLAGS_GROUP   evt;         // Группа флагов для взаимодействия с задачей приема
    char                   evt_grp_name[32];
    void                  *dbuf;        // Указатель на буфер с принимаемыми данными
    uint32_t               dsz;         // Количество принимаемых байт
    volatile uint8_t       head_n;     //  Индекс головы циклической очереди буферов приема
    volatile uint8_t       tail_n;     //  Индекс хвоста циклической очереди буферов приема
    volatile uint32_t      no_space;   //  Флаг заполненнной очереди приемных буфферов
    uint32_t               rd_pos;     //  Позиция чтения в текущем буфере
    T_telnet_rx_pack_cbl   rd_pack[IN_BUF_QUANTITY]; //  Массив управляющих структур приема-обработки входящих пакетов

} T_telnet_drv_cbl;

T_telnet_drv_cbl rndis_telnet_cbl;


static uint32_t  telnet_connected;

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
uint8_t Is_telnet_connected(void)
{
  return telnet_connected;
}
/*-----------------------------------------------------------------------------------------------------


  pcbl - указатель на указатель на структуру со специальными данными необходимыми драйверу
  pdrv - указатель на структуру T_monitor_driver

  \return int
-----------------------------------------------------------------------------------------------------*/
static int Mn_rndis_telnet_drv_init(void **pcbl, void *pdrv)
{
  T_telnet_drv_cbl *p;

  // Если драйвер еще не был инициализирован, то выделить память для управлющей структуры и ждать сигнала из интерфеса
  if (*pcbl == 0)
  {
    p           =&rndis_telnet_cbl;  //  Устанавливаем в управляющей структуре драйвера задачи указатель на управляющую структуру драйвера
    p->head_n   = 0;
    p->tail_n   = 0;
    p->no_space = 0;
    p->rd_pos   = 0;
    sprintf(p->evt_grp_name,   "RTLNT_RX_EVG");
    if (tx_event_flags_create(&(p->evt),p->evt_grp_name) != TX_SUCCESS)
    {
      return RES_ERROR;
    }
    *pcbl = p;
  }
  return RES_OK;
}


/*-----------------------------------------------------------------------------------------------------


  \param pcbl

  \return int
-----------------------------------------------------------------------------------------------------*/
static int Mn_rndis_telnet_drv_deinit(void **pcbl)
{
  T_telnet_drv_cbl *p;
  if (*pcbl != 0)
  {
    p =*pcbl;
    tx_event_flags_delete(&(p->evt));
    *pcbl = 0;
  }
  return RES_OK;
}


/*-----------------------------------------------------------------------------------------------------


  \param buf
  \param len

  \return int
-----------------------------------------------------------------------------------------------------*/
static int Mn_rndis_telnet_drv_send_buf(const void *buf, unsigned int len)
{
  UINT                     res;
  NX_PACKET                *packet;
  T_serial_io_driver *mdrv = (T_serial_io_driver *)(tx_thread_identify()->driver);

  res = nx_packet_allocate(&net_packet_pool,&packet, NX_TCP_PACKET, MS_TO_TICKS(10));
  if (res != NX_SUCCESS) return RES_ERROR;

  res = nx_packet_data_append(packet, (void *)buf, len,&net_packet_pool, MS_TO_TICKS(10));
  if (res != NX_SUCCESS)
  {
    nx_packet_release(packet);
    return RES_ERROR;
  }

  res = nx_telnet_server_packet_send(&rndis_telnet_server,RNDIS_TELNET_LOGICAL_CONNECTION,packet, MS_TO_TICKS(10));
  if (res != NX_SUCCESS)
  {
    nx_packet_release(packet);
    return RES_ERROR;
  }

  return RES_OK;
}


/*-----------------------------------------------------------------------------------------------------


  \param fmt_ptr

  \return int
-----------------------------------------------------------------------------------------------------*/
static int Mn_rndis_telnet_drv_printf(const char  *fmt_ptr, ...)
{
  UINT              res;
  NX_PACKET        *packet;
  T_serial_io_driver *mdrv = (T_serial_io_driver *)(tx_thread_identify()->driver);
  T_telnet_drv_cbl *p      = (T_telnet_drv_cbl *)(mdrv->pdrvcbl);
  char             *s = p->str;
  uint32_t         len;
  va_list           ap;


  va_start(ap, fmt_ptr);
  len = vsnprintf(s, TELNET_DRV_STR_SZ, (const char *)fmt_ptr, ap);
  va_end(ap);

  if (len > 0)
  {
    res = nx_packet_allocate(&net_packet_pool,&packet, NX_TCP_PACKET, MS_TO_TICKS(10));
    if (res != NX_SUCCESS) return RES_ERROR;

    res = nx_packet_data_append(packet, s, len,&net_packet_pool, MS_TO_TICKS(10));
    if (res != NX_SUCCESS)
    {
      nx_packet_release(packet);
      return RES_ERROR;
    }

    res = nx_telnet_server_packet_send(&rndis_telnet_server, RNDIS_TELNET_LOGICAL_CONNECTION, packet, MS_TO_TICKS(10));
    if (res != NX_SUCCESS)
    {
      nx_packet_release(packet);
      return RES_ERROR;
    }
  }


  return RES_OK;

}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void _Read_rndis_telnet_data(NX_PACKET *packet_ptr)
{
  UINT              res;
  ULONG             actual_length;
  ULONG             actual_flags;
  T_telnet_drv_cbl  p = rndis_telnet_cbl;
  uint32_t          n;

  n = rndis_telnet_cbl.head_n;

  res = nx_packet_data_retrieve(packet_ptr, rndis_telnet_cbl.rd_pack[n].buff,&actual_length);
  if (res == NX_SUCCESS)
  {
    rndis_telnet_cbl.rd_pack[n].len = actual_length;

    n++;
    if (n >= IN_BUF_QUANTITY) n = 0;
    rndis_telnet_cbl.head_n = n;

    // Выставляем флаг выполненного чтения
    if (tx_event_flags_set(&(rndis_telnet_cbl.evt), MB_TELNET_READ_DONE, TX_OR) == TX_SUCCESS)
    {
      // Если все буферы на прием заполнены, то значит системе не требуются данные
      if (rndis_telnet_cbl.tail_n == n)
      {
        // Перестаем принимать данные из USB и ждем 10 мс когда система обработает уже принятые данные и подаст сигнал к началу приема по USB
        // Если система в течении 10 мс не взяла данные, то данные перезаписываются новыми
        rndis_telnet_cbl.no_space = 1;
        tx_event_flags_get(&(rndis_telnet_cbl.evt), MB_TELNET_READ_REQUEST, TX_AND_CLEAR,&actual_flags, MS_TO_TICKS(10));
      }
    }
  }
}


/*-----------------------------------------------------------------------------------------------------


  \param b
  \param timeout

  \return int
-----------------------------------------------------------------------------------------------------*/
static int Mn_rndis_telnet_drv_wait_ch(unsigned char *b, int timeout)
{
  ULONG        actual_flags;
  uint32_t     n;
  uint8_t      h;

  T_serial_io_driver *mdrv = (T_serial_io_driver *)(tx_thread_identify()->driver);
  T_telnet_drv_cbl *p      = (T_telnet_drv_cbl *)(mdrv->pdrvcbl);


  // Если индексы буферов равны то это значит отсутствие принятых пакетов
  h = p->head_n;
  if (p->tail_n == h)
  {
    if (tx_event_flags_get(&(p->evt), MB_TELNET_READ_DONE, TX_AND_CLEAR,&actual_flags, MS_TO_TICKS(timeout)) != TX_SUCCESS)
    {
      return RES_ERROR;
    }
    // Еще раз проверяем наличие данных поскольку флаг мог остаться от предыдущего чтения, когда данные били приняты без проверки флага и соответственно без его сброса
    h = p->head_n;
    if (p->tail_n == h)
    {
      return RES_ERROR;
    }
  }

  n = p->tail_n;                      // Получаем индекс хвостового буфера
  *b = p->rd_pack[n].buff[p->rd_pos]; // Читаем байт данных из хвостового буфера
  p->rd_pos++;                        // Смещаем указатель на следующий байт данных

  // Если позиция достигла конца данных в текущем буфере, то буфер освобождается для приема
  if (p->rd_pos >= p->rd_pack[n].len)
  {
    p->rd_pos = 0;
    // Смещаем указатель хвоста очереди приемных буфферов
    // Появляется место для движения головы очереди приемных буфферов

    n++;
    if (n >= IN_BUF_QUANTITY) n = 0;
    p->tail_n = n;

    // Если очередь пакетов была заполнена, то сообщить задаче о продолжении приема
    if (p->no_space == 1)
    {
      p->no_space = 0;
      if (tx_event_flags_set(&(p->evt), MB_TELNET_READ_REQUEST, TX_OR) != TX_SUCCESS)
      {
        time_delay(timeout); // Задержка после ошибки нужна для того чтобы задача не захватила все ресурсы в случает постоянного появления ошибки
        return RES_ERROR;
      }
    }
  }
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param telnet_server_ptr
  \param logical_connection
-----------------------------------------------------------------------------------------------------*/
void Telnet_client_connect(NX_TELNET_SERVER *telnet_server_ptr, UINT logical_connection)
{
  uint32_t err = RES_ERROR;

  // Создать задачу VT100 для данного соединения и передать ей драйвер
  if ((logical_connection == RNDIS_TELNET_LOGICAL_CONNECTION) && (telnet_server_ptr == &rndis_telnet_server) && (rndis_telnet_vt100_alloc_indx < 0))
  {
    err = Task_VT100_create(&mon_rndis_telnet_drv_driver,&rndis_telnet_vt100_alloc_indx); // В контексте этого вызова будут выполнены функции драйвера init и deinit
    if (err == RES_OK)
    {
      APPLOG("RNDIS Telnet connected");
      telnet_connected = 1;
    }
  }
  else
  {
    APPLOG("Logical connection %d to Telnet rejected", logical_connection);
  }

  if (err != RES_OK)
  {
    nx_telnet_server_disconnect(telnet_server_ptr, logical_connection);
    return;
  }

}

/*-----------------------------------------------------------------------------------------------------


  \param telnet_server_ptr
  \param logical_connection
  \param packet_ptr
-----------------------------------------------------------------------------------------------------*/
void Telnet_receive_data(NX_TELNET_SERVER *telnet_server_ptr, UINT logical_connection, NX_PACKET *packet)
{
  if ((telnet_server_ptr == &rndis_telnet_server) && (logical_connection == RNDIS_TELNET_LOGICAL_CONNECTION))
  {
    _Read_rndis_telnet_data(packet);
  }
  // В конце всегда нужно пакет освободить или переиспользовать
  nx_packet_release(packet);
}



/*-----------------------------------------------------------------------------------------------------


  \param telnet_server_ptr
  \param logical_connection
-----------------------------------------------------------------------------------------------------*/
void Telnet_client_disconnect(NX_TELNET_SERVER *telnet_server_ptr, UINT logical_connection)
{
  if ((telnet_server_ptr == &rndis_telnet_server) && (logical_connection == RNDIS_TELNET_LOGICAL_CONNECTION))
  {
    telnet_connected = 0;
    Task_VT100_delete(&rndis_telnet_vt100_alloc_indx);
    APPLOG("RNDIS Telnet disconnected");
  }
}


/*-----------------------------------------------------------------------------------------------------


  \param telnet_server_ptr

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t TELNET_server_create(NX_TELNET_SERVER  *telnet_server_ptr)
{
  UINT    err;
  char    intf_name[16];

  //if (wvar.en_telnet == 0) return RES_ERROR;
  if (telnet_server_ptr->nx_telnet_server_id !=  0)  return RES_OK;

  strcpy(intf_name,"RNDIS telnet");
  err = nx_telnet_server_create(telnet_server_ptr,
       intf_name,
       &rndis_ip,
       &rndis_telnet_server_stack_memory[0],
       RNDIS_TELNET_SERVER_STACK_SIZE,
       Telnet_client_connect,
       Telnet_receive_data,
       Telnet_client_disconnect);


  if (NX_SUCCESS != err)
  {
    APPLOG("%s. Creation error %d",intf_name, err);
    return RES_ERROR;
  }
  else
  {
    APPLOG("%s. Telnet server created", intf_name);
  }


  err = nx_telnet_server_packet_pool_set(telnet_server_ptr,&net_packet_pool);
  if (NX_SUCCESS != err)
  {
    nx_telnet_server_delete(telnet_server_ptr);
    APPLOG("%s. Packet pool setting error %d",intf_name, err);
    return RES_ERROR;
  }

  err = nx_telnet_server_start(telnet_server_ptr);
  if (err != NX_SUCCESS)
  {
    nx_telnet_server_delete(telnet_server_ptr);
    APPLOG("%s. Starting error %d",intf_name, err);
    return RES_ERROR;
  }
  else
  {
    APPLOG("%s. Started",intf_name);
    return RES_OK;
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param telnet_server_ptr

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t TELNET_server_delete(NX_TELNET_SERVER  *telnet_server_ptr)
{
  UINT res;

  if (telnet_server_ptr->nx_telnet_server_id !=  0)
  {
    telnet_connected = 0;
    res = nx_telnet_server_delete(&rndis_telnet_server);
    APPLOG("RNDIS. Telnet delete result: %04X", res);
    return res;
  }
  else
  {
    return RES_OK;
  }
}

