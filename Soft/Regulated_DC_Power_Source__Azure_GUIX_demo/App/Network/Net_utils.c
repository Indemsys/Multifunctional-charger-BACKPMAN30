// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2019.07.01
// 18:43:12
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"
#include   "nxd_ftp_server.h"

#define IP_ADDR_LEN        4
#define IP6_ADDR_LEN       16

static TX_MUTEX   net_mutex;


/*-----------------------------------------------------------------------------------------------------
  Создание вспомогательного мьютекса

  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Prepare_net_mutex(void)
{
  return (uint32_t)tx_mutex_create(&net_mutex, "App net", TX_INHERIT);
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Get_net_mutex(uint32_t delay_ms)
{
  return tx_mutex_get(&net_mutex, MS_TO_TICKS(delay_ms));
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Put_net_mutex(void)
{
  return tx_mutex_put(&net_mutex);
}



/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
void Print_packet_poll_statistic_to_VT100(void)
{
  GET_MCBL;
  ULONG total_packets;
  ULONG free_packets;
  ULONG empty_pool_requests;
  ULONG empty_pool_suspensions;
  ULONG invalid_packet_releases;

  nx_packet_pool_info_get(&net_packet_pool,&total_packets,&free_packets,&empty_pool_requests,&empty_pool_suspensions,&invalid_packet_releases);

  MPRINTF(VT100_CLL_FM_CRSR"Packets: Total: %3d, Free: %3d, Empty req.: %3d, Empty susp.: %3d, Invalid: %3d\r\n",total_packets, free_packets, empty_pool_requests, empty_pool_suspensions, invalid_packet_releases);

}
/*-----------------------------------------------------------------------------------------------------


  \param src
  \param dst

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
int32_t Str_to_IP_v4(const char *src, uint8_t *dst)
{
  static char digits[] = "0123456789";
  INT         saw_digit = 0,
    octets = 0,
    ch;
  uint8_t       tmp[IP_ADDR_LEN],
    *tp;
  char        *pch;
  uint8_t       new_char;

  /* Initialize the pointer and the array */
  *(tp = tmp)= 0;

  while ((ch =*src++) != '\0')
  {
    /* Compare the character with the known characters for which
       we are looking.
     */
    pch = strchr(digits, ch);

    if (pch != NULL)
    {
      /* Convert the ASCII characters into it's numerical
          equivalent.
       */
      new_char = (uint8_t)(*tp * 10 +(pch - digits));


      /* Copy the number into a temporary buffer */
      *tp = new_char;
      if (!saw_digit)
      {
        /* If we have encountered too many period delimiters,
            error out.
         */
        if (++octets > 4) return (RES_ERROR);

        saw_digit = 1;
      }
    }

    else if (ch == '.' && saw_digit)
    {
      /* We should not encounter another delimiter once we have found
          the four octets of the IPv4 address.  Error out.
       */
      if (octets == 4) return (RES_ERROR);

      /* Zero out the next memory location that we will be placing the
          the next octet. */
      *++tp = 0;
      saw_digit = 0;
    }

    else return (RES_ERROR);
  }
  /* If we did not get all of the octets of the address, return an error */
  if (octets < 4) return (RES_ERROR);

  /* Copy the IP address into the provided buffer */

  dst[0] = tmp[3];
  dst[1] = tmp[2];
  dst[2] = tmp[1];
  dst[3] = tmp[0];

  return (RES_OK);

} /* SCK_Inet_PTON_v4 */


/*-----------------------------------------------------------------------------------------------------


  \param src
  \param dst

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
int32_t Str_to_IP_v6(char *src, uint8_t *dst)
{
  static char     xdigits_l[] = "0123456789abcdef", xdigits_u[] = "0123456789ABCDEF";
  uint8_t         tmp[IP6_ADDR_LEN],*tp,*endp,*colonp = NULL;
  char            * xdigits,*curtok,*pch;
  int32_t         ch,n,i,saw_xdigit;
  uint16_t        val;



  memset((tp = tmp), '\0', IP6_ADDR_LEN);

  endp = tp + IP6_ADDR_LEN;

  /* Leading :: requires some special handling. */
  if (*src == ':') if (*++src != ':') return (RES_ERROR);

  curtok = src;
  saw_xdigit = 0;
  val = 0;

  /* Walk the string until we find a null terminator */
  while ((ch =*src++) != '\0')
  {
    /* Due to the fact that Microsoft's FTP client appends a "%" and then
        interface number to the end of IPv6 addresses, we need to account
        for this case here by breaking out of the string-processing loop.
    */
    if (ch == '%') break;

    /*
       Compare ch with the known ASCII characters for which we are searching.
    */

    /* Test for lower case characters */
    pch = strchr((xdigits = xdigits_l), ch);

    if (pch == NULL)
      /* Test for upper case characters */
      pch = strchr((xdigits = xdigits_u), ch);

    if (pch != NULL)
    {
      /* Shift over any previously found characters and place the new
          character into val
      */
      val <<= 4;
      val |= (uint16_t)(pch - xdigits);

      saw_xdigit = 1;
      continue;
    }

    /* Handle the colon delimiters */
    if (ch == ':')
    {
      /* Save off a pointer to the character string we are processing */
      curtok = src;

      if (!saw_xdigit)
      {
        /* If we have not processed any characters and this is our second time
            getting a colon, then we have an error.  Get out. \
        */
        if (colonp) return (RES_ERROR);

        colonp = tp;

        continue;
      }

      /* There should not be a delimiter followed by the null terminator.  This
          is an error.  Get out.
      */
      else if (*src == '\0')
      {
        return (RES_ERROR);
      }

      /* If we do not have enough room in the buffer to copy the characters that
          we have read in, then error out.
      */
      if ((tp + sizeof(int16_t)) > endp) return (RES_ERROR);

      /* Copy the first byte into our temporary buffer */
      *tp++= (uint8_t)((val >> 8) & 0xff);

      /* Copy the second byte into our temporary buffer */
      *tp++= (uint8_t)(val & 0xff);

      /* Reset the saw_xdigit and val variables */
      saw_xdigit = 0;
      val = 0;
      continue;
    }

    /* If this is a IPv4-mapped-to-IPv6 address, then call
        Str_to_IP_v4 to handle the address.
    */
    if ((ch == '.') && ((tp + IP_ADDR_LEN) <= endp) &&
      (Str_to_IP_v4(curtok, tp) > 0))
    {
      tp += IP_ADDR_LEN;
      saw_xdigit = 0;
      break;  /* '\0' was seen by inet_pton4(). */
    }
    return (RES_ERROR);
  }

  /* We need to copy the last two bytes into the temporary buffer if there is
      room.  If not, error out.
  */
  if (saw_xdigit)
  {
    if ((tp + sizeof(int16_t)) > endp) return (RES_ERROR);

    /* Copy the first byte into our temporary buffer */
    *tp++= (char)((val >> 8) & 0xff);

    /* Copy the second byte into our temporary buffer */
    *tp++= (char)(val & 0xff);
  }

  if (colonp != NULL)
  {
    /*
     * Since some memmove()'s erroneously fail to handle
     * overlapping regions, we'll do the shift by hand.
     */
    n = (INT)(tp - colonp);

    if (tp == endp) return (RES_ERROR);

    for (i = 1; i <= n; i++)
    {
      endp[- i] = colonp[n - i];
      colonp[n - i] = 0;
    }

    tp = endp;
  }

  /* If we did not copy all 16 bytes into the temporary buffer, then error out. */
  if (tp != endp) return (RES_ERROR);

  /* Copy the temporary buffer into the buffer provided by the calling function. */
  memcpy(dst, tmp, IP6_ADDR_LEN);

  return (RES_OK);

} /* SCK_Inet_PTON_v6 */

/*-----------------------------------------------------------------------------------------------------


  \param err

  \return char const*
-----------------------------------------------------------------------------------------------------*/
char const* Get_NX_err_str(uint32_t err)
{
  switch (err)
  {
  case  NX_SUCCESS                 :
    return "SUCCESS             ";
  case  NX_NO_PACKET               :
    return "NO_PACKET           ";
  case  NX_UNDERFLOW               :
    return "UNDERFLOW           ";
  case  NX_OVERFLOW                :
    return "OVERFLOW            ";
  case  NX_NO_MAPPING              :
    return "NO_MAPPING          ";
  case  NX_DELETED                 :
    return "DELETED             ";
  case  NX_POOL_ERROR              :
    return "POOL_ERROR          ";
  case  NX_PTR_ERROR               :
    return "PTR_ERROR           ";
  case  NX_WAIT_ERROR              :
    return "WAIT_ERROR          ";
  case  NX_SIZE_ERROR              :
    return "SIZE_ERROR          ";
  case  NX_OPTION_ERROR            :
    return "OPTION_ERROR        ";
  case  NX_DELETE_ERROR            :
    return "DELETE_ERROR        ";
  case  NX_CALLER_ERROR            :
    return "CALLER_ERROR        ";
  case  NX_INVALID_PACKET          :
    return "INVALID_PACKET      ";
  case  NX_INVALID_SOCKET          :
    return "INVALID_SOCKET      ";
  case  NX_NOT_ENABLED             :
    return "NOT_ENABLED         ";
  case  NX_ALREADY_ENABLED         :
    return "ALREADY_ENABLED     ";
  case  NX_ENTRY_NOT_FOUND         :
    return "ENTRY_NOT_FOUND     ";
  case  NX_NO_MORE_ENTRIES         :
    return "NO_MORE_ENTRIES     ";
  case  NX_ARP_TIMER_ERROR         :
    return "ARP_TIMER_ERROR     ";
  case  NX_RESERVED_CODE0          :
    return "RESERVED_CODE0      ";
  case  NX_WAIT_ABORTED            :
    return "WAIT_ABORTED        ";
  case  NX_IP_INTERNAL_ERROR       :
    return "IP_INTERNAL_ERROR   ";
  case  NX_IP_ADDRESS_ERROR        :
    return "IP_ADDRESS_ERROR    ";
  case  NX_ALREADY_BOUND           :
    return "ALREADY_BOUND       ";
  case  NX_PORT_UNAVAILABLE        :
    return "PORT_UNAVAILABLE    ";
  case  NX_NOT_BOUND               :
    return "NOT_BOUND           ";
  case  NX_RESERVED_CODE1          :
    return "RESERVED_CODE1      ";
  case  NX_SOCKET_UNBOUND          :
    return "SOCKET_UNBOUND      ";
  case  NX_NOT_CREATED             :
    return "NOT_CREATED         ";
  case  NX_SOCKETS_BOUND           :
    return "SOCKETS_BOUND       ";
  case  NX_NO_RESPONSE             :
    return "NO_RESPONSE         ";
  case  NX_POOL_DELETED            :
    return "POOL_DELETED        ";
  case  NX_ALREADY_RELEASED        :
    return "ALREADY_RELEASED    ";
  case  NX_RESERVED_CODE2          :
    return "RESERVED_CODE2      ";
  case  NX_MAX_LISTEN              :
    return "MAX_LISTEN          ";
  case  NX_DUPLICATE_LISTEN        :
    return "DUPLICATE_LISTEN    ";
  case  NX_NOT_CLOSED              :
    return "NOT_CLOSED          ";
  case  NX_NOT_LISTEN_STATE        :
    return "NOT_LISTEN_STATE    ";
  case  NX_IN_PROGRESS             :
    return "IN_PROGRESS         ";
  case  NX_NOT_CONNECTED           :
    return "NOT_CONNECTED       ";
  case  NX_WINDOW_OVERFLOW         :
    return "WINDOW_OVERFLOW     ";
  case  NX_ALREADY_SUSPENDED       :
    return "ALREADY_SUSPENDED   ";
  case  NX_DISCONNECT_FAILED       :
    return "DISCONNECT_FAILED   ";
  case  NX_STILL_BOUND             :
    return "STILL_BOUND         ";
  case  NX_NOT_SUCCESSFUL          :
    return "NOT_SUCCESSFUL      ";
  case  NX_UNHANDLED_COMMAND       :
    return "UNHANDLED_COMMAND   ";
  case  NX_NO_FREE_PORTS           :
    return "NO_FREE_PORTS       ";
  case  NX_INVALID_PORT            :
    return "INVALID_PORT        ";
  case  NX_INVALID_RELISTEN        :
    return "INVALID_RELISTEN    ";
  case  NX_CONNECTION_PENDING      :
    return "CONNECTION_PENDING  ";
  case  NX_TX_QUEUE_DEPTH          :
    return "TX_QUEUE_DEPTH      ";
  case  NX_NOT_IMPLEMENTED         :
    return "NOT_IMPLEMENTED     ";
  case  NX_NOT_SUPPORTED           :
    return "NOT_SUPPORTED       ";
  case  NX_INVALID_INTERFACE       :
    return "INVALID_INTERFACE   ";
  case  NX_INVALID_PARAMETERS      :
    return "INVALID_PARAMETERS  ";
  case  NX_NOT_FOUND               :
    return "NOT_FOUND           ";
  case  NX_CANNOT_START            :
    return "CANNOT_START        ";
  case  NX_NO_INTERFACE_ADDRESS    :
    return "NO_INTERFACE_ADDRESS";
  case  NX_INVALID_MTU_DATA        :
    return "INVALID_MTU_DATA    ";
  case  NX_DUPLICATED_ENTRY        :
    return "DUPLICATED_ENTRY    ";
  case  NX_PACKET_OFFSET_ERROR     :
    return "PACKET_OFFSET_ERROR ";
  case  NX_OPTION_HEADER_ERROR     :
    return "OPTION_HEADER_ERROR ";
  case  NX_CONTINUE                :
    return "CONTINUE            ";
  case  NX_FTP_ERROR               :
    return "FTP_ERROR            ";
  case  NX_FTP_TIMEOUT             :
    return "FTP_TIMEOUT          ";
  case  NX_FTP_FAILED              :
    return "FTP_FAILED           ";
  case  NX_FTP_NOT_CONNECTED       :
    return "FTP_NOT_CONNECTED    ";
  case  NX_FTP_NOT_DISCONNECTED    :
    return "FTP_NOT_DISCONNECTED ";
  case  NX_FTP_NOT_OPEN            :
    return "FTP_NOT_OPEN         ";
  case  NX_FTP_NOT_CLOSED          :
    return "FTP_NOT_CLOSED       ";
  case  NX_FTP_END_OF_FILE         :
    return "FTP_END_OF_FILE      ";
  case  NX_FTP_END_OF_LISTING      :
    return "FTP_END_OF_LISTING   ";
  case  NX_FTP_EXPECTED_1XX_CODE   :
    return "FTP_EXPECTED_1XX_CODE";
  case  NX_FTP_EXPECTED_2XX_CODE   :
    return "FTP_EXPECTED_2XX_CODE";
  case  NX_FTP_EXPECTED_22X_CODE   :
    return "FTP_EXPECTED_22X_CODE";
  case  NX_FTP_EXPECTED_23X_CODE   :
    return "FTP_EXPECTED_23X_CODE";
  case  NX_FTP_EXPECTED_3XX_CODE   :
    return "FTP_EXPECTED_3XX_CODE";
  case  NX_FTP_EXPECTED_33X_CODE   :
    return "FTP_EXPECTED_33X_CODE";
  case  NX_FTP_INVALID_NUMBER      :
    return "FTP_INVALID_NUMBER   ";
  case  NX_FTP_INVALID_ADDRESS     :
    return "FTP_INVALID_ADDRESS  ";
  case  NX_FTP_INVALID_COMMAND     :
    return "FTP_INVALID_COMMAND  ";
  case  NX_FTP_INVALID_LOGIN       :
    return "FTP_INVALID_LOGIN    ";
  case  NX_FTP_INSUFFICIENT_PACKET_PAYLOAD:
    return "INSUFFICIENT_PACKET_PAYLOAD";

  default:
    return "Unknown             ";
  }
}


