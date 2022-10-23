#ifndef __FREEMASTER_VARS
  #define __FREEMASTER_VARS
#include "App.h"
#include "freemaster_cfg.h"
#include "freemaster.h"
#include "freemaster_tsa.h"

FMSTR_TSA_TABLE_BEGIN(wvar_vars)
FMSTR_TSA_RW_VAR( wvar.acc_current_offset              ,FMSTR_TSA_FLOAT     ) // Accumulator current offset (A) | def.val.= 0
FMSTR_TSA_RW_VAR( wvar.can_node_number                 ,FMSTR_TSA_UINT8     ) // CAN node number | def.val.= 1
FMSTR_TSA_RW_VAR( wvar.en_dhcp_client                  ,FMSTR_TSA_UINT8     ) // Enable DHCP client (0-No, 1-Yes) | def.val.= 1
FMSTR_TSA_RW_VAR( wvar.en_freemaster                   ,FMSTR_TSA_UINT8     ) // Enable FreeMaster protocol | def.val.= 1
FMSTR_TSA_RW_VAR( wvar.en_ftp_client                   ,FMSTR_TSA_UINT8     ) // Enable FTP client task | def.val.= 0
FMSTR_TSA_RW_VAR( wvar.en_matlab                       ,FMSTR_TSA_UINT8     ) // Enable MATLAB connection | def.val.= 1
FMSTR_TSA_RW_VAR( wvar.enable_ftp_server               ,FMSTR_TSA_UINT8     ) // Enable FTP server | def.val.= 1
FMSTR_TSA_RW_VAR( wvar.enable_HTTP_server              ,FMSTR_TSA_UINT8     ) // Enable WEB server | def.val.= 1
FMSTR_TSA_RW_VAR( wvar.enable_HTTPS                    ,FMSTR_TSA_UINT8     ) // Enable_HTTPS | def.val.= 0
FMSTR_TSA_RW_VAR( wvar.load_current_offset             ,FMSTR_TSA_FLOAT     ) // Load current offset (A) | def.val.= 0
FMSTR_TSA_RW_VAR( wvar.max_scv_file_sz                 ,FMSTR_TSA_UINT32    ) // Max .csv file size (byte) | def.val.= 100000
FMSTR_TSA_RW_VAR( wvar.mqtt_enable                     ,FMSTR_TSA_UINT8     ) // Enable MQTT client  | def.val.= 0
FMSTR_TSA_RW_VAR( wvar.mqtt_server_port                ,FMSTR_TSA_UINT16    ) // MQTT server port number | def.val.= 1883
FMSTR_TSA_RW_VAR( wvar.psrc_current_offset             ,FMSTR_TSA_FLOAT     ) // Power source current offset | def.val.= 0
FMSTR_TSA_RW_VAR( wvar.rndis_config                    ,FMSTR_TSA_UINT8     ) // RNDIS interface configuration (0-Win home net, 1 - DHCP server) | def.val.= 1
FMSTR_TSA_RW_VAR( wvar.usb_mode                        ,FMSTR_TSA_UINT8     ) // USB mode (0- RNDIS & VCOM device, 1- CDC ECM host) | def.val.= 1
FMSTR_TSA_TABLE_END();


#endif
