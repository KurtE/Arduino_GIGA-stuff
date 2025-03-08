#include <ILI9341_GIGA_zephyr.h>
#include "camera.h"

Camera cam;


#ifdef ARDUINO_PORTENTA_H7
#ifdef ZEPHYR_PINNAMES_H
#define TFT_DC 5
#define TFT_RST 4
#define TFT_CS 3
#else
#define TFT_DC PC_6
#define TFT_RST PC_7
#define TFT_CS PG_7
#endif
ILI9341_GIGA_n tft(&SPI, TFT_CS, TFT_DC, TFT_RST);
#else
#define TFT_DC 9
#define TFT_RST 8
#define TFT_CS 10
ILI9341_GIGA_n tft(&SPI1, TFT_CS, TFT_DC, TFT_RST);
#endif

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
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
  }

  Serial.begin(115200);
  while (!Serial && millis() < 5000) {}

  // put your setup code here, to run once:
  Serial.println("\n*** start display camera image on ILI9341 ***");
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);
  delay(500);
  tft.fillScreen(ILI9341_RED);
  delay(500);
  tft.fillScreen(ILI9341_GREEN);
  delay(500);
  tft.fillScreen(ILI9341_BLUE);
  delay(500);
  tft.fillScreen(ILI9341_BLACK);
  delay(500);

  if (!cam.begin(320, 240, CAMERA_RGB565)) {
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
    //tft.writeRect(0, 0, 320, 240, (const uint16_t*)fb.getBuffer());
    //tft.writeSubImageRectBytesReversed(0, 0, 320, 240, 0, 0, 320, 240, (const uint16_t*)fb.getBuffer());
    tft.writeSubImageRect(0, 0, 320, 240, 0, 0, 320, 240, (const uint16_t*)fb.getBuffer());
    cam.releaseFrame(fb);
  }

}
