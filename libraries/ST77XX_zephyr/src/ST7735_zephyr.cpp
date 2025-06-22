
#include "ST77XX_zephyr.h"

ST7735_zephyr::ST7735_zephyr(SPIClass *pspi, uint8_t CS, uint8_t DC, uint8_t RST) :
    ST77XX_zephyr(pspi, CS, DC, RST) 
{
}

#define DELAY 0x80
static const uint8_t PROGMEM
  Bcmd[] = {                  // Initialization commands for 7735B screens
    18,                       // 18 commands in list:
    ST77XX_SWRESET,   DELAY,  //  1: Software reset, no args, w/delay
      50,                     //     50 ms delay
    ST77XX_SLPOUT ,   DELAY,  //  2: Out of sleep mode, no args, w/delay
      255,                    //     255 = 500 ms delay
    ST77XX_COLMOD , 1+DELAY,  //  3: Set color mode, 1 arg + delay:
      0x05,                   //     16-bit color
      10,                     //     10 ms delay
    ST77XX_FRMCTR1, 3+DELAY,  //  4: Frame rate control, 3 args + delay:
      0x00,                   //     fastest refresh
      0x06,                   //     6 lines front porch
      0x03,                   //     3 lines back porch
      10,                     //     10 ms delay
    ST77XX_MADCTL , 1      ,  //  5: Memory access ctrl (directions), 1 arg:
      0x08,                   //     Row addr/col addr, bottom to top refresh
    ST77XX_DISSET5, 2      ,  //  6: Display settings #5, 2 args, no delay:
      0x15,                   //     1 clk cycle nonoverlap, 2 cycle gate
                              //     rise, 3 cycle osc equalize
      0x02,                   //     Fix on VTL
    ST77XX_INVCTR , 1      ,  //  7: Display inversion control, 1 arg:
      0x0,                    //     Line inversion
    ST77XX_PWCTR1 , 2+DELAY,  //  8: Power control, 2 args + delay:
      0x02,                   //     GVDD = 4.7V
      0x70,                   //     1.0uA
      10,                     //     10 ms delay
    ST77XX_PWCTR2 , 1      ,  //  9: Power control, 1 arg, no delay:
      0x05,                   //     VGH = 14.7V, VGL = -7.35V
    ST77XX_PWCTR3 , 2      ,  // 10: Power control, 2 args, no delay:
      0x01,                   //     Opamp current small
      0x02,                   //     Boost frequency
    ST77XX_VMCTR1 , 2+DELAY,  // 11: Power control, 2 args + delay:
      0x3C,                   //     VCOMH = 4V
      0x38,                   //     VCOML = -1.1V
      10,                     //     10 ms delay
    ST77XX_PWCTR6 , 2      ,  // 12: Power control, 2 args, no delay:
      0x11, 0x15,
    ST77XX_GMCTRP1,16      ,  // 13: Magical unicorn dust, 16 args, no delay:
      0x09, 0x16, 0x09, 0x20, //     (seriously though, not sure what
      0x21, 0x1B, 0x13, 0x19, //      these config values represent)
      0x17, 0x15, 0x1E, 0x2B,
      0x04, 0x05, 0x02, 0x0E,
    ST77XX_GMCTRN1,16+DELAY,  // 14: Sparkles and rainbows, 16 args + delay:
      0x0B, 0x14, 0x08, 0x1E, //     (ditto)
      0x22, 0x1D, 0x18, 0x1E,
      0x1B, 0x1A, 0x24, 0x2B,
      0x06, 0x06, 0x02, 0x0F,
      10,                     //     10 ms delay
    ST77XX_CASET  , 4      ,  // 15: Column addr set, 4 args, no delay:
      0x00, 0x02,             //     XSTART = 2
      0x00, 0x81,             //     XEND = 129
    ST77XX_RASET  , 4      ,  // 16: Row addr set, 4 args, no delay:
      0x00, 0x02,             //     XSTART = 1
      0x00, 0x81,             //     XEND = 160
    ST77XX_NORON  ,   DELAY,  // 17: Normal display on, no args, w/delay
      10,                     //     10 ms delay
    ST77XX_DISPON ,   DELAY,  // 18: Main screen turn on, no args, w/delay
      255 },                  //     255 = 500 ms delay

  Rcmd1[] = {                 // Init for 7735R, part 1 (red or green tab)
    15,                       // 15 commands in list:
    ST77XX_SWRESET,   DELAY,  //  1: Software reset, 0 args, w/delay
      150,                    //     150 ms delay
    ST77XX_SLPOUT ,   DELAY,  //  2: Out of sleep mode, 0 args, w/delay
      255,                    //     500 ms delay
    ST77XX_FRMCTR1, 3      ,  //  3: Frame rate ctrl - normal mode, 3 args:
      0x01, 0x2C, 0x2D,       //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
    ST77XX_FRMCTR2, 3      ,  //  4: Frame rate control - idle mode, 3 args:
      0x01, 0x2C, 0x2D,       //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
    ST77XX_FRMCTR3, 6      ,  //  5: Frame rate ctrl - partial mode, 6 args:
      0x01, 0x2C, 0x2D,       //     Dot inversion mode
      0x01, 0x2C, 0x2D,       //     Line inversion mode
    ST77XX_INVCTR , 1      ,  //  6: Display inversion ctrl, 1 arg, no delay:
      0x07,                   //     No inversion
    ST77XX_PWCTR1 , 3      ,  //  7: Power control, 3 args, no delay:
      0xA2,
      0x02,                   //     -4.6V
      0x84,                   //     AUTO mode
    ST77XX_PWCTR2 , 1      ,  //  8: Power control, 1 arg, no delay:
      0xC5,                   //     VGH25 = 2.4C VGSEL = -10 VGH = 3 * AVDD
    ST77XX_PWCTR3 , 2      ,  //  9: Power control, 2 args, no delay:
      0x0A,                   //     Opamp current small
      0x00,                   //     Boost frequency
    ST77XX_PWCTR4 , 2      ,  // 10: Power control, 2 args, no delay:
      0x8A,                   //     BCLK/2, Opamp current small & Medium low
      0x2A,  
    ST77XX_PWCTR5 , 2      ,  // 11: Power control, 2 args, no delay:
      0x8A, 0xEE,
    ST77XX_VMCTR1 , 1      ,  // 12: Power control, 1 arg, no delay:
      0x0E,
    ST77XX_INVOFF , 0      ,  // 13: Don't invert display, no args, no delay
    ST77XX_MADCTL , 1      ,  // 14: Memory access control (directions), 1 arg:
      0xC8,                   //     row addr/col addr, bottom to top refresh
    ST77XX_COLMOD , 1      ,  // 15: set color mode, 1 arg, no delay:
      0x05 },                 //     16-bit color

  Rcmd2green[] = {            // Init for 7735R, part 2 (green tab only)
    2,                        //  2 commands in list:
    ST77XX_CASET  , 4      ,  //  1: Column addr set, 4 args, no delay:
      0x00, 0x02,             //     XSTART = 0
      0x00, 0x7F+0x02,        //     XEND = 127
    ST77XX_RASET  , 4      ,  //  2: Row addr set, 4 args, no delay:
      0x00, 0x01,             //     XSTART = 0
      0x00, 0x9F+0x01 },      //     XEND = 159
  Rcmd2red[] = {              // Init for 7735R, part 2 (red tab only)
    2,                        //  2 commands in list:
    ST77XX_CASET  , 4      ,  //  1: Column addr set, 4 args, no delay:
      0x00, 0x00,             //     XSTART = 0
      0x00, 0x7F,             //     XEND = 127
    ST77XX_RASET  , 4      ,  //  2: Row addr set, 4 args, no delay:
      0x00, 0x00,             //     XSTART = 0
      0x00, 0x9F },           //     XEND = 159

  Rcmd2green144[] = {         // Init for 7735R, part 2 (green 1.44 tab)
    2,                        //  2 commands in list:
    ST77XX_CASET  , 4      ,  //  1: Column addr set, 4 args, no delay:
      0x00, 0x00,             //     XSTART = 0
      0x00, 0x7F,             //     XEND = 127
    ST77XX_RASET  , 4      ,  //  2: Row addr set, 4 args, no delay:
      0x00, 0x00,             //     XSTART = 0
      0x00, 0x7F },           //     XEND = 127

  Rcmd2green160x80[] = {            // 7735R init, part 2 (mini 160x80)
    2,                              //  2 commands in list:
    ST77XX_CASET,   4,              //  1: Column addr set, 4 args, no delay:
      0x00, 0x00,                   //     XSTART = 0
      0x00, 0x4F,                   //     XEND = 79
    ST77XX_RASET,   4,              //  2: Row addr set, 4 args, no delay:
      0x00, 0x00,                   //     XSTART = 0
      0x00, 0x9F},                 //     XEND = 159
  Rcmd2minist7735s[] = {
    3,                        //  2 commands in list:
    ST77XX_CASET  , 4      ,  //  1: Column addr set, 4 args, no delay:
      0x00, 0x00+26,             //     XSTART = 0
      0x00, 0x7F+26,             //     XEND = 127
    ST77XX_RASET  , 4      ,  //  2: Row addr set, 4 args, no delay:
      0x00, 0x00+1,             //     XSTART = 0
      0x00, 0x4F+1,           //     XEND = 79
    ST77XX_INVON,  0},      // these displays need colors inversed

  Rcmd3[] = {                 // Init for 7735R, part 3 (red or green tab)
    4,                        //  4 commands in list:
    ST77XX_GMCTRP1, 16      , //  1: Magical unicorn dust, 16 args, no delay:
      0x02, 0x1c, 0x07, 0x12,
      0x37, 0x32, 0x29, 0x2d,
      0x29, 0x25, 0x2B, 0x39,
      0x00, 0x01, 0x03, 0x10,
    ST77XX_GMCTRN1, 16      , //  2: Sparkles and rainbows, 16 args, no delay:
      0x03, 0x1d, 0x07, 0x06,
      0x2E, 0x2C, 0x29, 0x2D,
      0x2E, 0x2E, 0x37, 0x3F,
      0x00, 0x00, 0x02, 0x10,
    ST77XX_NORON  ,    DELAY, //  3: Normal display on, no args, w/delay
      10,                     //     10 ms delay
    ST77XX_DISPON ,    DELAY, //  4: Main screen turn on, no args w/delay
      100 
};                  //     100 ms delay

void ST7735_zephyr::initB(uint32_t spi_clock)                             // for ST7735B displays
{
  _SPI_CLOCK = spi_clock;
  common_init(Bcmd);
}

void ST7735_zephyr::initR(uint8_t options, uint32_t spi_clock) // for ST7735R
{
  _SPI_CLOCK = spi_clock;           // #define ST77XX_SPICLOCK 30000000
  
  common_init(Rcmd1);
  if (options == INITR_GREENTAB) {
    commandList(Rcmd2green);
    _colstart = 2;
    _rowstart = 1;
  } else if(options == INITR_144GREENTAB) {
    _screenHeight = ST7735_TFTHEIGHT_144;
    commandList(Rcmd2green144);
    _colstart = 2;
    _rowstart = 3;
  } else if(options == INITR_144GREENTAB_OFFSET) {
    _screenHeight = ST7735_TFTHEIGHT_144;
    commandList(Rcmd2green144);
    _colstart = 0;
    _rowstart = 32;
  } else if(options == INITR_MINI160x80) {
      _screenHeight   = ST7735_TFTHEIGHT_160;
      _screenWidth    = ST7735_TFTWIDTH_80;
      commandList(Rcmd2green160x80);
      _colstart = 24;
      _rowstart = 0;
  } else if (options == INITR_MINI160x80_ST7735S) {
      _screenHeight   = 160;
      _screenWidth    = 80;
      commandList(Rcmd2minist7735s);
      _colstart = 26;
      _rowstart = 1;
  } else {
    // _colstart, _rowstart left at default '0' values
    commandList(Rcmd2red);
  }
  commandList(Rcmd3);

  // if black or mini, change MADCTL color filter
  if ((options == INITR_BLACKTAB)  || (options == INITR_MINI160x80)){
    writecommand_cont(ST77XX_MADCTL);
    writedata8_last(0xC0);
  }

  tabcolor = options;
  setRotation(0); 

}

#define MADCTL_MY  0x80
#define MADCTL_MX  0x40
#define MADCTL_MV  0x20
#define MADCTL_ML  0x10
#define MADCTL_RGB 0x00
#define MADCTL_BGR 0x08
#define MADCTL_MH  0x04

void ST7735_zephyr::setRotation(uint8_t m)
{
  Serial.print("ST7735::setRotation ");
  Serial.print(m);
  Serial.print(" Tab: ");
  Serial.println(tabcolor);
  beginSPITransaction();
  writecommand_cont(ST77XX_MADCTL);
  _rotation = m % 4; // can't be higher than 3
  switch (_rotation) {
  case 0:
      if ((tabcolor == INITR_BLACKTAB) || (tabcolor == INITR_MINI160x80)) {
      writedata8_last(MADCTL_MX | MADCTL_MY | MADCTL_RGB);
    } else {
      writedata8_last(MADCTL_MX | MADCTL_MY | MADCTL_BGR);
    }
    _width  = _screenWidth;
    _height = _screenHeight;
      _xstart = _colstart;
      _ystart = _rowstart;
    break;
  case 1:
      if ((tabcolor == INITR_BLACKTAB) || (tabcolor == INITR_MINI160x80)) {
      writedata8_last(MADCTL_MY | MADCTL_MV | MADCTL_RGB);
    } else {
      writedata8_last(MADCTL_MY | MADCTL_MV | MADCTL_BGR);
    }
    _height = _screenWidth;
    _width  = _screenHeight;
      _ystart = _colstart;
      _xstart = _rowstart;
    break;
  case 2:
      if ((tabcolor == INITR_BLACKTAB) || (tabcolor == INITR_MINI160x80)) {
      writedata8_last(MADCTL_RGB);
    } else {
      writedata8_last(MADCTL_BGR);
    }
    _width  = _screenWidth;
    _height = _screenHeight;
      _xstart = _colstart;
      // hack to make work on a couple different displays
      _ystart = (_rowstart==0 || _rowstart==32)? 0 : 1;//_rowstart;
    break;
  case 3:
      if ((tabcolor == INITR_BLACKTAB) || (tabcolor == INITR_MINI160x80)) {
      writedata8_last(MADCTL_MX | MADCTL_MV | MADCTL_RGB);
    } else {
      writedata8_last(MADCTL_MX | MADCTL_MV | MADCTL_BGR);
    }
    _width = _screenHeight;
    _height = _screenWidth;
      _ystart = _colstart;
      // hack to make work on a couple different displays
      _xstart = (_rowstart==0 || _rowstart==32)? 0 : 1;//_rowstart;
    break;
  }
  endSPITransaction();

  //Serial.printf("SetRotation(%d) _xstart=%d _ystart=%d _width=%d, _height=%d\n", _rot, _xstart, _ystart, _width, _height);

  
  setClipRect();
  setOrigin();
  
  cursor_x = 0;
  cursor_y = 0;
}
