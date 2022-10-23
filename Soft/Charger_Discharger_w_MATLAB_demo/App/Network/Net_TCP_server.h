#ifndef NET_TCP_SERVER_H
  #define NET_TCP_SERVER_H



  /* Define the Server ID.  */

#define NET_TCP_SERVER_ID                 0x54454C4EUL



  /* Define TCP socket create options.  */

#ifndef NET_TCPSRV_TOS
#define NET_TCPSRV_TOS                    NX_IP_NORMAL
#endif

#ifndef NET_TCPSRV_FRAGMENT_OPTION
#define NET_TCPSRV_FRAGMENT_OPTION        NX_DONT_FRAGMENT
#endif

#ifndef NET_TCP_SERVER_WINDOW_SIZE
#define NET_TCP_SERVER_WINDOW_SIZE        2048
#endif

#ifndef NET_TCPSRV_TIME_TO_LIVE
#define NET_TCPSRV_TIME_TO_LIVE           0x80
#endif

#ifndef NET_TCP_SERVER_TIMEOUT
#define NET_TCP_SERVER_TIMEOUT            (10 * NX_IP_PERIODIC_RATE)
#endif

#ifndef NET_TCP_SERVER_PRIORITY
#define NET_TCP_SERVER_PRIORITY           16
#endif

#ifndef NET_TCPSRV_ACTIVITY_TIMEOUT
#define NET_TCPSRV_ACTIVITY_TIMEOUT       600         // Seconds allowed with no activity
#endif

#ifndef NET_TCPSRV_TIMEOUT_PERIOD
#define NET_TCPSRV_TIMEOUT_PERIOD         60          // Number of seconds to check
#endif


  /* Define commands that are optionally included in the data.  The application is responsible for
     recognizing and responding to the commands in accordance with the specification.  The option command
     requires three bytes, as follows:

          IAC, COMMAND, OPTION ID
  */

  /* Define byte indicating command follows.  */

#define NET_TCPSRV_IAC                       255         /* Command byte - two consecutive -> 255 data    */

  /* Define Server Negotiation Commands - Immediately follows IAC.  */

#define NET_TCPSRV_WILL                      251         /*  WILL - Sender wants to enable the option      */
#define NET_TCPSRV_WONT                      252         /*  WONT - Sender wants to disable the option     */
#define NET_TCPSRV_DO                        253         /*  DO -   Sender wants receiver to enable option */
#define NET_TCPSRV_DONT                      254         /*  DONT - Sender wants receiver to disable option*/


  /* Define the size of the Server packet pool. This will allow room for about 5-6 packets of 300 byte payload. */

#ifndef NET_TCP_SERVER_PACKET_POOL_SIZE
#define NET_TCP_SERVER_PACKET_POOL_SIZE      2048
#endif


  /* Define Server Option IDs.  */
#define NET_TCPSRV_ECHO                      1           /*  ECHO Option                                   */
#define NET_TCPSRV_SGA                       3           /*  SGA Option                                    */


  /* Define Server thread events.  */

#define NET_TCPSRV_CONNECT                   0x01        /*  connection is present                         */
#define NET_TCPSRV_DISCONNECT                0x02        /*  disconnection is present                      */
#define NET_TCPSRV_DATA                      0x04        /*  receive data is present                       */
#define NET_TCPSRV_ACTIVITY_TIMEOUT_CHK      0x08        /*  activity timeout check                        */
#define NET_TCPSRV_STOP_EVENT                0x10        /*  stop service                                  */
#define NET_TCPSRV_ANY_EVENT                 0xFF        /* Any Server event                                     */


  /* Define return code constants.  */

#define NET_TCPSRV_ERROR                     0xF0        /*  internal error                                */
#define NET_TCPSRV_TIMEOUT                   0xF1        /*  timeout occurred                              */
#define NET_TCPSRV_FAILED                    0xF2        /*  error                                         */
#define NET_TCPSRV_NOT_CONNECTED             0xF3        /*  not connected error                           */
#define NET_TCPSRV_NOT_DISCONNECTED          0xF4        /*  not disconnected error                        */
#define NET_TCPSRV_INVALID_PARAMETER         0xF5        /* Invalid non pointer input to Server function         */
#define NET_TCPSRV_NO_PACKET_POOL            0xF6        /*  server packet pool not set                    */


  typedef struct NET_TCP_SERVER_STRUCT
  {
      ULONG                             server_id;                           // Server ID
      USHORT                            server_port;                         // Номер порта TCP соединения
      UCHAR                             en_options_negotiation;              // Флаг разрешения процедуры согласования опций по протоколу Telnet
      VOID                             *stack_ptr;                           // Указатель на стек задачи сервера
      NX_TCP_SOCKET                     socket;                              // Client request socket
      ULONG                             activity_timeout;                    // Timeout for client activity
      ULONG                             activity_timeout_val;                //
      NX_IP                             *ip_ptr;                             // Associated IP structure
      ULONG                             connection_requests;                 // Number of connection requests
      ULONG                             disconnection_requests;              // Number of disconnection requests
      ULONG                             total_bytes_sent;                    // Number of total bytes sent
      ULONG                             total_bytes_received;                // Number of total bytes received
      ULONG                             relisten_errors;                     // Number of relisten errors
      ULONG                             activity_timeouts;                   // Number of activity timeouts
      NX_PACKET_POOL                   *packet_pool_ptr;                     // Pointer to packet pool
      TX_EVENT_FLAGS_GROUP              event_flags;                         // server thread events
      TX_TIMER                          timer;                               // server activity timeout timer
      TX_THREAD                         server_thread;                       // server thread
      void   (*New_connection_callback  )(struct NET_TCP_SERVER_STRUCT *telnet_server_ptr);                       //    New_connection_callback
      void   (*Receive_data_callback    )(struct NET_TCP_SERVER_STRUCT *telnet_server_ptr, NX_PACKET *packet_ptr);//    Receive_data_callback
      void   (*Connection_end_callback  )(struct NET_TCP_SERVER_STRUCT *telnet_server_ptr);                       //    Connection_end_callback
  } NET_TCP_SERVER;

  typedef  void   (*T_tcpsrv_new_connection_callback)(NET_TCP_SERVER *telnet_server_ptr);
  typedef  void   (*T_tcpsrv_receive_data_callback)(NET_TCP_SERVER *telnet_server_ptr, NX_PACKET *packet_ptr);
  typedef  void   (*T_tcpsrv_connection_end_callback)(NET_TCP_SERVER *telnet_server_ptr);


  typedef struct
  {
    NX_IP                            *ip_ptr;
    USHORT                            server_port;            // Номер порта TCP соединения
    ULONG                             stack_size;             // Размер стека задачи сервера
    UCHAR                             en_options_negotiation; // Флаг разрешения процедуры согласования опций по протоколу Telnet
    ULONG                             activity_timeout_val;   // Время отсутствия приеме после кторого соединение разрывается. если 0xFFFFFFFF то это время не контролируется
    NX_PACKET_POOL                   *packet_pool_ptr;        // Указатель на пул памяти сетевых пакетов
    T_tcpsrv_new_connection_callback  cbl_New_connection;
    T_tcpsrv_receive_data_callback    cbl_Receive_data;
    T_tcpsrv_connection_end_callback  cbl_Connection_end;

  } T_tcpsrv_setup;


  UINT    Net_tcp_server_create(NET_TCP_SERVER **server_pptr, T_tcpsrv_setup *setup, char *name);
  UINT    Net_tcp_server_delete(NET_TCP_SERVER *server_ptr);
  UINT    Net_tcp_server_disconnect(NET_TCP_SERVER *server_ptr);
  UINT    Net_tcp_server_packet_send(NET_TCP_SERVER *server_ptr, NX_PACKET *packet_ptr, ULONG wait_option);
  UINT    Net_tcp_server_start(NET_TCP_SERVER *server_ptr);
  UINT    Net_tcp_server_stop(NET_TCP_SERVER *server_ptr);

  /* Define internal functions.  */

  VOID    Net_tcp_server_thread(ULONG telnet_server);
  VOID    Net_tcp_server_connecting(NET_TCP_SERVER *server_ptr);
  VOID    Net_tcp_server_disconnecting(NET_TCP_SERVER *server_ptr);
  VOID    Net_tcp_server_data_receiving(NET_TCP_SERVER *server_ptr);
  VOID    Net_tcp_server_timeout(ULONG telnet_server_address);
  VOID    Net_tcp_server_timeout_processing(NET_TCP_SERVER *server_ptr);


  UINT    Net_tcp_server_send_option_requests(NET_TCP_SERVER *server_ptr);
  VOID    Net_tcp_server_process_option(NET_TCP_SERVER *server_ptr, NX_PACKET *packet_ptr, UINT *offset);
  VOID    Net_tcp_server_create_option_packet(UCHAR option_message_type, UCHAR option_id, UCHAR *stream);




#endif
