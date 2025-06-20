/*
Tests string alignment

Normally strings are printed relative to the top left corner but this can be
changed with the setTextDatum() function. The library has #defines for:

TL_DATUM = Top left
TC_DATUM = Top centre
TR_DATUM = Top right
ML_DATUM = Middle left
MC_DATUM = Middle centre
MR_DATUM = Middle right
BL_DATUM = Bottom left
BC_DATUM = Bottom centre
BR_DATUM = Bottom right
*/
#include <SPI.h>
#include <ST77XX_zephyr.h>
#include <ST77XX_zephyr_font_Arial.h>


#define TFT_CS   10  // CS & DC can use pins 2, 6, 9, 10, 15, 20, 21, 22, 23
#define TFT_DC    8  //  but certain pairs must NOT be used: 2+10, 6+9, 20+23, 21+22
#define TFT_RST   9  // RST can use any pin

// Use one or the other
//ST7735_zephyr tft = ST7735_zephyr(&SPI, TFT_CS, TFT_DC, TFT_RST);

// For 1.54" TFT with ST7789
//ST7789_zephyr tft = ST7789_zephyr(&SPI, TFT_CS, TFT_DC, TFT_RST);

// For 3.5" or 4.0" TFT with ST7796
ST7796_zephyr tft = ST7796_zephyr(&SPI1, TFT_CS, TFT_DC, TFT_RST);
unsigned long drawTime = 0;

void setup(void) {
  Serial.begin(115200);
  SPI.begin();
  // Use this initializer if you're using a 1.8" TFT 128x160 displays
  //tft.initR(INITR_BLACKTAB);

  // Or use this initializer (uncomment) if you're using a 1.44" TFT (128x128)
  //tft.initR(INITR_144GREENTAB);

  // Or use this initializer (uncomment) if you're using a .96" TFT(160x80)
  //tft.initR(INITR_MINI160x80);

  // Or use this initializer (uncomment) for Some 1.44" displays use different memory offsets
  // Try it if yours is not working properly
  // May need to tweek the offsets
  //tft.setRowColStart(32,0);

  // Or use this initializer (uncomment) if you're using a 1.54" 240x240 TFT
  //tft.init(240, 240);   // initialize a ST7789 chip, 240x240 pixels

  // OR use this initializer (uncomment) if using a 2.0" 320x240 TFT:
  //tft.init(240, 320);           // Init ST7789 320x240
  tft.init(320, 480);

  tft.setRotation(3);
  tft.setFont(Arial_18);
  //tft.setTextSize(4);
}

void loop() {

  tft.fillScreen(ST77XX_BLACK);
  
  for(byte datum = 0; datum < 9; datum++) {
    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    
    tft.setTextDatum(datum);
    
    tft.drawNumber(88,160,60);
    tft.fillCircle(160,120,5,ST77XX_RED);
    
    tft.setTextDatum(MC_DATUM);
    
    tft.setTextColor(ST77XX_YELLOW);
    tft.drawString("TEENSY 4",160,120);
    delay(1000);
    tft.fillScreen(ST77XX_BLACK);
  }

  tft.setTextDatum(MC_DATUM);
  
  tft.setTextColor(ST77XX_BLACK);
  tft.drawString("X",160,120);
  delay(1000);
  tft.fillScreen(ST77XX_BLACK);
  
  tft.setTextDatum(MC_DATUM);
  
  tft.setTextColor(ST77XX_BLACK);
  tft.drawString("X",160,120);
  delay(1000);
  tft.fillScreen(ST77XX_BLACK);

  tft.setTextColor(ST77XX_WHITE, ST77XX_BLUE);

  tft.setTextDatum(MC_DATUM);

  //Test floating point drawing function
  float test = 67.125;
  tft.drawFloat(test, 4, 160, 180);
  delay(1000);
  tft.fillScreen(ST77XX_BLACK);
  test = -0.555555;
  tft.drawFloat(test, 3, 160, 180);
  delay(1000);
  tft.fillScreen(ST77XX_BLACK);
  test = 0.1;
  tft.drawFloat(test, 4, 160, 180);
  delay(1000);
  tft.fillScreen(ST77XX_BLACK);
  test = 9999999;
  tft.drawFloat(test, 1, 160, 180);
  delay(1000);
  
  tft.fillCircle(160,180,5,ST77XX_YELLOW);
  
  tft.setTextDatum(MC_DATUM);
  
  tft.setTextColor(ST77XX_BLACK);
  tft.drawString("X",160,180);

  delay(4000);
}
