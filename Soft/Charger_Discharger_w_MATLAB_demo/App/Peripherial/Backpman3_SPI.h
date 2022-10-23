#ifndef BACKPMAN3_SPI_H
  #define BACKPMAN3_SPI_H


void     SPI_TFT_init(void);
uint32_t SPI_TFT_send(volatile uint8_t *buf, uint16_t len, uint8_t bits, uint8_t en_inc);

void     SPI_DAC_init(void);
uint32_t SPI_DAC_send(uint8_t cmd, uint16_t data);

#endif // BACKPMAN3_SPI_H



