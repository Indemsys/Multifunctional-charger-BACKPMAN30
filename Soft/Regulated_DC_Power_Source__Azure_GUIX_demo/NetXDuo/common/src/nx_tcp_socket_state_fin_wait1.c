/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** NetX Component                                                        */
/**                                                                       */
/**   Transmission Control Protocol (TCP)                                 */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_ip.h"
#include "nx_tcp.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcp_socket_state_fin_wait1                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes packets during the FIN WAIT 1 state,        */
/*    which is the state after the initial FIN was issued in an ready_to_send    */
/*    disconnect issued by the application.                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to owning socket      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_tcp_packet_send_ack               Send ACK message              */
/*    _nx_tcp_socket_thread_resume          Resume suspended thread       */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_tcp_socket_packet_process         Process TCP packet for socket */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
VOID  _nx_tcp_socket_state_fin_wait1(NX_TCP_SOCKET *socket_ptr)
{


    /* Determine if the peer has proper ACK number but FIN is not sent,
       move into the FIN WAIT 2 state and do nothing else.  */
    if ((socket_ptr -> nx_tcp_socket_fin_acked) &&
        (socket_ptr -> nx_tcp_socket_fin_received == NX_FALSE))
    {

        /* If trace is enabled, insert this event into the trace buffer.  */
        NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_TCP_STATE_CHANGE, socket_ptr -> nx_tcp_socket_ip_ptr, socket_ptr, socket_ptr -> nx_tcp_socket_state, NX_TCP_FIN_WAIT_2, NX_TRACE_INTERNAL_EVENTS, 0, 0);

        /* We have a legitimate ACK message.  Simply move into the WAIT FIN 2 state
           for the other side to finish its processing and disconnect.  */
        socket_ptr -> nx_tcp_socket_state =  NX_TCP_FIN_WAIT_2;

        /* Otherwise, simply clear the FIN timeout.  */
        socket_ptr -> nx_tcp_socket_timeout =  0;
    }
    else if ((socket_ptr -> nx_tcp_socket_fin_acked) &&
             (socket_ptr -> nx_tcp_socket_fin_sequence == socket_ptr -> nx_tcp_socket_rx_sequence))
    {

        /* Return to the proper socket state.  */

        /* Yes, return the socket to the TIMED WAIT state. */

        /* If trace is enabled, insert this event into the trace buffer.  */
        NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_TCP_STATE_CHANGE, socket_ptr -> nx_tcp_socket_ip_ptr, socket_ptr, socket_ptr -> nx_tcp_socket_state, NX_TCP_TIMED_WAIT, NX_TRACE_INTERNAL_EVENTS, 0, 0);

        /* Set the socket state to TIMED WAIT now.  */
        socket_ptr -> nx_tcp_socket_state = NX_TCP_TIMED_WAIT;

        /* Set the timeout as 2MSL (Maximum Segment Lifetime). */
        socket_ptr -> nx_tcp_socket_timeout = _nx_tcp_2MSL_timer_rate;

        /* Send ACK back to the other side of the connection.  */

        /* Increment the received sequence number.  */
        socket_ptr -> nx_tcp_socket_rx_sequence++;

        /* Send ACK message.  */
        _nx_tcp_packet_send_ack(socket_ptr, socket_ptr -> nx_tcp_socket_tx_sequence);

        /* Determine if we need to wake a thread suspended on the connection.  */
        if (socket_ptr -> nx_tcp_socket_disconnect_suspended_thread)
        {

            /* Resume the thread suspended for the disconnect.  */
            _nx_tcp_socket_thread_resume(&(socket_ptr -> nx_tcp_socket_disconnect_suspended_thread), NX_SUCCESS);
        }

        /* If given, call the application's disconnect callback function
           for disconnect.  */
        if (socket_ptr -> nx_tcp_disconnect_callback)
        {

            /* Call the application's disconnect handling function.  It is
               responsible for calling the socket disconnect function.  */
            (socket_ptr -> nx_tcp_disconnect_callback)(socket_ptr);
        }

#ifndef NX_DISABLE_EXTENDED_NOTIFY_SUPPORT

        /* Is a timed wait callback registered for this socket?  */
        if (socket_ptr -> nx_tcp_timed_wait_callback)
        {

            /* Call the timed wait callback for this socket to let the host
               know the socket can now be put in the timed wait state (if
               the RE-USE ADDRESS socket option is not enabled). */
            (socket_ptr -> nx_tcp_timed_wait_callback)(socket_ptr);
        }

        /* Is a disconnect complete callback registered with the TCP socket? */
        if (socket_ptr -> nx_tcp_disconnect_complete_notify)
        {

            /* Call the application's disconnect_complete callback function.    */
            (socket_ptr -> nx_tcp_disconnect_complete_notify)(socket_ptr);
        }
#endif
    }
    else if ((socket_ptr -> nx_tcp_socket_fin_received) &&
             (socket_ptr -> nx_tcp_socket_fin_sequence == socket_ptr -> nx_tcp_socket_rx_sequence))
    {

        /* If trace is enabled, insert this event into the trace buffer.  */
        NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_TCP_STATE_CHANGE, socket_ptr -> nx_tcp_socket_ip_ptr, socket_ptr, socket_ptr -> nx_tcp_socket_state, NX_TCP_CLOSING, NX_TRACE_INTERNAL_EVENTS, 0, 0);

        /* Move to the CLOSING state for simultaneous close situation.  */
        socket_ptr -> nx_tcp_socket_state =  NX_TCP_CLOSING;

        /* Send ACK back to the other side of the connection.  */

        /* Increment the received sequence number.  */
        socket_ptr -> nx_tcp_socket_rx_sequence++;

        /* Send ACK message.  */
        _nx_tcp_packet_send_ack(socket_ptr, socket_ptr -> nx_tcp_socket_tx_sequence);
    }
}

