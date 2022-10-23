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
    uint8_t  DCDC_PGOOD;
} T_fm_pins_state;


  #define FMCMD_RESET_DEVICE                   0x01
  #define FMCMD_CHECK_LOG_PIPE                 0x07
  #define FMCMD_SAVE_WVARS                     0x08
  #define FMCMD_START_STFS_TEST                0x09
  #define FMCMD_STOP_STFS_TEST                 0x0A
  #define FMCMD_DUMP_STFS_SECTOR_TO_LOG        0x0B
  #define FMCMD_SET_DCDC_MODE_TO_0             0x10
  #define FMCMD_SET_DCDC_MODE_TO_1             0x11
  #define FMCMD_SET_DCDC_MODE_TO_SYNC          0x12
  #define FMCMD_SET_DCDC_SYNC_FREQ             0x13

extern T_fm_pins_state             pst;
extern T_fm_pins_state             pst_prev;


uint32_t Thread_FreeMaster_create(void);
void     FreeMaster_task_delete(void);
void     Task_FreeMaster(uint32_t initial_data);

#endif

