/*******************************************************************************
*
* Copyright 2004-2014 Freescale Semiconductor, Inc.
*
* This software is owned or controlled by Freescale Semiconductor.
* Use of this software is governed by the Freescale FreeMASTER License
* distributed with this Material.
* See the LICENSE file distributed for more details.
*
****************************************************************************/
/*!
*
* @brief  FreeMASTER serial communication routines
*
*******************************************************************************/

#include "App.h"
#include "freemaster.h"
#include "freemaster_private.h"
#include "freemaster_protocol.h"
#include "freemaster_utils.h"

extern T_serial_io_driver *frm_drv;

/* FreeMASTER communication buffer (in/out) plus the STS, LEN(LEB) and CRC bytes */
static uint8_t fmstr_pCommBuffer[FMSTR_COMM_BUFFER_SIZE + 1 + 4 + 2];

/* FreeMASTER runtime flags */
/*lint -e{960} using union */
typedef volatile union
{
  uint8_t all;

  struct
  {
    unsigned bTxActive        : 1; /* response is being transmitted */
    unsigned bTxWaitTC        : 1; /* response sent, wait for transmission complete */
    unsigned bTxLastCharSOB   : 1; /* last transmitted char was equal to SOB  */
    unsigned bRxLastCharSOB   : 1; /* last received character was SOB */
    unsigned bRxMsgLengthNext : 1; /* expect the length byte next time */
    unsigned bTxFirstSobSend  : 1; /* to send SOB char at the begin of the packet */
  } flg;

} FMSTR_SERIAL_FLAGS;

static FMSTR_SERIAL_FLAGS _fmstr_wFlags;

/* receive and transmit buffers and counters */
static uint8_t fmstr_nTxTodo;  /* transmission to-do counter (0 when tx is idle) */
static uint8_t fmstr_nRxTodo;  /* reception to-do counter (0 when rx is idle) */
static uint8_t *fmstr_pTxBuff; /* pointer to next byte to transmit */
static uint8_t *fmstr_pRxBuff; /* pointer to next free place in RX buffer */
static uint8_t fmstr_nRxCrc8;  /* checksum of data being received */

static void    FMSTR_Listen(void);
static void    _FMSTR_SendError(uint8_t nErrCode);
static void    FMSTR_Process_OS_IO(void);

static int32_t FMSTR_Tx(uint8_t *pTxChar);
static int32_t FMSTR_Rx(uint8_t rx_char);

static FMSTR_BOOL _FMSTR_SerialInit(void);
static void       _FMSTR_SerialPoll(void);
static void       _FMSTR_SerialSendResponse(FMSTR_BPTR pResponse, FMSTR_SIZE nLength, FMSTR_U8 statusCode);

/* Interface of this serial driver */
const FMSTR_TRANSPORT_INTF FMSTR_SERIAL =
{
    FMSTR_C99_INIT(Init) _FMSTR_SerialInit,
    FMSTR_C99_INIT(Poll) _FMSTR_SerialPoll,
    FMSTR_C99_INIT(SendResponse) _FMSTR_SerialSendResponse,
};

#define FREEM_TX_BUFF_SZ 64
static uint8_t out_buff[FREEM_TX_BUFF_SZ];
/*******************************************************************************
*
* @brief    Handle OS IO serial communication (both TX and RX)
*
* This function calls MQX IO fread() function to get character and process it by
*
* FMSTR_Rx function when FreeMASTER packet is receiving. This function also transmit
*
* FreeMASTER response. Character to be send is provided by call of FMSTR_Tx function
*
* and passed down to fwrite() function.
*
*******************************************************************************/

static void FMSTR_Process_OS_IO(void)
{

  int status;
  if (!_fmstr_wFlags.flg.bTxActive)
  {
    uint8_t rx_char;
    do
    {
      status = frm_drv->_wait_char(&rx_char, 2);
      if (status == RES_OK)
      {
        //ITM_EVENT8(1, rx_char);

        if (FMSTR_Rx(rx_char))
          break;
      }
    } while (1);
  }


  if (_fmstr_wFlags.flg.bTxActive)
  {
    uint8_t tx_char;
    uint32_t cnt = 0;
    // Набираем в промежуточный буфер данные посылаемые движком FreeMaster
    while (1)
    {
      // Извлекаем очередной байт из буффера данных предназначенных для отправки
      if (FMSTR_Tx((uint8_t *)&tx_char) == FMSTR_TRUE)
      {
        // Здесь если больше нет данных для оправки в буфере движка FreeMaster
        if (cnt > 0)
        {
          // Отсылаем накопленные данные из промежуточного буфера
          frm_drv->_send_buf(out_buff, cnt);
        }
        break;
      }
      //ITM_EVENT8(2, tx_char);

      // Накапливаем данные в промежуточный буффер, чтобы не заставлять драйвер отправлять данные по одному байту
      out_buff[cnt] = tx_char;
      cnt++;
      if (cnt >= FREEM_TX_BUFF_SZ)
      {
        // Отправляем то, что накопили в промежуточный буффер
        frm_drv->_send_buf(out_buff, cnt);
        cnt = 0;
      }
    }
  }
}

/**************************************************************************/ /*!
*
* @brief    Start listening on a serial line
*
* Reset the receiver machine and start listening on a serial line
*
******************************************************************************/

static void FMSTR_Listen(void)
{
  fmstr_nRxTodo = 0U;
  /* disable transmitter state machine */
  _fmstr_wFlags.flg.bTxActive = 0U;
}

/**************************************************************************/ /*!
*
* @brief    Send response of given error code (no data)
*
* @param    nErrCode - error code to be sent
*
******************************************************************************/

static void _FMSTR_SendError(uint8_t nErrCode)
{
  /* fill & send single-byte response */
  FMSTR_SendResponse(&fmstr_pCommBuffer[2], 0U, nErrCode);
}

/**************************************************************************/ /*!
*
* @brief    Finalize transmit buffer before transmitting
*
* @param    nLength - response length (1 for status + data nLength)
*
*
* This Function takes the data already prepared in the transmit buffer
* (inlcuding the status byte). It computes the check sum and kicks on tx.
*
******************************************************************************/

static void  _FMSTR_SerialSendResponse(FMSTR_BPTR pResponse, FMSTR_SIZE nLength, FMSTR_U8 statusCode)
{
  uint8_t i;
  uint8_t c;

  if (nLength > 254 || pResponse != &fmstr_pCommBuffer[2])
  {
    /* The Serial driver doesn't support bigger responses than 254 bytes, change the response to status error */
    statusCode = 0x84U;
    nLength = 0;
  }

  /* remember the buffer to be sent */
  fmstr_pTxBuff = fmstr_pCommBuffer;
  /* send the message with status, length and the checksum and the SOB */
  fmstr_nTxTodo = (FMSTR_SIZE8)(nLength + 3);

  if (statusCode & FMSTR_STSF_VARLEN)
  {
    fmstr_pCommBuffer[0] = (FMSTR_BCHR)statusCode;
    fmstr_pCommBuffer[1] = (FMSTR_BCHR)nLength;
  }
  else
  {
    fmstr_pCommBuffer[1] = (FMSTR_BCHR)statusCode;
    fmstr_pTxBuff++;
    fmstr_nTxTodo--;
  }

  /* Initialize CRC algorithms */
  FMSTR_Crc8Init(&fmstr_nRxCrc8);

  /* status byte and data are already there, compute checksum only     */
  pResponse = fmstr_pTxBuff;
  for (i = 0U; i < (fmstr_nTxTodo - 1); i++)
  {
    pResponse = FMSTR_ValueFromBuffer8(&c, pResponse);
    /* add character to checksum */
    FMSTR_Crc8AddByte(&fmstr_nRxCrc8, c);
  }

  /* store checksum after the message */
  pResponse = FMSTR_ValueToBuffer8(pResponse, fmstr_nRxCrc8);

  /* now transmitting the response */
  _fmstr_wFlags.flg.bTxActive = 1U;
  _fmstr_wFlags.flg.bTxWaitTC = 0U;

  /* do not replicate the initial SOB  */
  _fmstr_wFlags.flg.bTxLastCharSOB = 0U;
  _fmstr_wFlags.flg.bTxFirstSobSend = 1;
}

/**************************************************************************/ /*!
*
* @brief    Output buffer transmission
*
* @param  pTxChar  The character to be transmit
*
* get ready buffer(prepare data to send)
*
******************************************************************************/

static int32_t FMSTR_Tx(uint8_t *pTxChar)
{
  /* to send first SOB byte*/
  if (_fmstr_wFlags.flg.bTxFirstSobSend)
  {
    *pTxChar = FMSTR_SOB;
    _fmstr_wFlags.flg.bTxFirstSobSend = 0U;
    return FMSTR_FALSE;
  }
  if (fmstr_nTxTodo)
  {
    /* fetch & send character ready to transmit */
    /*lint -e{534} ignoring return value */
    (void)FMSTR_ValueFromBuffer8(pTxChar, fmstr_pTxBuff);

    /* first, handle the replicated SOB characters */
    if (*pTxChar == FMSTR_SOB)
    {
      _fmstr_wFlags.flg.bTxLastCharSOB ^= 1U;
      if ((_fmstr_wFlags.flg.bTxLastCharSOB))
      {
        /* yes, repeat the SOB next time */
        return FMSTR_FALSE;
      }
    }
    /* no, advance tx buffer pointer */
    fmstr_nTxTodo--;
    fmstr_pTxBuff = FMSTR_SkipInBuffer(fmstr_pTxBuff, 1U);
    return FMSTR_FALSE;
  }

  /* start listening immediately */
  FMSTR_Listen();

  return FMSTR_TRUE;
}

/**************************************************************************/ /*!
*
* @brief  Handle received character
*
* @param  nRxChar  The character to be processed
*
* Handle the character received and -if the message is complete- call the
* protocol decode routine.
*
******************************************************************************/

static int32_t FMSTR_Rx(uint8_t rxChar)
{
  FMSTR_SERIAL_FLAGS *pflg = &_fmstr_wFlags;
  /* first, handle the replicated SOB characters */
  if (rxChar == FMSTR_SOB)
  {
    pflg->flg.bRxLastCharSOB ^= 1;
    if (pflg->flg.bRxLastCharSOB)
    {
      /* this is either the first byte of replicated SOB or a  */
      /* real Start-of-Block mark - we will decide next time in FMSTR_Rx */
      return FMSTR_FALSE;
    }
  }

  /* we have got a common character preceded by the SOB -  */
  /* this is the command code! */
  if (pflg->flg.bRxLastCharSOB)
  {
    /* reset receiving process */
    fmstr_pRxBuff = fmstr_pCommBuffer;

    FMSTR_Crc8Init(&fmstr_nRxCrc8);
    FMSTR_Crc8AddByte(&fmstr_nRxCrc8, rxChar);

    *(fmstr_pRxBuff++) = rxChar;
    fmstr_nRxTodo = 0;

    /* if the standard command was received, the message length will come in next byte */
    pflg->flg.bRxMsgLengthNext = 1U;

    /* command code stored & processed */
    pflg->flg.bRxLastCharSOB = 0U;
    return FMSTR_FALSE;
  }

  /* we are waiting for the length byte */
  if (pflg->flg.bRxMsgLengthNext)
  {
    /* total data length and the checksum */
    fmstr_nRxTodo = (FMSTR_SIZE8)(rxChar + 1);
    FMSTR_Crc8AddByte(&fmstr_nRxCrc8, rxChar);

    *(fmstr_pRxBuff++) = rxChar;

    /* now read the data bytes */
    pflg->flg.bRxMsgLengthNext = 0U;

    return FMSTR_FALSE;
  }

  /* waiting for a data byte? */
  if (fmstr_nRxTodo)
  {
    /* decrease number of expected bytes */
    fmstr_nRxTodo--;
    /* was it the last byte of the message (checksum)? */
    if (!fmstr_nRxTodo)
    {
      /* receive buffer overflow? */
      if (fmstr_pRxBuff == NULL)
      {
        _FMSTR_SendError(FMSTR_STC_CMDTOOLONG);
      }
      /* checksum error? */
      else if (fmstr_nRxCrc8 != rxChar)
      {
        _FMSTR_SendError(FMSTR_STC_CMDCSERR);
      }
      /* message is okay */
      else
      {
        FMSTR_BPTR pMessageIO = fmstr_pCommBuffer;
        FMSTR_U8 nCmd, nSize;

        /* command code comes first in the message */
        /*lint -e{534} return value is not used */
        pMessageIO = FMSTR_ValueFromBuffer8(&nCmd, pMessageIO);
        /* length of command follows */
        /*lint -e{534} return value is not used */
        pMessageIO = FMSTR_ValueFromBuffer8(&nSize, pMessageIO);

        /* do decode now! */
        FMSTR_ProtocolDecoder(pMessageIO, nSize, nCmd);
      }

      return FMSTR_TRUE;
    }
    /* not the last character yet */
    else
    {
      /* add this byte to checksum */
      FMSTR_Crc8AddByte(&fmstr_nRxCrc8, rxChar);

      /* is there still a space in the buffer? */
      if (fmstr_pRxBuff)
      {
        /*lint -e{946} pointer arithmetic is okay here (same array) */
        if (fmstr_pRxBuff < (fmstr_pCommBuffer + FMSTR_COMM_BUFFER_SIZE))
        {
          /* store byte  */
          *fmstr_pRxBuff++ = rxChar;
        }
        /* buffer is full! */
        else
        {
          /* NULL rx pointer means buffer overflow - but we still need */
          /* to receive all message characters (for the single-wire mode) */
          /* so keep "receiving" - but throw away all characters from now */
          fmstr_pRxBuff = NULL;
        }
      }
    }
  }
  return FMSTR_FALSE;
}

/**************************************************************************/ /*!
*
* @brief    Serial communication initialization
*
******************************************************************************/
static FMSTR_BOOL _FMSTR_SerialInit(void)
{
  /* initialize all state variables */
  _fmstr_wFlags.all = 0U;
  fmstr_nTxTodo = 0U;

  /* start listening for commands */
  FMSTR_Listen();
  return FMSTR_TRUE;
}

/*******************************************************************************
*
* @brief    API: Main "Polling" call from the application main loop
*
* This function either handles all the SCI communication (polling-only mode =
* FMSTR_POLL_DRIVEN) or decodes messages received on the background by SCI interrupt
* (short-interrupt mode = FMSTR_SHORT_INTR).
*
* In the JTAG interrupt-driven mode (both short and long), this function also checks
* if setting the JTAG RIE bit failed recently. This may happen because of the
* RIE is held low by the EONCE hardware until the EONCE is first accessed from host.
* FMSTR_Init (->FMSTR_Listen) is often called while the PC-side FreeMASTER is still
* turned off. So really, the JTAG is not enabled by this time and RIE bit is not set.
* This problem is detected (see how bJtagRIEPending is set above in FSMTR_Listen)
* and it is tried to be fixed periodically here in FMSTR_Poll.
*
*******************************************************************************/

static void _FMSTR_SerialPoll(void)
{
  /* polled OS IO mode */
  FMSTR_Process_OS_IO();
}
