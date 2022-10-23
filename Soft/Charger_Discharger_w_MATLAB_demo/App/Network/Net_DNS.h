#ifndef NET_DNS_H
  #define NET_DNS_H


void     DNS_client_controller(void);
UINT     DNS_get_host_address(UCHAR *host_name_ptr, ULONG *dns_address, ULONG wait_option);
uint32_t Is_DNS_created(void);

#endif // NET_DNS_H



