#include <ST77XX_zephyr.h>
#include "camera.h"

Camera cam;
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

void fatal_error(const char *msg) {
  Serial.println(msg);
  pinMode(LED_BUILTIN, OUTPUT);
  while (1) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
  }
}



void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  while (!Serial && millis() < 5000) {}

  Serial.println("\n*** start display camera image on ILI9341 ***");
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

  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);
  delay(500);
  tft.fillScreen(ST77XX_RED);
  delay(500);
  tft.fillScreen(ST77XX_GREEN);
  delay(500);
  tft.fillScreen(ST77XX_BLUE);
  delay(500);
  tft.fillScreen(ST77XX_BLACK);
  delay(500);

  if (!cam.begin(320, 240, CAMERA_RGB565, true)) {
    fatal_error("Camera begin failed");
  }
  cam.setVerticalFlip(false);
  cam.setHorizontalMirror(false);
}

void loop() {
  // put your main code here, to run repeatedly:
  FrameBuffer fb;
  if (cam.grabFrame(fb)) {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    tft.writeRect(0, 0, 320, 240, (const uint16_t*)fb.getBuffer());
    //tft.writeSubImageRectBytesReversed(0, 0, 320, 240, 0, 0, 320, 240, (const uint16_t*)fb.getBuffer());
    //tft.writeSubImageRect(0, 0, 320, 240, 0, 0, 320, 240, (const uint16_t*)fb.getBuffer());
    cam.releaseFrame(fb);
  }
}
