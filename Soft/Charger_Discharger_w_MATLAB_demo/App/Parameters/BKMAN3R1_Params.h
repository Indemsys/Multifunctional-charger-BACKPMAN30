#ifndef __PARAMS_H
  #define __PARAMS_H

#define  APP_PROFILE        BKMAN3R1
#define  MAIN_PARAMS_ROOT   BKMAN3R1_main
#define  PARAMS_ROOT        BKMAN3R1_0

#define  DWVAR_SIZE        32
#define  PARMNU_ITEM_NUM   9

#define  SELECTORS_NUM     5



  #define VAL_LOCAL_EDITED 0x01  //
  #define VAL_READONLY     0x02  // Можно только читать
  #define VAL_PROTECT      0x04  // Защишено паролем
  #define VAL_UNVISIBLE    0x08  // Не выводится на дисплей
  #define VAL_NOINIT       0x10  // Не инициализируется


enum vartypes
{
    tint8u  = 1,
    tint16u  = 2,
    tint32u  = 3,
    tfloat  = 4,
    tarrofdouble  = 5,
    tstring  = 6,
    tarrofbyte  = 7,
    tint32s  = 8,
};


enum enm_parmnlev
{
    BKMAN3R1_0,
    BKMAN3R1_main,
    BKMAN3R1_General,
    BKMAN3R1_RNDIS,
    BKMAN3R1_CDC_ECM,
    BKMAN3R1_WEB_SERVER,
    BKMAN3R1_FTP_SERVER,
    BKMAN3R1_CALIBRATION,
    BKMAN3R1_FTP_CLIENT,
    BKMAN3R1_MQTT,
};


typedef struct 
{
  enum enm_parmnlev prevlev;
  enum enm_parmnlev currlev;
  const char* name;
  const char* shrtname;
  const char  visible;
}
T_parmenu;


typedef struct
{
  const uint8_t*     var_name;        // Имя параметра
  const uint8_t*     var_description; // Строковое описание
  const uint8_t*     var_alias;       // Короткая аббревиатура
  void*              val;          // Указатель на значение переменной в RAM
  enum  vartypes     vartype;      // Идентификатор типа переменной
  float              defval;       // Значение по умолчанию
  float              minval;       // Минимальное возможное значение
  float              maxval;       // Максимальное возможное значение  
  uint8_t            attr;         // Аттрибуты переменной
  unsigned int       parmnlev;     // Подгруппа к которой принадлежит параметр
  const  void*       pdefval;      // Указатель на данные для инициализации
  const  char*       format;       // Строка форматирования при выводе на дисплей
  void               (*func)(void);// Указатель на функцию выполняемую после редактирования
  uint16_t           varlen;       // Длинна переменной
  uint32_t           menu_pos;     // Позиция в меню
  uint32_t           selector_id;  // Идентификатор селектора
} T_work_params;


typedef struct
{
  uint32_t           val;
  const uint8_t*     caption;
  int32_t            img_indx;
} T_selector_items;


typedef struct
{
  const uint8_t*           name;
  uint32_t                 items_cnt;
  const T_selector_items*  items_list;
} T_selectors_list;

extern const T_selectors_list selectors_list[];

typedef struct
{
  float          acc_current_offset;            // Accumulator current offset (A) | def.val.= 0
  uint8_t        can_node_number;               // CAN node number | def.val.= 1
  uint8_t        default_gateway_addr[16];      // Default gateway address | def.val.= 192.168.1.254
  uint8_t        default_ip_addr[16];           // Default IP address | def.val.= 192.168.1.1
  uint8_t        default_net_mask[16];          // Default network mask  | def.val.= 255.255.255.0
  uint8_t        en_dhcp_client;                // Enable DHCP client (0-No, 1-Yes) | def.val.= 1
  uint8_t        en_freemaster;                 // Enable FreeMaster protocol | def.val.= 1
  uint8_t        en_ftp_client;                 // Enable FTP client task | def.val.= 0
  uint8_t        en_matlab;                     // Enable MATLAB connection | def.val.= 1
  uint8_t        enable_ftp_server;             // Enable FTP server | def.val.= 1
  uint8_t        enable_HTTP_server;            // Enable WEB server | def.val.= 1
  uint8_t        enable_HTTPS;                  // Enable_HTTPS | def.val.= 0
  uint8_t        ftp_client_server_adress[16];  // FTP server address | def.val.= 192.168.3.2
  uint8_t        ftp_client_server_login[16];   // FTP server login | def.val.= login
  uint8_t        ftp_client_server_pass[16];    // FTP server password | def.val.= pass
  uint8_t        ftp_serv_login[16];            // Login | def.val.= ftp_login
  uint8_t        ftp_serv_password[64];         // Password  | def.val.= ftp_pass
  uint8_t        HTTP_server_password[32];      // HTTP server pasword | def.val.= 123456789
  float          load_current_offset;           // Load current offset (A) | def.val.= 0
  uint32_t       max_scv_file_sz;               // Max .csv file size (byte) | def.val.= 100000
  uint8_t        mqtt_client_id[16];            // Client ID | def.val.= Client1
  uint8_t        mqtt_enable;                   // Enable MQTT client  | def.val.= 0
  uint8_t        mqtt_password[16];             // User password | def.val.= pass
  uint8_t        mqtt_server_ip[16];            // MQTT server IP address | def.val.= 192.168.3.2
  uint16_t       mqtt_server_port;              // MQTT server port number | def.val.= 1883
  uint8_t        mqtt_user_name[16];            // User name | def.val.= user
  uint8_t        name[64];                      // Product  name | def.val.= BKMAN3R1
  float          psrc_current_offset;           // Power source current offset | def.val.= 0
  uint8_t        rndis_config;                  // RNDIS interface configuration (0-Win home net, 1 - DHCP server) | def.val.= 1
  uint8_t        this_host_name[16];            // This device host name | def.val.= host1
  uint8_t        usb_mode;                      // USB mode (0- RNDIS & VCOM device, 1- CDC ECM host) | def.val.= 1
  uint8_t        ver[64];                       // Firmware version | def.val.= 1.0
} WVAR_TYPE;


#endif



// Selector description:  Выбор между Yes и No
#define BINARY_NO                                 0
#define BINARY_YES                                1

// Selector description:  Confuguration of RNDIS interface
#define RNDIS_CONFIG_WINDOWS_HOME_NETWORK_        0
#define RNDIS_CONFIG_PRECONFIGURED_DHCP_SERVER    1

// Selector description:  Режим работы USB интерфеса
#define USB_MODE_DEVICE                           0
#define USB_MODE_HOST                             1
