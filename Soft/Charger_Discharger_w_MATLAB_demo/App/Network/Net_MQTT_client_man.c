// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2019.08.30
// 11:21:30
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"

extern const NX_SECURE_TLS_CRYPTO   nx_crypto_tls_ciphers;
extern const NX_SECURE_TLS_CRYPTO   nx_crypto_tls_ciphers_synergys7;

#ifdef USE_HARDWARE_CRIPTO_ENGINE
  #define CRIPTO_TLS_CYPHERS  nx_crypto_tls_ciphers_synergys7
#else
  #define CRIPTO_TLS_CYPHERS  nx_crypto_tls_ciphers
#endif


#define    CLIENT_ID_MAX_LEN         12
#define    MQTT_CLIENT_STACK_SIZE    2048


#define    MQTT_KEEP_ALIVE_TIMER     300  // Define the MQTT keep alive timer for 5 minutes

extern const USHORT nx_crypto_ecc_supported_groups_synergys7[];
extern const NX_CRYPTO_METHOD *nx_crypto_ecc_curves_synergys7[];

extern T_app_net_props        app_net_props;

#pragma data_alignment=8
uint8_t                       mqtt_client_stack[MQTT_CLIENT_STACK_SIZE]  @ "DTCM";

static char                   mqtt_client_id[CLIENT_ID_MAX_LEN];

NXD_MQTT_CLIENT               mqtt_client;
NX_IP                        *mqtt_ip_ptr;

static uint8_t                mqtt_client_created;
static uint8_t                mqtt_client_connected;
static TX_EVENT_FLAGS_GROUP   mqtt_app_flag;

static uint32_t               mqtt_conn_time; // Время последней попытки соедиения с MQTT брокером в тикам RTOS
static uint32_t               mqtt_connection_attempt_cnt;

static void _mqtt_client_id_callback(char *client_id, uint32_t *client_id_length);


UCHAR                              *mqtt_tls_crypto_metadata;
UCHAR                              *mqtt_tls_certificate_buffer;
UCHAR                              *mqt_tls_session_packet_buffer;

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Is_mqtt_client_created(void)
{
  return mqtt_client_created;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Is_mqtt_client_connected(void)
{
  return mqtt_client_connected;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Get_last_mqtt_connection_time(void)
{
  return mqtt_conn_time;
}

/*-----------------------------------------------------------------------------------------------------


  \param mqtt_client_ptr
-----------------------------------------------------------------------------------------------------*/
static void _mqtt_disconnect_callback(NXD_MQTT_CLIENT *mqtt_client_ptr)
{
  APPLOG("MQTT disconnected");
  mqtt_client_connected = 0;
}

/*-----------------------------------------------------------------------------------------------------
  Функция вызывается в контексте задачи "MQTT client" ,приоритет MQTT_TASK_PRIO, функция _nxd_mqtt_thread_entry
  Вызов происходит до того как будет выслано MQTT подтверждение приема пакета для пакетов с QoS 1 и 2
  Здесь нельзя использовать сервисы с задержкой поскольку эта функция вызывается с захваченым мьютексом модуля MQTT
  Этим же мьютексом защищается вызов nxd_mqtt_client_message_get

  \param client_ptr
  \param number_of_messages
-----------------------------------------------------------------------------------------------------*/
static void _mqtt_receive_calback(NXD_MQTT_CLIENT *client_ptr, UINT number_of_messages)
{
  Send_event_to_Net_task(EVT_MQTT_MSG);
}


/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
uint32_t Net_mqtt_client_create(NX_IP  *ip)
{
  UINT     res;
  uint32_t mqtt_client_id_len = 0;

  if (wvar.mqtt_enable == 0) return RES_ERROR;
  if (mqtt_ip_ptr != 0) return RES_ERROR;

  res = tx_event_flags_create(&mqtt_app_flag, "mqtt");
  if (res != TX_SUCCESS)
  {
    APPLOG("Failed to create evenr. Error %d", res);
    return RES_ERROR;
  }

  _mqtt_client_id_callback(mqtt_client_id,&mqtt_client_id_len);  // Declaration of user callback function. This function MUST be defined in the user application.

  // Создаем задачу приемник пакетов MQTT
  res = nxd_mqtt_client_create(&mqtt_client, "MQTT Client", mqtt_client_id,  mqtt_client_id_len,
                               ip,
                               &net_packet_pool,
                               (VOID *)mqtt_client_stack, MQTT_CLIENT_STACK_SIZE,
                               MQTT_TASK_PRIO,
                               0, 0);

  if (res != NXD_MQTT_SUCCESS)
  {
    APPLOG("MQTT client creation error %d", res);
    return RES_ERROR;
  }

  res = nxd_mqtt_client_receive_notify_set(&mqtt_client, _mqtt_receive_calback);
  if (res != NXD_MQTT_SUCCESS)
  {
    nxd_mqtt_client_delete(&mqtt_client);
    APPLOG("MQTT client receive callback setting error %d", res);
    return RES_ERROR;
  }


  res= nxd_mqtt_client_disconnect_notify_set(&mqtt_client,_mqtt_disconnect_callback);
  if (res != NXD_MQTT_SUCCESS)
  {
    nxd_mqtt_client_delete(&mqtt_client);
    APPLOG("MQTT client disconnect callback setting error %d", res);
    return RES_ERROR;
  }

  mqtt_ip_ptr = ip;
  APPLOG("MQTT client created successfully");
  mqtt_client_created = 1;
  return RES_OK;
}



/*-----------------------------------------------------------------------------------------------------
  Функция возвращающая уникальный ClientID

  \param client_id
  \param client_id_length
-----------------------------------------------------------------------------------------------------*/
static void _mqtt_client_id_callback(char *client_id, uint32_t *client_id_length)
{
  uint32_t len, vlen;
  len = CLIENT_ID_MAX_LEN-1;
  vlen = strlen((char const *)wvar.mqtt_client_id);
  if (len > vlen) len = vlen;

  strncpy(client_id,(char const *) wvar.mqtt_client_id, len);
  *client_id_length = len;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Net_mqtt_set_message_flag(void)
{
  return  tx_event_flags_set(&mqtt_app_flag, MQTT_MESSAGE_EVENT, TX_OR);
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Net_mqtt_wait_message_flag(ULONG wait_option)
{
  ULONG actual_flags;
  return tx_event_flags_get(&mqtt_app_flag, MQTT_MESSAGE_EVENT, TX_OR_CLEAR,&actual_flags,  wait_option);
}

/*-----------------------------------------------------------------------------------------------------



  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Net_mqtt_client_connect(void)
{
  uint32_t       res;
  ULONG          actual_status;
  NXD_ADDRESS    ip_addr;
  ULONG          addr;

  mqtt_connection_attempt_cnt++;
  mqtt_conn_time = tx_time_get();

  if (Is_mqtt_client_created() == 0)  return RES_ERROR;


  res = nx_ip_status_check(mqtt_ip_ptr, NX_IP_LINK_ENABLED,&actual_status, 100);
  if (res != NX_SUCCESS)
  {
    if (mqtt_connection_attempt_cnt < 5)
    {
      APPLOG("Failed to start MQTT client session. Error %d", res);
    }
    return RES_ERROR;
  }

  ip_addr.nxd_ip_version = NX_IP_VERSION_V4;
  res = Str_to_IP_v4((char const *)wvar.mqtt_server_ip,(uint8_t *)&ip_addr.nxd_ip_address.v4);
  if (res != RES_OK)
  {
    res =  DNS_get_host_address((UCHAR *)wvar.mqtt_server_ip,&addr, 400);
    if (res == NX_SUCCESS)
    {
      memcpy(&ip_addr.nxd_ip_address.v4,&addr , 4);
      res = RES_OK;
    }
    else
    {
      res = RES_ERROR;
    }
  }

  if (res != RES_OK)
  {
    if (mqtt_connection_attempt_cnt < 5)
    {
      APPLOG("Wrong MQTT server IP string %s", wvar.mqtt_server_ip);
    }
    return RES_ERROR;
  }

  res = nxd_mqtt_client_login_set(&mqtt_client, (CHAR *)wvar.mqtt_user_name, strlen((char const *)wvar.mqtt_user_name), (CHAR *)wvar.mqtt_password, strlen((char const *)wvar.mqtt_password));
  if (res != NXD_MQTT_SUCCESS)
  {
    if (mqtt_connection_attempt_cnt < 5)
    {
      APPLOG("Failed to set  MQTT user and password. Error %d", res);
    }
    return RES_ERROR;
  }

  UINT clean_session =  NX_FALSE;

  res  = nxd_mqtt_client_connect(&mqtt_client,&ip_addr, wvar.mqtt_server_port,  MQTT_KEEP_ALIVE_TIMER, clean_session, NX_WAIT_FOREVER);
  if (res != NXD_MQTT_SUCCESS)
  {
    if (mqtt_connection_attempt_cnt < 5)
    {
      APPLOG("Failed to connect to  MQTT server. Error %d", res);
    }
    return RES_ERROR;
  }
  mqtt_client_connected = 1;
  mqtt_connection_attempt_cnt = 0;

  APPLOG("MQTT client connected to server %03d.%03d.%03d.%03d", IPADDR(ip_addr.nxd_ip_address.v4));
  return RES_OK;
}


/*-----------------------------------------------------------------------------------------------------


  \param ip_ptr

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Net_mqtt_disconnect_mqtt_client(NX_IP *ip_ptr)
{
  uint32_t res;

  if (mqtt_ip_ptr == 0)  return RES_ERROR;
  if (ip_ptr != mqtt_ip_ptr) return RES_ERROR;

  if (mqtt_client_connected != 0)
  {
    // Отключаем клиента mqtt
    res = nxd_mqtt_client_disconnect(&mqtt_client);
    if (res == NXD_MQTT_SUCCESS)
    {
      mqtt_client_connected = 0;
      APPLOG("MQTT client disconnected from server");
      return RES_OK;
    }
    else
    {
      APPLOG("Failed to disconnect MQTT client from server. Error %d", res);
      return RES_ERROR;
    }
  }

  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param ip

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Net_MQTT_client_delete(void)
{
  uint32_t res;
  if (mqtt_client_connected != 0)
  {
    res = Net_mqtt_disconnect_mqtt_client(mqtt_ip_ptr);
    if (res != RES_OK)
    {
      Send_flag_to_app(APP_DO_SYSTEM_RESTART);
      return res;
    }
  }

  tx_event_flags_delete(&mqtt_app_flag);

  res = nxd_mqtt_client_delete(&mqtt_client);
  if (res != NXD_MQTT_SUCCESS)
  {
    APPLOG("Failed to delete MQTT client. Error %d", res);
    // Если не удалось закрыть сессию MQTT то сбросить устройство.
    Send_flag_to_app(APP_DO_SYSTEM_RESTART);
    return RES_ERROR;
  }
  else
  {
    mqtt_ip_ptr = 0;
    APPLOG("MQTT client deleted successfully");
    mqtt_client_created = 0;
    return RES_OK;

  }

}

