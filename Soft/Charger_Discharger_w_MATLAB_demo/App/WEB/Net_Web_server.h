#ifndef NET_WEB_SERVER_H
  #define NET_WEB_SERVER_H

#define  AUTH_USER_NAME      "user" 
#define  REALM_NAME          "IoT_realm"
#define  WEB_FILES_DIR_NAME  "WWW"
#define  SETTINGS_URL        "/data.json"
#define  DEV_LOG_URL         "/log.txt"
#define  RESET_CMD_URL       "/reset"
#define  RESET_LOG_CMD_URL   "/reset_log"
#define  INDEX_URL           "/"
#define  MAX_PUT_DATA_SIZE   1024*30

void HTTP_server_controller(void);

#endif // NET_WEB_SERVER_H



