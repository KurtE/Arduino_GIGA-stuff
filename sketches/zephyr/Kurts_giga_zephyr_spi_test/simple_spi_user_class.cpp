#include "simple_spi_user_class.h"

SimpleSPIUserClass::SimpleSPIUserClass(SPIClass *pspi, DMA_Stream_TypeDef * dmaStream ) {
  _pspi = pspi;
  _dmaStream = dmaStream;
}

uint8_t SimpleSPIUserClass::transfer(uint8_t b) {
  return _pspi->transfer(b);
}