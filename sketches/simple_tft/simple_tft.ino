#include "SPI.h"
#include "ILI9341_simple_giga.h"

// For the Adafruit shield, these are the default.
#define TFT_DC 9
#define TFT_RST 8
#define TFT_CS 10


//#define USE_NEW
#ifdef USE_NEW
ILI9341_GIGA_n *ptft = nullptr;
#else
ILI9341_GIGA_n tft(TFT_CS, TFT_DC, TFT_RST);
#endif

void step() {
  Serial.println("Waiting...");
  while (Serial.available() == 0) { k_yield();}
  while (Serial.available() != 0) {Serial.read();}
}

void setup() {
  Serial.begin(115200);
  delay(250);
  Serial.println("After Serial begin");
  //SPI1.begin();
  Serial.println("After SPI1 begin");
  SPI1.beginTransaction(SPISettings(3000000, MSBFIRST, SPI_MODE0));
  Serial.println("After begin Transaction");
//  step();
  Serial.println("Before tft begin"); Serial.flush(); 
  #ifdef USE_NEW
  ptft = new ILI9341_GIGA_n (TFT_CS, TFT_DC, TFT_RST);
  //ptft->begin(3000000);
  #else
  tft.setSPI(SPI1);
  tft.begin(30000000);
  #endif

  Serial.println("After ptft->begin");

}


void loop() {
#ifdef USE_NEW
  ptft->fillScreen(ILI9341_RED);
  delay(500);
  ptft->fillScreen(ILI9341_GREEN);
  delay(500);
  ptft->fillScreen(ILI9341_BLUE);
  delay(500);
  ptft->fillScreen(ILI9341_BLACK);
  delay(500);
#else
  uint32_t start_time = millis();
  tft.fillScreen(ILI9341_RED);
  //delay(500);
  tft.fillScreen(ILI9341_GREEN);
  //delay(500);
  tft.fillScreen(ILI9341_BLUE);
  //delay(500);
  tft.fillScreen(ILI9341_BLACK);
  Serial.println(millis()-start_time, DEC);
  delay(500);
#endif
}
