#ifndef __FREEMASTER_VARS
  #define __FREEMASTER_VARS
#include "App.h"
#include "freemaster_cfg.h"
#include "freemaster.h"
#include "freemaster_tsa.h"

FMSTR_TSA_TABLE_BEGIN(wvar_vars)
FMSTR_TSA_RW_VAR( wvar.can_node_number                 ,FMSTR_TSA_UINT8     ) // CAN node number | def.val.= 1
FMSTR_TSA_RW_VAR( wvar.enable_HTTP_server              ,FMSTR_TSA_UINT8     ) // Enable WEB server | def.val.= 1
FMSTR_TSA_RW_VAR( wvar.enable_HTTPS                    ,FMSTR_TSA_UINT8     ) // Enable_HTTPS | def.val.= 0
FMSTR_TSA_RW_VAR( wvar.rndis_config                    ,FMSTR_TSA_UINT8     ) // RNDIS interface configuration (0-Win home net, 1 - DHCP server) | def.val.= 1
FMSTR_TSA_RW_VAR( wvar.vcom_mode                       ,FMSTR_TSA_UINT8     ) // Select VCOM mode (0 - VT100 terminal, 1 - FreeMaster)  | def.val.= 1
FMSTR_TSA_TABLE_END();


#endif
