#ifndef __FREEMASTER_LOOP
  #define __FREEMASTER_LOOP

typedef struct
{
    uint8_t  OUT1;
    uint8_t  OUT2;
    uint8_t  OUT3;
    uint8_t  OUT4;
    uint8_t  LED_green;
    uint8_t  LED_red;
    uint8_t  ASW_R;
    uint8_t  ASW_F;
    uint8_t  PSW_R;
    uint8_t  PSW_F;
    uint8_t  LSW_F;
    uint8_t  OLED_RES;
    uint8_t  OLEDV;
    uint8_t  OLED_DC;
    uint8_t  OLED_CS;
    uint8_t  EN_CHARGER;
    uint8_t  DCDC_MODE;
} T_fm_pins_state;


  #define FMCMD_RESET_DEVICE       0x01
  #define FMCMD_CHECK_LOG_PIPE     0x07
  #define FMCMD_SAVE_WVARS         0x08
  #define FMCMD_START_STFS_TEST    0x09
  #define FMCMD_STOP_STFS_TEST     0x0A
  #define FMCMD_DUMP_SECTOR        0x0B


extern T_fm_pins_state             pst;
extern T_fm_pins_state             pst_prev;


uint32_t FreeMaster_task_create(ULONG initial_data);
void     Task_FreeMaster(uint32_t initial_data);

#endif

