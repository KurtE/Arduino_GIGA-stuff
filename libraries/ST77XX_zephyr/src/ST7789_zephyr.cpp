
#include "ST77XX_zephyr.h"

ST7789_zephyr::ST7789_zephyr(SPIClass *pspi, uint8_t CS, uint8_t DC, uint8_t RST) :
    ST77XX_zephyr_n(pspi, CS, DC, RST) 
{
}


#define ST796SS_DFC 96
// Probably should use generic names like Adafruit..

#define ST7789_240x240_XSTART 0
#define ST7789_240x240_YSTART 80

// Probably should use generic names like Adafruit..
#define DELAY 0x80
static const uint8_t PROGMEM
  cmd_st7789[] = {                  // Initialization commands for 7735B screens
    9,                       // 9 commands in list:
    ST77XX_SWRESET,   CMD_LIST_DELAY,  //  1: Software reset, no args, w/delay
      150,                     //    150 ms delay
    ST77XX_SLPOUT ,   CMD_LIST_DELAY,  //  2: Out of sleep mode, no args, w/delay
      255,                    //     255 = 500 ms delay
    ST77XX_COLMOD , 1+CMD_LIST_DELAY,  //  3: Set color mode, 1 arg + delay:
      0x55,                   //     16-bit color
      10,                     //     10 ms delay
    ST77XX_MADCTL , 1      ,  //  4: Memory access ctrl (directions), 1 arg:
      0x08,                   //     Row addr/col addr, bottom to top refresh
    ST77XX_CASET  , 4      ,  //  5: Column addr set, 4 args, no delay:
      0x00, 
      0x00,                   //     XSTART = 0
      0x00, 
      240,                    //      XEND = 240
    ST77XX_RASET  , 4      ,  // 6: Row addr set, 4 args, no delay:
      0x00, 
      0x00,                   //     YSTART = 0
      320>>8, 
      320 & 0xFF,             //      YEND = 320
    ST77XX_INVON ,   CMD_LIST_DELAY,   // 7: hack
      10,
    ST77XX_NORON  ,   CMD_LIST_DELAY,  // 8: Normal display on, no args, w/delay
      10,                     //     10 ms delay
    ST77XX_DISPON ,   CMD_LIST_DELAY,  // 9: Main screen turn on, no args, w/delay
    255 
};                  //     255 = 500 ms delay


void  ST7789_zephyr::begin(uint16_t width, uint16_t height, uint8_t mode, uint32_t spi_clock)
{
  printk("ST7789 begin(%u, %u, %u, %u)\n", width, height, mode, spi_clock);
	_width = _screenWidth = width;
	_height = _screenHeight = height;
    _SPI_CLOCK = spi_clock;           // #define ST77XX_SPICLOCK 30000000
    _SPI_MODE = mode;
    printk("common init called\n");
    common_init(nullptr);
	if ((width == 240) && (height == 240)) {
		_colstart = 0;
		_colstart2 = 0;
		_rowstart = 80;
		_rowstart2 = 0;
	} else if ((width == 135) && (height == 240)) { // 1.13" display Their smaller display
		_colstart = 53;
		_colstart2 = 52;  // odd size
		_rowstart = 40;
		_rowstart2 = 40;
	} else {  // lets compute it.
		// added support for other sizes
		_rowstart = _rowstart2 = (int)((320 - height) / 2);
		_colstart = _colstart2 = (int)((240 - width) / 2);
	}
  	commandList(cmd_st7789);
  	setRotation(0);

}

void  ST7789_zephyr::setRotation(uint8_t m) 
{
  beginSPITransaction();
  writecommand_cont(ST77XX_MADCTL);
  _rotation = m % 4; // can't be higher than 3
  switch (_rotation) {
   case 0:
     writedata8_last(ST77XX_MADCTL_MX | ST77XX_MADCTL_MY | ST77XX_MADCTL_RGB);

     _xstart = _colstart;
     _ystart = _rowstart;
     _width = _screenWidth;
     _height = _screenHeight;
     break;
   case 1:
     writedata8_last(ST77XX_MADCTL_MY | ST77XX_MADCTL_MV | ST77XX_MADCTL_RGB);

     _xstart = _rowstart;
     _ystart = _colstart2;
     _height = _screenWidth;
     _width = _screenHeight;
     break;
  case 2:
     writedata8_last(ST77XX_MADCTL_RGB); 
     _xstart = _colstart2;
     _ystart = _rowstart2;
     _width = _screenWidth;
     _height = _screenHeight;
     break;

   case 3:
     writedata8_last(ST77XX_MADCTL_MX | ST77XX_MADCTL_MV | ST77XX_MADCTL_RGB);
     _xstart = _rowstart2;
     _ystart = _colstart;
     _height = _screenWidth;
     _width = _screenHeight;
     break;
  }

  endSPITransaction();
//  Serial.printf("Set _rotation %d start(%d %d) row: %d, col: %d\n", m, _xstart, _ystart, _rowstart, _colstart);
  setClipRect();
  setOrigin();
	
	cursor_x = 0;
	cursor_y = 0;
}
