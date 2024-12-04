
//#include <Arduino.h>
#include "ILI9341_simple_giga.h"
#include <SPI.h>


#define WIDTH ILI9341_TFTWIDTH
#define HEIGHT ILI9341_TFTHEIGHT
#define CBALLOC (ILI9341_TFTHEIGHT * ILI9341_TFTWIDTH * 2)


// Constructor when using hardware ILI9241_KINETISK__pspi->  Faster, but must
// use SPI pins
// specific to each board type (e.g. 11,13 for Uno, 51,52 for Mega, etc.)

// Constructor when using hardware ILI9241_KINETISK__pspi->  Faster, but must
// use SPI pins
// specific to each board type (e.g. 11,13 for Uno, 51,52 for Mega, etc.)


//=======================================================================

void ILI9341_GIGA_n::setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1,
                                   uint16_t y1) {
    beginSPITransaction(_SPI_CLOCK);
    setAddr(x0, y0, x1, y1);
    writecommand_last(ILI9341_RAMWR);  // write to RAM
    endSPITransaction();
}

void ILI9341_GIGA_n::fillScreen(uint16_t color) {
    fillRect(0, 0, _width, _height, color);
}

// fill a rectangle
uint16_t row_buff[ILI9341_TFTHEIGHT]; // 
void ILI9341_GIGA_n::fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
                              uint16_t color) {
    // printf("\tfillRect(%d, %d, %d, %d, %x)\n", x, y, w, h, color);
    // TODO: this can result in a very long transaction time
    // should break this into multiple transactions, even though
    // it'll cost more overhead, so we don't stall other SPI libs
    beginSPITransaction(_SPI_CLOCK);
    setAddr(x, y, x + w - 1, y + h - 1);
    writecommand_cont(ILI9341_RAMWR);
    setDataMode();
    uint16_t color_swapped = (color >> 8) | ((color & 0xff) << 8);
    for (y = h; y > 0; y--) {
      #if 1
      for (uint16_t i = 0; i < w; i++) row_buff[i] = color_swapped;
      _pspi->transfer(row_buff, w * 2);

      #else
        for (x = w; x > 1; x--) {
            writedata16_cont(color);
        }
        writedata16_cont(color);  // was last
      #endif  
    }
    waitTransmitComplete(6);
    endSPITransaction();
  // printf("\tfillRect - end\n");
}

static const uint8_t PROGMEM init_commands[] = { 4, 0xEF, 0x03, 0x80, 0x02,
                                                 4, 0xCF, 0x00, 0XC1, 0X30,
                                                 5, 0xED, 0x64, 0x03, 0X12, 0X81,
                                                 4, 0xE8, 0x85, 0x00, 0x78,
                                                 6, 0xCB, 0x39, 0x2C, 0x00, 0x34, 0x02,
                                                 2, 0xF7, 0x20,
                                                 3, 0xEA, 0x00, 0x00,
                                                 2, ILI9341_PWCTR1, 0x23,        // Power control
                                                 2, ILI9341_PWCTR2, 0x10,        // Power control
                                                 3, ILI9341_VMCTR1, 0x3e, 0x28,  // VCM control
                                                 2, ILI9341_VMCTR2, 0x86,        // VCM control2
                                                 2, ILI9341_MADCTL, 0x48,        // Memory Access Control
                                                 2, ILI9341_PIXFMT, 0x55,
                                                 3, ILI9341_FRMCTR1, 0x00, 0x18,
                                                 4, ILI9341_DFUNCTR, 0x08, 0x82, 0x27,  // Display Function Control
                                                 2, 0xF2, 0x00,                         // Gamma Function Disable
                                                 2, ILI9341_GAMMASET, 0x01,             // Gamma curve selected
                                                 16, ILI9341_GMCTRP1, 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E,
                                                 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00,  // Set Gamma
                                                 16, ILI9341_GMCTRN1, 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31,
                                                 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F,  // Set Gamma
                                                 3, 0xb1, 0x00, 0x10,                             // FrameRate Control 119Hz
                                                 0 };

void ILI9341_GIGA_n::begin(uint32_t spi_clock, uint32_t spi_clock_read) {
    // verify SPI pins are valid;
    // allow user to say use current ones...
    _SPI_CLOCK = spi_clock;            // #define ILI9341_SPICLOCK 30000000
    _SPI_CLOCK_READ = spi_clock_read;  //#define ILI9341_SPICLOCK_READ 2000000

    // Serial.printf("_t3n::begin mosi:%d miso:%d SCLK:%d CS:%d DC:%d SPI clocks:
    // %lu %lu\n", _mosi, _miso, _sclk, _cs, _dc, _SPI_CLOCK, _SPI_CLOCK_READ);
    // Serial.flush();
    if (_pspi == nullptr) return;
        //_pspi = &SPI;

    _pspi->begin();

    // TLC
    //pcs_data = 0;
    //pcs_command = 0;
    pinMode(_cs, OUTPUT);
    digitalWrite(_cs, HIGH);

    pinMode(_dc, OUTPUT);
    digitalWrite(_dc, HIGH);
    _dcpinAsserted = 0;


    // toggle RST low to reset
    if (_rst < 255) {
        pinMode(_rst, OUTPUT);
        digitalWrite(_rst, HIGH);
        delay(5);
        digitalWrite(_rst, LOW);
        delay(20);
        digitalWrite(_rst, HIGH);
        delay(150);
    }

    beginSPITransaction(_SPI_CLOCK / 4);
    const uint8_t *addr = init_commands;
    while (1) {
        uint8_t count = *addr++;
        if (count-- == 0)
            break;
        writecommand_cont(*addr++);
        while (count-- > 0) {
            writedata8_cont(*addr++);
        }
    }
    writecommand_last(ILI9341_SLPOUT);  // Exit Sleep
    endSPITransaction();
    delay(120);
    beginSPITransaction(_SPI_CLOCK);
    writecommand_last(ILI9341_DISPON);  // Display on
    endSPITransaction();

    // Serial.println("_t3n::begin - completed"); Serial.flush();
}
