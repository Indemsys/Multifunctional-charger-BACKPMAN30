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
/** USBX Component                                                        */ 
/**                                                                       */
/**   Host Stack                                                          */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/* Include necessary system files.  */

#define UX_SOURCE_CODE

#include "ux_api.h"
#include "ux_host_stack.h"


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _ux_host_stack_new_endpoint_create                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */ 
/*    This function creates a new endpoint for the current interface      */ 
/*    scanned. The endpoint is hooked to the interface that owns it.      */ 
/*    It is not ready_to_send yet until either the default interface for the     */
/*    configuration is selected by a SET_CONFIGURATION or when an         */ 
/*    alternate setting for this interface is set.                        */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    interface                             Interface container that owns */ 
/*                                            this endpoint               */
/*    endpoint_pointer                      Pointer to a unparsed         */ 
/*                                            endpoint descriptor         */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    Completion Status                                                   */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _ux_utility_descriptor_parse          Parse the descriptor          */ 
/*    _ux_utility_memory_allocate           Allocate memory block         */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    USBX Components                                                     */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  05-19-2020     Chaoqiong Xiao           Initial Version 6.0           */
/*  09-30-2020     Chaoqiong Xiao           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _ux_host_stack_new_endpoint_create(UX_INTERFACE *interface,
                                                 UCHAR * interface_endpoint)
{

UX_ENDPOINT     *endpoint;
UX_ENDPOINT     *list_endpoint;

    /* Obtain memory for storing this new endpoint.  */
    endpoint =  (UX_ENDPOINT *) _ux_utility_memory_allocate(UX_NO_ALIGN, UX_REGULAR_MEMORY, sizeof(UX_ENDPOINT));
    if (endpoint == UX_NULL)
        return(UX_MEMORY_INSUFFICIENT);

    /* If trace is enabled, insert this event into the trace buffer.  */
    UX_TRACE_IN_LINE_INSERT(UX_TRACE_HOST_STACK_NEW_ENDPOINT_CREATE, interface, endpoint, 0, 0, UX_TRACE_HOST_STACK_EVENTS, 0, 0)

    /* Save the endpoint handle in the container, this is for ensuring the
       endpoint container is not corrupted.  */
    endpoint -> ux_endpoint =  (ULONG) (ALIGN_TYPE) endpoint;

    /* The endpoint container has a built in transfer_request. 
       The transfer_request needs to point to the endpoint as well.  */
    endpoint -> ux_endpoint_transfer_request.ux_transfer_request_endpoint =  endpoint;

    /* Save the pointer to the device. This is useful for the HCD layer.  */
    endpoint -> ux_endpoint_device =  interface -> ux_interface_configuration -> ux_configuration_device;

    /* Parse the interface descriptor and make it machine independent.  */
    _ux_utility_descriptor_parse(interface_endpoint,
                            _ux_system_endpoint_descriptor_structure,
                            UX_ENDPOINT_DESCRIPTOR_ENTRIES,
                            (UCHAR *) &endpoint -> ux_endpoint_descriptor);

    /* The interface that owns this endpoint is memorized in the 
       endpoint container itself, easier for back chaining.  */
    endpoint -> ux_endpoint_interface =  interface;

    /* There is 2 cases for the creation of the endpoint descriptor 
       if this is the first one, the endpoint descriptor is hooked
       to the interface. 
       If it is not the first one, the endpoint is hooked to the
       end of the chain of endpoints.  */
    if (interface -> ux_interface_first_endpoint == UX_NULL)
    {

        interface -> ux_interface_first_endpoint =  endpoint;
    }
    else
    {

        list_endpoint =  interface -> ux_interface_first_endpoint;
        
        /* Traverse the list until the end.  */
        while (list_endpoint -> ux_endpoint_next_endpoint != UX_NULL)
            list_endpoint =  list_endpoint -> ux_endpoint_next_endpoint;

        /* Hook the endpoint.  */
        list_endpoint -> ux_endpoint_next_endpoint =  endpoint;
    }

    /* Return successful status.  */
    return(UX_SUCCESS);
}
    
