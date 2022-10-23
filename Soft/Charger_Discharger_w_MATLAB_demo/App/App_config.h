#ifndef MAIN_CONFIG_H
  #define MAIN_CONFIG_H


#define SD_INSTANCE                         0

/* COM define */
#define USE_COM_LOG                         0U
#define USE_BSP_COM_FEATURE                 0U

/* I2C BUS timing define */
#define I2C_VALID_TIMING_NBR                128U

/* Audio codecs defines */
#define USE_AUDIO_CODEC_WM8994              1U
#define USE_BSP_PDM_LIB_FEATURE             1U

/* Default Audio IN internal buffer size */
#define DEFAULT_AUDIO_IN_BUFFER_SIZE        2048U
#define USE_BSP_CPU_CACHE_MAINTENANCE       1U

/* LCD defines */
#define LCD_LAYER_0_ADDRESS                 0x70000000U
#define LCD_LAYER_1_ADDRESS                 0x70200000U
#define USE_DMA2D_TO_FILL_RGB_RECT          0U
#define USE_LCD_CTRL_RK043FN48H             1U

/* TS defines */
#define USE_TS_GESTURE                      1U
#define USE_TS_MULTI_TOUCH                  1U
#define TS_TOUCH_NBR                        2U

/* OSPI RAM interrupt priority */
#define BSP_OSPI_RAM_IT_PRIORITY            0x07UL
#define BSP_OSPI_RAM_DMA_IT_PRIORITY        0x07UL

/* IRQ priorities */
#define BSP_BUTTON_USER_IT_PRIORITY         15U
#define BSP_AUDIO_OUT_IT_PRIORITY           14U
#define BSP_AUDIO_IN_IT_PRIORITY            15U
#define BSP_SD_IT_PRIORITY                  14U
#define BSP_SD_RX_IT_PRIORITY               14U
#define BSP_SD_TX_IT_PRIORITY               15U
#define BSP_TS_IT_PRIORITY                  15U


#endif