#ifndef NET_MQTT_H
  #define NET_MQTT_H

#define  MQTT_QOS0                      0
#define  MQTT_QOS1                      1

#define  MQTT_RETAIN                    1
#define  MQTT_NO_RETAIN                 0

#define  MQTT_MESSAGE_EVENT        BIT(0)
extern   NXD_MQTT_CLIENT           mqtt_client;

uint32_t Is_mqtt_client_created(void);
uint32_t Is_mqtt_client_connected(void);
uint32_t Get_last_mqtt_connection_time(void);
uint32_t Net_mqtt_client_create(NX_IP *ip);
uint32_t Net_mqtt_client_connect(void);
uint32_t Net_mqtt_disconnect_mqtt_client(NX_IP *ip_ptr);
uint32_t Net_MQTT_client_delete(void);
uint32_t Net_mqtt_set_message_flag(void);
uint32_t Net_mqtt_wait_message_flag(ULONG wait_option);

#endif // NET_MQTT_H



