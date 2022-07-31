#include "App.h"
#include "freemaster_cfg.h"
#include "freemaster.h"
#include "freemaster_tsa.h"

const T_parmenu parmenu[5]=
{
  {BKMAN3R1_0,BKMAN3R1_main,"Parameters and settings","PARAMETERS", -1},//  �������� ���������
  {BKMAN3R1_main,BKMAN3R1_General,"General settings","GENERAL SETTINGS", -1},//  
  {BKMAN3R1_main,BKMAN3R1_RNDIS,"RNDIS device mode settings","RNDIS SETTINGS", -1},//  
  {BKMAN3R1_main,BKMAN3R1_CDC_ECM,"CDC ECM host mode settings","ECM SETTINGS", -1},//  
  {BKMAN3R1_main,BKMAN3R1_WEB_SERVER,"Web server settings","WEB SETTINGS", -1},//  
};


const T_work_params dwvar[14]=
{
  {
    "default_ip_addr",
    "Default IP address",
    "DEFIPAD",
    (void*)&wvar.default_ip_addr,
    tstring,
    0,
    0,
    0,
    0,
    BKMAN3R1_CDC_ECM,
    "192.168.1.1",
    "%s",
    0,
    sizeof(wvar.default_ip_addr),
    1,
    4,
  },
  {
    "default_net_mask",
    "Default network mask ",
    "DEFNTMS",
    (void*)&wvar.default_net_mask,
    tstring,
    0,
    0,
    0,
    0,
    BKMAN3R1_CDC_ECM,
    "255.255.255.0",
    "%s",
    0,
    sizeof(wvar.default_net_mask),
    2,
    4,
  },
  {
    "default_gateway_addr",
    "Default gateway address",
    "DEFGATE",
    (void*)&wvar.default_gateway_addr,
    tstring,
    0,
    0,
    0,
    0,
    BKMAN3R1_CDC_ECM,
    "192.168.1.254",
    "%s",
    0,
    sizeof(wvar.default_gateway_addr),
    3,
    4,
  },
  {
    "en_dhcp_client",
    "Enable DHCP client (0-No, 1-Yes)",
    "ENDHCPC",
    (void*)&wvar.en_dhcp_client,
    tint8u,
    1,
    0,
    1,
    0,
    BKMAN3R1_CDC_ECM,
    "",
    "%d",
    0,
    sizeof(wvar.en_dhcp_client),
    4,
    1,
  },
  {
    "this_host_name",
    "This device host name",
    "HOSTNAM",
    (void*)&wvar.this_host_name,
    tstring,
    0,
    0,
    0,
    0,
    BKMAN3R1_CDC_ECM,
    "host1",
    "%s",
    0,
    sizeof(wvar.this_host_name),
    5,
    0,
  },
  {
    "name",
    "Product  name",
    "-",
    (void*)&wvar.name,
    tstring,
    0,
    0,
    0,
    0,
    BKMAN3R1_General,
    "BKMAN3R1",
    "%s",
    0,
    sizeof(wvar.name),
    1,
    0,
  },
  {
    "ver",
    "Firmware version",
    "-",
    (void*)&wvar.ver,
    tstring,
    0,
    0,
    0,
    0,
    BKMAN3R1_General,
    "1.0",
    "%s",
    0,
    sizeof(wvar.ver),
    2,
    0,
  },
  {
    "can_node_number",
    "CAN node number",
    "-",
    (void*)&wvar.can_node_number,
    tint8u,
    1,
    1,
    255,
    0,
    BKMAN3R1_General,
    "",
    "%d",
    0,
    sizeof(wvar.can_node_number),
    3,
    0,
  },
  {
    "usb_mode",
    "USB mode (0- RNDIS & VCOM device, 1- CDC ECM host)",
    "-",
    (void*)&wvar.usb_mode,
    tint8u,
    1,
    0,
    1,
    0,
    BKMAN3R1_General,
    "",
    "%d",
    0,
    sizeof(wvar.usb_mode),
    4,
    3,
  },
  {
    "en_freemaster",
    "Enable FreeMaster protocol",
    "-",
    (void*)&wvar.en_freemaster,
    tint8u,
    1,
    0,
    1,
    0,
    BKMAN3R1_General,
    "",
    "%d",
    0,
    sizeof(wvar.en_freemaster),
    5,
    1,
  },
  {
    "rndis_config",
    "RNDIS interface configuration (0-Win home net, 1 - DHCP server)",
    "-",
    (void*)&wvar.rndis_config,
    tint8u,
    1,
    0,
    1,
    0,
    BKMAN3R1_RNDIS,
    "",
    "%d",
    0,
    sizeof(wvar.rndis_config),
    1,
    2,
  },
  {
    "enable_HTTP_server",
    "Enable WEB server",
    "-",
    (void*)&wvar.enable_HTTP_server,
    tint8u,
    1,
    0,
    1,
    0,
    BKMAN3R1_WEB_SERVER,
    "",
    "%d",
    0,
    sizeof(wvar.enable_HTTP_server),
    2,
    1,
  },
  {
    "enable_HTTPS",
    "Enable_HTTPS",
    "-",
    (void*)&wvar.enable_HTTPS,
    tint8u,
    0,
    0,
    1,
    0,
    BKMAN3R1_WEB_SERVER,
    "",
    "%d",
    0,
    sizeof(wvar.enable_HTTPS),
    3,
    1,
  },
  {
    "HTTP_server_password",
    "HTTP server pasword",
    "-",
    (void*)&wvar.HTTP_server_password,
    tstring,
    0,
    0,
    0,
    0,
    BKMAN3R1_WEB_SERVER,
    "123456789",
    "%s",
    0,
    sizeof(wvar.HTTP_server_password),
    4,
    0,
  },
};
 
 
// Selector description:  ����� ����� Yes � No
static const T_selector_items selector_1[2] = 
{
  {0, "No", 0},
  {1, "Yes", 1},
};
 
// Selector description:  Confuguration of RNDIS interface
static const T_selector_items selector_2[2] = 
{
  {0, "Windows home network ", -1},
  {1, "Preconfigured DHCP server", -1},
};
 
// Selector description:  ����� ������ USB ���������
static const T_selector_items selector_3[2] = 
{
  {0, "Device", -1},
  {1, "Host", -1},
};
 
const T_selectors_list selectors_list[5] = 
{
  {"string"            , 0    , 0             },
  {"binary"            , 2    , selector_1    },
  {"rndis_config"      , 2    , selector_2    },
  {"usb_mode"          , 2    , selector_3    },
  {"ip_addr"           , 0    , 0             },
};
 
