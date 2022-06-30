#ifndef NET_UTILS_H
  #define NET_UTILS_H

  #define IPADDR(x) (((uint32_t)x) >> 24) & 0xFF , (((uint32_t)x) >> 16) & 0xFF,  (((uint32_t)x) >> 8) & 0xFF,  ((uint32_t)x) & 0xFF
  #define MACADDR(x) x.octet[0],x.octet[1],x.octet[2],x.octet[3],x.octet[4],x.octet[5]
  #define MACADDR_PT(x) x->octet[0],x->octet[1],x->octet[2],x->octet[3],x->octet[4],x->octet[5]

  #define INTF0IPADDR(x) \
           ((unsigned char *)&x.nx_ip_interface[0].nx_interface_ip_address)[3],\
           ((unsigned char *)&x.nx_ip_interface[0].nx_interface_ip_address)[2],\
           ((unsigned char *)&x.nx_ip_interface[0].nx_interface_ip_address)[1],\
           ((unsigned char *)&x.nx_ip_interface[0].nx_interface_ip_address)[0]

  #ifndef NX_DISABLE_IPV6
    #ifndef FILL_NXD_IPV6_ADDRESS
      #define FILL_NXD_IPV6_ADDRESS(ipv6,f0,f1,f2,f3,f4,f5,f6,f7) do { \
                                                                       ipv6.nxd_ip_address.v6[0] = (((uint32_t)f0 << 16) & 0xFFFF0000) | ((uint32_t)f1 & 0x0000FFFF);\
                                                                       ipv6.nxd_ip_address.v6[1] = (((uint32_t)f2 << 16) & 0xFFFF0000) | ((uint32_t)f3 & 0x0000FFFF);\
                                                                       ipv6.nxd_ip_address.v6[2] = (((uint32_t)f4 << 16) & 0xFFFF0000) | ((uint32_t)f5 & 0x0000FFFF);\
                                                                       ipv6.nxd_ip_address.v6[3] = (((uint32_t)f6 << 16) & 0xFFFF0000) | ((uint32_t)f7 & 0x0000FFFF);\
                                                                       ipv6.nxd_ip_version       = NX_IP_VERSION_V6;\
                                                                   } while(0);
    #endif /* FILL_NXD_IPV6_ADDRESS */
  #endif



void         Print_packet_poll_statistic_to_RTT(void);
void         Print_packet_poll_statistic_to_VT100(void);
int32_t      Str_to_IP_v4(const char *src, uint8_t *dst);
int32_t      Str_to_IP_v6(char *src, uint8_t *dst);

uint32_t     Prepare_net_mutex(void);
uint32_t     Get_net_mutex(uint32_t delay_ms);
uint32_t     Put_net_mutex(void);
char const*  Get_NX_err_str(uint32_t err);




#endif // NET_UTILS_H



