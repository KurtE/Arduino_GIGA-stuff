#pragma once
#include <Arduino.h>
#include <Adafruit_GFX.h>

///  A GFX 16-bit canvas context for graphics
class SDRAMGFXcanvas16 : public Adafruit_GFX {
public:
  SDRAMGFXcanvas16(uint16_t w, uint16_t h);
  ~SDRAMGFXcanvas16(void);
  void begin(); //
  void drawPixel(int16_t x, int16_t y, uint16_t color);
  void fillScreen(uint16_t color);
  void byteSwap(void);
  void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
  void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
  uint16_t getPixel(int16_t x, int16_t y) const;
  /**********************************************************************/
  /*!
    @brief    Get a pointer to the internal buffer memory
    @returns  A pointer to the allocated buffer
  */
  /**********************************************************************/
  uint16_t *getBuffer(void) const { return buffer; }

protected:
  uint16_t getRawPixel(int16_t x, int16_t y) const;
  void drawFastRawVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
  void drawFastRawHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
  uint16_t *buffer; ///< Raster data: no longer private, allow subclass access
};
