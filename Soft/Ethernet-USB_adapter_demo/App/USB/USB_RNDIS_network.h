#ifndef USB_RNDIS_DRIVER_H
  #define USB_RNDIS_DRIVER_H


UINT     Register_rndis_class(void);
UINT     Register_cdc_ecm_class(void);
void     RNDIS_init_network_stack(void);
void     RNDIS_network_controller(void);
uint8_t  Is_RNDIS_network_active(void);
uint32_t RNDIS_Get_MAC(char* mac_str, uint32_t max_str_len);
uint32_t RNDIS_Get_MASK_IP(char* ip_str, char* mask_str,uint32_t max_str_len);
uint32_t RNDIS_Get_Gateway_IP(char* gate_str,uint32_t max_str_len);
void     HAL_PCD_SOFCallback(PCD_HandleTypeDef *hpcd);
#endif // USB_RNDIS_DRIVER_H



