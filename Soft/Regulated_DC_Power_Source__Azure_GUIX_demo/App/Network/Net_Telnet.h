#ifndef NET_TELNET_H
  #define NET_TELNET_H

#include   "nxd_telnet_server.h"

void     Telnet_client_connect(struct NX_TELNET_SERVER_STRUCT *telnet_server_ptr, UINT logical_connection);
void     Telnet_receive_data(struct NX_TELNET_SERVER_STRUCT *telnet_server_ptr, UINT logical_connection, NX_PACKET *packet_ptr);
void     Telnet_client_disconnect(struct NX_TELNET_SERVER_STRUCT *telnet_server_ptr, UINT logical_connection);

uint32_t TELNET_server_create(NX_TELNET_SERVER  *telnet_server_ptr);
uint32_t TELNET_server_delete(NX_TELNET_SERVER  *telnet_server_ptr);

uint8_t  Is_telnet_connected(void);


#endif // NET_TELNET_H



