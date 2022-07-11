#ifndef USB_RNDIS_DRIVER_H
  #define USB_RNDIS_DRIVER_H


UINT    Register_rndis_class(void);
UINT    Register_cdc_ecm_class(void);
void    RNDIS_init_network_stack(void);
void    RNDIS_network_controller(void);
uint8_t Is_RNDIS_network_active(void);

#endif // USB_RNDIS_DRIVER_H



