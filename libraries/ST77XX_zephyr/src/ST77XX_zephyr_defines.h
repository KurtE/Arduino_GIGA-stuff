#ifndef _ST77XX_ZEPHYR_DEFINES_H_
#define _ST77XX_ZEPHYR_DEFINES_H_

#define ST77XX_TFTWIDTH 320
#define ST77XX_TFTHEIGHT 480

#define ST7735_TFTWIDTH  128
#define ST7735_TFTWIDTH_80     80 // for mini
// for 1.44" display
#define ST7735_TFTHEIGHT_144 128
// for 1.8" display and mini
#define ST7735_TFTHEIGHT_160  160 // for 1.8" and mini display

#define ST77XX_NOP     0x00
#define ST77XX_SWRESET 0x01
#define ST77XX_RDDID   0x04
#define ST77XX_RDDST   0x09

#define ST77XX_SLPIN   0x10
#define ST77XX_SLPOUT  0x11
#define ST77XX_PTLON   0x12
#define ST77XX_NORON   0x13

#define ST77XX_INVOFF  0x20
#define ST77XX_INVON   0x21
#define ST77XX_DISPOFF 0x28
#define ST77XX_DISPON  0x29
#define ST77XX_CASET   0x2A
#define ST77XX_RASET   0x2B
#define ST77XX_RAMWR   0x2C
#define ST77XX_RAMRD   0x2E

#define ST77XX_PTLAR   0x30
#define ST77XX_COLMOD  0x3A
#define ST77XX_MADCTL  0x36

#define ST77XX_FRMCTR1 0xB1
#define ST77XX_FRMCTR2 0xB2
#define ST77XX_FRMCTR3 0xB3
#define ST77XX_INVCTR  0xB4
#define ST77XX_DISSET5 0xB6

#define ST77XX_PWCTR1  0xC0
#define ST77XX_PWCTR2  0xC1
#define ST77XX_PWCTR3  0xC2
#define ST77XX_PWCTR4  0xC3
#define ST77XX_PWCTR5  0xC4
#define ST77XX_VMCTR1  0xC5

#define ST77XX_RDID1   0xDA
#define ST77XX_RDID2   0xDB
#define ST77XX_RDID3   0xDC
#define ST77XX_RDID4   0xDD

#define ST77XX_PWCTR6  0xFC

#define ST77XX_GMCTRP1 0xE0
#define ST77XX_GMCTRN1 0xE1

#define ST77XX_MADCTL_MY  0x80
#define ST77XX_MADCTL_MX  0x40
#define ST77XX_MADCTL_MV  0x20
#define ST77XX_MADCTL_ML  0x10
#define ST77XX_MADCTL_RGB 0x00
#define ST77XX_MADCTL_BGR 0x08
#define ST7796S_NOP            0x00
#define ST7796S_SWRESET        0x01

#define ST7796S_RDDID          0x04
#define ST7796S_RDDST          0x09
#define ST7796S_RDMODE         0x0A
#define ST7796S_RDMADCTL       0x0B
#define ST7796S_RDPIXFMT       0x0C
#define ST7796S_RDIMGFMT       0x0D
#define ST7796S_RDSELFDIAG     0x0F

#define ST7796S_SLPIN          0x10
#define ST7796S_SLPOUT         0x11
#define ST7796S_PTLON          0x12
#define ST7796S_NORON          0x13

#define ST7796S_INVOFF         0x20
#define ST7796S_INVON          0x21
//#define ST7796S_GAMMASET       0x26
#define ST7796S_DISPOFF        0x28
#define ST7796S_DISPON         0x29

#define ST7796S_CASET          0x2A
#define ST7796S_PASET          0x2B
#define ST7796S_RAMWR          0x2C
#define ST7796S_RAMRD          0x2E

#define ST7796S_PTLAR          0x30
#define ST7796S_VSCRDEF        0x33
#define ST7796S_MADCTL         0x36
#define ST7796S_VSCRSADD       0x37     /* Vertical Scrolling Start Address */
#define ST7796S_PIXFMT         0x3A     /* COLMOD: Pixel Format Set */

#define ST7796S_RGB_INTERFACE  0xB0     /* RGB Interface Signal Control */
#define ST7796S_FRMCTR1        0xB1
#define ST7796S_FRMCTR2        0xB2
#define ST7796S_FRMCTR3        0xB3
#define ST7796S_INVCTR         0xB4
#define ST7796S_DFUNCTR        0xB6     /* Display Function Control */

#define ST7796S_PWCTR1         0xC0
#define ST7796S_PWCTR2         0xC1
#define ST7796S_PWCTR3         0xC2
#define ST7796S_PWCTR4         0xC3
#define ST7796S_PWCTR5         0xC4
#define ST7796S_VMCTR1         0xC5

#define ST7796S_RDID1          0xDA
#define ST7796S_RDID2          0xDB
#define ST7796S_RDID3          0xDC
#define ST7796S_RDID4          0xDD

#define ST7796S_GMCTRP1        0xE0
#define ST7796S_GMCTRN1        0xE1
#define ST7796S_DGCTR1         0xE2
#define ST7796S_DGCTR2         0xE3

#define ST7796S_CSCON           0xFF

//-----------------------------------------------------------------------------
#define ST7796S_MAD_RGB        0x08
#define ST7796S_MAD_BGR        0x00

#define ST7796S_MAD_VERTICAL   0x20
#define ST7796S_MAD_X_LEFT     0x00
#define ST7796S_MAD_X_RIGHT    0x40
#define ST7796S_MAD_Y_UP       0x80
#define ST7796S_MAD_Y_DOWN     0x00


// Color definitions
#define ST77XX_BLACK 0x0000       /*   0,   0,   0 */
#define ST77XX_NAVY 0x000F        /*   0,   0, 128 */
#define ST77XX_DARKGREEN 0x03E0   /*   0, 128,   0 */
#define ST77XX_DARKCYAN 0x03EF    /*   0, 128, 128 */
#define ST77XX_MAROON 0x7800      /* 128,   0,   0 */
#define ST77XX_PURPLE 0x780F      /* 128,   0, 128 */
#define ST77XX_OLIVE 0x7BE0       /* 128, 128,   0 */
#define ST77XX_LIGHTGREY 0xC618   /* 192, 192, 192 */
#define ST77XX_DARKGREY 0x7BEF    /* 128, 128, 128 */
#define ST77XX_BLUE 0x001F        /*   0,   0, 255 */
#define ST77XX_GREEN 0x07E0       /*   0, 255,   0 */
#define ST77XX_CYAN 0x07FF        /*   0, 255, 255 */
#define ST77XX_RED 0xF800         /* 255,   0,   0 */
#define ST77XX_MAGENTA 0xF81F     /* 255,   0, 255 */
#define ST77XX_YELLOW 0xFFE0      /* 255, 255,   0 */
#define ST77XX_WHITE 0xFFFF       /* 255, 255, 255 */
#define ST77XX_ORANGE 0xFD20      /* 255, 165,   0 */
#define ST77XX_GREENYELLOW 0xAFE5 /* 173, 255,  47 */
#define ST77XX_PINK 0xF81F



// These enumerate the text plotting alignment (reference datum point)
#define TL_DATUM 0 // Top left (default)
#define TC_DATUM 1 // Top centre
#define TR_DATUM 2 // Top right
#define ML_DATUM 3 // Middle left
#define CL_DATUM 3 // Centre left, same as above
#define MC_DATUM 4 // Middle centre
#define CC_DATUM 4 // Centre centre, same as above
#define MR_DATUM 5 // Middle right
#define CR_DATUM 5 // Centre right, same as above
#define BL_DATUM 6 // Bottom left
#define BC_DATUM 7 // Bottom centre
#define BR_DATUM 8 // Bottom right
//#define L_BASELINE  9 // Left character baseline (Line the 'A' character would
//sit on)
//#define C_BASELINE 10 // Centre character baseline
//#define R_BASELINE 11 // Right character baseline

#define CMD_LIST_DELAY 0x80


#endif // _ST77XX_ZEPHYR_DEFINES_H_