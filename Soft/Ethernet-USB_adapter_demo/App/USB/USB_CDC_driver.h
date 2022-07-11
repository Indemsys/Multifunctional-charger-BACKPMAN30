#ifndef USB_CDC_DRIVER_H
  #define USB_CDC_DRIVER_H



#define   USB_DRV_STR_SZ               512
#define   USB_DRV_RECV_THREAD_STACK_SZ 2048
#define   IN_BUF_QUANTITY              2           // Количество приемных буферов
#define   USBDRV_BUFFER_MAX_LENGTH     512



// Ведем прием циклически в N приемных буферов
typedef struct
{
    uint32_t  len;  // Длина пакета
    uint8_t   buff[USBDRV_BUFFER_MAX_LENGTH]; // Буфер с пакетов

} T_rx_pack_cbl;


typedef struct
{
    volatile uint8_t       ready_to_send;
    volatile uint8_t       ready_to_receive;
    UX_SLAVE_CLASS_CDC_ACM *cdc;
    char                   str[USB_DRV_STR_SZ];
    TX_THREAD              recv_thread; // Задача прием
    char                   recv_task_name[32];
    uint8_t                recv_thread_stack[USB_DRV_RECV_THREAD_STACK_SZ];
    TX_EVENT_FLAGS_GROUP   evt;         // Группа флагов для взаимодействия с задачей приема
    char                   evt_grp_name[32];
    void                  *dbuf;        // Указатель на буфер с принимаемыми данными
    uint32_t               dsz;         // Количество принимаемых байт

    volatile int32_t       head_n;     //  Индекс головы циклической очереди буферов приема
    volatile int32_t       tail_n;     //  Индекс хвоста циклической очереди буферов приема
    volatile int32_t       no_space;   //  Флаг заполненнной очереди приемных буфферов
    int32_t                rd_pos;     //  Позиция чтения в текущем буфере
    T_rx_pack_cbl          rd_pack[IN_BUF_QUANTITY]; //  Массив управляющих структур приема-обработки входящих пакетов


} T_usbfs_drv_cbl;


T_serial_io_driver* Mnsdrv_get_usbfs_vcom0_driver(void);
T_serial_io_driver* Mnsdrv_get_usbfs_vcom1_driver(void);


VOID    ux_cdc_device0_instance_activate(VOID *cdc_instance);
VOID    ux_cdc_device0_instance_deactivate(VOID *cdc_instance);
VOID    ux_cdc_device0_parameter_change(VOID *cdc_instance);

VOID    ux_cdc_device1_instance_activate(VOID *cdc_instance);
VOID    ux_cdc_device1_instance_deactivate(VOID *cdc_instance);
VOID    ux_cdc_device1_parameter_change(VOID *cdc_instance);


UINT    Register_USB_CDC_ACM_class(void);

#endif // USB_CDC_DRIVER_H



