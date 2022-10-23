#ifndef NET_MQTT_MSG_CONTROLLER_H
  #define NET_MQTT_MSG_CONTROLLER_H

// Значения ключа 'OpCode' для JSON структур без аргументов получаемых от внешнего управляющего агента
  #define  OPCODE_RESET                        1
  #define  OPCODE_RESET_TO_DEFAULT             2
  #define  OPCODE_REQUEST_SCHEMA               3  // Запрос у устройства схемы параметров и их значения для передачи в программу
  #define  OPCODE_REQUEST_PARAMETERS           4
  #define  OPCODE_REQUEST_STATE_INFO           13  // Запрос на получение информации о состоянии устройства
  #define  OPCODE_GET_LOG_FILE                 17  // Выслать агенту логфайл
  #define  OPCODE_RESET_LOG_FILE               18  // Стереть логфайл


// Значения ключа 'OpCode' для JSON структур с аргументами получаемых от внешнего управляющего агента
  #define  OPCODE_SAVE_PARAMETERS_TO_RAM       100    // Передача параметров из программы на PC в оперативную память устройства
  #define  OPCODE_SAVE_PARAMETERS_TO_NV        101    // Передача параметров из программы на PC в постоянную память устройства
  #define  OPCODE_GET_RECORD_FILE              102    // Начать передачу файла аудиозаписи с указанным именем на FTP сервер
  #define  OPCODE_DELETE_FILES_BY_LIST         103    // Стереть файлы по списку прилагающихся имен файлов
  #define  OPCODE_GET_FILES_BY_LIST            104    // Начать передачу файлов по списку прилагающихся имен файлов



  #define COMMAND_KEY                          "OpCode"               // Идентификатор JSON блока с командой устройству
  #define MAIN_PARAMETERS_KEY                  "Parameters"           //
  #define DATETIME_SETTINGS_KEY                "DateTime"             //
  #define DEVICE_HEADER_KEY                    "Device"               //
  #define PARAMETERS_TREE_KEY                  "Parameters_tree"      //

// Имена топиков от которых получаем данные
  #define MQTT_SUBSCR_TOPIC_BROADCAST          "brodcast"

// Имена топиков которым посылаем данные
  #define MQTT_PUBLISH_TOPIC_SCHEMA            "schema"               // Для отправки схемы параметров и их значений
  #define MQTT_PUBLISH_TOPIC_VALUES            "params"               // Для отправки значений параметров
  #define MQTT_PUBLISH_TOPIC_FILES             "files"                // Для отправки списка файлов
  #define MQTT_PUBLISH_TOPIC_STATE             "state"                // Для отправки информации о текущем состоянии устройства
  #define MQTT_PUBLISH_TOPIC_ACK               "Ack"                  // Для отправки подтверждения выполнения команды
  #define MQTT_PUBLISH_TOPIC_LOG               "log"                  // Для отправки подтверждения выполнения команды



  #define MQTT_PACK_FILE_EXT                   ".pac"  // Расширения для файла со сжатыми данными
  #define MQTT_MAX_FILE_NAME_LEN               254     // Максимальная длина имени файла

  #define MQTTMC_COMPRESSION_ALG               SIXPACK_ALG

// Флаги в заголовке пакетов в потоке
  #define FIRST_PACKET_FLAG                       BIT(0)  // Флаг обозначающий первый пакет в потоке
  #define LAST_PACKET_FLAG                        BIT(1)  // Флаг обозначающий последний пакет в потоке
  #define BRAKE_STREAM_FLAG                       BIT(2)  // Флаг обозначающий разрыв потока. После этого пакета поток прекращается
  #define FILE_STREAM_FLAG                        BIT(4)  // Флаг обозначающий поток предназначенный для записи в файл
  #define LONG_COMPRESSED_STREAM_FLAG             BIT(5)  // Флаг обозначающий длинный сжатый поток, кторый предварительно следует сохранять в файл
  #define SHORT_COMPRESSED_STREAM_FLAG            BIT(6)  // Флаг обозначающий короткий сжатый поток, который может быть распакован из памяти
  #define STREAM_PACKET_FLAG                      BIT(7)  // Обязательный флаг в заголовке пакета

  #define UNCOMPRESSED_STREAM                  0


  #define ENABLE_COMPRESSION                   1
  #define DISABLE_COMPRESSION                  0


  #define  MQTT_STREAM_ERROR_BROKEN_STREAM     1
  #define  MQTT_STREAM_ERROR_FILE_CREATION     2
  #define  MQTT_STREAM_ERROR_ALLOC_ERROR       3
  #define  MQTT_STREAM_ERROR_FS_WRITE_ERR      4
  #define  MQTT_STREAM_ERROR_ZERO_SZ           5
  #define  MQTT_STREAM_ERROR_SEQUENCE          6
  #define  MQTT_STREAM_ERROR_PAYLOAD_SZ        7


void     MQTTMC_messages_processor(void);
void     MQTTMC_set_opcode(uint32_t opcode);
void     MQTT_client_controller(void);


uint32_t MQTTMC_Send_Ack(uint32_t ack_code);
uint32_t MQTTMC_Send_File_transfer_Ack(uint32_t ack_code);
uint32_t MQTTMC_Send_Device_State(void);
uint32_t MQTTMC_Send_params_schema(void);
uint32_t MQTTMC_Send_params_values(void);
void     MQTTMC_Set_request_send_records_list(void);
uint32_t Is_mqtt_session_active(void);
#endif // NET_MQTT_MSG_CONTROLLER_H



