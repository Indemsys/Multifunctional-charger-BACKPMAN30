#ifndef FX_STM32_SD_DRIVER_H
#define FX_STM32_SD_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif


/* Default timeout used to wait for fx operations */
#define FX_STM32_SD_DEFAULT_TIMEOUT                      (10 * TX_TIMER_TICKS_PER_SECOND)

/* Let the filex low-level driver initialize the SD driver */
#define FX_STM32_SD_INIT                                 1

/* Enable the cache mainatenance, required when using the SD DMA */
#define FX_STM32_SD_CACHE_MAINTENANCE                    0


/* SDIO instance to be used by FileX */
#define FX_STM32_SD_INSTANCE                             0

/* Default sector size, used by the driver */
#define FX_STM32_SD_DEFAULT_SECTOR_SIZE                  512


VOID    STM32_SD_driver(FX_MEDIA *media_ptr);
UINT    SD_FX_read_data(FX_MEDIA *media_ptr, ULONG sector, UINT num_sectors, UINT use_scratch_buffer);
UINT    SD_FX_write_data(FX_MEDIA *media_ptr, ULONG sector, UINT num_sectors, UINT use_scratch_buffer);
UINT    SD_read_data(uint8_t *buf, ULONG start_sector, UINT num_sectors);
UINT    SD_write_data(uint8_t *buf, ULONG start_sector, UINT num_sectors);

#ifdef __cplusplus
}
#endif

#endif /* FX_STM32_SD_DRIVER_H */
