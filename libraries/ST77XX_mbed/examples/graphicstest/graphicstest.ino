/***************************************************
  This is a library for several Adafruit displays based on ST77* drivers.

  Works with the Adafruit 1.8" TFT Breakout w/SD card
    ----> http://www.adafruit.com/products/358
  The 1.8" TFT shield
    ----> https://www.adafruit.com/product/802
  The 1.44" TFT breakout
    ----> https://www.adafruit.com/product/2088
  The 1.54" TFT breakout
    ----> https://www.adafruit.com/product/3787
  The 2.0" TFT breakout
    ----> https://www.adafruit.com/product/4311
  as well as Adafruit raw 1.8" TFT display
    ----> http://www.adafruit.com/products/618

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/

// This Teensy3 and 4 native optimized and extended version
// requires specific pins. 
// If you use the short version of the constructor and the DC
// pin is hardware CS pin, then it will be slower.


//#define TFT_DC   4 
//#define TFT_CS   2  
//#define TFT_RST  3

#define TFT_DC   8 
#define TFT_CS   10  
#define TFT_RST  9


// Note the above pins are for the SPI object.  For those Teensy boards which have
// more than one SPI object, such as T3.5, T3.6, T4 which have at SPI1 and SPI2
// LC with SPI1, look at the cards that come with the teensy or the web page
// https://www.pjrc.com/teensy/pinout.html to select the appropriate IO pins.

#include <ST77XX_mbed.h>

// Option 1: use any pins but a little slower
// Note: code will detect if specified pins are the hardware SPI pins
//       and will use hardware SPI if appropriate
// For 1.44" and 1.8" TFT with ST7735 use
//ST7735_zephyr tft = ST7735_zephyr(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCK, TFT_RST);

// For 1.54" or other TFT with ST7789, This has worked with some ST7789
// displays without CS pins, for those you can pass in -1 or 0xff for CS
// More notes by the tft.init call
//ST7789_zephyr tft = ST7789_zephyr(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCK, TFT_RST);

// Option 2: must use the hardware SPI pins
// (for UNO thats sclk = 13 and sid = 11) and pin 10 must be
// an output. This is much faster - also required if you want
// to use the microSD card (see the image drawing example)
// For 1.44" and 1.8" TFT with ST7735 use
//ST7735_zephyr tft = ST7735_zephyr(&SPI, TFT_CS, TFT_DC, TFT_RST);

// For 1.54" TFT with ST7789
//ST7789_zephyr tft = ST7789_zephyr(&SPI, TFT_CS, TFT_DC, TFT_RST);

// For 3.5" or 4.0" TFT with ST7796
ST7796_zephyr tft = ST7796_zephyr(&SPI1, TFT_CS, TFT_DC, TFT_RST);

// forward references:
// forward references:
extern void tftPrintTest();
extern void testdrawtext(const char *text, uint16_t color);
extern void testlines(uint16_t color);
extern void testfastlines(uint16_t color1, uint16_t color2);
extern void testdrawrects(uint16_t color);
extern void testfillrects(uint16_t color1, uint16_t color2);
extern void testfillcircles(uint8_t radius, uint16_t color);
extern void testdrawcircles(uint8_t radius, uint16_t color);
extern void testroundrects();
extern void testtriangles();
extern void mediabuttons();
extern unsigned long testText();




float p = 3.1415926;


void setup(void) {
  //pinMode(SD_CS, INPUT_PULLUP);  // don't touch the SD card
  printk("Setup called\n");
  Serial.begin(9600);
  Serial.print("hello!");

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
  printk("tft.init called\n");
  // OR use this initializer (uncomment) if using a 240x240 clone 
  // that does not have a CS pin2.0" 320x240 TFT:
  //tft.init(240, 240, SPI_MODE2);           // Init ST7789 240x240 no CS

  Serial.println("init");

  tft.fillScreen(ST77XX_RED);
  delay(500);
  tft.fillScreen(ST77XX_GREEN);
  delay(500);
  tft.fillScreen(ST77XX_BLUE);
  delay(500);

  uint16_t time = millis();
  tft.fillScreen(ST77XX_BLACK);
  time = millis() - time;

  Serial.println(time, DEC);
  delay(500);

  // large block of text
  tft.fillScreen(ST77XX_BLACK);
  testdrawtext("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Curabitur adipiscing ante sed nibh tincidunt feugiat. Maecenas enim massa, fringilla sed malesuada et, malesuada sit amet turpis. Sed porttitor neque ut ante pretium vitae malesuada nunc bibendum. Nullam aliquet ultrices massa eu hendrerit. Ut sed nisi lorem. In vestibulum purus a tortor imperdiet posuere. ", ST77XX_WHITE);
  delay(1000);

  // tft print function!
  tftPrintTest();
  delay(4000);

  // a single pixel
  tft.drawPixel(tft.width()/2, tft.height()/2, ST77XX_GREEN);
  delay(500);

  // line draw test
  testlines(ST77XX_YELLOW);
  delay(1000);

  // optimized lines
  testfastlines(ST77XX_RED, ST77XX_BLUE);
  delay(1000);

  testdrawrects(ST77XX_GREEN);
  delay(1000);

  testfillrects(ST77XX_YELLOW, ST77XX_MAGENTA);
  delay(1000);

  tft.fillScreen(ST77XX_BLACK);
  testfillcircles(10, ST77XX_BLUE);
  testdrawcircles(10, ST77XX_WHITE);
  delay(500);

  testroundrects();
  delay(1000);

  testtriangles();
  delay(1000);

  mediabuttons();
  delay(500);

  Serial.println("done");
  delay(1000);
  tft.invertDisplay(true);
  delay(500);
  tft.invertDisplay(false);
  delay(500);

}

void loop() {
#if 1
  for (uint8_t rotation = 0; rotation < 4; rotation++) {
    tft.setRotation(rotation);
    testText();
    delay(1000);
  }
#else
  tft.invertDisplay(true);
  delay(500);
  tft.invertDisplay(false);
  delay(500);
#endif  
}



void testlines(uint16_t color) {
  tft.fillScreen(ST77XX_BLACK);
  for (int16_t x=0; x < tft.width(); x+=6) {
    tft.drawLine(0, 0, x, tft.height()-1, color);
    delay(200);
  }
  for (int16_t y=0; y < tft.height(); y+=6) {
    tft.drawLine(0, 0, tft.width()-1, y, color);
  }

  tft.fillScreen(ST77XX_BLACK);
  for (int16_t x=0; x < tft.width(); x+=6) {
    tft.drawLine(tft.width()-1, 0, x, tft.height()-1, color);
  }
  for (int16_t y=0; y < tft.height(); y+=6) {
    tft.drawLine(tft.width()-1, 0, 0, y, color);
  }

  tft.fillScreen(ST77XX_BLACK);
  for (int16_t x=0; x < tft.width(); x+=6) {
    tft.drawLine(0, tft.height()-1, x, 0, color);
  }
  for (int16_t y=0; y < tft.height(); y+=6) {
    tft.drawLine(0, tft.height()-1, tft.width()-1, y, color);
  }

  tft.fillScreen(ST77XX_BLACK);
  for (int16_t x=0; x < tft.width(); x+=6) {
    tft.drawLine(tft.width()-1, tft.height()-1, x, 0, color);
  }
  for (int16_t y=0; y < tft.height(); y+=6) {
    tft.drawLine(tft.width()-1, tft.height()-1, 0, y, color);
  }
}

void testdrawtext(const char *text, uint16_t color) {
  tft.setCursor(0, 0);
  tft.setTextColor(color);
  tft.setTextWrap(true);
  tft.print(text);
}

void testfastlines(uint16_t color1, uint16_t color2) {
  tft.fillScreen(ST77XX_BLACK);
  for (int16_t y=0; y < tft.height(); y+=5) {
    tft.drawFastHLine(0, y, tft.width(), color1);
  }
  for (int16_t x=0; x < tft.width(); x+=5) {
    tft.drawFastVLine(x, 0, tft.height(), color2);
  }
}

void testdrawrects(uint16_t color) {
  tft.fillScreen(ST77XX_BLACK);
  for (int16_t x=0; x < tft.width(); x+=6) {
    tft.drawRect(tft.width()/2 -x/2, tft.height()/2 -x/2 , x, x, color);
  }
}

void testfillrects(uint16_t color1, uint16_t color2) {
  tft.fillScreen(ST77XX_BLACK);
  for (int16_t x=tft.width()-1; x > 6; x-=6) {
    tft.fillRect(tft.width()/2 -x/2, tft.height()/2 -x/2 , x, x, color1);
    tft.drawRect(tft.width()/2 -x/2, tft.height()/2 -x/2 , x, x, color2);
  }
}

void testfillcircles(uint8_t radius, uint16_t color) {
  for (int16_t x=radius; x < tft.width(); x+=radius*2) {
    for (int16_t y=radius; y < tft.height(); y+=radius*2) {
      tft.fillCircle(x, y, radius, color);
    }
  }
}

void testdrawcircles(uint8_t radius, uint16_t color) {
  for (int16_t x=0; x < tft.width()+radius; x+=radius*2) {
    for (int16_t y=0; y < tft.height()+radius; y+=radius*2) {
      tft.drawCircle(x, y, radius, color);
    }
  }
}

void testtriangles() {
  tft.fillScreen(ST77XX_BLACK);
  int color = 0xF800;
  int t;
  int w = tft.width()/2;
  int x = tft.height()-1;
  int y = 0;
  int z = tft.width();
  for(t = 0 ; t <= 15; t+=1) {
    tft.drawTriangle(w, y, y, x, z, x, color);
    x-=4;
    y+=4;
    z-=4;
    color+=100;
  }
}

void testroundrects() {
  tft.fillScreen(ST77XX_BLACK);
  int color = 100;
  int i;
  int t;
  for(t = 0 ; t <= 4; t+=1) {
    int x = 0;
    int y = 0;
    int w = tft.width()-2;
    int h = tft.height()-2;
    for(i = 0 ; i <= 16; i+=1) {
      tft.drawRoundRect(x, y, w, h, 5, color);
      x+=2;
      y+=3;
      w-=4;
      h-=6;
      color+=1100;
    }
    color+=100;
  }
}

void tftPrintTest() {
  tft.setTextWrap(false);
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 30);
  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(1);
  tft.println("Hello World!");
  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextSize(2);
  tft.println("Hello World!");
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(3);
  tft.println("Hello World!");
  tft.setTextColor(ST77XX_BLUE);
  tft.setTextSize(4);
  tft.print(1234.567);
  delay(1500);
  tft.setCursor(0, 0);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(0);
  tft.println("Hello World!");
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_GREEN);
  tft.print(p, 6);
  tft.println(" Want pi?");
  tft.println(" ");
  tft.print(8675309, HEX); // print 8,675,309 out in HEX!
  tft.println(" Print HEX!");
  tft.println(" ");
  tft.setTextColor(ST77XX_WHITE);
  tft.println("Sketch has been");
  tft.println("running for: ");
  tft.setTextColor(ST77XX_MAGENTA);
  tft.print(millis() / 1000);
  tft.setTextColor(ST77XX_WHITE);
  tft.print(" seconds.");
}

void mediabuttons() {
  // play
  tft.fillScreen(ST77XX_BLACK);
  tft.fillRoundRect(25, 10, 78, 60, 8, ST77XX_WHITE);
  tft.fillTriangle(42, 20, 42, 60, 90, 40, ST77XX_RED);
  delay(500);
  // pause
  tft.fillRoundRect(25, 90, 78, 60, 8, ST77XX_WHITE);
  tft.fillRoundRect(39, 98, 20, 45, 5, ST77XX_GREEN);
  tft.fillRoundRect(69, 98, 20, 45, 5, ST77XX_GREEN);
  delay(500);
  // play color
  tft.fillTriangle(42, 20, 42, 60, 90, 40, ST77XX_BLUE);
  delay(50);
  // pause color
  tft.fillRoundRect(39, 98, 20, 45, 5, ST77XX_RED);
  tft.fillRoundRect(69, 98, 20, 45, 5, ST77XX_RED);
  // play color
  tft.fillTriangle(42, 20, 42, 60, 90, 40, ST77XX_GREEN);
}
unsigned long testText() {
  tft.fillScreen(ST77XX_BLACK);
  unsigned long start = micros();
  tft.setCursor(0, 0);
  tft.setTextColor(ST77XX_WHITE);  tft.setTextSize(1);
  tft.println("Hello World!");
  tft.setTextColor(ST77XX_YELLOW); tft.setTextSize(2);
  tft.println(1234.56);
  tft.setTextColor(ST77XX_RED);    tft.setTextSize(3);
  tft.println(0xDEADBEEF, HEX);
  tft.println();
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(5);
  tft.println("Groop");
  tft.setTextSize(2);
  tft.println("I implore thee,");
  tft.setTextSize(1);
  tft.println("my foonting turlingdromes.");
  tft.println("And hooptiously drangle me");
  tft.println("with crinkly bindlewurdles,");
  tft.println("Or I will rend thee");
  tft.println("in the gobberwarts");
  tft.println("with my blurglecruncheon,");
  tft.println("see if I don't!");
  return micros() - start;
}

