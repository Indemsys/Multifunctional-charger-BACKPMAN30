#include "App.h"

const T_IO_pins_configuration BACKPMAN3_pins_conf[] =
{
  //  gpio   pin_num   MODER  AFR    OSPEEDR OTYPER  PUPDR    init
  //
  { GPIOA,   0,        ANAL,  ALT00,  SPD_L, OUT_PP, PUPD_DIS, 0 }, // VREF_1.65V                          PA0      G2    ADD=(ADC1_INP16/WKUP0)  AF0=(-)  AF1=(TIM2_CH1/TIM2_ETR)  AF2=(TIM5_CH1)  AF3=(TIM8_ETR)  AF4=(TIM15_BKIN)  AF5=(-)  AF6=(-)  AF7=(USART2_CTS/USART2_NSS)  AF8=(UART4_TX)  AF9=(SDMMC2_CMD)  AF10=(SAI2_SD_B)  AF11=(ETH_MII_CRS)  AF12=(-)  AF13=(-)  AF14=(-)
  { GPIOA,   1,        ANAL,  ALT00,  SPD_L, OUT_PP, PUPD_DIS, 0 }, // AIN2                                PA1      H2    ADD=(ADC1_INN16/ADC1_INP17)  AF0=(-)  AF1=(TIM2_CH2)  AF2=(TIM5_CH2)  AF3=(LPTIM3_OUT)  AF4=(TIM15_CH1N)  AF5=(-)  AF6=(-)  AF7=(USART2_RTS/USART2_DE)  AF8=(UART4_RX)  AF9=(QUADSPI_BK1_IO3)  AF10=(SAI2_MCLK_B)  AF11=(ETH_MII_RX_CLK/ETH_RMII_REF_CLK)  AF12=(-)  AF13=(-)  AF14=(LCD_R2)
  { GPIOA,   2,        ANAL,  ALT00,  SPD_L, OUT_PP, PUPD_DIS, 0 }, // AIN1                                PA2      J2    ADD=(ADC12_INP14/WKUP1)  AF0=(-)  AF1=(TIM2_CH3)  AF2=(TIM5_CH3)  AF3=(LPTIM4_OUT)  AF4=(TIM15_CH1)  AF5=(-)  AF6=(-)  AF7=(USART2_TX)  AF8=(SAI2_SCK_B)  AF9=(-)  AF10=(-)  AF11=(ETH_MDIO)  AF12=(MDIOS_MDIO)  AF13=(-)  AF14=(LCD_R1)
  { GPIOA,   3,        ANAL,  ALT00,  SPD_L, OUT_PP, PUPD_DIS, 0 }, // AIN3                                PA3      K2    ADD=(ADC12_INP15)  AF0=(-)  AF1=(TIM2_CH4)  AF2=(TIM5_CH4)  AF3=(LPTIM5_OUT)  AF4=(TIM15_CH2)  AF5=(-)  AF6=(-)  AF7=(USART2_RX)  AF8=(-)  AF9=(LCD_B2)  AF10=(OTG_HS_ULPI_D0)  AF11=(ETH_MII_COL)  AF12=(-)  AF13=(-)  AF14=(LCD_B5)
  { GPIOA,   4,        GPIO,  ALT00,  SPD_V, OUT_PP, PUPD_DIS, 1 }, // SPI1_NSS Управление DAC DC/DC       PA4      G3    ADD=(ADC12_INP18/DAC1_OUT1)  AF0=(D1PWREN)  AF1=(-)  AF2=(TIM5_ETR)  AF3=(-)  AF4=(-)  AF5=(SPI1_NSS/I2S1_WS)  AF6=(SPI3_NSS/I2S3_WS)  AF7=(USART2_CK)  AF8=(SPI6_NSS)  AF9=(-)  AF10=(-)  AF11=(-)  AF12=(OTG_HS_SOF)  AF13=(DCMI_HSYNC)  AF14=(LCD_VSYNC)
  { GPIOA,   5,        ALTF,  ALT05,  SPD_V, OUT_PP, PUPD_DIS, 0 }, // SPI1_SCK Управление DAC DC/DC       PA5      H3    ADD=(ADC12_INN18/ADC12_INP19/DAC1_OUT2)  AF0=(D2PWREN)  AF1=(TIM2_CH1/TIM2_ETR)  AF2=(-)  AF3=(TIM8_CH1N)  AF4=(-)  AF5=(SPI1_SCK/I2S1_CK)  AF6=(-)  AF7=(-)  AF8=(SPI6_SCK)  AF9=(-)  AF10=(OTG_HS_ULPI_CK)  AF11=(-)  AF12=(-)  AF13=(-)  AF14=(LCD_R4)
  { GPIOA,   6,        ANAL,  ALT00,  SPD_L, OUT_PP, PUPD_DIS, 0 }, // ACC_I    Сигнал с TMCS1100A2QDRQ1   PA6      J3    ADD=(ADC12_INP3)  AF0=(-)  AF1=(TIM1_BKIN)  AF2=(TIM3_CH1)  AF3=(TIM8_BKIN)  AF4=(-)  AF5=(SPI1_MISO/I2S1_SDI)  AF6=(-)  AF7=(-)  AF8=(SPI6_MISO)  AF9=(TIM13_CH1)  AF10=(TIM8_BKIN_COMP12)  AF11=(MDIOS_MDC)  AF12=(TIM1_BKIN_COMP12)  AF13=(DCMI_PIXCLK)  AF14=(LCD_G2)
  { GPIOA,   7,        ANAL,  ALT00,  SPD_L, OUT_PP, PUPD_DIS, 0 }, // PSRC_I   Сигнал с TMCS1100A2QDRQ1   PA7      K3    ADD=(ADC12_INN3/ADC12_INP7/OPAMP1_VINM)  AF0=(-)  AF1=(TIM1_CH1N)  AF2=(TIM3_CH2)  AF3=(TIM8_CH1N)  AF4=(-)  AF5=(SPI1_MOSI/I2S1_SDO)  AF6=(-)  AF7=(-)  AF8=(SPI6_MOSI)  AF9=(TIM14_CH1)  AF10=(-)  AF11=(ETH_MII_RX_DV/ETH_RMII_CRS_DV)  AF12=(FMC_SDNWE)  AF13=(-)  AF14=(-)
  { GPIOA,   8,        ALTF,  ALT01,  SPD_L, OUT_PP, PUPD_DIS, 0 }, // ENC_A    (TIM1_CH1)                 PA8      D9    ADD=()  AF0=(MCO1)  AF1=(TIM1_CH1)  AF2=(HRTIM_CHB2)  AF3=(TIM8_BKIN2)  AF4=(I2C3_SCL)  AF5=(-)  AF6=(-)  AF7=(USART1_CK)  AF8=(-)  AF9=(-)  AF10=(OTG_FS_SOF)  AF11=(UART7_RX)  AF12=(TIM8_BKIN2_COMP12)  AF13=(LCD_B3)  AF14=(LCD_R6)
  { GPIOA,   9,        INPT,  ALT00,  SPD_L, OUT_PP, PUPD_DIS, 0 }, // USB_VBUS                            PA9      C9    ADD=(OTG_FS_VBUS)  AF0=(-)  AF1=(TIM1_CH2)  AF2=(HRTIM_CHC1)  AF3=(LPUART1_TX)  AF4=(I2C3_SMBA)  AF5=(SPI2_SCK/I2S2_CK)  AF6=(-)  AF7=(USART1_TX)  AF8=(-)  AF9=(FDCAN1_RXFD_MODE)  AF10=(-)  AF11=(-)  AF12=(-)  AF13=(DCMI_D0)  AF14=(LCD_R5)
  { GPIOA,  10,        ALTF,  ALT10,  SPD_L, OUT_PP, PUPD_DIS, 0 }, // USB_ID                              PA10     D10   ADD=()  AF0=(-)  AF1=(TIM1_CH3)  AF2=(HRTIM_CHC2)  AF3=(LPUART1_RX)  AF4=(-)  AF5=(-)  AF6=(-)  AF7=(USART1_RX)  AF8=(-)  AF9=(FDCAN1_TXFD_MODE)  AF10=(OTG_FS_ID)  AF11=(MDIOS_MDIO)  AF12=(LCD_B4)  AF13=(DCMI_D1)  AF14=(LCD_B1)
  { GPIOA,  11,        ALTF,  ALT10,  SPD_V, OUT_PP, PUPD_DIS, 0 }, // USB_N                               PA11     C10   ADD=()  AF0=(-)  AF1=(TIM1_CH4)  AF2=(HRTIM_CHD1)  AF3=(LPUART1_CTS)  AF4=(-)  AF5=(SPI2_NSS/I2S2_WS)  AF6=(UART4_RX)  AF7=(USART1_CTS/USART1_NSS)  AF8=(-)  AF9=(FDCAN1_RX)  AF10=(OTG_FS_DM)  AF11=(-)  AF12=(-)  AF13=(-)  AF14=(LCD_R4)
  { GPIOA,  12,        ALTF,  ALT10,  SPD_V, OUT_PP, PUPD_DIS, 0 }, // USB_P                               PA12     B10   ADD=()  AF0=(-)  AF1=(TIM1_ETR)  AF2=(HRTIM_CHD2)  AF3=(LPUART1_RTS/LPUART1_DE)  AF4=(-)  AF5=(SPI2_SCK/I2S2_CK)  AF6=(UART4_TX)  AF7=(USART1_RTS/USART1_DE)  AF8=(SAI2_FS_B)  AF9=(FDCAN1_TX)  AF10=(OTG_FS_DP)  AF11=(-)  AF12=(-)  AF13=(-)  AF14=(LCD_R5)
  { GPIOA,  13,        ALTF,  ALT00,  SPD_V, OUT_PP, PULL__UP, 0 }, // SWD_DIO                             PA13     A10   ADD=()  AF0=(JTMS-SWDIO)  AF1=(-)  AF2=(-)  AF3=(-)  AF4=(-)  AF5=(-)  AF6=(-)  AF7=(-)  AF8=(-)  AF9=(-)  AF10=(-)  AF11=(-)  AF12=(-)  AF13=(-)  AF14=(-)
  { GPIOA,  14,        ALTF,  ALT00,  SPD_L, OUT_PP, PULL__DN, 0 }, // SWD_CLK                             PA14     A9    ADD=()  AF0=(JTCK-SWCLK)  AF1=(-)  AF2=(-)  AF3=(-)  AF4=(-)  AF5=(-)  AF6=(-)  AF7=(-)  AF8=(-)  AF9=(-)  AF10=(-)  AF11=(-)  AF12=(-)  AF13=(-)  AF14=(-)
  { GPIOA,  15,        GPIO,  ALT00,  SPD_L, OUT_PP, PUPD_DIS, 0 }, // LED_GR                              PA15     A8    ADD=()  AF0=(JTDI)  AF1=(TIM2_CH1/TIM2_ETR)  AF2=(HRTIM_FLT1)  AF3=(-)  AF4=(CEC)  AF5=(SPI1_NSS/I2S1_WS)  AF6=(SPI3_NSS/I2S3_WS)  AF7=(SPI6_NSS)  AF8=(UART4_RTS/UART4_DE)  AF9=(-)  AF10=(-)  AF11=(UART7_TX)  AF12=(-)  AF13=(-)  AF14=(-)

  { GPIOB,   0,        ANAL,  ALT00,  SPD_L, OUT_PP, PUPD_DIS, 0 }, // SYS_V                               PB0      J4    ADD=(ADC12_INN5/ADC12_INP9/OPAMP1_VINP/COMP1_INP)  AF0=(-)  AF1=(TIM1_CH2N)  AF2=(TIM3_CH3)  AF3=(TIM8_CH2N)  AF4=(-)  AF5=(-)  AF6=(DFSDM1_CKOUT)  AF7=(-)  AF8=(UART4_CTS)  AF9=(LCD_R3)  AF10=(OTG_HS_ULPI_D1)  AF11=(ETH_MII_RXD2)  AF12=(-)  AF13=(-)  AF14=(LCD_G1)
  { GPIOB,   1,        ANAL,  ALT00,  SPD_L, OUT_PP, PUPD_DIS, 0 }, // AIN4                                PB1      K4    ADD=(ADC12_INP5/COMP1_INM)  AF0=(-)  AF1=(TIM1_CH3N)  AF2=(TIM3_CH4)  AF3=(TIM8_CH3N)  AF4=(-)  AF5=(-)  AF6=(DFSDM1_DATIN1)  AF7=(-)  AF8=(-)  AF9=(LCD_R6)  AF10=(OTG_HS_ULPI_D2)  AF11=(ETH_MII_RXD3)  AF12=(-)  AF13=(-)  AF14=(LCD_G0)
  { GPIOB,   2,        INPT,  ALT00,  SPD_L, OUT_PP, PULL__DN, 0 }, // -                                   PB2      G5    ADD=(COMP1_INP)  AF0=(RTC_OUT)  AF1=(-)  AF2=(SAI1_D1)  AF3=(-)  AF4=(DFSDM1_CKIN1)  AF5=(-)  AF6=(SAI1_SD_A)  AF7=(SPI3_MOSI/I2S3_SDO)  AF8=(SAI4_SD_A)  AF9=(QUADSPI_CLK)  AF10=(SAI4_D1)  AF11=(-)  AF12=(-)  AF13=(-)  AF14=(-)
  { GPIOB,   3,        ALTF,  ALT00,  SPD_V, OUT_PP, PUPD_DIS, 0 }, // SWO                                 PB3      A7    ADD=()  AF0=(JTDO/TRACESWO)  AF1=(TIM2_CH2)  AF2=(HRTIM_FLT4)  AF3=(-)  AF4=(-)  AF5=(SPI1_SCK/I2S1_CK)  AF6=(SPI3_SCK/I2S3_CK)  AF7=(-)  AF8=(SPI6_SCK)  AF9=(SDMMC2_D2)  AF10=(CRS_SYNC)  AF11=(UART7_RX)  AF12=(-)  AF13=(-)  AF14=(-)
  { GPIOB,   4,        INPT,  ALT00,  SPD_L, OUT_PP, PULL__DN, 0 }, // -                                   PB4      A6    ADD=()  AF0=(NJTRST)  AF1=(TIM16_BKIN)  AF2=(TIM3_CH1)  AF3=(HRTIM_EEV6)  AF4=(-)  AF5=(SPI1_MISO/I2S1_SDI)  AF6=(SPI3_MISO/I2S3_SDI)  AF7=(SPI2_NSS/I2S2_WS)  AF8=(SPI6_MISO)  AF9=(SDMMC2_D3)  AF10=(-)  AF11=(UART7_TX)  AF12=(-)  AF13=(-)  AF14=(-)
  { GPIOB,   5,        ALTF,  ALT05,  SPD_V, OUT_PP, PUPD_DIS, 0 }, // SPI1_MOSI Управление DAC DC/DC      PB5      C5    ADD=()  AF0=(-)  AF1=(TIM17_BKIN)  AF2=(TIM3_CH2)  AF3=(HRTIM_EEV7)  AF4=(I2C1_SMBA)  AF5=(SPI1_MOSI/I2S1_SDO)  AF6=(I2C4_SMBA)  AF7=(SPI3_MOSI/I2S3_SDO)  AF8=(SPI6_MOSI)  AF9=(FDCAN2_RX)  AF10=(OTG_HS_ULPI_D7)  AF11=(ETH_PPS_OUT)  AF12=(FMC_SDCKE1)  AF13=(DCMI_D10)  AF14=(UART5_RX)
  { GPIOB,   6,        INPT,  ALT00,  SPD_L, OUT_OD, PULL__DN, 0 }, // -                                   PB6      B5    ADD=()  AF0=(-)  AF1=(TIM16_CH1N)  AF2=(TIM4_CH1)  AF3=(HRTIM_EEV8)  AF4=(I2C1_SCL)  AF5=(CEC)  AF6=(I2C4_SCL)  AF7=(USART1_TX)  AF8=(LPUART1_TX)  AF9=(FDCAN2_TX)  AF10=(QUADSPI_BK1_NCS)  AF11=(DFSDM1_DATIN5)  AF12=(FMC_SDNE1)  AF13=(DCMI_D5)  AF14=(UART5_TX)
  { GPIOB,   7,        INPT,  ALT00,  SPD_L, OUT_OD, PULL__DN, 0 }, // -                                   PB7      A5    ADD=(PVD_IN)  AF0=(-)  AF1=(TIM17_CH1N)  AF2=(TIM4_CH2)  AF3=(HRTIM_EEV9)  AF4=(I2C1_SDA)  AF5=(-)  AF6=(I2C4_SDA)  AF7=(USART1_RX)  AF8=(LPUART1_RX)  AF9=(FDCAN2_TXFD_MODE)  AF10=(-)  AF11=(DFSDM1_CKIN5)  AF12=(FMC_NL)  AF13=(DCMI_VSYNC)  AF14=(-)
  { GPIOB,   8,        GPIO,  ALT00,  SPD_L, OUT_PP, PUPD_DIS, 0 }, // ASW_R Управление ключом Q3          PB8      B4    ADD=()  AF0=(-)  AF1=(TIM16_CH1)  AF2=(TIM4_CH3)  AF3=(DFSDM1_CKIN7)  AF4=(I2C1_SCL)  AF5=(-)  AF6=(I2C4_SCL)  AF7=(SDMMC1_CKIN)  AF8=(UART4_RX)  AF9=(FDCAN1_RX)  AF10=(SDMMC2_D4)  AF11=(ETH_MII_TXD3)  AF12=(SDMMC1_D4)  AF13=(DCMI_D6)  AF14=(LCD_B6)
  { GPIOB,   9,        GPIO,  ALT00,  SPD_L, OUT_PP, PUPD_DIS, 0 }, // ASW_F Управление ключом Q4          PB9      A4    ADD=()  AF0=(-)  AF1=(TIM17_CH1)  AF2=(TIM4_CH4)  AF3=(DFSDM1_DATIN7)  AF4=(I2C1_SDA)  AF5=(SPI2_NSS/I2S2_WS)  AF6=(I2C4_SDA)  AF7=(SDMMC1_CDIR)  AF8=(UART4_TX)  AF9=(FDCAN1_TX)  AF10=(SDMMC2_D5)  AF11=(I2C4_SMBA)  AF12=(SDMMC1_D5)  AF13=(DCMI_D7)  AF14=(LCD_B7)
  { GPIOB,  10,        GPIO,  ALT00,  SPD_L, OUT_PP, PUPD_DIS, 0 }, // LED_RD                              PB10     J7    ADD=()  AF0=(-)  AF1=(TIM2_CH3)  AF2=(HRTIM_SCOUT)  AF3=(LPTIM2_IN1)  AF4=(I2C2_SCL)  AF5=(SPI2_SCK/I2S2_CK)  AF6=(DFSDM1_DATIN7)  AF7=(USART3_TX)  AF8=(-)  AF9=(QUADSPI_BK1_NCS)  AF10=(OTG_HS_ULPI_D3)  AF11=(ETH_MII_RX_ER)  AF12=(-)  AF13=(-)  AF14=(LCD_G4)
  { GPIOB,  11,        INPT,  ALT00,  SPD_L, OUT_PP, PULL__DN, 0 }, // -                                   PB11     K7    ADD=()  AF0=(-)  AF1=(TIM2_CH4)  AF2=(HRTIM_SCIN)  AF3=(LPTIM2_ETR)  AF4=(I2C2_SDA)  AF5=(-)  AF6=(DFSDM1_CKIN7)  AF7=(USART3_RX)  AF8=(-)  AF9=(-)  AF10=(OTG_HS_ULPI_D4)  AF11=(ETH_MII_TX_EN/ETH_RMII_TX_EN)  AF12=(-)  AF13=(-)  AF14=(LCD_G5)
  { GPIOB,  12,        INPT,  ALT00,  SPD_L, OUT_PP, PULL__DN, 0 }, // -                                   PB12     K8    ADD=()  AF0=(-)  AF1=(TIM1_BKIN)  AF2=(-)  AF3=(-)  AF4=(I2C2_SMBA)  AF5=(SPI2_NSS/I2S2_WS)  AF6=(DFSDM1_DATIN1)  AF7=(USART3_CK)  AF8=(-)  AF9=(FDCAN2_RX)  AF10=(OTG_HS_ULPI_D5)  AF11=(ETH_MII_TXD0/ETH_RMII_TXD0)  AF12=(OTG_HS_ID)  AF13=(TIM1_BKIN_COMP12)  AF14=(UART5_RX)
  { GPIOB,  13,        GPIO,  ALT00,  SPD_L, OUT_PP, PUPD_DIS, 0 }, // PSW_R Управление ключом Q2          PB13     J8    ADD=(OTG_HS_VBUS)  AF0=(-)  AF1=(TIM1_CH1N)  AF2=(-)  AF3=(LPTIM2_OUT)  AF4=(-)  AF5=(SPI2_SCK/I2S2_CK)  AF6=(DFSDM1_CKIN1)  AF7=(USART3_CTS/USART3_NSS)  AF8=(-)  AF9=(FDCAN2_TX)  AF10=(OTG_HS_ULPI_D6)  AF11=(ETH_MII_TXD1/ETH_RMII_TXD1)  AF12=(-)  AF13=(-)  AF14=(UART5_TX)
  { GPIOB,  14,        GPIO,  ALT00,  SPD_L, OUT_PP, PUPD_DIS, 0 }, // PSW_F Управление ключом Q1          PB14     H10   ADD=()  AF0=(-)  AF1=(TIM1_CH2N)  AF2=(TIM12_CH1)  AF3=(TIM8_CH2N)  AF4=(USART1_TX)  AF5=(SPI2_MISO/I2S2_SDI)  AF6=(DFSDM1_DATIN2)  AF7=(USART3_RTS/USART3_DE)  AF8=(UART4_RTS/UART4_DE)  AF9=(SDMMC2_D0)  AF10=(-)  AF11=(-)  AF12=(OTG_HS_DM)  AF13=(-)  AF14=(-)
  { GPIOB,  15,        GPIO,  ALT00,  SPD_L, OUT_PP, PUPD_DIS, 1 }, // CAN_EN                              PB15     G10   ADD=()  AF0=(RTC_REFIN)  AF1=(TIM1_CH3N)  AF2=(TIM12_CH2)  AF3=(TIM8_CH3N)  AF4=(USART1_RX)  AF5=(SPI2_MOSI/I2S2_SDO)  AF6=(DFSDM1_CKIN2)  AF7=(-)  AF8=(UART4_CTS)  AF9=(SDMMC2_D1)  AF10=(-)  AF11=(-)  AF12=(OTG_HS_DP)  AF13=(-)  AF14=(-)

  { GPIOC,   0,        ANAL,  ALT00,  SPD_L, OUT_PP, PUPD_DIS, 0 }, // TERMO_SENS                          PC0      F1    ADD=(ADC123_INP10)  AF0=()  AF1=(-)  AF2=(-)  AF3=(DFSDM1_CKIN0)  AF4=(-)  AF5=(-)  AF6=(DFSDM1_DATIN4)  AF7=(-)  AF8=(SAI2_FS_B)  AF9=(-)  AF10=(OTG_HS_ULPI_STP)  AF11=(-)  AF12=(FMC_SDNWE)  AF13=(-)  AF14=(LCD_R5)
  { GPIOC,   1,        INPT,  ALT00,  SPD_L, OUT_PP, PULL__DN, 0 }, // -                                   PC1      F2    ADD=(ADC123_INN10/ADC123_INP11/RTC_TAMP3/WKUP5)  AF0=(TRACED0)  AF1=(-)  AF2=(SAI1_D1)  AF3=(DFSDM1_DATIN0)  AF4=(DFSDM1_CKIN4)  AF5=(SPI2_MOSI/I2S2_SDO)  AF6=(SAI1_SD_A)  AF7=(-)  AF8=(SAI4_SD_A)  AF9=(SDMMC2_CK)  AF10=(SAI4_D1)  AF11=(ETH_MDC)  AF12=(MDIOS_MDC)  AF13=(-)  AF14=(-)
  { GPIOC,   2,        ANAL,  ALT00,  SPD_L, OUT_PP, PUPD_DIS, 0 }, // LOAD_I                              PC2_     E2    ADD=(ADC3_INN1/ADC3_INP0)  AF0=(CDSLEEP)  AF1=(-)  AF2=(-)  AF3=(DFSDM1_CKIN1)  AF4=(-)  AF5=(SPI2_MISO/I2S2_SDI)  AF6=(DFSDM1_CKOUT)  AF7=(-)  AF8=(-)  AF9=(-)  AF10=(OTG_HS_ULPI_DIR)  AF11=(ETH_MII_TXD2)  AF12=(FMC_SDNE0)  AF13=(-)  AF14=(-)
  { GPIOC,   3,        ANAL,  ALT00,  SPD_L, OUT_PP, PUPD_DIS, 0 }, // PSRC_V                              PC3_     F3    ADD=(ADC3_INP1)  AF0=(CSLEEP)  AF1=(-)  AF2=(-)  AF3=(DFSDM1_DATIN1)  AF4=(-)  AF5=(SPI2_MOSI/I2S2_SDO)  AF6=(-)  AF7=(-)  AF8=(-)  AF9=(-)  AF10=(OTG_HS_ULPI_NXT)  AF11=(ETH_MII_TX_CLK)  AF12=(FMC_SDCKE0)  AF13=(-)  AF14=(-)
  { GPIOC,   4,        ANAL,  ALT00,  SPD_L, OUT_PP, PUPD_DIS, 0 }, // ACC_V                               PC4      G4    ADD=(ADC12_INP4/OPAMP1_VOUT/COMP1_INM)  AF0=(-)  AF1=(-)  AF2=(-)  AF3=(DFSDM1_CKIN2)  AF4=(-)  AF5=(I2S1_MCK)  AF6=(-)  AF7=(-)  AF8=(-)  AF9=(SPDIFRX1_IN3)  AF10=(-)  AF11=(ETH_MII_RXD0/ETH_RMII_RXD0)  AF12=(FMC_SDNE0)  AF13=(-)  AF14=(-)
  { GPIOC,   5,        ANAL,  ALT00,  SPD_L, OUT_PP, PUPD_DIS, 0 }, // LOAD_V                              PC5      H4    ADD=(ADC12_INN4/ADC12_INP8/OPAMP1_VINM)  AF0=(-)  AF1=(-)  AF2=(SAI1_D3)  AF3=(DFSDM1_DATIN2)  AF4=(-)  AF5=(-)  AF6=(-)  AF7=(-)  AF8=()  AF9=(SPDIFRX1_IN4)  AF10=(SAI4_D3)  AF11=(ETH_MII_RXD1/ETH_RMII_RXD1)  AF12=(FMC_SDCKE0)  AF13=(COMP1_OUT)  AF14=(-)
  { GPIOC,   6,        GPIO,  ALT00,  SPD_L, OUT_PP, PUPD_DIS, 0 }, // OUT1                                PC6      F10   ADD=(SWPMI_IO)  AF0=(-)  AF1=(HRTIM_CHA1)  AF2=(TIM3_CH1)  AF3=(TIM8_CH1)  AF4=(DFSDM1_CKIN3)  AF5=(I2S2_MCK)  AF6=(-)  AF7=(USART6_TX)  AF8=(SDMMC1_D0DIR)  AF9=(FMC_NWAIT)  AF10=(SDMMC2_D6)  AF11=(-)  AF12=(SDMMC1_D6)  AF13=(DCMI_D0)  AF14=(LCD_HSYNC)
  { GPIOC,   7,        GPIO,  ALT00,  SPD_L, OUT_PP, PUPD_DIS, 0 }, // OUT2                                PC7      E10   ADD=()  AF0=(TRGIO)  AF1=(HRTIM_CHA2)  AF2=(TIM3_CH2)  AF3=(TIM8_CH2)  AF4=(DFSDM1_DATIN3)  AF5=(-)  AF6=(I2S3_MCK)  AF7=(USART6_RX)  AF8=(SDMMC1_D123DIR)  AF9=(FMC_NE1)  AF10=(SDMMC2_D7)  AF11=(SWPMI_TX)  AF12=(SDMMC1_D7)  AF13=(DCMI_D1)  AF14=(LCD_G6)
  { GPIOC,   8,        ALTF,  ALT12,  SPD_H, OUT_PP, PULL__UP, 0 }, // SDMMC_D0                            PC8      F9    ADD=()  AF0=(TRACED1)  AF1=(HRTIM_CHB1)  AF2=(TIM3_CH3)  AF3=(TIM8_CH3)  AF4=(-)  AF5=(-)  AF6=(-)  AF7=(USART6_CK)  AF8=(UART5_RTS/UART5_DE)  AF9=(FMC_NE2/FMC_NCE)  AF10=(-)  AF11=(SWPMI_RX)  AF12=(SDMMC1_D0)  AF13=(DCMI_D2)  AF14=(-)
  { GPIOC,   9,        ALTF,  ALT12,  SPD_H, OUT_PP, PULL__UP, 0 }, // SDMMC_D1                            PC9      E9    ADD=()  AF0=(MCO2)  AF1=(-)  AF2=(TIM3_CH4)  AF3=(TIM8_CH4)  AF4=(I2C3_SDA)  AF5=(I2S_CKIN)  AF6=(-)  AF7=(-)  AF8=(UART5_CTS)  AF9=(QUADSPI_BK1_IO0)  AF10=(LCD_G3)  AF11=(SWPMI_SUSPEND)  AF12=(SDMMC1_D1)  AF13=(DCMI_D3)  AF14=(LCD_B2)
  { GPIOC,  10,        ALTF,  ALT12,  SPD_H, OUT_PP, PULL__UP, 0 }, // SDMMC_D2                            PC10     B9    ADD=()  AF0=(-)  AF1=(-)  AF2=(HRTIM_EEV1)  AF3=(DFSDM1_CKIN5)  AF4=(-)  AF5=(-)  AF6=(SPI3_SCK/I2S3_CK)  AF7=(USART3_TX)  AF8=(UART4_TX)  AF9=(QUADSPI_BK1_IO1)  AF10=(-)  AF11=(-)  AF12=(SDMMC1_D2)  AF13=(DCMI_D8)  AF14=(LCD_R2)
  { GPIOC,  11,        ALTF,  ALT12,  SPD_H, OUT_PP, PULL__UP, 0 }, // SDMMC_D3                            PC11     B8    ADD=()  AF0=(-)  AF1=(-)  AF2=(HRTIM_FLT2)  AF3=(DFSDM1_DATIN5)  AF4=(-)  AF5=(-)  AF6=(SPI3_MISO/I2S3_SDI)  AF7=(USART3_RX)  AF8=(UART4_RX)  AF9=(QUADSPI_BK2_NCS)  AF10=(-)  AF11=(-)  AF12=(SDMMC1_D3)  AF13=(DCMI_D4)  AF14=(-)
  { GPIOC,  12,        ALTF,  ALT12,  SPD_H, OUT_PP, PULL__UP, 0 }, // SDMMC_CLK                           PC12     C8    ADD=()  AF0=(TRACED3)  AF1=(-)  AF2=(HRTIM_EEV2)  AF3=(-)  AF4=(-)  AF5=(-)  AF6=(SPI3_MOSI/I2S3_SDO)  AF7=(USART3_CK)  AF8=(UART5_TX)  AF9=(-)  AF10=(-)  AF11=(-)  AF12=(SDMMC1_CK)  AF13=(DCMI_D9)  AF14=(-)
  { GPIOC,  13,        GPIO,  ALT00,  SPD_L, OUT_PP, PUPD_DIS, 0 }, // OLED_RES                            PC13     A2    ADD=(RTC_TAMP1/RTC_TS/WKUP2)  AF0=()  AF1=(-)  AF2=(-)  AF3=(-)  AF4=(-)  AF5=(-)  AF6=(-)  AF7=(-)  AF8=(-)  AF9=(-)  AF10=(-)  AF11=(-)  AF12=(-)  AF13=(-)  AF14=(-)
  { GPIOC,  14,        ANAL,  ALT00,  SPD_L, OUT_PP, PUPD_DIS, 0 }, // OSC32IN                             PC14     A1    ADD=(OSC32_IN)  AF0=()  AF1=(-)  AF2=(-)  AF3=(-)  AF4=(-)  AF5=(-)  AF6=(-)  AF7=(-)  AF8=(-)  AF9=(-)  AF10=(-)  AF11=(-)  AF12=(-)  AF13=(-)  AF14=(-)
  { GPIOC,  15,        ANAL,  ALT00,  SPD_L, OUT_PP, PUPD_DIS, 0 }, // OSC32OUT                            PC15     B1    ADD=(OSC32_OUT)  AF0=()  AF1=(-)  AF2=(-)  AF3=(-)  AF4=(-)  AF5=(-)  AF6=(-)  AF7=(-)  AF8=(-)  AF9=(-)  AF10=(-)  AF11=(-)  AF12=(-)  AF13=(-)  AF14=(-)

  { GPIOD,   0,        ALTF,  ALT09,  SPD_L, OUT_PP, PUPD_DIS, 0 }, // CAN0_RX                             PD0      D8    ADD=()  AF0=(-)  AF1=(-)  AF2=(-)  AF3=(DFSDM1_CKIN6)  AF4=(-)  AF5=(-)  AF6=(SAI3_SCK_A)  AF7=(-)  AF8=(UART4_RX)  AF9=(FDCAN1_RX)  AF10=(-)  AF11=(-)  AF12=(FMC_D2/FMC_DA2)  AF13=(-)  AF14=(-)
  { GPIOD,   1,        ALTF,  ALT09,  SPD_L, OUT_PP, PUPD_DIS, 0 }, // CAN0_TX                             PD1      E8    ADD=()  AF0=(-)  AF1=(-)  AF2=(-)  AF3=(DFSDM1_DATIN6)  AF4=(-)  AF5=(-)  AF6=(SAI3_SD_A)  AF7=(-)  AF8=(UART4_TX)  AF9=(FDCAN1_TX)  AF10=(-)  AF11=(-)  AF12=(FMC_D3/FMC_DA3)  AF13=(-)  AF14=(-)
  { GPIOD,   2,        ALTF,  ALT12,  SPD_H, OUT_PP, PULL__UP, 0 }, // SDMMC_CMD                           PD2      B7    ADD=()  AF0=(TRACED2)  AF1=(-)  AF2=(TIM3_ETR)  AF3=(-)  AF4=(-)  AF5=(-)  AF6=(-)  AF7=(-)  AF8=(UART5_RX)  AF9=(-)  AF10=(-)  AF11=(-)  AF12=(SDMMC1_CMD)  AF13=(DCMI_D11)  AF14=(-)
  { GPIOD,   3,        INPT,  ALT00,  SPD_L, OUT_PP, PULL__DN, 0 }, // -                                   PD3      C7    ADD=()  AF0=(-)  AF1=(-)  AF2=(-)  AF3=(DFSDM1_CKOUT)  AF4=(-)  AF5=(SPI2_SCK/I2S2_CK)  AF6=(-)  AF7=(USART2_CTS/USART2_NSS)  AF8=(-)  AF9=(-)  AF10=(-)  AF11=(-)  AF12=(FMC_CLK)  AF13=(DCMI_D5)  AF14=(LCD_G7)
  { GPIOD,   4,        GPIO,  ALT00,  SPD_L, OUT_PP, PUPD_DIS, 1 }, // CAN_STB                             PD4      D7    ADD=()  AF0=(-)  AF1=(-)  AF2=(HRTIM_FLT3)  AF3=(-)  AF4=(-)  AF5=(-)  AF6=(SAI3_FS_A)  AF7=(USART2_RTS/USART2_DE)  AF8=(-)  AF9=(FDCAN1_RXFD_MODE)  AF10=(-)  AF11=(-)  AF12=(FMC_NOE)  AF13=(-)  AF14=(-)
  { GPIOD,   5,        INPT,  ALT00,  SPD_L, OUT_PP, PULL__DN, 0 }, // -                                   PD5      B6    ADD=()  AF0=(-)  AF1=(-)  AF2=(HRTIM_EEV3)  AF3=(-)  AF4=(-)  AF5=(-)  AF6=(-)  AF7=(USART2_TX)  AF8=(-)  AF9=(FDCAN1_TXFD_MODE)  AF10=(-)  AF11=(-)  AF12=(FMC_NWE)  AF13=(-)  AF14=(-)
  { GPIOD,   6,        GPIO,  ALT00,  SPD_L, OUT_PP, PUPD_DIS, 0 }, // OLEDV Подсветка LCD дисплея         PD6      C6    ADD=()  AF0=(-)  AF1=(-)  AF2=(SAI1_D1)  AF3=(DFSDM1_CKIN4)  AF4=(DFSDM1_DATIN1)  AF5=(SPI3_MOSI/I2S3_SDO)  AF6=(SAI1_SD_A)  AF7=(USART2_RX)  AF8=(SAI4_SD_A)  AF9=(FDCAN2_RXFD_MODE)  AF10=(SAI4_D1)  AF11=(SDMMC2_CK)  AF12=(FMC_NWAIT)  AF13=(DCMI_D10)  AF14=(LCD_B2)
  { GPIOD,   7,        INPT,  ALT00,  SPD_L, OUT_PP, PULL__DN, 0 }, // -                                   PD7      D6    ADD=()  AF0=(-)  AF1=(-)  AF2=(-)  AF3=(DFSDM1_DATIN4)  AF4=(-)  AF5=(SPI1_MOSI/I2S1_SDO)  AF6=(DFSDM1_CKIN1)  AF7=(USART2_CK)  AF8=(-)  AF9=(SPDIFRX1_IN1)  AF10=(-)  AF11=(SDMMC2_CMD)  AF12=(FMC_NE1)  AF13=(-)  AF14=(-)
  { GPIOD,   8,        INPT,  ALT00,  SPD_L, OUT_PP, PULL__DN, 0 }, // -                                   PD8      K9    ADD=()  AF0=(-)  AF1=(-)  AF2=(-)  AF3=(DFSDM1_CKIN3)  AF4=(-)  AF5=(-)  AF6=(SAI3_SCK_B)  AF7=(USART3_TX)  AF8=(-)  AF9=(SPDIFRX1_IN2)  AF10=(-)  AF11=(-)  AF12=(FMC_D13/FMC_DA13)  AF13=(-)  AF14=(-)
  { GPIOD,   9,        INPT,  ALT00,  SPD_L, OUT_PP, PULL__DN, 0 }, // -                                   PD9      J9    ADD=()  AF0=(-)  AF1=(-)  AF2=(-)  AF3=(DFSDM1_DATIN3)  AF4=(-)  AF5=(-)  AF6=(SAI3_SD_B)  AF7=(USART3_RX)  AF8=(-)  AF9=(FDCAN2_RXFD_MODE)  AF10=(-)  AF11=(-)  AF12=(FMC_D14/FMC_DA14)  AF13=(-)  AF14=(-)
  { GPIOD,  10,        INPT,  ALT00,  SPD_L, OUT_PP, PULL__DN, 0 }, // -                                   PD10     H9    ADD=()  AF0=(-)  AF1=(-)  AF2=(-)  AF3=(DFSDM1_CKOUT)  AF4=(-)  AF5=(-)  AF6=(SAI3_FS_B)  AF7=(USART3_CK)  AF8=(-)  AF9=(FDCAN2_TXFD_MODE)  AF10=(-)  AF11=(-)  AF12=(FMC_D15/FMC_DA15)  AF13=(-)  AF14=(LCD_B3)
  { GPIOD,  11,        GPIO,  ALT00,  SPD_L, OUT_PP, PUPD_DIS, 1 }, // LEDCAN0                             PD11     G9    ADD=()  AF0=(-)  AF1=(-)  AF2=(-)  AF3=(LPTIM2_IN2)  AF4=(I2C4_SMBA)  AF5=(-)  AF6=(-)  AF7=(USART3_CTS/USART3_NSS)  AF8=(-)  AF9=(QUADSPI_BK1_IO0)  AF10=(SAI2_SD_A)  AF11=(-)  AF12=(FMC_A16)  AF13=(-)  AF14=(-)
  { GPIOD,  12,        INPT,  ALT00,  SPD_L, OUT_PP, PULL__DN, 0 }, // -                                   PD12     K10   ADD=()  AF0=(-)  AF1=(LPTIM1_IN1)  AF2=(TIM4_CH1)  AF3=(LPTIM2_IN1)  AF4=(I2C4_SCL)  AF5=(-)  AF6=(-)  AF7=(USART3_RTS/USART3_DE)  AF8=(-)  AF9=(QUADSPI_BK1_IO1)  AF10=(SAI2_FS_A)  AF11=(-)  AF12=(FMC_A17)  AF13=(-)  AF14=(-)
  { GPIOD,  13,        INPT,  ALT00,  SPD_L, OUT_PP, PUPD_DIS, 0 }, // ENC_SW                              PD13     J10   ADD=()  AF0=(-)  AF1=(LPTIM1_OUT)  AF2=(TIM4_CH2)  AF3=(-)  AF4=(I2C4_SDA)  AF5=(-)  AF6=(-)  AF7=()  AF8=(-)  AF9=(QUADSPI_BK1_IO3)  AF10=(SAI2_SCK_A)  AF11=(-)  AF12=(FMC_A18)  AF13=(-)  AF14=(-)
  { GPIOD,  14,        GPIO,  ALT00,  SPD_L, OUT_PP, PUPD_DIS, 0 }, // LSW_F Управление ключом Q12 Q13     PD14     H8    ADD=()  AF0=(-)  AF1=(-)  AF2=(TIM4_CH3)  AF3=(-)  AF4=(-)  AF5=(-)  AF6=(SAI3_MCLK_B)  AF7=(-)  AF8=(UART8_CTS)  AF9=(-)  AF10=(-)  AF11=(-)  AF12=(FMC_D0/FMC_DA0)  AF13=(-)  AF14=(-)
  { GPIOD,  15,        GPIO,  ALT00,  SPD_L, OUT_PP, PUPD_DIS, 0 }, // UART1_RTS                           PD15     G8    ADD=()  AF0=(-)  AF1=(-)  AF2=(TIM4_CH4)  AF3=(-)  AF4=(-)  AF5=(-)  AF6=(SAI3_MCLK_A)  AF7=(-)  AF8=(UART8_RTS/UART8_DE)  AF9=(-)  AF10=(-)  AF11=(-)  AF12=(FMC_D1/FMC_DA1)  AF13=(-)  AF14=(-)

  { GPIOE,   0,        ALTF,  ALT08,  SPD_L, OUT_PP, PUPD_DIS, 0 }, // UART1_RX (UART8)                    PE0      D4    ADD=()  AF0=(-)  AF1=(LPTIM1_ETR)  AF2=(TIM4_ETR)  AF3=(HRTIM_SCIN)  AF4=(LPTIM2_ETR)  AF5=(-)  AF6=(-)  AF7=(-)  AF8=(UART8_RX)  AF9=(FDCAN1_RXFD_MODE)  AF10=(SAI2_MCLK_A)  AF11=(-)  AF12=(FMC_NBL0)  AF13=(DCMI_D2)  AF14=(-)
  { GPIOE,   1,        ALTF,  ALT08,  SPD_L, OUT_PP, PUPD_DIS, 0 }, // UART1_TX (UART8)                    PE1      C4    ADD=()  AF0=(-)  AF1=(LPTIM1_IN2)  AF2=(-)  AF3=(HRTIM_SCOUT)  AF4=(-)  AF5=(-)  AF6=(-)  AF7=(-)  AF8=(UART8_TX)  AF9=(FDCAN1_TXFD_MODE)  AF10=(-)  AF11=(-)  AF12=(FMC_NBL1)  AF13=(DCMI_D3)  AF14=(-)
  { GPIOE,   2,        ALTF,  ALT05,  SPD_V, OUT_PP, PUPD_DIS, 0 }, // OLED_CLK (SPI4_SCK)                 PE2      A3    ADD=()  AF0=(TRACECLK)  AF1=(-)  AF2=(SAI1_CK1)  AF3=(-)  AF4=(-)  AF5=(SPI4_SCK)  AF6=(SAI1_MCLK_A)  AF7=(-)  AF8=(SAI4_MCLK_A)  AF9=(QUADSPI_BK1_IO2)  AF10=(SAI4_CK1)  AF11=(ETH_MII_TXD3)  AF12=(FMC_A23)  AF13=(-)  AF14=(-)
  { GPIOE,   3,        GPIO,  ALT00,  SPD_V, OUT_PP, PUPD_DIS, 0 }, // OLED_DC                             PE3      B3    ADD=()  AF0=(TRACED0)  AF1=(-)  AF2=(-)  AF3=(-)  AF4=(TIM15_BKIN)  AF5=(-)  AF6=(SAI1_SD_B)  AF7=(-)  AF8=(SAI4_SD_B)  AF9=(-)  AF10=(-)  AF11=(-)  AF12=(FMC_A19)  AF13=(-)  AF14=(-)
  { GPIOE,   4,        GPIO,  ALT00,  SPD_V, OUT_PP, PUPD_DIS, 1 }, // OLED_CS                             PE4      C3    ADD=()  AF0=(TRACED1)  AF1=(-)  AF2=(SAI1_D2)  AF3=(DFSDM1_DATIN3)  AF4=(TIM15_CH1N)  AF5=(SPI4_NSS)  AF6=(SAI1_FS_A)  AF7=(-)  AF8=(SAI4_FS_A)  AF9=(-)  AF10=(SAI4_D2)  AF11=(-)  AF12=(FMC_A20)  AF13=(DCMI_D4)  AF14=(LCD_B0)
  { GPIOE,   5,        INPT,  ALT00,  SPD_L, OUT_PP, PULL__DN, 0 }, // -                                   PE5      D3    ADD=()  AF0=(TRACED2)  AF1=(-)  AF2=(SAI1_CK2)  AF3=(DFSDM1_CKIN3)  AF4=(TIM15_CH1)  AF5=(SPI4_MISO)  AF6=(SAI1_SCK_A)  AF7=(-)  AF8=(SAI4_SCK_A)  AF9=(-)  AF10=(SAI4_CK2)  AF11=(-)  AF12=(FMC_A21)  AF13=(DCMI_D6)  AF14=(LCD_G0)
  { GPIOE,   6,        ALTF,  ALT05,  SPD_V, OUT_PP, PUPD_DIS, 0 }, // OLED_DIN (SPI4_MOSI)                PE6      E3    ADD=()  AF0=(TRACED3)  AF1=(TIM1_BKIN2)  AF2=(SAI1_D1)  AF3=(-)  AF4=(TIM15_CH2)  AF5=(SPI4_MOSI)  AF6=(SAI1_SD_A)  AF7=(-)  AF8=(SAI4_SD_A)  AF9=(SAI4_D1)  AF10=(SAI2_MCLK_B)  AF11=(TIM1_BKIN2_COMP12)  AF12=(FMC_A22)  AF13=(DCMI_D7)  AF14=(LCD_G1)
  { GPIOE,   7,        INPT,  ALT00,  SPD_L, OUT_PP, PUPD_DIS, 0 }, // 1WRX                                PE7      H5    ADD=(OPAMP2_VOUT/COMP2_INM)  AF0=(-)  AF1=(TIM1_ETR)  AF2=(-)  AF3=(DFSDM1_DATIN2)  AF4=(-)  AF5=(-)  AF6=(-)  AF7=(UART7_RX)  AF8=(-)  AF9=(-)  AF10=(QUADSPI_BK2_IO0)  AF11=(-)  AF12=(FMC_D4/FMC_DA4)  AF13=(-)  AF14=(-)
  { GPIOE,   8,        GPIO,  ALT00,  SPD_L, OUT_PP, PUPD_DIS, 0 }, // 1WTX                                PE8      J5    ADD=(OPAMP2_VINM)  AF0=(-)  AF1=(TIM1_CH1N)  AF2=(-)  AF3=(DFSDM1_CKIN2)  AF4=(-)  AF5=(-)  AF6=(-)  AF7=(UART7_TX)  AF8=(-)  AF9=(-)  AF10=(QUADSPI_BK2_IO1)  AF11=(-)  AF12=(FMC_D5/FMC_DA5)  AF13=(COMP2_OUT)  AF14=(-)
  { GPIOE,   9,        GPIO,  ALT00,  SPD_L, OUT_PP, PUPD_DIS, 0 }, // EN_CHARGER                          PE9      K5    ADD=(OPAMP2_VINP/COMP2_INP)  AF0=(-)  AF1=(TIM1_CH1)  AF2=(-)  AF3=(DFSDM1_CKOUT)  AF4=(-)  AF5=(-)  AF6=(-)  AF7=(UART7_RTS/UART7_DE)  AF8=(-)  AF9=(-)  AF10=(QUADSPI_BK2_IO2)  AF11=(-)  AF12=(FMC_D6/FMC_DA6)  AF13=(-)  AF14=(-)
  { GPIOE,  10,        INPT,  ALT00,  SPD_L, OUT_PP, PULL__DN, 0 }, // -                                   PE10     G6    ADD=(COMP2_INM)  AF0=(-)  AF1=(TIM1_CH2N)  AF2=(-)  AF3=(DFSDM1_DATIN4)  AF4=(-)  AF5=(-)  AF6=(-)  AF7=(UART7_CTS)  AF8=(-)  AF9=(-)  AF10=(QUADSPI_BK2_IO3)  AF11=(-)  AF12=(FMC_D7/FMC_DA7)  AF13=(-)  AF14=(-)
  { GPIOE,  11,        ALTF,  ALT01,  SPD_L, OUT_PP, PUPD_DIS, 0 }, // ENC_B  (TIM1_CH2)                   PE11     H6    ADD=(COMP2_INP)  AF0=(-)  AF1=(TIM1_CH2)  AF2=(-)  AF3=(DFSDM1_CKIN4)  AF4=(-)  AF5=(SPI4_NSS)  AF6=(-)  AF7=(-)  AF8=(-)  AF9=(-)  AF10=(SAI2_SD_B)  AF11=(-)  AF12=(FMC_D8/FMC_DA8)  AF13=(-)  AF14=(LCD_G3)
  { GPIOE,  12,        INPT,  ALT00,  SPD_L, OUT_PP, PUPD_DIS, 0 }, // DCDC_PGOOD                          PE12     J6    ADD=()  AF0=(-)  AF1=(TIM1_CH3N)  AF2=(-)  AF3=(DFSDM1_DATIN5)  AF4=(-)  AF5=(SPI4_SCK)  AF6=(-)  AF7=(-)  AF8=(-)  AF9=(-)  AF10=(SAI2_SCK_B)  AF11=(-)  AF12=(FMC_D9/FMC_DA9)  AF13=(COMP1_OUT)  AF14=(LCD_B4)
  { GPIOE,  13,        ALTF,  ALT01,  SPD_L, OUT_PP, PUPD_DIS, 0 }, // DCDC_MODE                           PE13     K6    ADD=()  AF0=(-)  AF1=(TIM1_CH3)  AF2=(-)  AF3=(DFSDM1_CKIN5)  AF4=(-)  AF5=(SPI4_MISO)  AF6=(-)  AF7=(-)  AF8=(-)  AF9=(-)  AF10=(SAI2_FS_B)  AF11=(-)  AF12=(FMC_D10/FMC_DA10)  AF13=(COMP2_OUT)  AF14=(LCD_DE)
  { GPIOE,  14,        GPIO,  ALT00,  SPD_L, OUT_PP, PUPD_DIS, 0 }, // OUT3                                PE14     G7    ADD=()  AF0=(-)  AF1=(TIM1_CH4)  AF2=(-)  AF3=(-)  AF4=(-)  AF5=(SPI4_MOSI)  AF6=(-)  AF7=(-)  AF8=(-)  AF9=(-)  AF10=(SAI2_MCLK_B)  AF11=(-)  AF12=(FMC_D11/FMC_DA11)  AF13=(-)  AF14=(LCD_CLK)
  { GPIOE,  15,        GPIO,  ALT00,  SPD_L, OUT_PP, PUPD_DIS, 0 }, // OUT4                                PE15     H7    ADD=()  AF0=(-)  AF1=(TIM1_BKIN)  AF2=(-)  AF3=(-)  AF4=(-)  AF5=(-)  AF6=(-)  AF7=(-)  AF8=(-)  AF9=(-)  AF10=()  AF11=(-)  AF12=(FMC_D12/FMC_DA12)  AF13=(TIM1_BKIN_COMP12/COMP_TIM1_BKIN)  AF14=(LCD_R7)

  { GPIOH,   0,        ANAL,  ALT00,  SPD_L, OUT_PP, PUPD_DIS, 0 }, // OSC_IN                              PH0      C1    ADD=(OSC_IN)  AF0=()  AF1=(-)  AF2=(-)  AF3=(-)  AF4=(-)  AF5=(-)  AF6=(-)  AF7=(-)  AF8=(-)  AF9=(-)  AF10=(-)  AF11=(-)  AF12=(-)  AF13=(-)  AF14=(-)
  { GPIOH,   1,        ANAL,  ALT00,  SPD_L, OUT_PP, PUPD_DIS, 0 }, // OSC_OUT                             PH1      D1    ADD=(OSC_OUT)  AF0=()  AF1=(-)  AF2=(-)  AF3=(-)  AF4=(-)  AF5=(-)  AF6=(-)  AF7=(-)  AF8=(-)  AF9=(-)  AF10=(-)  AF11=(-)  AF12=(-)  AF13=(-)  AF14=(-)

};

/*------------------------------------------------------------------------------

 ------------------------------------------------------------------------------*/
void Config_pin(const T_IO_pins_configuration pinc)
{
  // Сбрасываем все биты в позиции pinc.pin_num в регистре MODER и переводим пин а режим Input
  pinc.gpio->MODER   &= ~(0x03 <<(pinc.pin_num * 2));

  // Сбрасываем в 0 биты в позиции pinc.pin_num
  pinc.gpio->PUPDR   &= ~(0x03  <<(pinc.pin_num * 2));
  pinc.gpio->OSPEEDR &= ~(0x03 <<(pinc.pin_num * 2));
  pinc.gpio->OTYPER  &= ~(0x01 << pinc.pin_num);

  // Сбрасываем в 0 биты в позиции pinc.pin_num в регистрах альтернативной функции
  if (pinc.pin_num < 8)
  {
    pinc.gpio->AFR[0] &= ~(0x0F <<(pinc.pin_num * 4));
  }
  else
  {
    pinc.gpio->AFR[1] &= ~(0x0F <<((pinc.pin_num-8) * 4));
  }


  // Устанавливаем начальное значение пина
  if (pinc.init == 0)
  {
    pinc.gpio->BSRR =(1 << pinc.pin_num)<< 16;
  }
  else
  {
    pinc.gpio->BSRR = 1 << pinc.pin_num;
  }

  pinc.gpio->PUPDR   |=(pinc.PUPDR & 0x03)<<(pinc.pin_num * 2);
  pinc.gpio->OSPEEDR |=(pinc.OSPEEDR & 0x03)<<(pinc.pin_num * 2);
  pinc.gpio->OTYPER  |=(pinc.OTYPER & 0x01)<< pinc.pin_num;

  if (pinc.pin_num < 8)
  {
    pinc.gpio->AFR[0] |=(pinc.AFR & 0x0F)<<(pinc.pin_num * 4);
  }
  else
  {
    pinc.gpio->AFR[1] |=(pinc.AFR & 0x0F)<<((pinc.pin_num-8) * 4);
  }

  pinc.gpio->MODER |=(pinc.MODER & 0x03)<<(pinc.pin_num * 2);
}

/*------------------------------------------------------------------------------



 \return int
 ------------------------------------------------------------------------------*/
int Backpman3_init_pins(void)
{
  volatile uint32_t dummy;

  // Включаем тактирование на всех портах
  RCC->AHB4ENR |= RCC_AHB4ENR_GPIOAEN + RCC_AHB4ENR_GPIOBEN  + RCC_AHB4ENR_GPIOCEN + RCC_AHB4ENR_GPIODEN + RCC_AHB4ENR_GPIOEEN + RCC_AHB4ENR_GPIOFEN + RCC_AHB4ENR_GPIOGEN + RCC_AHB4ENR_GPIOHEN;
  dummy = RCC->AHB4ENR; // Читаем обратно чтобы соблюсти задержку

  for (uint32_t i = 0; i < (sizeof(BACKPMAN3_pins_conf) / sizeof(BACKPMAN3_pins_conf[0])); i++)
  {
    Config_pin(BACKPMAN3_pins_conf[i]);
  }

  return 0;
}


/*-----------------------------------------------------------------------------------------------------


  \param val
-----------------------------------------------------------------------------------------------------*/
void Set_OUT1_level(uint32_t val)
{
  if (val != 0) GPIOC->BSRR  = BIT(6);
  else GPIOC->BSRR  = BIT(6)<< 16;
}
/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Get_OUT1_level(void)
{
  if (GPIOC->IDR & BIT(6)) return 1;
  else return 0;
}

/*-----------------------------------------------------------------------------------------------------


  \param val
-----------------------------------------------------------------------------------------------------*/
void Set_OUT2_level(uint32_t val)
{
  if (val != 0) GPIOC->BSRR  = BIT(7);
  else GPIOC->BSRR  = BIT(7)<< 16;
}
/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Get_OUT2_level(void)
{
  if (GPIOC->IDR & BIT(7)) return 1;
  else return 0;
}


/*-----------------------------------------------------------------------------------------------------


  \param val
-----------------------------------------------------------------------------------------------------*/
void Set_OUT3_level(uint32_t val)
{
  if (val != 0) GPIOE->BSRR  = BIT(14);
  else GPIOE->BSRR  = BIT(14)<< 16;
}
/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Get_OUT3_level(void)
{
  if (GPIOE->IDR & BIT(14)) return 1;
  else return 0;
}

/*-----------------------------------------------------------------------------------------------------


  \param val
-----------------------------------------------------------------------------------------------------*/
void Set_OUT4_level(uint32_t val)
{
  if (val != 0) GPIOE->BSRR  = BIT(15);
  else GPIOE->BSRR  = BIT(15)<< 16;
}
/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Get_OUT4_level(void)
{
  if (GPIOE->IDR & BIT(15)) return 1;
  else return 0;
}

/*-----------------------------------------------------------------------------------------------------


  \param val
-----------------------------------------------------------------------------------------------------*/
void Set_LED_green(uint32_t val)
{
  if (val != 0) GPIOA->BSRR  = BIT(15);
  else GPIOA->BSRR  = BIT(15)<< 16;
}
/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Get_LED_green(void)
{
  if (GPIOA->IDR & BIT(15)) return 1;
  else return 0;
}

/*-----------------------------------------------------------------------------------------------------


  \param val
-----------------------------------------------------------------------------------------------------*/
void Set_LED_red(uint32_t val)
{
  if (val != 0) GPIOB->BSRR  = BIT(10);
  else GPIOB->BSRR  = BIT(10)<< 16;
}
/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Get_LED_red(void)
{
  if (GPIOB->IDR & BIT(10)) return 1;
  else return 0;
}


/*-----------------------------------------------------------------------------------------------------


  \param val
-----------------------------------------------------------------------------------------------------*/
void Set_ASW_R(uint32_t val)
{
  if (val != 0) GPIOB->BSRR  = BIT(8);
  else GPIOB->BSRR  = BIT(8)<< 16;
}
/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Get_ASW_R(void)
{
  if (GPIOB->IDR & BIT(8)) return 1;
  else return 0;
}

/*-----------------------------------------------------------------------------------------------------


  \param val
-----------------------------------------------------------------------------------------------------*/
void Set_ASW_F(uint32_t val)
{
  if (val != 0) GPIOB->BSRR  = BIT(9);
  else GPIOB->BSRR  = BIT(9)<< 16;
}
/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Get_ASW_F(void)
{
  if (GPIOB->IDR & BIT(9)) return 1;
  else return 0;
}

/*-----------------------------------------------------------------------------------------------------


  \param val
-----------------------------------------------------------------------------------------------------*/
void Set_PSW_R(uint32_t val)
{
  if (val != 0) GPIOB->BSRR  = BIT(13);
  else GPIOB->BSRR  = BIT(13)<< 16;
}
/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Get_PSW_R(void)
{
  if (GPIOB->IDR & BIT(13)) return 1;
  else return 0;
}


/*-----------------------------------------------------------------------------------------------------


  \param val
-----------------------------------------------------------------------------------------------------*/
void Set_PSW_F(uint32_t val)
{
  if (val != 0) GPIOB->BSRR  = BIT(14);
  else GPIOB->BSRR  = BIT(14)<< 16;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Get_PSW_F(void)
{
  if (GPIOB->IDR & BIT(14)) return 1;
  else return 0;
}



/*-----------------------------------------------------------------------------------------------------


  \param val
-----------------------------------------------------------------------------------------------------*/
void Set_LSW_F(uint32_t val)
{
  if (val != 0) GPIOD->BSRR  = BIT(14);
  else GPIOD->BSRR  = BIT(14)<< 16;
}
/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Get_LSW_F(void)
{
  if (GPIOD->IDR & BIT(14)) return 1;
  else return 0;
}


/*-----------------------------------------------------------------------------------------------------


  \param val
-----------------------------------------------------------------------------------------------------*/
void Set_OLED_RES(uint32_t val)
{
  if (val != 0) GPIOC->BSRR  = BIT(13);
  else GPIOC->BSRR  = BIT(13)<< 16;
}
/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Get_OLED_RES(void)
{
  if (GPIOC->IDR & BIT(13)) return 1;
  else return 0;
}


/*-----------------------------------------------------------------------------------------------------


  \param val
-----------------------------------------------------------------------------------------------------*/
void Set_OLEDV(uint32_t val)
{
  if (val != 0) GPIOD->BSRR  = BIT(6);
  else GPIOD->BSRR  = BIT(6)<< 16;
}
/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Get_OLEDV(void)
{
  if (GPIOD->IDR & BIT(6)) return 1;
  else return 0;
}

/*-----------------------------------------------------------------------------------------------------


  \param val
-----------------------------------------------------------------------------------------------------*/
void Set_OLED_DC(uint32_t val)
{
  if (val != 0) GPIOE->BSRR  = BIT(3);
  else GPIOE->BSRR  = BIT(3)<< 16;
}
/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Get_OLED_DC(void)
{
  if (GPIOE->IDR & BIT(3)) return 1;
  else return 0;
}


/*-----------------------------------------------------------------------------------------------------


  \param val
-----------------------------------------------------------------------------------------------------*/
void Set_OLED_CS(uint32_t val)
{
  if (val != 0) GPIOE->BSRR  = BIT(4);
  else GPIOE->BSRR  = BIT(4)<< 16;
}
/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Get_OLED_CS(void)
{
  if (GPIOE->IDR & BIT(4)) return 1;
  else return 0;
}


/*-----------------------------------------------------------------------------------------------------


  \param val
-----------------------------------------------------------------------------------------------------*/
void Set_EN_CHARGER(uint32_t val)
{
  if (val != 0) GPIOE->BSRR  = BIT(9);
  else GPIOE->BSRR  = BIT(9)<< 16;
}
/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Get_EN_CHARGER(void)
{
  if (GPIOE->IDR & BIT(9)) return 1;
  else return 0;
}


/*-----------------------------------------------------------------------------------------------------


  \param val
-----------------------------------------------------------------------------------------------------*/
void Set_DCDC_MODE_pin(uint32_t val)
{
  if (val == 1) GPIOE->BSRR  = BIT(13);
  else if ((val == 0) || (val == 2)) GPIOE->BSRR  = BIT(13)<< 16;
}
/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Get_DCDC_MODE_pin(void)
{
  if (GPIOE->IDR & BIT(13)) return 1;
  else return 0;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t  Get_DCDC_PGOOD(void)
{
  if (GPIOE->IDR & BIT(12)) return 1;
  else return 0;
}

/*-----------------------------------------------------------------------------------------------------


  \param val
-----------------------------------------------------------------------------------------------------*/
void Set_DAC_CS(uint32_t val)
{
  if (val != 0) GPIOA->BSRR  = BIT(4);
  else GPIOA->BSRR  = BIT(4)<< 16;
}


/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Get_smpl_enc_a(void)
{
  if (GPIOA->IDR & BIT(8)) return 1;
  else return 0;
}
/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Get_smpl_enc_b(void)
{
  if (GPIOE->IDR & BIT(11)) return 1;
  else return 0;

}
/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Get_smpl_enc_sw(void)
{
  if (GPIOD->IDR & BIT(13)) return 0;
  else return 1;
}

