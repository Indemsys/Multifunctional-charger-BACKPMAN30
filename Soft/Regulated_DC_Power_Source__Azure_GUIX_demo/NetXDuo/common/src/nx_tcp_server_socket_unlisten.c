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
#include "nx_tcp.h"
#include "nx_packet.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcp_server_socket_unlisten                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function removes a previous listen request for the specified   */
/*    TCP port.                                                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP instance        */
/*    port                                  TCP port number               */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_packet_release                    Release queued connection     */
/*    tx_mutex_get                          Obtain protection mutex       */
/*    tx_mutex_put                          Release protection mutex      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
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
UINT  _nx_tcp_server_socket_unlisten(NX_IP *ip_ptr, UINT port)
{

NX_TCP_SOCKET               *socket_ptr;
NX_PACKET                   *packet_ptr;
NX_PACKET                   *next_packet_ptr;
ULONG                        queue_count;
struct NX_TCP_LISTEN_STRUCT *listen_ptr;


    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_TCP_SERVER_SOCKET_UNLISTEN, ip_ptr, port, 0, 0, NX_TRACE_TCP_EVENTS, 0, 0);

    /* Obtain the IP protection.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Search through the ready_to_send listen requests to see if we can find
       one for this port.  */
    listen_ptr =  ip_ptr -> nx_ip_tcp_active_listen_requests;
    if (listen_ptr)
    {

        /* Search the ready_to_send listen requests for this port.  */
        do
        {

            /* Determine if there is a listen request for the specified port.  */
            if (listen_ptr -> nx_tcp_listen_port == port)
            {

                /* Pickup the socket for the listen request.  */
                socket_ptr =  listen_ptr -> nx_tcp_listen_socket_ptr;

                /* Determine if there was a socket dedicated for listening.  */
                if (socket_ptr)
                {

                    /* If trace is enabled, insert this event into the trace buffer.  */
                    NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_TCP_STATE_CHANGE, ip_ptr, socket_ptr, socket_ptr -> nx_tcp_socket_state, NX_TCP_CLOSED, NX_TRACE_INTERNAL_EVENTS, 0, 0);

                    /* Yes, clear any connection suspension on this socket.  */
                    if (socket_ptr -> nx_tcp_socket_state != NX_TCP_LISTEN_STATE)
                    {

                        /* Release protection.  */
                        tx_mutex_put(&(ip_ptr -> nx_ip_protection));

                        /* Return error code.  */
                        return(NX_NOT_LISTEN_STATE);
                    }

                    /* Change the state of the socket back to closed.  */
                    socket_ptr -> nx_tcp_socket_state = NX_TCP_CLOSED;

                    /* Clear the socket pointer in the listen structure.  */
                    listen_ptr -> nx_tcp_listen_socket_ptr =  NX_NULL;
                }
                else
                {

                    /* Check for queued connection requests, if found, release
                       them all.  */
                    queue_count =  listen_ptr -> nx_tcp_listen_queue_current;
                    packet_ptr =   listen_ptr -> nx_tcp_listen_queue_head;

                    /* Clear the listen connection queue pointers.  */
                    listen_ptr -> nx_tcp_listen_queue_head =  NX_NULL;
                    listen_ptr -> nx_tcp_listen_queue_tail =  NX_NULL;

                    /* Clear the listen connection count as well.  */
                    listen_ptr -> nx_tcp_listen_queue_current =  0;

                    /* Loop through and release the packets representing queued
                       connections.  */
                    while (queue_count--)
                    {

                        /* Save the next pointer.  */
                        next_packet_ptr =  packet_ptr -> nx_packet_queue_next;

                        /* Release the packet.  */
                        _nx_packet_release(packet_ptr);

                        /* Move to the next packet.  */
                        packet_ptr =  next_packet_ptr;
                    }
                }

                /* Unlink the listen structure from the ready_to_send listen requests.  */

                /* See if the listen structure is the only one on the list.  */
                if (listen_ptr == listen_ptr -> nx_tcp_listen_next)
                {

                    /* Only ready_to_send listen, just set the ready_to_send list to NULL.  */
                    ip_ptr -> nx_ip_tcp_active_listen_requests =  NX_NULL;
                }
                else
                {

                    /* Link-up the neighbors.  */
                    (listen_ptr -> nx_tcp_listen_next) -> nx_tcp_listen_previous =
                        listen_ptr -> nx_tcp_listen_previous;
                    (listen_ptr -> nx_tcp_listen_previous) -> nx_tcp_listen_next =
                        listen_ptr -> nx_tcp_listen_next;

                    /* See if we have to update the ready_to_send list head pointer.  */
                    if (ip_ptr -> nx_ip_tcp_active_listen_requests == listen_ptr)
                    {

                        /* Yes, move the head pointer to the next link. */
                        ip_ptr -> nx_ip_tcp_active_listen_requests =  listen_ptr -> nx_tcp_listen_next;
                    }
                }

                /* Add the listen request back to the available list.  */
                listen_ptr -> nx_tcp_listen_next =  ip_ptr -> nx_ip_tcp_available_listen_requests;
                ip_ptr -> nx_ip_tcp_available_listen_requests =  listen_ptr;

                /* Release the protection.  */
                tx_mutex_put(&(ip_ptr -> nx_ip_protection));

                /* Return success!  */
                return(NX_SUCCESS);
            }

            /* Move to the next listen request.  */
            listen_ptr =  listen_ptr -> nx_tcp_listen_next;
        } while (listen_ptr != ip_ptr -> nx_ip_tcp_active_listen_requests);
    }

    /* Unsuccessful listen request, release the protection.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    /* Return error code.  */
    return(NX_ENTRY_NOT_FOUND);
}

