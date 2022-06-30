#ifndef BACKPMAN3_DMA_H
  #define BACKPMAN3_DMA_H

#define ENABLE_MEM_ADDR_INC    1
#define DISABLE_MEM_ADDR_INC   0


#define  DMA_MEM_DATA_SIZE_8   0
#define  DMA_MEM_DATA_SIZE_16  1
#define  DMA_MEM_DATA_SIZE_32  2

#define DMA1_REQ_ADC1_DMA      9
#define DMA1_REQ_ADC2_DMA      10
#define DMA1_REQ_ADC3_DMA      115
#define DMA1_REQ_SPI4_TX_DMA   84


#define ADC_RES_READY   BIT(0)

extern TX_EVENT_FLAGS_GROUP   adc_flag;

void DMA1_from_ADC_init(void);
void DMA1_from_SPI4_init(void);
void DMA1_mem_to_SPI4_start(uint8_t *mem_addr, uint16_t len, uint8_t bits, uint8_t en_inc);

#endif // BACKPMAN3_DMA_H



