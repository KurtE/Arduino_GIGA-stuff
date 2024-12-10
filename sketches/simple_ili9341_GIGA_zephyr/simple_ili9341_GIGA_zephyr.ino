/***************************************************
  This is our GFX example for the Adafruit ILI9341 Breakout and Shield
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


#include "SPI.h"
#include "ILI9341_GIGA_zephyr.h"
// *************** Change to your Pin numbers ***************
#define TFT_DC 9
#define TFT_RST 8
#define TFT_CS 10

//ILI9341_GIGA_n tft(&SPI1, TFT_CS, TFT_DC, TFT_RST);
ILI9341_GIGA_n tft(TFT_CS, TFT_DC, TFT_RST);

void setup() {
  Serial.begin(9600);
  while (!Serial && millis() < 5000) ; // wait for Arduino Serial Monitor
  delay(500);
  Serial.println("ILI9341 Test!");
  tft.setSPI(SPI1);
  tft.begin(30000000);
  Serial.println("after TFT.begin");

}


void loop(void) {
  tft.fillScreen(ILI9341_RED);
  delay(1000);
  tft.fillScreen(ILI9341_GREEN);
  delay(1000);
  tft.fillScreen(ILI9341_BLUE);
  delay(1000);

}
