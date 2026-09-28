#define TFT_DC   4 
#define TFT_CS   2  
#define TFT_RST  3
#include "ST77XX_zephyr.h" // Hardware-specific library

ST7796_zephyr tft = ST7796_zephyr(nullptr, TFT_CS, TFT_DC, TFT_RST);
//ST7789_zephyr tft = ST7789_zephyr(nullptr, TFT_CS, TFT_DC, TFT_RST);
//ST77XX_zephyr tft = ST77XX_zephyr(&SPI, TFT_CS, TFT_DC, TFT_RST);


float p = 3.1415926;


void setup() {
  printk("Setup called\n");
  Serial.begin(115200);
  while (!Serial && millis() < 5000) {}
  Serial.println("Setup called");
  delay(1000);

  //tft.init(320, 480);
  
  //tft.initR(INITR_BLACKTAB);
  tft.begin(320, 480, SPI_MODE0, 2500000);
  //tft.begin(240, 320, SPI_MODE0, 30000000);
  Serial.println("Init called");
}
void loop() {
  tft.fillScreen(ST77XX_RED);
  delay(500);
  tft.fillScreen(ST77XX_GREEN);
  delay(500);
  tft.fillScreen(ST77XX_BLUE);
  
  delay(500);
  if (Serial.available()) {
    while (Serial.read() != -1) {}
    Serial.println("** Paused **");
    while (Serial.read() == -1) {}
    while (Serial.read() != -1) {}
  }
}