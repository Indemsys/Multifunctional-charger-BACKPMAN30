// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2022-03-31
// 16:41:05 
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"


T_power_controller_state pwr_cbl;

static uint8_t   dac_inited;
uint16_t         g_dac_data = 0xFFFF;  // Устанавливаем код так чтобы было минимальное напряжение на выходе DCDC

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void DCDC_toggle_state(void)
{
  if (pwr_cbl.dcdc_enable)
  {
    pwr_cbl.dcdc_enable = 0;
    Set_EN_CHARGER(0);
    Set_output_off(OUTP_LED_GREEN);
  }
  else
  {
    pwr_cbl.dcdc_enable = 1;

    if (Get_PSW_F() == 0)
    {
      Set_PSW_R(1);
      Set_PSW_F(1);
      Wait_ms(100);
    }
    Set_EN_CHARGER(1);
    Set_output_on(OUTP_LED_GREEN);
  }
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void DAC_proc(uint16_t data)
{
  if (dac_inited == 0)
  {
    dac_inited = 1;
    SPI_DAC_init();
    SPI_DAC_send(4, 0); // BUFF-GAIN = 1 Диапазон выходного напряжения от 0 до 2.5 В
  }

  SPI_DAC_send(8, data);
}

