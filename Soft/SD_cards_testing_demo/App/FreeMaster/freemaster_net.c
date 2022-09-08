#include "freemaster.h"
#include "freemaster_private.h"
#include "freemaster_netx_tcp.h"
#include "freemaster_protocol.h"
#include "freemaster_net.h"
#include "freemaster_utils.h"


extern const FMSTR_NET_DRV_INTF    fmstr_net_drv;


/* Offset command payload in network buffer */
#define FMSTR_NET_PAYLOAD_OFFSET 6

typedef struct FMSTR_NET_SESSION_S
{
    FMSTR_U32 lastUsed;     /* Last used session */
    FMSTR_NET_ADDR address; /* TCP/UDP address */
}
FMSTR_NET_SESSION;



static FMSTR_NET_SESSION    fmstr_pNetSessions[FMSTR_SESSION_COUNT];
static FMSTR_U32            fmstr_nSessionCounter = 0U;


static FMSTR_BCHR fmstr_pNetBuffer[FMSTR_COMM_BUFFER_SIZE + 2 + 1 + 1 + 2 + 1];  // FreeMASTER communication buffer (in/out) plus the Length, sequence number, command, payload and CRC

static FMSTR_SIZE received_bytes_cnt = 0U;
static FMSTR_U8   sequence_number    = 1U;


static FMSTR_BOOL         _FMSTR_NetInit(void);
static void               _FMSTR_NetPoll(void);
static void               _FMSTR_NetSendResponse(FMSTR_BPTR pResponse, FMSTR_SIZE nLength, FMSTR_U8 statusCode, void *identification);
static FMSTR_NET_SESSION* _FMSTR_FindNetSession(FMSTR_NET_ADDR *addr, FMSTR_BOOL create);
static void               _FMSTR_NetCloseSession(FMSTR_NET_SESSION *ses);
static FMSTR_BOOL         _FMSTR_NetProcess(void);
static void               _FMSTR_NetSendStatus(FMSTR_BCHR nErrCode, FMSTR_NET_SESSION *session);


#if FMSTR_NET_AUTODISCOVERY != 0
static void _FMSTR_NetSendDiscovery(const FMSTR_NET_ADDR *address);
#endif


const FMSTR_TRANSPORT_INTF FMSTR_NET =
{
  FMSTR_C99_INIT(Init) _FMSTR_NetInit,
  FMSTR_C99_INIT(Poll) _FMSTR_NetPoll,
  FMSTR_C99_INIT(SendResponse) _FMSTR_NetSendResponse,
};

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return FMSTR_BOOL
-----------------------------------------------------------------------------------------------------*/
static FMSTR_BOOL _FMSTR_NetInit(void)
{
  FMSTR_MemSet(&fmstr_pNetSessions, 0, sizeof(fmstr_pNetSessions));

  if (fmstr_net_drv.Init() == FMSTR_FALSE) return FMSTR_FALSE;

  return FMSTR_TRUE;
}

/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
static void _FMSTR_NetPoll(void)
{
  FMSTR_BOOL res = FMSTR_FALSE;
  do
  {
    fmstr_net_drv.Poll();
    res = _FMSTR_NetProcess();
  } while (res == FMSTR_FALSE);
}

/*-----------------------------------------------------------------------------------------------------


  \param pResponse
  \param nLength
  \param statusCode
  \param identification
-----------------------------------------------------------------------------------------------------*/
static void _FMSTR_NetSendResponse(FMSTR_BPTR pResponse, FMSTR_SIZE nLength, FMSTR_U8 statusCode, void *identification)
{
  FMSTR_U16 i;
  FMSTR_U8 c;
  FMSTR_U16 todo;
  FMSTR_U16 sent   = 0U;
  FMSTR_S32 res    = 1;
  FMSTR_BCHR chSum = 0U;
  FMSTR_BPTR pMessageIO;

  FMSTR_NET_SESSION *session = (FMSTR_NET_SESSION *)identification;
  FMSTR_ASSERT(session != NULL);

  if ((nLength > (FMSTR_SIZE)(FMSTR_COMM_BUFFER_SIZE)) || pResponse != &fmstr_pNetBuffer[FMSTR_NET_PAYLOAD_OFFSET])
  {
    // The Network driver doesn't support bigger responses than FMSTR_COMM_BUFFER_SIZE bytes, change the response to status error
    statusCode = FMSTR_STC_RSPBUFFOVF;
    nLength    = 0U;
  }

  pMessageIO =&fmstr_pNetBuffer[0];

    /* Send the message with status, length and checksum. */
  todo = (FMSTR_U16)(nLength + 7U);

    /* Total frame length */
  pMessageIO = FMSTR_ValueToBuffer16BE(pMessageIO, todo);

    /* Sequence number */
  pMessageIO = FMSTR_ValueToBuffer8(pMessageIO, sequence_number);

    /* Status code */
  pMessageIO = FMSTR_ValueToBuffer8(pMessageIO, (FMSTR_BCHR)statusCode);

    /* Response length */
  pMessageIO = FMSTR_ValueToBuffer16BE(pMessageIO, (FMSTR_U16)nLength);

    /* Initialize CRC algorithms */
  FMSTR_Crc8Init(&chSum);

    /* Checksum CRC8 */
  pResponse =&fmstr_pNetBuffer[3];
  for (i = 0; i < nLength + 3U; i++)
  {
        /* Get from FMSTR buffer */
    pResponse = FMSTR_ValueFromBuffer8(&c, pResponse);

        /* add character to checksum */
    FMSTR_Crc8AddByte(&chSum, c);
  }

    /* Store checksum after the message */
  pResponse = FMSTR_ValueToBuffer8(pResponse, chSum);

    /* Send via network */
  while (res > 0 && sent < todo)
  {
    res = fmstr_net_drv.Send(&session->address,&fmstr_pNetBuffer[sent], todo - sent);

    if (res < 0)
    {
            /* Socket error condition */
      _FMSTR_NetCloseSession(session);
      return;
    }

    sent += (FMSTR_U16)res;
  }
}

/*-----------------------------------------------------------------------------------------------------


  This function returns TRUE when network transmission is complete.
  It returns FALSE when received middle of the frame (TCP) and needs to be
  called asap after any new data are received.

  \return FMSTR_BOOL
-----------------------------------------------------------------------------------------------------*/
static FMSTR_BOOL _FMSTR_NetProcess(void)
{
  FMSTR_BCHR    crc = 0U, recv_crc;
  FMSTR_U16     i;
  FMSTR_U8      c;
  int           received   = 0;
  FMSTR_U16     todo = 0;
  FMSTR_BPTR    msg_ptr;
  FMSTR_BPTR    command_payload_ptr;
  FMSTR_BPTR    crc_ptr;
  FMSTR_U8      command_code = 0;
  FMSTR_U16     command_len;
  FMSTR_BOOL    isBroadcast = FMSTR_FALSE;

  FMSTR_NET_SESSION *session = NULL;
  FMSTR_NET_ADDR address;
  FMSTR_MemSet(&address, 0, sizeof(address));


  received = fmstr_net_drv.Recv(&fmstr_pNetBuffer[received_bytes_cnt],(FMSTR_COMM_BUFFER_SIZE + 7)- received_bytes_cnt,&address,&isBroadcast);

  if (received < 0)
  {
    // В случае ошибки приема закрыть сессию
    session = _FMSTR_FindNetSession(&address, FMSTR_FALSE);
    if (session != NULL) _FMSTR_NetCloseSession(session);
  }

  if (received <= 0) return FMSTR_TRUE;            // Выход если была ошибка приема или если ничего не принято

  received_bytes_cnt += (FMSTR_SIZE)received;
  if (isBroadcast == FMSTR_FALSE) session = _FMSTR_FindNetSession(&address, FMSTR_TRUE);

  msg_ptr =&fmstr_pNetBuffer[0];
  if (received_bytes_cnt < 2U)  return FMSTR_FALSE; // Выходим если слишком короткий пакет

  msg_ptr = FMSTR_ValueFromBuffer16BE(&todo, msg_ptr);
  if (todo > ((FMSTR_U16)FMSTR_COMM_BUFFER_SIZE + 7U))
  {
    if (session != NULL) _FMSTR_NetCloseSession(session);
    return FMSTR_TRUE;                             // Выходим если надо принять больше байт чем поместится в буфер
  }

  if (received_bytes_cnt < todo) return FMSTR_FALSE;

  FMSTR_Crc8Init(&crc);                                         // Инициализация расчитываемой CRC
  msg_ptr = FMSTR_ValueFromBuffer8(&sequence_number, msg_ptr);  // Получить порядковый номер пакета
  crc_ptr = msg_ptr;                                            // Запомнить адрес с кторого начинается расчет CRC
  msg_ptr = FMSTR_ValueFromBuffer8(&command_code, msg_ptr);
  msg_ptr = FMSTR_ValueFromBuffer16BE(&command_len, msg_ptr);

  if (todo != (command_len + 7U))
  {
    if (session != NULL) _FMSTR_NetCloseSession(session);       // Тут выходим потому что неправильная длина команды
    return FMSTR_TRUE;
  }
  command_payload_ptr = msg_ptr;
  msg_ptr = FMSTR_SkipInBuffer(msg_ptr, command_len);           // Устанавливаем указатель на поле CRC в пакете
  msg_ptr = FMSTR_ValueFromBuffer8(&recv_crc, msg_ptr);         // Читаем CRC из пакета

  // Пересчитываем CRC данных в пакете
  for (i = 0; i < command_len + 3U; i++)
  {
    crc_ptr = FMSTR_ValueFromBuffer8(&c, crc_ptr);
    FMSTR_Crc8AddByte(&crc, c);
  }

  if (recv_crc == crc)
  {
    // Здесь если CRC правильное
    if (command_code == FMSTR_NET_PING)
    {
      if (isBroadcast == FMSTR_FALSE) _FMSTR_NetSendStatus(FMSTR_STS_OK, session);  /* PING response to unicast request. */
    }
#if FMSTR_NET_AUTODISCOVERY != 0

    else if (command_code == FMSTR_NET_DISCOVERY)   /* Network auto-discovery command */
    {
      _FMSTR_NetSendDiscovery(&address);            /* Send discovery (also to broadcast) */
      if (session != NULL) session->lastUsed = 0U;  /* Invalidate the session immediatelly, so it is reused next time */
    }
#endif
    else
    {
      if (isBroadcast == FMSTR_FALSE)
      {
        (void)FMSTR_ProtocolDecoder(command_payload_ptr, command_len, command_code, session);
      }
    }
  }
  else
  {
    /* Report error only for unicast communiction */
    if (isBroadcast == FMSTR_FALSE)
    {
      _FMSTR_NetSendStatus(FMSTR_STC_CMDCSERR, session);
    }
  }

  received_bytes_cnt = 0U;

  return FMSTR_TRUE;
}

#if FMSTR_NET_AUTODISCOVERY != 0

/*-----------------------------------------------------------------------------------------------------


  \param address
-----------------------------------------------------------------------------------------------------*/
static void _FMSTR_NetSendDiscovery(const FMSTR_NET_ADDR *address)
{
  FMSTR_NET_IF_CAPS caps;
  FMSTR_NET_SESSION session;
  FMSTR_BPTR pMessageIO;
  FMSTR_U8 nameLen  = 0;
  FMSTR_U8 protocol = 0;

    /* Get protocol from low-level */
  FMSTR_MemSet(&caps, 0, sizeof(caps));
  fmstr_net_drv.GetCaps(&caps);
  if ((caps.flags & FMSTR_NET_IF_CAPS_FLAG_UDP) != 0U)
  {
    protocol = (FMSTR_U8)FMSTR_NET_PROTOCOL_UDP;
  }
  else if ((caps.flags & FMSTR_NET_IF_CAPS_FLAG_TCP) != 0U)
  {
    protocol = (FMSTR_U8)FMSTR_NET_PROTOCOL_TCP;
  }
  else
  {
    FMSTR_ASSERT(FMSTR_FALSE);
  }

  FMSTR_MemCpy(&session.address, address, sizeof(FMSTR_NET_ADDR));

    /* Lenght of board name */
  nameLen = (FMSTR_U8)(FMSTR_StrLen(FMSTR_APPLICATION_STR)+ 1U);
  if (nameLen > ((FMSTR_U8)FMSTR_COMM_BUFFER_SIZE - 3U))
  {
    nameLen = (FMSTR_U8)FMSTR_COMM_BUFFER_SIZE - 3U;
  }

  pMessageIO =&fmstr_pNetBuffer[6];

    /* Discovery command version */
  pMessageIO = FMSTR_ValueToBuffer8(pMessageIO, FMSTR_NET_DISCOVERY_VERSION);

    /* Protocol (TCP/UDP) */
  pMessageIO = FMSTR_ValueToBuffer8(pMessageIO, protocol);

    /* Length of board name */
  pMessageIO = FMSTR_ValueToBuffer8(pMessageIO, nameLen);

    /* Copy name */
  pMessageIO = FMSTR_CopyToBuffer(pMessageIO, (FMSTR_ADDR)(char *)FMSTR_APPLICATION_STR, nameLen);

    /* send response */
  _FMSTR_NetSendResponse(&fmstr_pNetBuffer[6], (FMSTR_SIZE)nameLen + 3U, FMSTR_STS_OK,&session);
}
#endif /* FMSTR_NET_AUTODISCOVERY */

static void _FMSTR_NetSendStatus(FMSTR_BCHR nErrCode, FMSTR_NET_SESSION *session)
{
    /* fill & send single-byte response */
  _FMSTR_NetSendResponse(&fmstr_pNetBuffer[6], 0U, nErrCode, session);
}

/*-----------------------------------------------------------------------------------------------------


  \param ses
-----------------------------------------------------------------------------------------------------*/
static void _FMSTR_NetCloseSession(FMSTR_NET_SESSION *ses)
{
  FMSTR_ASSERT(ses != NULL);

  /* Close socket */
  fmstr_net_drv.Close(&ses->address);

  /* Free protocol session, if closed socket */
  FMSTR_FreeSession(ses);
}

/*-----------------------------------------------------------------------------------------------------


  \param addr
  \param create

  \return FMSTR_NET_SESSION*
-----------------------------------------------------------------------------------------------------*/
static FMSTR_NET_SESSION* _FMSTR_FindNetSession(FMSTR_NET_ADDR *addr, FMSTR_BOOL create)
{
  FMSTR_NET_SESSION *ses           = NULL;
  FMSTR_NET_SESSION *freeSession   = NULL;
  FMSTR_NET_SESSION *oldestSession = NULL;
  FMSTR_INDEX i;

  FMSTR_ASSERT(addr != NULL);

  fmstr_nSessionCounter++;

  for (i = 0; i < FMSTR_SESSION_COUNT; i++)
  {
    ses =&fmstr_pNetSessions[i];

    /* Find session by address */
    if (FMSTR_MemCmp(&ses->address, addr, sizeof(FMSTR_NET_ADDR)) == 0)
    {
      ses->lastUsed = fmstr_nSessionCounter;
      return ses;
    }

    /* Find free session */
    if (freeSession == NULL && ses->lastUsed == 0U)  freeSession = ses;

    /* Find oldest session */
    if (oldestSession == NULL || oldestSession->lastUsed > ses->lastUsed) oldestSession = ses;
  }

  ses =(freeSession != NULL ? freeSession : oldestSession);

  if (ses != NULL && create != FMSTR_FALSE)
  {
    /* If reusing last used session, call protocol to free this session */
    if (ses->lastUsed != 0U)
    {
      _FMSTR_NetCloseSession(ses);
    }

    FMSTR_MemCpy(&ses->address, addr, sizeof(FMSTR_NET_ADDR));
    ses->lastUsed = fmstr_nSessionCounter;
  }

  return ses;
}


