// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2019.08.31
// 15:42:10
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"



#define                    FTP_SERVER_STACK_SIZE     4096
#pragma data_alignment=8
uint8_t                    ftp_server_stack_memory[FTP_SERVER_STACK_SIZE];


static NX_FTP_SERVER       ftp_server;
static NX_IP              *ftp_ip_ptr;

/*-----------------------------------------------------------------------------------------------------


  \param ftp_server_ptr
  \param client_ip_address
  \param client_port
  \param name
  \param password
  \param extra_info

  \return UINT
-----------------------------------------------------------------------------------------------------*/
#ifndef NX_DISABLE_IPV6
static UINT ftp_server_login(struct NX_FTP_SERVER_STRUCT *ftp_server_ptr, NXD_ADDRESS *client_ip_address, UINT client_port, CHAR *name, CHAR *password, CHAR *extra_info)
{
  uint32_t ipaddr = client_ip_address->nxd_ip_address.v4;

  if (strcmp(name, (char const *)wvar.ftp_serv_login) == 0)
  {
    if (strcmp(password, (char const *)wvar.ftp_serv_password) == 0)
    {
      APPLOG("FTP server login from: %03d.%03d.%03d.%03d port: %d. Access is allowed", IPADDR(ipaddr), client_port);
      return NX_SUCCESS;
    }
  }
  APPLOG("FTP server login from: %03d.%03d.%03d.%03d port: %d. Access is denied", IPADDR(ipaddr), client_port);
  return NX_NOT_SUCCESSFUL;
}
#else
static UINT ftp_server_login(struct NX_FTP_SERVER_STRUCT *ftp_server_ptr, ULONG client_ip_address, UINT client_port, CHAR *name, CHAR *password, CHAR *extra_info)
{
  if (strcmp(name, (char const *)wvar.ftp_serv_login) == 0)
  {
    if (strcmp(password, (char const *)wvar.ftp_serv_password) == 0)
    {
      APPLOG("FTP server login from: %03d.%03d.%03d.%03d port: %d. Access is allowed", IPADDR(client_ip_address), client_port);
      return NX_SUCCESS;
    }
  }
  APPLOG("FTP server login from: %03d.%03d.%03d.%03d port: %d. Access is denied", IPADDR(client_ip_address), client_port);
  return NX_NOT_SUCCESSFUL;
}
#endif

/*-----------------------------------------------------------------------------------------------------


  \param ftp_server_ptr
  \param client_ip_address
  \param client_port
  \param name
  \param password
  \param extra_info

  \return UINT
-----------------------------------------------------------------------------------------------------*/
#ifndef NX_DISABLE_IPV6
static UINT ftp_server_logout(struct NX_FTP_SERVER_STRUCT *ftp_server_ptr, NXD_ADDRESS *client_ip_address, UINT client_port, CHAR *name, CHAR *password, CHAR *extra_info)
{
  uint32_t ipaddr = client_ip_address->nxd_ip_address.v4;
  APPLOG("FTP server logout from: %03d.%03d.%03d.%03d port: %d.", IPADDR(ipaddr), client_port);
  return NX_SUCCESS;
}
#else
static UINT ftp_server_logout(struct NX_FTP_SERVER_STRUCT *ftp_server_ptr, ULONG client_ip_address, UINT client_port, CHAR *name, CHAR *password, CHAR *extra_info)
{

  APPLOG("FTP server logout from: %03d.%03d.%03d.%03d port: %d.", IPADDR(client_ip_address), client_port);
  return NX_SUCCESS;
}
#endif


/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Net_FTP_server_create(NX_IP  *ip_ptr)
{
  UINT res;

  if (wvar.enable_ftp_server == 0) return RES_ERROR;
  if (ftp_ip_ptr != 0) return RES_ERROR;


#ifndef NX_DISABLE_IPV6
  res = nxd_ftp_server_create(&ftp_server,
       "FTP Server",
       ip_ptr,
       &fat_fs_media,
       &ftp_server_stack_memory[0],
       FTP_SERVER_STACK_SIZE,
       &net_packet_pool,
       ftp_server_login,
       ftp_server_logout);
#else
  res = nx_ftp_server_create(&ftp_server,
       "FTP Server",
       ip_ptr,
       &fat_fs_media,
       &ftp_server_stack_memory[0],
       FTP_SERVER_STACK_SIZE,
       &net_packet_pool,
       ftp_server_login,
       ftp_server_logout);
#endif
  if (NX_SUCCESS != res)
  {
    APPLOG("Failed to create FTP server. Error %d", res);
    return RES_ERROR;
  }

  res = nx_ftp_server_start(&ftp_server);
  if (NX_SUCCESS != res)
  {
    nx_ftp_server_delete(&ftp_server);
    APPLOG("Failed to start FTP server. Error %d", res);
    return RES_ERROR;
  }

  ftp_ip_ptr = ip_ptr;
  APPLOG("FTP server started");
  return RES_OK;

}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Net_FTP_server_delete(void)
{
  UINT res;

  if (ftp_ip_ptr == 0) return RES_OK;



  res = nx_ftp_server_stop(&ftp_server);
  if (NX_SUCCESS != res)
  {
    APPLOG("Failed to stop  FTP server. Error %d", res);
  }


  res = nx_ftp_server_delete(&ftp_server);
  if (NX_SUCCESS != res)
  {
    ftp_ip_ptr = 0;
    APPLOG("Failed to delete  FTP server. Error %d", res);
    return RES_ERROR;
  }

  ftp_ip_ptr = 0;
  APPLOG("FTP server stopped");
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Is_FTP_server_created(void)
{
  if (ftp_ip_ptr == 0) return 0;
  else return 1;
}


