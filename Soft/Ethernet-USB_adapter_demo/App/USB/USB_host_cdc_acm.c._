#include   "App.h"
#include   "ux_host_class_cdc_acm.h"
#include   "ux_host_class_cdc_ecm.h"
#include   "USB_host_cdc_acm.h"

#define USBX_HOST_MEMORY_SIZE      (32 * 1024)

uint8_t                   usb_mem_buff1[USBX_HOST_MEMORY_SIZE] @ ".sram2";
uint8_t                   usb_mem_buff2[USBX_HOST_MEMORY_SIZE] @ ".sram2";


static  UX_HOST_CLASS_CDC_ACM      *p_host_cdc_acm = UX_NULL;
#define CDCACM_CONNECTED            BIT(0)
#define CDCACM_REMOVED              BIT(1)

TX_EVENT_FLAGS_GROUP           usb_hcd_flag;

uint32_t usb_host_status = TX_FEATURE_NOT_ENABLED;

uint8_t  usb_cdc_acm_dev_connected = 0;

extern UINT  _ux_hcd_stm32_initialize_fscore(UX_HCD *hcd);
extern UINT  _ux_hcd_stm32_initialize(UX_HCD *hcd);

/*-----------------------------------------------------------------------------------------------------


  \param hcd

  \return UINT
-----------------------------------------------------------------------------------------------------*/
static UINT _USB_initialize_hcd_transfer_support(UX_HCD *hcd)
{
  //return (UINT)_ux_hcd_stm32_initialize_fscore(hcd);
  return (UINT)_ux_hcd_stm32_initialize(hcd);
}



/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
static uint32_t USB_host_init(void)
{
  UINT   status;



  ux_system_initialize(usb_mem_buff1, USBX_HOST_MEMORY_SIZE, usb_mem_buff2, USBX_HOST_MEMORY_SIZE);

  status =  ux_host_stack_initialize(USB_host_change_function);
  if (status == UX_SUCCESS)
  {
    status = UX_SUCCESS;
    //status =  ux_host_stack_class_register(_ux_system_host_class_cdc_acm_name, ux_host_class_cdc_acm_entry);
    if (status == UX_SUCCESS)
    {
      status =  ux_host_stack_class_register(_ux_system_host_class_cdc_ecm_name, ux_host_class_cdc_ecm_entry);
      if (status == UX_SUCCESS)
      {
        /* Initialize the LL driver */
        MX_USB_OTG_FS_HCD_Init();

        status = ux_host_stack_hcd_register((UCHAR *) "ux_hcd_fs_0", _USB_initialize_hcd_transfer_support, USB2_OTG_FS_PERIPH_BASE, (ULONG)&hhcd_USB_OTG_FS);

        HAL_HCD_Start(&hhcd_USB_OTG_FS);
      }
    }
  }
  return  status;
}


/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
uint32_t USB_host_cdc_acm_init(void)
{
  uint32_t status;

  status = tx_event_flags_create(&usb_hcd_flag, (CHAR *) "USB_Host");

  if (status == TX_SUCCESS)
  {
    status = USB_host_init();
    if (status == UX_SUCCESS)
    {
      status = TX_SUCCESS;
    }
  }

  usb_host_status = status;

  return status;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t USB_host_get_status(void)
{
  return usb_host_status;
}
/*-----------------------------------------------------------------------------------------------------
   Функция вызывается по цепочке: 
   ux_system_host_enum_thread->_ux_host_stack_rh_change_process->_ux_host_stack_rh_device_insertion->...
   _ux_host_stack_new_device_create->_ux_host_stack_class_interface_scan->_ux_host_stack_configuration_interface_scan->...
   _ux_host_class_cdc_ecm_entry->_ux_host_class_cdc_ecm_activate

  \param event
  \param host_class
  \param instance

  \return UINT
-----------------------------------------------------------------------------------------------------*/
UINT USB_host_change_function(ULONG event, UX_HOST_CLASS *host_class, VOID *instance)
{

  if (event == UX_DEVICE_INSERTION) /* Check if there is a device insertion. */
  {
    if (_ux_utility_memory_compare(_ux_system_host_class_cdc_ecm_name, host_class->ux_host_class_name ,_ux_utility_string_length_get(_ux_system_host_class_cdc_ecm_name)) == UX_SUCCESS)
    {
      UX_HOST_CLASS_CDC_ECM  *p_class = UX_NULL;

      p_class = (UX_HOST_CLASS_CDC_ECM *)instance;

      ULONG idVendor  = p_class->ux_host_class_cdc_ecm_device->ux_device_descriptor.idVendor;
      ULONG idProduct = p_class->ux_host_class_cdc_ecm_device->ux_device_descriptor.idProduct;
      ULONG dev_state = p_class->ux_host_class_cdc_ecm_device->ux_device_state;

      ULONG interface_id  = p_class->ux_host_class_cdc_ecm_interface_data->ux_interface_descriptor.bInterfaceClass;
      ULONG interface_num = p_class->ux_host_class_cdc_ecm_interface_data->ux_interface_descriptor.bInterfaceNumber;
      APPLOG("Inserted USB device VID=%04X PID=%04X Intf.Class=%04X Intf.Num=%d Dev.State=%d. Class=%s", idVendor , idProduct, interface_id, interface_num, dev_state, host_class);

      uint8_t *m = p_class->ux_host_class_cdc_ecm_node_id;
      // К этому моменту уже была выполнена функция _ux_network_driver_activate
      APPLOG("USB ECM MAC ADDRESS: %02X %02X %02X %02X %02X %02X %02X %02X", m[0],m[1],m[2],m[3],m[4],m[5],m[6]);
      // Далее из контекста _ux_host_class_cdc_ecm_thread будет вызвана функция _ux_network_driver_link_up
      // в которой будет установлен флаг usb_network_device_ptr -> ux_network_device_usb_link_up = NX_TRUE;
      // и будет установлен флаг  usb_network_device_ptr -> ux_network_device_link_status = NX_TRUE при условии запущенного сетевого стека

    }
    else if (_ux_utility_memory_compare(_ux_system_host_class_cdc_acm_name, host_class->ux_host_class_name ,_ux_utility_string_length_get(_ux_system_host_class_cdc_acm_name)) == UX_SUCCESS)
    {
      UX_HOST_CLASS_CDC_ACM  *p_class = UX_NULL;

      p_class = (UX_HOST_CLASS_CDC_ACM *)instance;

      ULONG idVendor  = p_class->ux_host_class_cdc_acm_device->ux_device_descriptor.idVendor;
      ULONG idProduct = p_class->ux_host_class_cdc_acm_device->ux_device_descriptor.idProduct;
      ULONG dev_state = p_class->ux_host_class_cdc_acm_device->ux_device_state;

      ULONG interface_id  = p_class->ux_host_class_cdc_acm_interface->ux_interface_descriptor.bInterfaceClass;
      ULONG interface_num = p_class->ux_host_class_cdc_acm_interface->ux_interface_descriptor.bInterfaceNumber;
      APPLOG("Inserted USB device VID=%04X PID=%04X Intf.Class=%04X Intf.Num=%d Dev.State=%d. Class=%s", idVendor , idProduct, interface_id, interface_num, dev_state, host_class);

      if (p_class->ux_host_class_cdc_acm_interface->ux_interface_descriptor.bInterfaceClass != UX_HOST_CLASS_CDC_DATA_CLASS)
      {
          /* It seems the DATA class is on the second interface. Or we hope !  */
        p_class = p_class->ux_host_class_cdc_acm_next_instance;

        if (p_class != UX_NULL)
        {
        /* Check again this interface, if this is not the data interface, we give up.  */
          if (p_class->ux_host_class_cdc_acm_interface->ux_interface_descriptor.bInterfaceClass != UX_HOST_CLASS_CDC_DATA_CLASS)
          {
            /* We did not find a proper data interface. */
            p_class = UX_NULL;
          }
        }
      }

      // Принимаем только устройство с классом UX_HOST_CLASS_CDC_DATA_CLASS и номером интерфейса 1 (устройства могут иметь несколько интерфейсов)
      if ((p_class != UX_NULL) && (interface_num == 1))
      {
        APPLOG("USB device class accepted.");
        p_host_cdc_acm = p_class;
        tx_event_flags_set(&usb_hcd_flag, CDCACM_CONNECTED, TX_OR);
        //M2M_set_interface_active_flag(1);
        usb_cdc_acm_dev_connected = 1;
      }

    }

  }
  else if (event == UX_DEVICE_REMOVAL) /* Check if there is a device removal. */
  {
    APPLOG("Removed USB device.");
    tx_event_flags_set(&usb_hcd_flag, CDCACM_REMOVED, TX_OR);
    p_host_cdc_acm = UX_NULL;
    //M2M_set_interface_active_flag(0);
    usb_cdc_acm_dev_connected = 0;
  }

  return UX_SUCCESS;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
uint8_t USB_host_cdc_acm_is_connected(void)
{
  return  usb_cdc_acm_dev_connected;
}
/*-----------------------------------------------------------------------------------------------------


  \param in_buf
  \param len
  \param received
  \param timeout_ticks

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t USB_host_cdc_acm_read(uint8_t *in_buf, uint32_t len, uint32_t *received,  uint32_t timeout_ticks)
{
  ULONG actual_length;
  UINT  status;

  *received = 0;
  if (usb_cdc_acm_dev_connected)
  {
    p_host_cdc_acm->ux_host_class_cdc_acm_bulk_in_endpoint->ux_endpoint_transfer_request.ux_transfer_request_timeout_value = timeout_ticks;
    status = ux_host_class_cdc_acm_read(p_host_cdc_acm,  in_buf, len ,&actual_length);
    if (status == UX_SUCCESS)
    {
      *received = actual_length;
    }
    else
    {
      return status;
    }
  }
  else
  {
    tx_thread_sleep(timeout_ticks);
    return UX_NO_DEVICE_CONNECTED;
  }
  return TX_SUCCESS;
}

/*-----------------------------------------------------------------------------------------------------


  \param out_buf
  \param len
  \param timeout_ticks

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t USB_host_cdc_acm_write(uint8_t *out_buf, uint32_t len, uint32_t timeout_ticks)
{
  UINT   status;
  ULONG  actual_length;

  if (usb_cdc_acm_dev_connected == 0) return UX_NO_DEVICE_CONNECTED;

  do
  {
    p_host_cdc_acm->ux_host_class_cdc_acm_bulk_in_endpoint->ux_endpoint_transfer_request.ux_transfer_request_timeout_value = timeout_ticks;
    status = ux_host_class_cdc_acm_write(p_host_cdc_acm, out_buf, len ,&actual_length);
    if (status != UX_SUCCESS)
    {
      return status;
    }
    len -= actual_length;
  } while (len != 0);

  return status;
}

