// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2022-03-29
// 17:41:21
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"

uint8_t     spi_b      @ "SRAM1";
#pragma data_alignment=8
uint8_t    spi_buf[4]  @ "SRAM1";


//static void TFT_clear_screen(void);

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void     TFT_init(void)
{
  uint8_t tmp;

  SPI_TFT_init();

  Set_OLEDV(1);
  Set_OLED_RES(0);
  Wait_ms(10);
  Set_OLED_RES(1);

  Wait_ms(100);
  TFT_wr__cmd(0x11); //exit SLEEP mode

  Wait_ms(100);

  TFT_wr__cmd(0x36);

  //
  tmp = 0
       + LSHIFT(1, 7) // MY  - Page Address Order       | 0 = Top to Bottom, 1 = Bottom to Top
       + LSHIFT(0, 6) // MX  - Column Address Order     | 0 = Left to Right, 1 = Right to Left
       + LSHIFT(1, 5) // MV  - Page/Column Order        | 0 = Normal Mode, 1 = Reverse Mode
       + LSHIFT(0, 4) // ML  - Line Address Order       | 0 = LCD Refresh Top to Bottom, 1 = LCD Refresh Bottom to Top
       + LSHIFT(0, 3) // RGB - RGB/BGR Order            | 0 = RGB, 1 = BGR
       + LSHIFT(0, 2) // MH  - Display Data Latch Order | 0 = LCD Refresh Left to Right, 1 = LCD Refresh Right to Left
  ;


  TFT_wr_data(tmp); //MADCTL: memory data access control

  TFT_wr__cmd(0x3A);
  TFT_wr_data(0x55); //COLMOD: 16 bit/pixel

  TFT_wr__cmd(0xB2);
  TFT_wr_data(0x0C);
  TFT_wr_data(0x0C);
  TFT_wr_data(0x00);
  TFT_wr_data(0x33);
  TFT_wr_data(0x33); //PORCTRK: Porch setting

  TFT_wr__cmd(0xB7);
  TFT_wr_data(0x35); //GCTRL: Gate Control

  TFT_wr__cmd(0xBB);
  TFT_wr_data(0x2B); //VCOMS: VCOM setting

  TFT_wr__cmd(0xC0);
  TFT_wr_data(0x2C); //LCMCTRL: LCM Control

  TFT_wr__cmd(0xC2);
  TFT_wr_data(0x01);
  TFT_wr_data(0xFF); //VDVVRHEN: VDV and VRH Command Enable

  TFT_wr__cmd(0xC3);
  TFT_wr_data(0x11); //VRHS: VRH Set

  TFT_wr__cmd(0xC4);
  TFT_wr_data(0x20); //VDVS: VDV Set

  TFT_wr__cmd(0xC6);
  TFT_wr_data(0x0F); //FRCTRL2: Frame Rate control in normal mode

  TFT_wr__cmd(0xD0);
  TFT_wr_data(0xA4);
  TFT_wr_data(0xA1); //PWCTRL1: Power Control 1

  TFT_wr__cmd(0xE0);
  TFT_wr_data(0xD0);
  TFT_wr_data(0x00);
  TFT_wr_data(0x05);
  TFT_wr_data(0x0E);
  TFT_wr_data(0x15);
  TFT_wr_data(0x0D);
  TFT_wr_data(0x37);
  TFT_wr_data(0x43);
  TFT_wr_data(0x47);
  TFT_wr_data(0x09);
  TFT_wr_data(0x15);
  TFT_wr_data(0x12);
  TFT_wr_data(0x16);
  TFT_wr_data(0x19); //PVGAMCTRL: Positive Voltage Gamma control

  TFT_wr__cmd(0xE1);
  TFT_wr_data(0xD0);
  TFT_wr_data(0x00);
  TFT_wr_data(0x05);
  TFT_wr_data(0x0D);
  TFT_wr_data(0x0C);
  TFT_wr_data(0x06);
  TFT_wr_data(0x2D);
  TFT_wr_data(0x44);
  TFT_wr_data(0x40);
  TFT_wr_data(0x0E);
  TFT_wr_data(0x1C);
  TFT_wr_data(0x18);
  TFT_wr_data(0x16);
  TFT_wr_data(0x19); //NVGAMCTRL: Negative Voltage Gamma control

  TFT_wr__cmd(0x2A);
  TFT_wr_data(0x00);
  TFT_wr_data(0x50); // X от 80
  TFT_wr_data(0x01);
  TFT_wr_data(0x3F); // до 319

  TFT_wr__cmd(0x2B);
  TFT_wr_data(0x00);
  TFT_wr_data(0x00); // Y от 0
  TFT_wr_data(0x00);
  TFT_wr_data(0xEF); // до 239

  TFT_wr__cmd(0x21); // Inverse

  Wait_ms(5);
  //TFT_wr__cmd(0x29); //display ON

  //TFT_clear_screen();
}


/*-----------------------------------------------------------------------------------------------------


  \param x0
  \param y0
  \param x1
  \param y1
-----------------------------------------------------------------------------------------------------*/
void     TFT_Set_rect(int x0, int y0, int x1, int y1)
{
  uint16_t xaddr0;
  uint16_t xaddr1;

  if (x0 > 239) x0 = 239;
  if (y0 > 239) y0 = 239;

  if (x1 > 239) x1 = 239;
  if (y1 > 239) y1 = 239;

  xaddr0 = 0x50 + x0;
  xaddr1 = 0x50 + x1;

  TFT_wr__cmd(0x2A);
  spi_buf[0] =(xaddr0 >> 8) & 0xFF;
  spi_buf[1] = xaddr0 & 0xFF;
  spi_buf[2] =(xaddr1 >> 8) & 0xFF;
  spi_buf[3] = xaddr1 & 0xFF;
  TFT_wr_data_buf(spi_buf, 4);

  TFT_wr__cmd(0x2B);
  spi_buf[0] =(y0 >> 8) & 0xFF;
  spi_buf[1] = y0 & 0xFF;
  spi_buf[2] =(y1 >> 8) & 0xFF;
  spi_buf[3] = y1 & 0xFF;
  TFT_wr_data_buf(spi_buf, 4);

}

/*-----------------------------------------------------------------------------------------------------


  \param x
-----------------------------------------------------------------------------------------------------*/
void     TFT_Set_x(uint32_t x)
{
  uint16_t xaddr;

  if (x > 239) x = 239;
  xaddr = 0x50 + x;

  TFT_wr__cmd(0x2A);
  spi_buf[0] =(xaddr >> 8) & 0xFF;
  spi_buf[1] = xaddr & 0xFF;
  spi_buf[2] = 0x01;
  spi_buf[3] = 0x3F;
  TFT_wr_data_buf(spi_buf, 4);

}

/*-----------------------------------------------------------------------------------------------------


  \param y
-----------------------------------------------------------------------------------------------------*/
void     TFT_Set_y(uint32_t y)
{
  if (y > 239) y = 239;

  TFT_wr__cmd(0x2B);
  spi_buf[0] =(y >> 8) & 0xFF;
  spi_buf[1] = y & 0xFF;
  spi_buf[2] = 0x00;
  spi_buf[3] = 0xEF;
  TFT_wr_data_buf(spi_buf, 4);
}

/*-----------------------------------------------------------------------------------------------------


  \param data
-----------------------------------------------------------------------------------------------------*/
void     TFT_wr__cmd(uint8_t data)
{
  spi_b = data;
  Set_OLED_DC(0);
  Set_OLED_CS(0);
  SPI_TFT_send(&spi_b, 1, DMA_MEM_DATA_SIZE_8, ENABLE_MEM_ADDR_INC);
  Set_OLED_CS(1);
}


/*-----------------------------------------------------------------------------------------------------


  \param data
-----------------------------------------------------------------------------------------------------*/
void     TFT_wr_data(uint8_t data)
{
  spi_b = data;
  Set_OLED_DC(1);
  Set_OLED_CS(0);
  SPI_TFT_send(&spi_b, 1, DMA_MEM_DATA_SIZE_8, ENABLE_MEM_ADDR_INC);
  Set_OLED_CS(1);
}


/*-----------------------------------------------------------------------------------------------------


  \param buf
  \param buf_sz

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t TFT_wr_data_buf(uint8_t *buf, uint32_t buf_sz)
{
  uint32_t res;
  uint16_t sz;
  Set_OLED_DC(1);
  Set_OLED_CS(0);
  while (buf_sz > 0)
  {
    if (buf_sz > 65535) sz = 65535;
    else sz = buf_sz;

    res = SPI_TFT_send(buf, sz, DMA_MEM_DATA_SIZE_8, ENABLE_MEM_ADDR_INC);
    buf_sz -= sz;
    buf += sz;
  }
  Set_OLED_CS(1);
  return res;
}

/*-----------------------------------------------------------------------------------------------------
  Заполнение области на TFT дисплее 16-битным пикселом в формате RGB 565

  \param w
  \param data_sz

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t TFT_fill_by_pixel(uint16_t *w, uint32_t data_sz)
{
  uint32_t res;
  uint16_t sz;

  spi_buf[0] =(*w) & 0xFF;
  spi_buf[1] =((*w)>> 8) & 0xFF;

  Set_OLED_DC(1);
  Set_OLED_CS(0);
  while (data_sz > 0)
  {
    if (data_sz > 32768) sz = 32768;
    else sz = data_sz;

    res = SPI_TFT_send(spi_buf, sz, DMA_MEM_DATA_SIZE_16, DISABLE_MEM_ADDR_INC);
    data_sz -= sz;
  }
  Set_OLED_CS(1);
  return res;
}


///*-----------------------------------------------------------------------------------------------------
//
//
//  \param void
//-----------------------------------------------------------------------------------------------------*/
//static void TFT_clear_screen(void)
//{
//  uint16_t w = 0x0000; // Заполняем черным цветом
//  TFT_Set_rect(0, 0 , LCD_X_SIZE-1, LCD_Y_SIZE-1);
//  TFT_wr__cmd(0x2C); // Команда записи в дисплей
//  TFT_fill_by_pixel(&w, LCD_X_SIZE * LCD_Y_SIZE * 2);
//  TFT_wr__cmd(0x29); // display ON
//}




