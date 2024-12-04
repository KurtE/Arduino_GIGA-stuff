#ifndef _ILI9341_GIGA_N_H_
#define _ILI9341_GIGA_N_H_

// Allow us to enable or disable capabilities, particully Frame Buffer and
// Clipping for speed and size
#ifndef DISABLE_ILI9341_FRAMEBUFFER
// disable for first pass
#define ENABLE_ILI9341_FRAMEBUFFER
#endif

// Allow way to override using SPI

#ifdef __cplusplus
#include "Arduino.h"
#include <SPI.h>

#endif
#include <stdint.h>

//#include "ILI9341_fonts.h"

#define ILI9341_TFTWIDTH 240
#define ILI9341_TFTHEIGHT 320

#define ILI9341_NOP 0x00
#define ILI9341_SWRESET 0x01
#define ILI9341_RDDID 0x04
#define ILI9341_RDDST 0x09

#define ILI9341_SLPIN 0x10
#define ILI9341_SLPOUT 0x11
#define ILI9341_PTLON 0x12
#define ILI9341_NORON 0x13

#define ILI9341_RDMODE 0x0A
#define ILI9341_RDMADCTL 0x0B
#define ILI9341_RDPIXFMT 0x0C
#define ILI9341_RDIMGFMT 0x0D
#define ILI9341_RDSELFDIAG 0x0F

#define ILI9341_INVOFF 0x20
#define ILI9341_INVON 0x21
#define ILI9341_GAMMASET 0x26
#define ILI9341_DISPOFF 0x28
#define ILI9341_DISPON 0x29

#define ILI9341_CASET 0x2A
#define ILI9341_PASET 0x2B
#define ILI9341_RAMWR 0x2C
#define ILI9341_RAMRD 0x2E

#define ILI9341_PTLAR 0x30
#define ILI9341_VSCRDEF 0x33
#define ILI9341_MADCTL 0x36
#define ILI9341_VSCRSADD 0x37
#define ILI9341_PIXFMT 0x3A

#define ILI9341_FRMCTR1 0xB1
#define ILI9341_FRMCTR2 0xB2
#define ILI9341_FRMCTR3 0xB3
#define ILI9341_INVCTR 0xB4
#define ILI9341_DFUNCTR 0xB6

#define ILI9341_PWCTR1 0xC0
#define ILI9341_PWCTR2 0xC1
#define ILI9341_PWCTR3 0xC2
#define ILI9341_PWCTR4 0xC3
#define ILI9341_PWCTR5 0xC4
#define ILI9341_VMCTR1 0xC5
#define ILI9341_VMCTR2 0xC7

#define ILI9341_RDID1 0xDA
#define ILI9341_RDID2 0xDB
#define ILI9341_RDID3 0xDC
#define ILI9341_RDID4 0xDD

#define ILI9341_GMCTRP1 0xE0
#define ILI9341_GMCTRN1 0xE1
/*
#define ILI9341_PWCTR6  0xFC

*/

// Color definitions
#define ILI9341_BLACK 0x0000       /*   0,   0,   0 */
#define ILI9341_NAVY 0x000F        /*   0,   0, 128 */
#define ILI9341_DARKGREEN 0x03E0   /*   0, 128,   0 */
#define ILI9341_DARKCYAN 0x03EF    /*   0, 128, 128 */
#define ILI9341_MAROON 0x7800      /* 128,   0,   0 */
#define ILI9341_PURPLE 0x780F      /* 128,   0, 128 */
#define ILI9341_OLIVE 0x7BE0       /* 128, 128,   0 */
#define ILI9341_LIGHTGREY 0xC618   /* 192, 192, 192 */
#define ILI9341_DARKGREY 0x7BEF    /* 128, 128, 128 */
#define ILI9341_BLUE 0x001F        /*   0,   0, 255 */
#define ILI9341_GREEN 0x07E0       /*   0, 255,   0 */
#define ILI9341_CYAN 0x07FF        /*   0, 255, 255 */
#define ILI9341_RED 0xF800         /* 255,   0,   0 */
#define ILI9341_MAGENTA 0xF81F     /* 255,   0, 255 */
#define ILI9341_YELLOW 0xFFE0      /* 255, 255,   0 */
#define ILI9341_WHITE 0xFFFF       /* 255, 255, 255 */
#define ILI9341_ORANGE 0xFD20      /* 255, 165,   0 */
#define ILI9341_GREENYELLOW 0xAFE5 /* 173, 255,  47 */
#define ILI9341_PINK 0xF81F


#ifndef CL
#define CL(_r,_g,_b) ((((_r)&0xF8)<<8)|(((_g)&0xFC)<<3)|((_b)>>3))
#endif
#define sint16_t int16_t

#ifdef __cplusplus
// At all other speeds, _pspi->beginTransaction() will use the fastest available
// clock
#define ILI9341_SPICLOCK 30000000
#define ILI9341_SPICLOCK_READ 1000000

class ILI9341_GIGA_n : public Print {
public:

constexpr ILI9341_GIGA_n(uint8_t CS, uint8_t DC, uint8_t RST = 255 ) : _cs(CS), _dc(DC), _rst(RST) {}

  // Begin - main method to initialze the display.
  void setSPI(SPIClass &spi) {_pspi = &spi;}
  void begin(uint32_t spi_clock = ILI9341_SPICLOCK,
             uint32_t spi_clock_read = ILI9341_SPICLOCK_READ);
  void fillScreen(uint16_t color);
  inline void fillWindow(uint16_t color) { fillScreen(color); }
  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
  void setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
  // Pass 8-bit (each) R,G,B, get back 16-bit packed color
  static uint16_t color565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
  }

  // color565toRGB		- converts 565 format 16 bit color to RGB
  static void color565toRGB(uint16_t color, uint8_t &r, uint8_t &g,
                            uint8_t &b) {
    r = (color >> 8) & 0x00F8;
    g = (color >> 3) & 0x00FC;
    b = (color << 3) & 0x00F8;
  }
  // overwrite print functions:
    virtual size_t write(uint8_t) {return 0;};
//  virtual size_t write(const uint8_t *buffer, size_t size);

  int16_t width(void) { return _width; }
  int16_t height(void) { return _height; }
  uint8_t getRotation(void);

  SPIClass *_pspi = nullptr;

  //uint8_t _spi_num;         // Which buss is this spi on?
  uint32_t _SPI_CLOCK = ILI9341_SPICLOCK;      // #define ILI9341_SPICLOCK 30000000
  uint32_t _SPI_CLOCK_READ = ILI9341_SPICLOCK_READ; //#define ILI9341_SPICLOCK_READ 2000000

  int16_t _width = ILI9341_TFTWIDTH;
  int16_t _height = ILI9341_TFTHEIGHT; // Display w/h as modified by current rotation
  
  uint16_t _x0_last = 0xffff;
  uint16_t _x1_last = 0xffff;
  uint16_t _y0_last = 0xffff;
  uint16_t _y1_last = 0xffff;

  uint8_t _cs, _dc, _rst;
  //uint8_t pcs_data, pcs_command;
  //uint8_t _miso, _mosi, _sclk;

///////////////////////////////
// BUGBUG:: reorganize this area better!
//////////////////////////////
  void setAddr(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
      __attribute__((always_inline)) {
    if ((x0 != _x0_last) || (x1 != _x1_last)) {
      writecommand_cont(ILI9341_CASET); // Column addr set
      writedata16_cont(x0);             // XSTART
      writedata16_cont(x1);             // XEND
      _x0_last = x0;
      _x1_last = x1;
    }
    if ((y0 != _y0_last) || (y1 != _y1_last)) {
      writecommand_cont(ILI9341_PASET); // Row addr set
      writedata16_cont(y0);             // YSTART
      writedata16_cont(y1);             // YEND
      _y0_last = y0;
      _y1_last = y1;
    }
  }
//. From Onewire utility files

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

  void beginSPITransaction(uint32_t clock) __attribute__((always_inline)) {
    digitalWrite(_cs, LOW);
    _pspi->beginTransaction(SPISettings(clock, MSBFIRST, SPI_MODE0));
  }

  void endSPITransaction() __attribute__((always_inline)) {
    _pspi->endTransaction();
    digitalWrite(_cs, HIGH);
  }

  // Start off stupid
  uint8_t _dcpinAsserted = 0;

  void waitTransmitComplete(uint8_t called_from = 0) {
#if 0
    // so far nothing
    uint32_t start_time_us;
    uint32_t sr = 0;
    static uint32_t wtcCallCount = 0;

//    while((((sr = _pgigaSpi->SR) & SPI_SR_TXC) == 0) && (--to)) {
    wtcCallCount++; // increment the count of how many times we were called.
//    static uint8_t trace_count = 64;
    uint32_t sr_prev = _pgigaSpi->SR;
//    if (trace_count) {
//      Serial.print(wtcCallCount, DEC);
//      Serial.print(" ");
//      Serial.print(sr_prev, HEX);
//      Serial.print(":");
//      Serial.print(_data_sent_not_completed, DEC);
//    }

    // lets try with either TXC or EOT...
    start_time_us = micros();
    static const uint32_t TIMEOUT_US = 250;
    uint32_t delta_time = 0;
    while((delta_time = (micros() - start_time_us)) < TIMEOUT_US) {
      sr = _pgigaSpi->SR;
//      if (trace_count) {
//        if (sr != sr_prev) {
//          Serial.print("->");
//          sr_prev = sr;
//          Serial.print(sr_prev, HEX);
//          Serial.print(":");
//          Serial.print(_data_sent_not_completed, DEC);
//          start_time = micros();  // keep the printing from screwing up timing
//        }
//      }

      if (sr & SPI_SR_OVR) {
        // have overflow, clear it
        _pgigaSpi->IFCR = SPI_IFCR_OVRC;
        // printf("@@@@@@@@@@ OVERFLOW @@@@@@@@@@\n");
      }

      if (sr & SPI_SR_RXP ) {
        uint8_t unused __attribute__((unused));
        unused = *((__IO uint8_t *)&_pgigaSpi->RXDR);
        //if (_data_sent_not_completed) _data_sent_not_completed--;
        continue; // go check for more
      }

      // If nothing has been sent, we can get out of here now.
      if (!_data_sent_since_last_transmit_complete) {
        break;
      }

      // Now check for end of transmission.
      if ((sr & (SPI_SR_EOT | SPI_SR_TXC)) != 0) {
        break;
      }

    }
//    if (trace_count) {
//      Serial.print("$");
//      Serial.println(_data_sent_not_completed, DEC);
//      trace_count--;
//    }

    if (delta_time >= TIMEOUT_US) {
      Serial.print("**TO** WTC:  ");
      Serial.print(wtcCallCount, DEC);
      Serial.print(" ");
      Serial.print(called_from, DEC);
      Serial.print(" ");
      Serial.print(sr_prev, HEX);
      Serial.print(" ");
      Serial.println(sr, HEX);
//      Serial.print(" ");
//      Serial.println(_data_sent_not_completed, DEC);
    }
    _data_sent_since_last_transmit_complete = false;
    #endif
  }

  void setCommandMode() __attribute__((always_inline)) {
    if (!_dcpinAsserted) {
      waitTransmitComplete(1);
      digitalWrite(_dc, LOW);
      //digitalWrite(_dc, LOW);
      _dcpinAsserted = 1;
    }
  }

  void setDataMode() __attribute__((always_inline)) {
    if (_dcpinAsserted) {
      waitTransmitComplete(2);
      digitalWrite(_dc, HIGH);
      //digitalWrite(_dc, HIGH);
      _dcpinAsserted = 0;
    }
  }

  void outputToSPI(uint8_t c) {
    _pspi->transfer(c);
  }

  void outputToSPI16(uint16_t data) {
    #if 0
    _pspi->transfer16(data);
    #else
    outputToSPI(data >> 8); //MSB
    outputToSPI(data & 0xff);
    #endif
  }

  void writecommand_cont(uint8_t c) {
    setCommandMode();
    outputToSPI(c);
  }
  void writedata8_cont(uint8_t c) {
    setDataMode();
    outputToSPI(c);
  }

  void writedata16_cont(uint16_t c) {
    setDataMode();
    outputToSPI16(c);
  }

  void writecommand_last(uint8_t c) {
    setCommandMode();
    outputToSPI(c);
    waitTransmitComplete(3);
  }
  void writedata8_last(uint8_t c) {
    setDataMode();
    outputToSPI(c);
    waitTransmitComplete(4);
  }
  void writedata16_last(uint16_t c) {
    setDataMode();
    outputToSPI16(c);
    waitTransmitComplete(5);
  }

};

#endif // __cplusplus

#endif

