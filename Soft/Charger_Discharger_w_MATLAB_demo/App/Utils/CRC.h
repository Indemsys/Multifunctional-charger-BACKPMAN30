#ifndef CRC_H
  #define CRC_H


uint16_t CRC16_ccitt(uint8_t *buf, uint32_t len);
uint16_t CRC16_x25_ccitt(uint8_t *buf, uint32_t len);
uint16_t CRC16_ccitt_seed(uint8_t *buf, uint32_t len, uint16_t seed);
uint16_t CRC16_0x5935(uint8_t *buf, uint32_t len, uint16_t seed);
uint16_t CRC16_0x755B(uint8_t *buf, uint32_t len, uint16_t seed);
uint16_t CRC16_matlab(uint8_t *buf, uint32_t len);

#endif // CRC_H



