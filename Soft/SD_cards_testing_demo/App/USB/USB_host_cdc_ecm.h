#ifndef APP_HOST_CDC_ECM_H
  #define APP_HOST_CDC_ECM_H

uint32_t USB_host_cdc_ecm_init(void);
UINT     USB_host_change_function(ULONG event, UX_HOST_CLASS *host_class, VOID *instance);

uint8_t  Is_ECM_usb_link_up(void);
uint8_t  Is_ECM_usb_network_link_up(void);
uint32_t ECM_Get_MAC(char* mac_str, uint32_t max_str_len);
uint32_t ECM_Get_MASK_IP(char* ip_str, char* mask_str,uint32_t max_str_len);
uint32_t ECM_Get_Gateway_IP(char* gate_str,uint32_t max_str_len);

#endif



