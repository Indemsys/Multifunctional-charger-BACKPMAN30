/*
 * Copyright 2021 NXP
 *
 * License: NXP LA_OPT_NXP_Software_License
 *
 * NXP Confidential. This software is owned or controlled by NXP and may
 * only be used strictly in accordance with the applicable license terms.
 * By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that
 * you have read, and that you agree to comply with and are bound by,
 * such license terms.  If you do not agree to be bound by the applicable
 * license terms, then you may not retain, install, activate or otherwise
 * use the software.  This code may only be used in a microprocessor,
 * microcontroller, sensor or digital signal processor ("NXP Product")
 * supplied directly or indirectly from NXP.  See the full NXP Software
 * License Agreement in license/LA_OPT_NXP_Software_License.pdf
 *
 * FreeMASTER Communication Driver - Network LWIP TCP driver
 */

#include "freemaster.h"
#include "freemaster_private.h"
#include "freemaster_netx_tcp.h"
#include "freemaster_protocol.h"
#include "freemaster_net.h"
#include "nx_api.h"
#include "nxd_bsd.h"

/******************************************************************************
 * Adapter configuration
 ******************************************************************************/
#if (defined(FMSTR_SHORT_INTR) && FMSTR_SHORT_INTR) || (defined(FMSTR_LONG_INTR) && FMSTR_LONG_INTR)
  #error The FreeMASTER network TCP lwip driver does not support interrupt mode.
#endif

/******************************************************************************
 * Local types
 ******************************************************************************/

typedef struct FMSTR_TCP_SESSION_S
{
    int sock;
    FMSTR_BOOL receivePending;
    FMSTR_NET_ADDR address;
} FMSTR_TCP_SESSION;

/******************************************************************************
 * Local functions
 ******************************************************************************/

static FMSTR_BOOL _FMSTR_Net_Tcp_Init(void);
static void _FMSTR_Net_Tcp_Poll(void);
static FMSTR_S32 _FMSTR_Net_Tcp_Recv(FMSTR_BPTR msgBuff, FMSTR_SIZE msgMaxSize, FMSTR_NET_ADDR *recvAddr, FMSTR_BOOL *isBroadcast);
static FMSTR_S32 _FMSTR_Net_Tcp_Send(FMSTR_NET_ADDR *sendAddr, FMSTR_BPTR msgBuff, FMSTR_SIZE msgSize);
static void _FMSTR_Net_Tcp_Close(FMSTR_NET_ADDR *addr);
static void _FMSTR_Net_Tcp_Get_Caps(FMSTR_NET_IF_CAPS *caps);
static void _FMSTR_NetAddrToFmstr(struct sockaddr *remoteAddr, FMSTR_NET_ADDR *fmstrAddr);

/******************************************************************************
 * Local variables
 ******************************************************************************/

/* TCP sessions */
static FMSTR_TCP_SESSION fmstrTcpSessions[FMSTR_TCP_SESSION_COUNT];
/* TCP listen socket */
static int tcp_listen_socket = 0;

#if FMSTR_NET_AUTODISCOVERY != 0
/* UDP Broadcast socket */
static int udp_broadcast_socket = 0;
#endif /* FMSTR_NET_AUTODISCOVERY */

/******************************************************************************
 * Driver interface
 ******************************************************************************/
/* Interface of this network TCP driver */
const FMSTR_NET_DRV_INTF fmstr_net_drv =
{
  .Init    = _FMSTR_Net_Tcp_Init,
  .Poll    = _FMSTR_Net_Tcp_Poll,
  .Recv    = _FMSTR_Net_Tcp_Recv,
  .Send    = _FMSTR_Net_Tcp_Send,
  .Close   = _FMSTR_Net_Tcp_Close,
  .GetCaps = _FMSTR_Net_Tcp_Get_Caps,
};



/*-----------------------------------------------------------------------------------------------------



  \return FMSTR_BOOL
-----------------------------------------------------------------------------------------------------*/
static FMSTR_BOOL _FMSTR_Net_Tcp_Init(void)
{
  struct sockaddr_in destAddr4;
#if FMSTR_NET_AUTODISCOVERY != 0
  struct sockaddr_in bindAddr;
#endif
  FMSTR_INDEX i;
  int err;

  FMSTR_MemSet(&fmstrTcpSessions, 0, sizeof(fmstrTcpSessions));
  FMSTR_MemSet(&destAddr4, 0, sizeof(destAddr4));
  destAddr4.sin_family = AF_INET; // TODO: prepare address for IPv4 or IPv6 ?

    /* Prepare sockets */
  for (i = 0; i < FMSTR_TCP_SESSION_COUNT; i++)
  {
    fmstrTcpSessions[i].sock = -1;
  }

    /* TCP listen port */
  destAddr4.sin_port = htons(FMSTR_NET_PORT);

    /* Create new listen socket */
  tcp_listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP); // TODO: IPv6?
  if (tcp_listen_socket < 0)
  {
    return FMSTR_FALSE;
  }

#if FMSTR_NET_BLOCKING_TIMEOUT == 0
  {
        /* Set non-blocking socket */
    int flags = fcntl(tcp_listen_socket, F_GETFL, 0);
    err       = fcntl(tcp_listen_socket, F_SETFL, flags | O_NONBLOCK);
    if (err < 0)
    {
      return FMSTR_FALSE;
    }
  }
#endif

    /* Socket bind */
  err = bind(tcp_listen_socket, (struct sockaddr *)&destAddr4, sizeof(destAddr4));
  if (err < 0)
  {
    return FMSTR_FALSE;
  }

    /* Listen */
  err = listen(tcp_listen_socket, 0);
  if (err < 0)
  {
    return FMSTR_FALSE;
  }

#if FMSTR_NET_AUTODISCOVERY != 0

  bindAddr.sin_family      = AF_INET;
  bindAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  bindAddr.sin_port        = htons(FMSTR_NET_PORT);

    /* Create new UDP listen socket */
  udp_broadcast_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
  if (udp_broadcast_socket < 0)
  {
    return FMSTR_FALSE;
  }

    /* Socket bind */
  err = bind(udp_broadcast_socket, (struct sockaddr *)&bindAddr, sizeof(bindAddr));
  if (err < 0)
  {
    return FMSTR_FALSE;
  }

#if FMSTR_NET_BLOCKING_TIMEOUT == 0
  {
        /* Set non-blocking socket */
    int flags = fcntl(udp_broadcast_socket, F_GETFL, 0);
    err       = fcntl(udp_broadcast_socket, F_SETFL, flags | O_NONBLOCK);
    if (err < 0)
    {
      return FMSTR_FALSE;
    }
  }
#endif

  fmstrTcpSessions[0].sock = udp_broadcast_socket;

#endif /* FMSTR_NET_AUTODISCOVERY */

  return FMSTR_TRUE;
}

/*-----------------------------------------------------------------------------------------------------



  \return FMSTR_TCP_SESSION*
-----------------------------------------------------------------------------------------------------*/
static FMSTR_TCP_SESSION* _FMSTR_NetLwipTcpSessionPending(void)
{
  FMSTR_INDEX i;

  for (i = 0; i < FMSTR_TCP_SESSION_COUNT; i++)
  {
        /* Find pending session */
    if (fmstrTcpSessions[i].sock >= 0 && fmstrTcpSessions[i].receivePending != FMSTR_FALSE)
    {
      return &fmstrTcpSessions[i];
    }
  }

  return NULL;
}

/*-----------------------------------------------------------------------------------------------------


  \param sendAddr

  \return FMSTR_TCP_SESSION*
-----------------------------------------------------------------------------------------------------*/
static FMSTR_TCP_SESSION* _FMSTR_NetLwipTcpSessionFind(FMSTR_NET_ADDR *sendAddr)
{
  FMSTR_INDEX i;

  for (i = 0; i < FMSTR_TCP_SESSION_COUNT; i++)
  {
    /* Find free session */
    if (sendAddr == NULL)
    {
      if (fmstrTcpSessions[i].sock < 0)
      {
        return &fmstrTcpSessions[i];
      }
    }
    /* Find session by address */
    else
    {
      if (FMSTR_MemCmp(&fmstrTcpSessions[i].address, sendAddr, sizeof(FMSTR_NET_ADDR)) == 0)
      {
        return &fmstrTcpSessions[i];
      }
    }
  }

  return NULL;
}

/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
static void _FMSTR_NetLwipTcpAccept(void)
{
  struct sockaddr remote_addr;
  INT    length;
  int    socket = 0;

  FMSTR_MemSet(&remote_addr, 0, sizeof(remote_addr));
  length = sizeof(remote_addr);

  /* Accept socket */
  socket = accept(tcp_listen_socket,&remote_addr,&length);
  if (socket >= 0)
  {
    FMSTR_TCP_SESSION *newSes;

#if FMSTR_NET_BLOCKING_TIMEOUT == 0
    /* Set non-blocking socket */
    int flags = fcntl(socket, F_GETFL, 0);
    fcntl(socket, F_SETFL, flags | O_NONBLOCK);
#endif

    newSes = _FMSTR_NetLwipTcpSessionFind(NULL);
    if (newSes != NULL)
    {
      newSes->sock = socket;
      _FMSTR_NetAddrToFmstr(&remote_addr,&newSes->address);
    }
  }
}

/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
static void _FMSTR_Net_Tcp_Poll(void)
{
  FMSTR_TCP_SESSION *session_cbl;
  FMSTR_INDEX i;

  // Выход без задержки если есть хотя бы одна сессия с открытым сокетом и ожидающая приема
  // После выхода вызвавшая задача затем сразу вызовет функцию приема.
  if (_FMSTR_NetLwipTcpSessionPending() != NULL) return;

  // Здесь если еще нет ни одной слушающей сесиии
  // Получим первую же доступную структуру для организации сессии обмена
  session_cbl = _FMSTR_NetLwipTcpSessionFind(NULL);

#if FMSTR_NET_BLOCKING_TIMEOUT == 0

  // Пытаемся получить сокет открытого соедиения
  if (session_cbl != NULL) _FMSTR_NetLwipTcpAccept();

  // Если были открыты сокеты, то отметить их как ожидающие приема
  for (i = 0; i < FMSTR_TCP_SESSION_COUNT; i++) if (fmstrTcpSessions[i].sock >= 0) fmstrTcpSessions[i].receivePending = FMSTR_TRUE;

#else
  {
    int              maxFd = 0;
    fd_set           readset;
    struct timeval   tv;

    FD_ZERO(&readset);
    if (session_cbl != NULL)
    {
      /* Listen socket */
      FD_SET(tcp_listen_socket,&readset);
      maxFd = tcp_listen_socket;
    }

    /* set timeout */
    tv.tv_sec  = FMSTR_NET_BLOCKING_TIMEOUT / 1000;
    tv.tv_usec =(FMSTR_NET_BLOCKING_TIMEOUT % 1000) * 1000;

        /* Prepare active sockets for read */
    for (i = 0; i < FMSTR_TCP_SESSION_COUNT; i++)
    {
      if (fmstrTcpSessions[i].sock >= 0)
      {
        FD_SET(fmstrTcpSessions[i].sock,&readset);

        if (maxFd < fmstrTcpSessions[i].sock)
        {
          maxFd = fmstrTcpSessions[i].sock;
        }
      }
    }

    if (select(maxFd + 1, (fd_set *)&readset, NULL, NULL,&tv) >= 1)
    {
      /* Pending accept */
      if (FD_ISSET(tcp_listen_socket,&readset))
      {
        _FMSTR_NetLwipTcpAccept();
      }

      /* Pending receive */
      for (i = 0; i < FMSTR_TCP_SESSION_COUNT; i++)
      {
        if (fmstrTcpSessions[i].sock >= 0 && FD_ISSET(fmstrTcpSessions[i].sock,&readset))
        {
          fmstrTcpSessions[i].receivePending = FMSTR_TRUE;
        }
      }
    }
  }
#endif
}

/*-----------------------------------------------------------------------------------------------------


  \param msgBuff
  \param msgMaxSize
  \param recvAddr
  \param isBroadcast

  \return FMSTR_S32
  возвращает:
  0 если нет сессий работающих на прием
 -1 если произошла ошибка прием
 >0 количество принятых байт

-----------------------------------------------------------------------------------------------------*/
static FMSTR_S32 _FMSTR_Net_Tcp_Recv(FMSTR_BPTR msgBuff, FMSTR_SIZE msgMaxSize, FMSTR_NET_ADDR *recvAddr, FMSTR_BOOL *isBroadcast)
{
  int res                = 0;
  FMSTR_TCP_SESSION *ses = NULL;

  FMSTR_ASSERT(msgBuff != NULL);
  FMSTR_ASSERT(recvAddr != NULL);
  FMSTR_ASSERT(isBroadcast != NULL);

  *isBroadcast = FMSTR_FALSE;

  if (tcp_listen_socket < 0)  return 0;

  ses = _FMSTR_NetLwipTcpSessionPending();
  if (ses == NULL) return 0;

#if FMSTR_NET_AUTODISCOVERY != 0
  // Receive UDP broadcast
  if (ses->sock == udp_broadcast_socket)
  {
    struct sockaddr remote_addr;
    INT    length = sizeof(remote_addr);

    *isBroadcast = FMSTR_TRUE;

    res = recvfrom(ses->sock, (CHAR *)msgBuff, msgMaxSize, 0,&remote_addr,&length);

    /* Copy address */
    _FMSTR_NetAddrToFmstr(&remote_addr,&ses->address);
    /* Copy address */
    FMSTR_MemCpy(recvAddr,&ses->address, sizeof(FMSTR_NET_ADDR));
  }
  else
#endif /* FMSTR_NET_AUTODISCOVERY */

  {
    res = recv(ses->sock, msgBuff, msgMaxSize, 0);
    FMSTR_MemCpy(recvAddr,&ses->address, sizeof(FMSTR_NET_ADDR));
  }
#if FMSTR_NET_BLOCKING_TIMEOUT != 0
  if (res == 0)
  {
    res = -1;
  }
#else
  if (res < 0 && errno == EWOULDBLOCK)
  {
    res = 0;
  }

  if (res == 0)
  {
    ses->receivePending = FMSTR_FALSE;
  }
#endif

  if (res < 0 && errno == EWOULDBLOCK)
  {
    res = 0;
  }

  if (res == 0)
  {
    ses->receivePending = FMSTR_FALSE;
  }

  return res;
}

/*-----------------------------------------------------------------------------------------------------


  \param sendAddr
  \param msgBuff
  \param msgSize

  \return FMSTR_S32
-----------------------------------------------------------------------------------------------------*/
static FMSTR_S32 _FMSTR_Net_Tcp_Send(FMSTR_NET_ADDR *sendAddr, FMSTR_BPTR msgBuff, FMSTR_SIZE msgSize)
{
  FMSTR_TCP_SESSION *ses = NULL;
  int res                = 0;

  FMSTR_ASSERT(msgBuff != NULL);
  FMSTR_ASSERT(sendAddr != NULL);

    // TODO: what should do, when async discovery want to send (not found session)

    /* Find session by address */
  ses = _FMSTR_NetLwipTcpSessionFind(sendAddr);
  if (ses == NULL)
  {
        /* Same as socket error */
    return -1;
  }

    /* This session is not pending now */
  ses->receivePending = FMSTR_FALSE;

#if FMSTR_NET_AUTODISCOVERY != 0
    /* Receive UDP broadcast */
  if (ses->sock == udp_broadcast_socket)
  {
    struct sockaddr_in destAddr4;
    FMSTR_MemSet(&destAddr4, 0, sizeof(destAddr4));
    destAddr4.sin_family = AF_INET;
    destAddr4.sin_port   = htons(sendAddr->port);
    FMSTR_MemCpy(&destAddr4.sin_addr.s_addr, sendAddr->addr.v4, 4);

        /* Send data */
    res = sendto(ses->sock, (CHAR *)msgBuff, msgSize, 0, (struct sockaddr *)&destAddr4, sizeof(destAddr4));
  }
  else
#endif
  {
        /* Send data */
    res = send(ses->sock, (CHAR *)msgBuff, msgSize, 0);
  }

  return res;
}

/*-----------------------------------------------------------------------------------------------------


  \param addr
-----------------------------------------------------------------------------------------------------*/
static void _FMSTR_Net_Tcp_Close(FMSTR_NET_ADDR *addr)
{
  FMSTR_TCP_SESSION *ses = NULL;

    /* Find session by address */
  ses = _FMSTR_NetLwipTcpSessionFind(addr);
  if (ses == NULL)
  {
        /* Session not found */
    return;
  }

#if FMSTR_NET_AUTODISCOVERY != 0
  if (ses->sock == udp_broadcast_socket)
  {
        /* Broadcast session cannot be closed */
    return;
  }
#endif

    /* Close socket */
  (void)soc_close(ses->sock);

  FMSTR_MemSet(ses, 0, sizeof(FMSTR_TCP_SESSION));
  ses->sock = -1;
}

/*-----------------------------------------------------------------------------------------------------


  \param caps
-----------------------------------------------------------------------------------------------------*/
static void _FMSTR_Net_Tcp_Get_Caps(FMSTR_NET_IF_CAPS *caps)
{
  FMSTR_ASSERT(caps != NULL);

  caps->flags |= FMSTR_NET_IF_CAPS_FLAG_TCP;
}

/*-----------------------------------------------------------------------------------------------------


  \param remoteAddr
  \param fmstrAddr
-----------------------------------------------------------------------------------------------------*/
static void _FMSTR_NetAddrToFmstr(struct sockaddr *remoteAddr, FMSTR_NET_ADDR *fmstrAddr)
{
  FMSTR_ASSERT(remoteAddr != NULL);
  FMSTR_ASSERT(fmstrAddr != NULL);

  if ((((struct sockaddr *)remoteAddr)->sa_family & AF_INET) != 0U)
  {
    struct sockaddr_in *in = (struct sockaddr_in *)remoteAddr;
    fmstrAddr->type        = FMSTR_NET_ADDR_TYPE_V4;
    FMSTR_MemCpy(fmstrAddr->addr.v4,&in->sin_addr.s_addr, sizeof(fmstrAddr->addr.v4));
    fmstrAddr->port = htons(in->sin_port);
  }
#if LWIP_IPV6
  else if ((((struct sockaddr *)remoteAddr)->sa_family & AF_INET6) != 0U)
  {
    struct sockaddr_in6 *in6 = (struct sockaddr_in6 *)remoteAddr;

    fmstrAddr->type = FMSTR_NET_ADDR_TYPE_V6;
    FMSTR_MemCpy(fmstrAddr->addr.v6,&(in6->sin6_addr.s6_addr), sizeof(fmstrAddr->addr.v6));
    fmstrAddr->port = htons(in6->sin6_port);
  }
#endif
}



