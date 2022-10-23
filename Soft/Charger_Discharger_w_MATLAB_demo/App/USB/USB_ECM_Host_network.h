#ifndef USB_ECM_HOST_LAN_H
  #define USB_ECM_HOST_LAN_H

uint8_t  Is_ECM_Host_network_active(void);
void     ECM_Host_init_network_stack(void);
void     ECM_Host_network_controller(void);
uint32_t ECM_DHCP_client_start(void);

#endif // USB_ECM_HOST_LAN_H



