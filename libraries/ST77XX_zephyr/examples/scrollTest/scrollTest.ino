/***************************************************
  This is our GFX example for the Adafruit ST7735 Breakout and Shield
  ----> http://www.adafruit.com/products/1651

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/

#include <ST77XX_zephyr.h>
//#include "ST77XX_zephyr_font_ComicSansMS.h"



#define TFT_DC   4 
#define TFT_CS   2  
#define TFT_RST  3


ST7796_zephyr tft = ST7796_zephyr(&SPI, TFT_CS, TFT_DC, TFT_RST);

// If using the breakout, change pins as desired
//Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

void setup() {

  Serial.begin(9600);
 
  //tft.init(240, 320);           // Init ST7789 320x240
  tft.init(320, 480);

  tft.setRotation(3);
  tft.useFrameBuffer(true);
  tft.fillScreen(ST77XX_BLACK);
  while (!Serial) ; 
  tft.setTextColor(ST77XX_WHITE);  tft.setTextSize(1);
  tft.enableScroll();
  tft.setScrollTextArea(0,0,120,240);
  tft.setScrollBackgroundColor(ST77XX_GREEN);

  tft.setCursor(180, 100);

 // tft.setFont(ComicSansMS_12);
  tft.print("Fixed text");

  tft.setCursor(0, 0);

  tft.setTextColor(ST77XX_BLACK); 

  for(int i=0;i<20;i++){
    tft.print("  this is line ");
    tft.println(i);
    tft.updateScreen();
    delay(100);
  }

  tft.fillScreen(ST77XX_BLACK);
  tft.setScrollTextArea(40,50,120,120);
  tft.setScrollBackgroundColor(ST77XX_GREEN);
  //tft.setFont(ComicSansMS_10);

  tft.setTextSize(2);
  tft.setCursor(40, 50);

  for(int i=0;i<20;i++){
    tft.print("  this is line ");
    tft.println(i);
    tft.updateScreen();
    delay(500);
  }


}



void loop(void) {


}