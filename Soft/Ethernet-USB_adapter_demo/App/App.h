#ifndef APP_HEADER_H
  #define APP_HEADER_H





  #include   <stdio.h>
  #include   <time.h>
  #include   <math.h>
  #include   <stdint.h>
  #include   <stdarg.h>
  #include   <ctype.h>
  #include   <stdbool.h>
  #include   <arm_itm.h>

  #include   "App_config.h"
  #include   "tx_api.h"
  #include   "ux_api.h"
  #include   "Main.h"
  #include   <core_cm7.h>
  #include   "Backpman3_PINS.h"
  #include   "Backpman3_ADC.h"
  #include   "Backpman3_CAN.h"
  #include   "Backpman3_Clock.h"
  #include   "Backpman3_Timers.h"
  #include   "Backpman3_DMA.h"
  #include   "Backpman3_Memory.h"
  #include   "Backpman3_RTC.h"
  #include   "Backpman3_SPI.h"

  #include   "SEGGER_RTT.h"

  #include   "../app/CAN_bus_defs.h"

  #include   "Serial_driver.h"

//#include   "stm32h735g_discovery_sd.h"
  #include   "ux_dcd_stm32.h"
  #include   "ux_device_class_storage.h"
  #include   "ux_device_class_cdc_acm.h"
  #include   "ux_device_class_rndis.h"
  #include   "ux_device_class_cdc_ecm.h"
  #include   "ux_host_class_cdc_ecm.h"
  #include   "USB_descriptors_builder.h"
  #include   "USB_periph_init.h"
  #include   "USB_device_init.h"
  #include   "USB_msc_driver.h"
  #include   "USB_CDC_driver.h"
  #include   "USB_host_cdc_ecm.h"
  #include   "USB_RNDIS_network.h"
  #include   "USB_ECM_Host_network.h"


  #include   "STfs_int.h"
  #include   "STfs_api.h"
  #include   "STfs_test_task.h"
  #include   "BKMAN3R1_Params.h"
  #include   "BKMAN3R1_Params_dyn_tables.h"
  #include   "jansson.h"
  #include   "compress.h"
  #include   "FS_utils.h"
  #include   "MonitorVT100.h"
  #include   "Params_editor.h"
  #include   "Main_thread.h"
  #include   "App_ISRs.h"
  #include   "Time_utils.h"
  #include   "Mem_man.h"
  #include   "App_utils.h"
  #include   "App_str_utils.h"
  #include   "logger.h"
  #include   "Net_common.h"
  #include   "Net_utils.h"
  #include   "NV_store.h"
  #include   "App_JSON_serializer.h"
  #include   "App_JSON_deserializer.h"
  #include   "FreeMaster_thread.h"
  #include   "FreeMaster_vars_map.h"
  #include   "IDLE_task.h"
  #include   "Net_thread.h"
  #include   "Net_Web_server.h"
  #include   "Outputs_control.h"
  #include   "CAN_thread.h"
  #include   "Inputs_controller.h"
  #include   "IO_tasks.h"
  #include   "Filters.h"
  #include   "gx_api.h"
  #include   "TFT_display_control.h"
  #include   "DCDC_controller.h"

  #define        APP_TO_RTT_LOG

  #define        SOFTWARE_VERSION                  "1.0"
  #define        HARDWARE_VERSION                  "BACKPMAN3 Rev1.0"

  #define        AXI_SRAM_END                      0x2407FFFF

  #define        STFS_PARAMS_FILE_NAME             "PARAMS.JSON"
  #define        STFS_COMPRESSED_PARAMS_FILE_NAME  "PARAMS.BIN"

  #define        WINDOWS_DIR                       "System Volume Information"
  #define        MISC_DIR_NAME                     "MISC"
  #define        RECORDS_DIR_NAME                  "RECORDS"
  #define        LOG_DIR_NAME                      "LOG"
  #define        LOG_FILE_NAME                     "log.txt"
  #define        PREV_LOG_FILE_NAME                "prev_log.txt"
  #define        LOG_FILE_PATH                     LOG_DIR_NAME"\\"LOG_FILE_NAME

  #define        MAX_SIZE_OF_PARAMS_FILE           65536

  #define        SCAN_PERIOD                       1000  // Период вызова задачи сканирования в мкс

  #define        SFFS_H753_FLASH_DRIVER
//#define        ENABLE_SECTORS_ERASING_LOG

  #define        RES_OK                0
  #define        RES_ERROR             1

  #define        OS_PRIORITY_LEVEL     14
  #define        DISABLE_OS_PRI_LEV    (OS_PRIORITY_LEVEL << (8 - __NVIC_PRIO_BITS))  // Маска приоритета в регистре BASEPRI запрещающая прерывания PendSV и  SysTick
  #define        ENABLE_OS_PRI_LEV     0   // Маска приоритета в регистре BASEPRI разрешающая прерывания с любыми приоритетами

  #define        DISABLE_OS_INTERRUPTS __set_BASEPRI(DISABLE_OS_PRI_LEV)
  #define        ENABLE_OS_INTERRUPTS  __set_BASEPRI(ENABLE_OS_PRI_LEV)

  #define        BIT(n)         (1u << n)
  #define        LSHIFT(v,n)    (((unsigned int)(v) << n))
  #define        MAX_UINT_32    (0xFFFFFFFFUL)

  #define        STR_CRLF                       "\r\n"

extern void    Delay_m7(int cnt); // Задержка на (cnt+1)*7 тактов . Передача нуля недопускается

  #define        DELAY_1us    Delay_m7(64)             // 0.992     мкс при частоте 480 МГц
  #define        DELAY_4us    Delay_m7(272)            // 4.025     мкс при частоте 480 МГц
  #define        DELAY_8us    Delay_m7(544)            // 7.992     мкс при частоте 480 МГц
  #define        DELAY_12us   Delay_m7(544+272)        //
  #define        DELAY_32us   Delay_m7(2192)           // 32.025    мкс при частоте 480 МГц
  #define        DELAY_ms(x)  Delay_m7(68572*(x)-1)    // 999.95*N  мкс при частоте 480 МГц

  #define        REF_TIME_INTERVAL                  100    // Задержка в мс для замера калибровочного интервала времени


  #define        THREAD_MAIN_STACK_SIZE             3000
  #define        VT100_TASK_STACK_SIZE              4096
  #define        THREAD_FREEMASTER_STACK_SIZE       2048
  #define        THREAD_IDLE_STACK_SIZE             1024
  #define        THREAD_STFS_TEST_STACK_SIZE        2048
  #define        THREAD_NET_STACK_SIZE              4096
  #define        THREAD_CAN_STACK_SIZE              2048
  #define        THREAD_IO_STACK_SIZE               2048
  #define        THREAD_LOCK_STACK_SIZE             2048
  #define        THREAD_HMI_STACK_SIZE              2048


  #define        THREAD_MAIN_PRIORITY               3
  #define        THREAD_IO_PRIORITY                 4
  #define        THREAD_CAN_PRIORITY                5
  #define        THREAD_LOCK_PRIORITY               6
  #define        THREAD_FREEMASTER_PRIORITY         7
  #define        THREAD_NET_PRIORITY                9
  #define        VT100_TASK_PRIO                    10
  #define        THREAD_HMI_PRIORITY                11
  #define        THREAD_STFS_TEST_PRIORITY          21
  #define        THREAD_IDLE_PRIORITY               30

//#define        ADC12_ISR_PRIO                     7
//#define        ADC3_ISR_PRIO                      7
  #define        DMA_ADC_ISR_PRIO                   6  // Приоритет прерывания каналов DMA при приеме данных от ADC
  #define        SPI4_ISR_PRIO                      7  // Приоритет канала обмена с дисплеем
  #define        SPI1_ISR_PRIO                      5  // Приоритет канала обмена с DAC80501


typedef struct
{
  uint8_t                inserted;
  ULONG                  idVendor;
  ULONG                  idProduct;
  ULONG                  dev_state;
  ULONG                  interface_id;
  ULONG                  interface_num;
  UX_HOST_CLASS_CDC_ECM *ecm_class_ptr;

} T_usb_app_info;


extern T_app_state                app_state;
extern         WVAR_TYPE          wvar;
extern const   T_work_params      dwvar[];
extern uint64_t                   ref_time;             // Калибровочная константа предназначенная для измерения нагрузки микропроцессора
extern volatile uint32_t          g_cpu_usage;
extern volatile float             g_cpu_usage_fp;
extern char                       cpu_id_str[];
extern uint32_t                   prioritygroup;

extern uint32_t                   dcdc_duty_cycle;
extern uint32_t                   dcdc_duty_cycle_last;
extern uint32_t                   dcdc_compare_val;
extern uint32_t                   adc_trig_compare_val;

extern float                      TS_CAL2;
extern float                      TS_CAL1;

extern T_run_average_uint32_20    flt_ACC_I;
extern T_run_average_uint32_20    flt_PSRC_I;
extern T_run_average_uint32_20    flt_LOAD_V;
extern T_run_average_uint32_20    flt_AIN2;
extern T_run_average_uint32_20    flt_VREF165;
extern T_run_average_uint32_20    flt_ACC_V;
extern T_run_average_uint32_20    flt_SYS_V;
extern T_run_average_uint32_20    flt_AIN3;
extern T_run_average_uint32_20    flt_AIN1;
extern T_run_average_uint32_20    flt_AIN4;
extern T_run_average_uint32_20    flt_TSENS;
extern T_run_average_uint32_20    flt_PSRC_V;
extern T_run_average_uint32_20    flt_LOAD_I;
extern T_run_average_uint32_20    flt_TERM;
extern T_run_average_uint32_20    flt_VREF;

extern T_can_land_cmd             g_can_cmd;

extern T_usb_app_info            uinf;

void   Thread_HMI_create(void);

#endif



