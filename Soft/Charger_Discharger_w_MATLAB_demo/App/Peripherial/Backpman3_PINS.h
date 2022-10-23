#ifndef BACKPMAN3_PINS_H
# define BACKPMAN3_PINS_H

typedef struct
{
    GPIO_TypeDef  *gpio;
    unsigned char  pin_num;
    unsigned char  MODER;   //  Port mode
                            //  00: Input mode
                            //  01: General purpose output mode
                            //  10: Alternate function mode
                            //  11: Analog mode (reset state)

    unsigned char  AFR;    //  Port alternate function

    unsigned int   OSPEEDR; //  Port output speed
                            //  00: Low speed
                            //  01: Medium speed
                            //  10: High speed
                            //  11: Very high speed


    unsigned char  OTYPER;  //  Port output type
                            //  0: Output push-pull (reset state)
                            //  1: Output open-drain


    unsigned char  PUPDR;   //  Port pull-up/pull-down
                            //  00: No pull-up, pull-down
                            //  01: Pull-up
                            //  10: Pull-down
                            //  11: Reserved


    unsigned char  init;    // Init state

} T_IO_pins_configuration;

#define   INPT          0  // 00: Input mode
#define   GPIO          1  // 01: General purpose output mode
#define   ALTF          2  // 10: Alternate function mode
#define   ANAL          3  // 11: Analog mode (reset state)

#define   ALT00          0 // Alternative 0
#define   ALT01          1 // Alternative 1
#define   ALT02          2 //
#define   ALT03          3 //
#define   ALT04          4 //
#define   ALT05          5 //
#define   ALT06          6 //
#define   ALT07          7 //
#define   ALT08          8 //
#define   ALT09          9 //
#define   ALT10         10 //
#define   ALT11         11 //
#define   ALT12         12 //
#define   ALT13         13 //
#define   ALT14         14 //
#define   ALT15         15 //

#define   SPD_L         0  //  00: Low speed
#define   SPD_M         1  //  01: Medium speed
#define   SPD_H         2  //  10: High speed
#define   SPD_V         3  //  11: Very high speed

#define   OUT_PP        0  //  0: Output push-pull (reset state)
#define   OUT_OD        1  //  1: Output open-drain

#define   PUPD_DIS      0  // 00: No pull-up, pull-down
#define   PULL__UP      1  // 01: Pull-up
#define   PULL__DN      2  // 10: Pull-down
#define   PULL__RS      3  // 11: Reserved


#define   GP_INP        0 // 0 Pin is configured as general purpose input, if configured for the GPIO function
#define   GP_OUT        1 // 1 Pin is configured for general purpose output, if configured for the GPIO function


void      Config_pin(const T_IO_pins_configuration pinc);
int       Backpman3_init_pins(void);

void      Set_OUT1_level(uint32_t val);
void      Set_OUT2_level(uint32_t val);
void      Set_OUT3_level(uint32_t val);
void      Set_OUT4_level(uint32_t val);
void      Set_LED_green(uint32_t val);
void      Set_LED_red(uint32_t val);
void      Set_ASW_R(uint32_t val);
void      Set_ASW_F(uint32_t val);
void      Set_PSW_R(uint32_t val);
void      Set_PSW_F(uint32_t val);
void      Set_LSW_F(uint32_t val);
void      Set_OLED_RES(uint32_t val);
void      Set_OLEDV(uint32_t val);
void      Set_OLED_DC(uint32_t val);
void      Set_OLED_CS(uint32_t val);
void      Set_EN_CHARGER(uint32_t val);
void      Set_DCDC_MODE_pin(uint32_t val);
void      Set_DAC_CS(uint32_t val);

uint32_t  Get_OUT1_level(void);
uint32_t  Get_OUT2_level(void);
uint32_t  Get_OUT3_level(void);
uint32_t  Get_OUT4_level(void);
uint32_t  Get_LED_green(void);
uint32_t  Get_LED_red(void);
uint32_t  Get_ASW_R(void);
uint32_t  Get_ASW_F(void);
uint32_t  Get_PSW_R(void);
uint32_t  Get_PSW_F(void);
uint32_t  Get_LSW_F(void);
uint32_t  Get_OLED_RES(void);
uint32_t  Get_OLEDV(void);
uint32_t  Get_OLED_DC(void);
uint32_t  Get_OLED_CS(void);
uint32_t  Get_EN_CHARGER(void);
uint32_t  Get_DCDC_MODE_pin(void);
uint32_t  Get_DCDC_PGOOD(void);

uint32_t  Get_smpl_enc_a (void);
uint32_t  Get_smpl_enc_b (void);
uint32_t  Get_smpl_enc_sw(void);


#endif // BACKPMAN3_PINS_H



