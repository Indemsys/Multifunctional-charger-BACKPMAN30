#ifndef SERIAL_DRIVER_H
  #define SERIAL_DRIVER_H


  #define GET_MCBL     T_monitor_cbl      *mcbl = (T_monitor_cbl*)(tx_thread_identify()->environment); \
                       T_serial_io_driver *mdrv = (T_serial_io_driver*)(tx_thread_identify()->driver);

  #define MPRINTF      mdrv->_printf
  #define WAIT_CHAR    mdrv->_wait_char
  #define SEND_BUF     mdrv->_send_buf


  #define MN_DRIVER_MARK         0x87654321
  #define MN_RTT0_DRIVER         1
  #define MN_USBFS_VCOM0_DRIVER  2
  #define MN_USBFS_VCOM1_DRIVER  3
  #define MN_RNDIS_TELNET_DRIVER 5


typedef struct
{
    const uint32_t   mark;                                            // Magic word. Идентифицирует блок как управляющую структуру драйвера
    const int        driver_type;                                     // Идентификатор типа драйвера
    int              (*_init)(void **pcbl, void *pdrv);               // Инициализация
    int              (*_send_buf)(const void *buf, unsigned int len); // Отсылка буфера с данными
    int              (*_wait_char)(unsigned char *b,  int ticks);     // Ожидание символа. ticks - время ожидания выражается в тиках (если 0 то без ожидания)
    int              (*_printf)(const char *, ...);                   // Вывод форматированной строки
    int              (*_deinit)(void **pcbl);                         // Деинициализация
    void              *pdrvcbl;                                       // Указатель на управляющую структуру необходимую для работы драйвера
} T_serial_io_driver;


#endif // SERIAL_DRIVER_H



