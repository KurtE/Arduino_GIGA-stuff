#pragma once
#include <SPI.h>
class SimpleSPIUserClass {
  public:
    SimpleSPIUserClass(SPIClass *pspi=&SPI, DMA_Stream_TypeDef * dmaStream = DMA1_Stream1);
    uint8_t transfer(uint8_t b);
    SPIClass *_pspi;
    DMA_Stream_TypeDef *_dmaStream = DMA1_Stream1;

};