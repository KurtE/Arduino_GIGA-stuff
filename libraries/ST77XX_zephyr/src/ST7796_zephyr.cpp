#include "ST77XX_zephyr.h"

ST7796_zephyr::ST7796_zephyr(SPIClass *pspi, uint8_t CS, uint8_t DC, uint8_t RST) :
    ST77XX_zephyr_n(pspi, CS, DC, RST) 
{
}


#define ST796SS_DFC 96
// Probably should use generic names like Adafruit..

static const uint8_t PROGMEM
  cmd_ST7796[] = {                  // Initialization commands for ST7796 screens
    17,                             // 9 commands in list:

    ST77XX_SWRESET,   CMD_LIST_DELAY,        //  1: Software reset, no args, w/CMD_LIST_DELAY
      150,                     //    150 ms CMD_LIST_DELAY

    ST7796S_SLPOUT,   CMD_LIST_DELAY,        // sleep exit
      120,  

    0xF0, 1, 0xC3,                  //Command Set control Enable extension command 2 partI                                
    0xF0, 1, 0x96,                  //Command Set control Enable extension command 2 partII
    ST7796S_MADCTL, 1, 0x48,        //Memory Data Access Control MX, MY, RGB mode                                    
                                    //X-Mirror, Top-Left to right-Buttom, RGB  
  
    ST7796S_PIXFMT, 1, 0x55,           //Interface Pixel Format 16 bits
  
    ST7796S_INVCTR, 1, 0x01,        // 1 dont inversion.
    ST7796S_DFUNCTR,3, 0x80, 0x02, 0x3B, //

    0xE8, 8,               //Display Output Ctrl Adjust
      0x40, 0x8A, 0x00, 0x00,
      0x29,    //Source eqaulizing period time= 22.5 us
      0x19,    //Timing for "Gate start"=25 (Tclk)
      0xA5,    //Timing for "Gate End"=37 (Tclk), Gate driver EQ function ON
      0x33,

   ST7796S_PWCTR2, 1,//Power control2                          
      0x06,         //VAP(GVDD)=3.85+( vcom+vcom offset), VAN(GVCL)=-3.85+( vcom+vcom offset)
    ST7796S_PWCTR3, 1, // power control 3
      0xA7,    //Source driving current level=low, Gamma driving current level=High
   
    ST7796S_VMCTR1, CMD_LIST_DELAY | 1, //VCOM Control
      0x18,    //VCOM=0.9
      120,      // CMD_LIST_DELAY

  //ST7796 Gamma Sequence
    ST7796S_GMCTRP1,  14, //Gamma"+"                                             
      0xF0, 0x09, 0x0b, 0x06,
      0x04, 0x15, 0x2F, 0x54,
      0x42, 0x3C, 0x17, 0x14,
      0x18, 0x1B,
   
    ST7796S_GMCTRN1, CMD_LIST_DELAY | 14,  //Gamma"-"                                             
      0xE0, 0x09, 0x0B, 0x06,
      0x04, 0x03, 0x2B, 0x43,
      0x42, 0x3B, 0x16, 0x14,
      0x17, 0x1B,
      120,                      // CMD_LIST_DELAY
  
    0xF0, 1, 0x3C,            // disable command set part 1

    0xF0, CMD_LIST_DELAY | 1, 0x69,            // disable command set part 2
      120,                  // CMD_LIST_DELAY

    ST7796S_DISPON, CMD_LIST_DELAY,
      120
};

void  ST7796_zephyr::begin(uint16_t width, uint16_t height, uint8_t mode, uint32_t spi_clock)
{
	_width = _screenWidth = width;
	_height = _screenHeight = height;
	_SPI_CLOCK = spi_clock;           // #define ST77XX_SPICLOCK 30000000
	_SPI_MODE = mode;
	common_init(nullptr);
	if ((width == 320) && (height == 480)) {
		_colstart = 0;   
		_colstart2 = 0; 
		_rowstart = 0;   
		_rowstart2 = 0;
	} else {  // lets compute it.
   		// added support for other sizes
    	_rowstart = _rowstart2 = (int)((480 - height) / 2);
    	_colstart = _colstart2 = (int)((320 - width) / 2);
  	}
	commandList(cmd_ST7796);

  	setRotation(0); 

}

void  ST7796_zephyr::setRotation(uint8_t m) 
{
  beginSPITransaction(_SPI_CLOCK);
  writecommand_cont(ST77XX_MADCTL);
  _rotation = m % 4; // can't be higher than 3
  switch (_rotation) {
   case 0:
     writedata8_last(ST77XX_MADCTL_MX | ST77XX_MADCTL_BGR);

     _xstart = _colstart;
     _ystart = _rowstart;
     _width = _screenWidth;
     _height = _screenHeight;
     break;
   case 1:
     writedata8_last(ST77XX_MADCTL_MV | ST77XX_MADCTL_BGR);

     _xstart = _rowstart;
     _ystart = _colstart2;
     _height = _screenWidth;
     _width = _screenHeight;
     break;
  case 2:
     writedata8_last(ST77XX_MADCTL_MY | ST77XX_MADCTL_RGB); 
     _xstart = _colstart2;
     _ystart = _rowstart2;
     _width = _screenWidth;
     _height = _screenHeight;
     break;

   case 3:
     writedata8_last(ST77XX_MADCTL_MX | ST77XX_MADCTL_MY | ST77XX_MADCTL_MV | ST77XX_MADCTL_BGR);
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
