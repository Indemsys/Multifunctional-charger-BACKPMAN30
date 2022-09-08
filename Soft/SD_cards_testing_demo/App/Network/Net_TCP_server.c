#include    "App.h"

extern  TX_THREAD           *_tx_thread_current_ptr;
extern  volatile ULONG      _tx_thread_system_state;


/*-----------------------------------------------------------------------------------------------------
  Callback routine when a connect request arrives

  \param socket_ptr
  \param port
-----------------------------------------------------------------------------------------------------*/
static VOID  _Connection_callback(NX_TCP_SOCKET *socket_ptr, UINT port)
{
  NET_TCP_SERVER   *server_ptr;
  NX_PARAMETER_NOT_USED(port);
  server_ptr =  socket_ptr->nx_tcp_socket_reserved_ptr;                       // Pickup server pointer.  This is setup in the reserved field of the TCP socket.
  tx_event_flags_set(&(server_ptr->event_flags), NET_TCPSRV_CONNECT, TX_OR);  // Set the connect event flag.
}


/*-----------------------------------------------------------------------------------------------------


  \param socket_ptr
-----------------------------------------------------------------------------------------------------*/
static VOID  _Disconnect_callback(NX_TCP_SOCKET *socket_ptr)
{
  NET_TCP_SERVER   *server_ptr;
  server_ptr =  socket_ptr->nx_tcp_socket_reserved_ptr;                         // Pickup server pointer.  This is setup in the reserved field of the TCP socket.
  tx_event_flags_set(&(server_ptr->event_flags), NET_TCPSRV_DISCONNECT, TX_OR); // Set the disconnect event flag.
}


/*-----------------------------------------------------------------------------------------------------
  Routine to call when one or receive packets are available for the socket

  \param socket_ptr
-----------------------------------------------------------------------------------------------------*/
static VOID  _Data_receive_callback(NX_TCP_SOCKET *socket_ptr)
{
  NET_TCP_SERVER   *server_ptr;
  server_ptr =  socket_ptr->nx_tcp_socket_reserved_ptr;                    // Pickup server pointer.  This is setup in the reserved field of the TCP socket.
  tx_event_flags_set(&(server_ptr->event_flags), NET_TCPSRV_DATA, TX_OR);  // Set the data event flag.
}

/*-----------------------------------------------------------------------------------------------------


  \param option_message_type
  \param option_id
  \param stream
-----------------------------------------------------------------------------------------------------*/
static VOID _Set_option_packet(UCHAR option_message_type, UCHAR option_id, UCHAR *stream)
{

  *(stream++)= NET_TCPSRV_IAC;
  *(stream++)= option_message_type;
  *stream = option_id;
}


/*-----------------------------------------------------------------------------------------------------
  Создание сервера TCP

  \param server_ptr
  \param setup_ptr

  \return UINT
-----------------------------------------------------------------------------------------------------*/
UINT Net_tcp_server_create(NET_TCP_SERVER **server_pptr, T_tcpsrv_setup *setup_ptr, char *name)
{
  UINT            status;


 if (server_pptr == NULL) return TX_PTR_ERROR;

  NET_TCP_SERVER *server_ptr = *server_pptr;

  if (server_ptr != NULL) return TX_PTR_ERROR;

  server_ptr = App_malloc(sizeof(NET_TCP_SERVER));
  if (server_ptr == NULL)
  {
    status = TX_NO_MEMORY;
    goto err;
  }

  server_ptr->stack_ptr = App_malloc(setup_ptr->stack_size);
  if (server_ptr->stack_ptr == NULL)
  {
    status = TX_NO_MEMORY;
    goto err;
  }

  server_ptr->ip_ptr = setup_ptr->ip_ptr;

  // Create the Server thread.
  status =  tx_thread_create(&(server_ptr->server_thread), name,
                             Net_tcp_server_thread,
                             (ULONG) server_ptr,
                             server_ptr->stack_ptr,
                             setup_ptr->stack_size,
                             NET_TCP_SERVER_PRIORITY,
                             NET_TCP_SERVER_PRIORITY,
                             TX_NO_TIME_SLICE,
                             TX_DONT_START);

  if (status != TX_SUCCESS) goto err;

  status =  tx_event_flags_create(&(server_ptr->event_flags), name);
  if (status != TX_SUCCESS)
  {
    tx_thread_delete(&(server_ptr->server_thread));
    goto err;
  }

  // Create the ThreadX activity timeout timer.  This will be used to periodically check to see if a client connection has gone silent and needs to be terminated.
  status =  tx_timer_create(&(server_ptr->timer),
                            name,
                            Net_tcp_server_timeout,
                            (ULONG) server_ptr,
                            (NX_IP_PERIODIC_RATE * NET_TCPSRV_TIMEOUT_PERIOD),
                            (NX_IP_PERIODIC_RATE * NET_TCPSRV_TIMEOUT_PERIOD),
                            TX_NO_ACTIVATE);

  if (status != TX_SUCCESS)
  {
    tx_thread_delete(&(server_ptr->server_thread));
    tx_event_flags_delete(&(server_ptr->event_flags));
    goto err;
  }

  status +=  nx_tcp_socket_create(server_ptr->ip_ptr,
                                  &(server_ptr->socket),
                                  name,
                                  NET_TCPSRV_TOS,
                                  NET_TCPSRV_FRAGMENT_OPTION,
                                  NET_TCPSRV_TIME_TO_LIVE,
                                  NET_TCP_SERVER_WINDOW_SIZE,
                                  NX_NULL,
                                  _Disconnect_callback);
  if (status == NX_SUCCESS)
  {
    nx_tcp_socket_receive_notify(&(server_ptr->socket),_Data_receive_callback);  // Register the receive function.
  }
  server_ptr->socket.nx_tcp_socket_reserved_ptr =  server_ptr;  // Make sure each socket points to the server.
  if (status != NX_SUCCESS)
  {
    nx_tcp_socket_delete(&(server_ptr->socket));
    tx_thread_delete(&(server_ptr->server_thread));
    tx_event_flags_delete(&(server_ptr->event_flags));
    tx_timer_delete(&(server_ptr->timer));
    goto err;
  }

  server_ptr->packet_pool_ptr           = setup_ptr->packet_pool_ptr;
  server_ptr->server_port               = setup_ptr->server_port;
  server_ptr->en_options_negotiation    = setup_ptr->en_options_negotiation;
  server_ptr->server_id                 = NET_TCP_SERVER_ID;             // Set the server ID to indicate the server thread is ready.
  server_ptr->New_connection_callback   = setup_ptr->cbl_New_connection;
  server_ptr->Receive_data_callback     = setup_ptr->cbl_Receive_data;
  server_ptr->Connection_end_callback   = setup_ptr->cbl_Connection_end;
  server_ptr->activity_timeout_val      = setup_ptr->activity_timeout_val;

  *server_pptr = server_ptr;
  return (NX_SUCCESS);


err:

  if (server_ptr != NULL)
  {
    if (server_ptr->stack_ptr != NULL)  App_free(server_ptr->stack_ptr);
    App_free(server_ptr);
  }

  return status;
}


/*-----------------------------------------------------------------------------------------------------


  \param server_ptr

  \return UINT
-----------------------------------------------------------------------------------------------------*/
UINT  Net_tcp_server_delete(NET_TCP_SERVER *server_ptr)
{
  if (server_ptr == NULL) return TX_PTR_ERROR;
  server_ptr->server_id =  0;
  tx_thread_suspend(&(server_ptr->server_thread));
  tx_thread_terminate(&(server_ptr->server_thread));
  tx_thread_delete(&(server_ptr->server_thread));
  tx_event_flags_delete(&(server_ptr->event_flags));
  tx_timer_deactivate(&(server_ptr->timer));
  tx_timer_delete(&(server_ptr->timer));
  nx_tcp_socket_disconnect(&(server_ptr->socket), NX_NO_WAIT);
  nx_tcp_server_socket_unaccept(&(server_ptr->socket));
  nx_tcp_socket_delete(&(server_ptr->socket));
  nx_tcp_server_socket_unlisten(server_ptr->ip_ptr, server_ptr->server_port);

  if (server_ptr->stack_ptr != NULL)  App_free(server_ptr->stack_ptr);
  App_free(server_ptr);

  return (NX_SUCCESS);
}

/*-----------------------------------------------------------------------------------------------------


  \param server_ptr
  \param logical_connection

  \return UINT
-----------------------------------------------------------------------------------------------------*/
UINT  Net_tcp_server_disconnect(NET_TCP_SERVER *server_ptr)
{
  UINT                        status;

  if (server_ptr->socket.nx_tcp_socket_state >= NX_TCP_ESTABLISHED)  // Determine if the connection is alive.
  {
    nx_tcp_socket_disconnect(&(server_ptr->socket), NET_TCP_SERVER_TIMEOUT);
    if (server_ptr->Connection_end_callback)
    {
      (server_ptr->Connection_end_callback)(server_ptr);
    }
    nx_tcp_server_socket_unaccept(&(server_ptr->socket));
    server_ptr->activity_timeout =  0;
  }
  else
  {
    return (NET_TCPSRV_NOT_CONNECTED);
  }

  if (server_ptr->socket.nx_tcp_socket_state == NX_TCP_CLOSED) // Now see if this socket is closed.
  {
    status =  nx_tcp_server_socket_relisten(server_ptr->ip_ptr, server_ptr->server_port,&(server_ptr->socket)); // Relisten on this socket.
    if ((status != NX_SUCCESS) && (status != NX_CONNECTION_PENDING))
    {
      server_ptr->relisten_errors++;

    }

  }
  return (NX_SUCCESS);
}


/*-----------------------------------------------------------------------------------------------------


  \param server_ptr
  \param logical_connection
  \param packet_ptr
  \param wait_option

  \return UINT
-----------------------------------------------------------------------------------------------------*/
UINT  Net_tcp_server_packet_send(NET_TCP_SERVER *server_ptr, NX_PACKET *packet_ptr, ULONG wait_option)
{
  UINT                        status;
  status =  nx_tcp_socket_send(&(server_ptr->socket), packet_ptr, wait_option); // Send the packet to the client.
  if (status) status =  NET_TCPSRV_FAILED;
  return (status);
}

/*-----------------------------------------------------------------------------------------------------


  \param server_ptr

  \return UINT
-----------------------------------------------------------------------------------------------------*/
UINT  Net_tcp_server_start(NET_TCP_SERVER *server_ptr)
{
  UINT    status;
  ULONG   events;

  if (server_ptr->packet_pool_ptr == NX_NULL)  return NET_TCPSRV_NO_PACKET_POOL;
  status =  nx_tcp_server_socket_listen(server_ptr->ip_ptr, server_ptr->server_port, &(server_ptr->socket), 1, _Connection_callback); // Start listening on the socket.
  if (status != NX_SUCCESS) return (status);
  tx_timer_activate(&(server_ptr->timer));                                                                // Activate server timer.
  tx_event_flags_get(&(server_ptr->event_flags), NET_TCPSRV_STOP_EVENT, TX_OR_CLEAR,&events, TX_NO_WAIT); // Clear stop event.
  tx_thread_resume(&(server_ptr->server_thread));                                                         // Start the server thread.
  return (NX_SUCCESS);
}

/*-----------------------------------------------------------------------------------------------------


  \param server_ptr

  \return UINT
-----------------------------------------------------------------------------------------------------*/
UINT  Net_tcp_server_stop(NET_TCP_SERVER *server_ptr)
{
  tx_timer_deactivate(&(server_ptr->timer));                                     // Deactivate server timer.
  tx_event_flags_set(&(server_ptr->event_flags), NET_TCPSRV_STOP_EVENT, TX_OR);  // Suspend the server thread.
  nx_tcp_socket_disconnect(&(server_ptr->socket), NX_NO_WAIT);                   // Disconnect the socket.
  nx_tcp_server_socket_unaccept(&(server_ptr->socket));                          // Unaccept the socket.
  server_ptr->activity_timeout =  0;                                             // Reset client request.
  nx_tcp_server_socket_unlisten(server_ptr->ip_ptr, server_ptr->server_port);     // Unlisten on the port.
  return (NX_SUCCESS);
}


/*-----------------------------------------------------------------------------------------------------


  \param server_ptr
-----------------------------------------------------------------------------------------------------*/
VOID  Net_tcp_server_connecting(NET_TCP_SERVER *server_ptr)
{
  UINT                        status;

    /* Now see if this socket was the one that is in being connected.  */
  if ((server_ptr->socket.nx_tcp_socket_state > NX_TCP_CLOSED) &&
      (server_ptr->socket.nx_tcp_socket_state < NX_TCP_ESTABLISHED) &&
      (server_ptr->socket.nx_tcp_socket_connect_port))
  {
      /* Yes, we have found the socket being connected.  */

      /* Increment the number of connection requests.  */
    server_ptr->connection_requests++;

      /* Attempt to accept on this socket.  */
    status = nx_tcp_server_socket_accept(&(server_ptr->socket), NET_TCP_SERVER_TIMEOUT);

      /* Determine if it is successful.  */
    if (status)
    {

        /* Not successful, simply unaccept on this socket.  */
      nx_tcp_server_socket_unaccept(&(server_ptr->socket));
    }
    else
    {

        /* Reset the client request activity timeout.  */
      server_ptr->activity_timeout =  server_ptr->activity_timeout_val;

        /* Call the application's new connection callback routine.  */
      if (server_ptr->New_connection_callback)
      {
          /* Yes, there is a new connection callback routine - call it!  */
        (server_ptr->New_connection_callback)(server_ptr);
      }

      if (server_ptr->en_options_negotiation)
      {
        status = Net_tcp_server_send_option_requests(server_ptr);
        if (status != NX_SUCCESS) return;
      }
    }
  }

  // Now see if this socket is closed.
  if (server_ptr->socket.nx_tcp_socket_state == NX_TCP_CLOSED)
  {
    // Relisten on this socket.
    status =  nx_tcp_server_socket_relisten(server_ptr->ip_ptr, server_ptr->server_port,&(server_ptr->socket));
    // Check for bad status.
    if ((status != NX_SUCCESS) && (status != NX_CONNECTION_PENDING))
    {
      // Increment the error count and keep trying.
      server_ptr->relisten_errors++;
    }
  }
}


/*-----------------------------------------------------------------------------------------------------


  \param server_ptr
-----------------------------------------------------------------------------------------------------*/
VOID  Net_tcp_server_disconnecting(NET_TCP_SERVER *server_ptr)
{
  UINT                        status;
  UINT                        reset_client_request;


  reset_client_request = NX_FALSE;

    // Has the socket received a RST packet? If so NetX will put it in a CLOSED or LISTEN state and the socket activity timeout has not been reset yet.
  if (server_ptr->socket.nx_tcp_socket_state < NX_TCP_SYN_SENT)
  {
    if (server_ptr->activity_timeout > 0)
    {
      reset_client_request = NX_TRUE;
    }
  }
  else
  {
      // Now see if this socket has entered a disconnect state.
    while (server_ptr->socket.nx_tcp_socket_state > NX_TCP_ESTABLISHED)
    {
        // Yes, a disconnect is present, which signals an end of session for request.
        // First, cleanup this socket.
      nx_tcp_socket_disconnect(&(server_ptr->socket), NET_TCP_SERVER_TIMEOUT);
      reset_client_request = NX_TRUE;
    }
  }

    /* If this connection is closed, update the telnet data and notify the application of a disconnect. */
  if (reset_client_request == NX_TRUE)
  {
      /* Unaccept this socket.  */
    nx_tcp_server_socket_unaccept(&(server_ptr->socket));
      /* Reset the client request activity timeout.  */
    server_ptr->activity_timeout =  0;
      /* Call the application's end connection callback routine.  */
    if (server_ptr->Connection_end_callback)
    {
        /* Yes, there is a connection end callback routine - call it!  */
      (server_ptr->Connection_end_callback)(server_ptr);
    }
  }


  // Now see if this socket is closed.
  if (server_ptr->socket.nx_tcp_socket_state == NX_TCP_CLOSED)
  {
    // Relisten on this socket.
    status =  nx_tcp_server_socket_relisten(server_ptr->ip_ptr, server_ptr->server_port,&(server_ptr->socket));
    // Check for bad status.
    if ((status != NX_SUCCESS) && (status != NX_CONNECTION_PENDING))
    {
      // Increment the error count and keep trying.
      server_ptr->relisten_errors++;
    }
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param telnet_server_address
-----------------------------------------------------------------------------------------------------*/
VOID  Net_tcp_server_timeout(ULONG telnet_server_address)
{
  NET_TCP_SERVER   *server_ptr;
  /* Pickup server pointer.  */
  server_ptr =  (NET_TCP_SERVER *) telnet_server_address;
  /* Set the data event flag.  */
  tx_event_flags_set(&(server_ptr->event_flags), NET_TCPSRV_ACTIVITY_TIMEOUT_CHK, TX_OR);
}


/*-----------------------------------------------------------------------------------------------------


  \param server_ptr
-----------------------------------------------------------------------------------------------------*/
VOID  Net_tcp_server_timeout_processing(NET_TCP_SERVER *server_ptr)
{
  // Now see if this socket has an activity timeout ready_to_send.
  if (server_ptr->activity_timeout)
  {
    // Decrement the activity timeout for this client request.
    if (server_ptr->activity_timeout > NET_TCPSRV_TIMEOUT_PERIOD) server_ptr->activity_timeout =  server_ptr->activity_timeout - NET_TCPSRV_TIMEOUT_PERIOD;
    else server_ptr->activity_timeout =  0;

    if (server_ptr->activity_timeout == 0) // Determine if this entry has exceeded the activity timeout.
    {
      // Yes, the activity timeout has been exceeded.  Tear down and clean up the entire client request structure.
      server_ptr->activity_timeouts++; // Increment the activity timeout counter.
      nx_tcp_socket_disconnect(&(server_ptr->socket), NX_NO_WAIT); // Now disconnect the command socket.
      nx_tcp_server_socket_unaccept(&(server_ptr->socket)); // Unaccept the server socket.
      // Relisten on this socket. This will probably fail, but it is needed just in case all available clients were in use at the time of the last relisten.
      nx_tcp_server_socket_relisten(server_ptr->ip_ptr, server_ptr->server_port, &(server_ptr->socket));
      if (server_ptr->Connection_end_callback) // Call the application's end connection callback routine.
      {
        (server_ptr->Connection_end_callback)(server_ptr); // Yes, there is a connection end callback routine - call it!
      }
    }
  }
}


/*-----------------------------------------------------------------------------------------------------


  \param server_ptr
  \param client_req_ptr

  \return UINT
-----------------------------------------------------------------------------------------------------*/
UINT Net_tcp_server_send_option_requests(NET_TCP_SERVER *server_ptr)
{
  UINT        status;
  NX_PACKET   *packet_ptr;
  UINT        option_length;
  UINT        packet_available;
  UCHAR       option_stream[9];

  status = NX_SUCCESS;

  /* Indicate packet not available, nor needing to be released. */
  packet_available = NX_FALSE;

  /* Allocate a packet for replies to send to this telnet client. */
  status =  nx_packet_allocate(server_ptr->packet_pool_ptr,&packet_ptr, NX_TCP_PACKET, NX_NO_WAIT);

  if (status != NX_SUCCESS) return status;

  /* Now a packet is available, and if not used should be released. */
  packet_available = NX_TRUE;

  /* Initialize option data to zero bytes. */
  option_length = 0;

  /* Yes, create will echo request (3 bytes). */
  _Set_option_packet(NET_TCPSRV_WILL, NET_TCPSRV_ECHO,&option_stream[0]);

  /* Yes, create dont echo request (3 bytes). */
  _Set_option_packet(NET_TCPSRV_DONT, NET_TCPSRV_ECHO,&option_stream[3]);

  /* Yes, create will SGA request (3 bytes). */
  _Set_option_packet(NET_TCPSRV_WILL, NET_TCPSRV_SGA,&option_stream[6]);

  /* Update the the packet payload for number of bytes for a telnet option request. */
  option_length = 9;

  /* Add to the packet payload. */
  status = nx_packet_data_append(packet_ptr, option_stream, 9, server_ptr->packet_pool_ptr, NX_WAIT_FOREVER);
  if (status)
  {
    nx_packet_release(packet_ptr);
    return (status);
  }

  /* Check if we have a packet started, but not sent yet.  */
  if (option_length > 0)
  {
    status =  Net_tcp_server_packet_send(server_ptr,  packet_ptr, 100);
    if (status != NX_SUCCESS)
    {
      nx_packet_release(packet_ptr);
      return status;
    }
    packet_available = NX_FALSE;   // Indicate we need another packet, just sent out the last one.
  }
  if (packet_available == NX_TRUE) // Check for unused packet (needs to be released).
  {
    nx_packet_release(packet_ptr); // Release the packet we did not use.
  }
  return status;
}

/*-----------------------------------------------------------------------------------------------------


  \param server_ptr
  \param packet_ptr
  \param offset
  \param client_req_ptr
-----------------------------------------------------------------------------------------------------*/
VOID Net_tcp_server_process_option(NET_TCP_SERVER *server_ptr, NX_PACKET *packet_ptr, UINT *offset)
{
  UCHAR   data_char;
  UCHAR   *work_ptr;

  NX_PARAMETER_NOT_USED(server_ptr);

  // Получаем адрес кода опции
  work_ptr = (UCHAR *)(packet_ptr->nx_packet_prepend_ptr +(*offset)+ 2);


  if (work_ptr >= packet_ptr->nx_packet_append_ptr)
  {
    *offset = packet_ptr->nx_packet_length;
    return;
  }

  data_char =*work_ptr;

  if (data_char == NET_TCPSRV_ECHO)
  {
    /* Check whether the client replies with echo negotiation of the local echo disabled.  */
    work_ptr--;
    data_char =*work_ptr;
    if (data_char == NET_TCPSRV_DO)
    {

      /* Server will echo what received from the telnet connection.  */
      //server_ptr->agree_server_will_echo_success = NX_TRUE;

      /* Move the offset to past the option.  */
      (*offset)+=3;
    }
    else
    {

      /* Move the offset to past the option.  */
      (*offset)+=3;
    }
  }

  /* If it is a SGA option.  */
  else if (data_char == NET_TCPSRV_SGA)
  {

    /* Check whether the client replies with SGA negotiation.  */
    work_ptr--;
    data_char =*work_ptr;
    if (data_char == NET_TCPSRV_DO)
    {

      /* Server will enable SGA option.  */
      //server_ptr->agree_server_will_SGA_success = NX_TRUE;

      /* Move the offset to past the option.  */
      (*offset)+=3;
    }
    else
    {
      /* Move the offset to past the option.  */
      (*offset)+=3;
      return;
    }
  }

  /* We have not implemented this option, just return.  */
  else
  {

    /* See next three bytes.  */
    (*offset)+=3;
    return;
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param server_ptr
-----------------------------------------------------------------------------------------------------*/
VOID  Net_tcp_server_data_receiving(NET_TCP_SERVER *server_ptr)
{
  UINT                        status;
  NX_PACKET                   *packet_ptr;
  UCHAR                       data_char;
  UINT                        offset;

  while (server_ptr->socket.nx_tcp_socket_receive_queue_count)                      // Now see if this socket has data.  If so, process all of it now!
  {
    server_ptr->activity_timeout =  server_ptr->activity_timeout_val;               // Reset the client request activity timeout.
    status =  nx_tcp_socket_receive(&(server_ptr->socket),&packet_ptr, NX_NO_WAIT); // Attempt to read a packet from this socket.

    if (status != NX_SUCCESS) break; // reak to look at the next socket.

    if (server_ptr->en_options_negotiation)
    {
      // Если первый байт IAC то это пакет с командами Telnet
      if (*packet_ptr->nx_packet_prepend_ptr == NET_TCPSRV_IAC)
      {


#ifndef NX_DISABLE_PACKET_CHAIN
        if (packet_ptr->nx_packet_next)
        {
          nx_packet_release(packet_ptr); // Chained packet is not supported.
          break;
        }
#endif

        offset = 0;  // Начианаем с первой команды
        if (packet_ptr->nx_packet_length == 1)
        {
          // Команд длиной в один байт не бывает
          nx_packet_release(packet_ptr);
          break;
        }

        data_char =*(packet_ptr->nx_packet_prepend_ptr + 1);
        if ((data_char >= NET_TCPSRV_WILL) && (data_char <= NET_TCPSRV_DONT))
        {
          //  Здесь обрабатываем коды опций и команд. Кажадая опция или команда начинается с символа NET_TCPSRV_IAC
          while ((offset < packet_ptr->nx_packet_length) && (*(packet_ptr->nx_packet_prepend_ptr + offset) == NET_TCPSRV_IAC))
          {
            Net_tcp_server_process_option(server_ptr, packet_ptr, &offset);
          }
        }
        if ((offset < packet_ptr->nx_packet_length) && (server_ptr->Receive_data_callback))
        {
          // Оставшиеся байты после опций и команд являются принимаемыми данными
          packet_ptr->nx_packet_prepend_ptr += offset;
          packet_ptr->nx_packet_length      -= offset;
          (server_ptr->Receive_data_callback)(server_ptr, packet_ptr);
        }
        else nx_packet_release(packet_ptr);
      }
      else
      {
        // Получен пакет с данными
        if (server_ptr->Receive_data_callback)   (server_ptr->Receive_data_callback)(server_ptr, packet_ptr);
        else nx_packet_release(packet_ptr);
      }
    } // if (server_ptr->en_options_negotiation)
    else
    {
      // Получен пакет с данными
      if (server_ptr->Receive_data_callback)
      {
        (server_ptr->Receive_data_callback)(server_ptr, packet_ptr); // Yes, there is a process data callback routine - call it!
      }
      else
      {
        nx_packet_release(packet_ptr);
      }
    }
  } // while (server_ptr->socket.nx_tcp_socket_receive_queue_count)
}

/*-----------------------------------------------------------------------------------------------------


  \param telnet_server
-----------------------------------------------------------------------------------------------------*/
VOID  Net_tcp_server_thread(ULONG telnet_server)
{
  NET_TCP_SERVER        *server_ptr;
  UINT                    status;
  ULONG                   events;

  server_ptr =  (NET_TCP_SERVER *) telnet_server;    // Setup the server pointer.
  while (1)
  {
    status =  tx_event_flags_get(&(server_ptr->event_flags), NET_TCPSRV_ANY_EVENT, TX_OR_CLEAR,&events, TX_WAIT_FOREVER); /* Wait for an client activity.  */
    if (status)
    {
      continue;
    }
    if (events & NET_TCPSRV_STOP_EVENT)              // Check whether service is started.
    {
      tx_thread_suspend(&server_ptr->server_thread);
      continue;
    }
    if (events & NET_TCPSRV_CONNECT)                 // Otherwise, an event is present.  Process according to the event.  Check for a client connection event.
    {
      Net_tcp_server_connecting(server_ptr);
    }
    if  (events & NET_TCPSRV_DATA)                   // Check for a client write data event.
    {
      Net_tcp_server_data_receiving(server_ptr);
    }
    if  (events & NET_TCPSRV_DISCONNECT)             // Check for a client disconnect event.
    {
      Net_tcp_server_disconnecting(server_ptr);
    }
    if  (events & NET_TCPSRV_ACTIVITY_TIMEOUT_CHK)       // Check for a client activity timeout event.
    {
      Net_tcp_server_timeout_processing(server_ptr);
    }
  }
}

