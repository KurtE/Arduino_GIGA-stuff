//#include <arducam_dvp.h>

#include "SPI.h"
#include <ILI9341_GIGA_zephyr.h>
#include <ILI9341_fonts.h>

#include "camera.h"
#include "ov767x.h"

// Using a OV7675 cam

OV7675 ov767x;

Camera cam(ov767x);

// This define colour mode CAMERA_RGB565 (otherwise use CAMERA_GRAYSCALE for greyscale)
#define IMAGE_MODE CAMERA_RGB565


// For the Adafruit shield, these are the default.
#define CS_SD 6
#define TFT_DC 9
#define TFT_RST 8
#define TFT_CS 10
ILI9341_GIGA_n tft(TFT_CS, TFT_DC, TFT_RST);
#define TFT_SPI SPI1
#define TFT_SPEED 20000000u

/*
  Other buffer instantiation options:
  FrameBuffer fb(0x30000000);
  FrameBuffer fb(320,240,2);
*/
FrameBuffer fb(320, 240, 2);  // this defines a buffer to handle colour images - width, height & bits per pixel (change last parameter to 1 for greyscale)

unsigned long lastUpdate = 0;

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
//Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
//Adafruit_ILI9341 tft = Adafruit_ILI9341(&SPI1, TFT_DC, TFT_CS, TFT_RST);


void blinkLED(uint32_t count = 0xFFFFFFFF) {
  pinMode(LED_BUILTIN, OUTPUT);
  while (count--) {
    digitalWrite(LED_BUILTIN, LOW);   // turn the LED on (HIGH is the voltage level)
    delay(50);                        // wait for a second
    digitalWrite(LED_BUILTIN, HIGH);  // turn the LED off by making the voltage LOW
    delay(50);                        // wait for a second
  }
}


void setup() {
  // put your setup code here, to run once:

  Serial.begin(230400);
  while (!Serial) continue;
  Serial.println("Camera display on TFT demo!");

  tft.begin();

  // Init the cam QVGA, using 15fps instead of 30fps to get better refresh rate
  if (!cam.begin(CAMERA_R320x240, IMAGE_MODE, 15)) {
    blinkLED();
  }

  blinkLED(5);

  tft.fillScreen(ILI9341_BLACK);
  yield();
  tft.fillScreen(ILI9341_RED);
  yield();
  tft.fillScreen(ILI9341_GREEN);
  yield();
  tft.fillScreen(ILI9341_BLUE);
  yield();
  tft.fillScreen(ILI9341_BLACK);
  yield();
}

void loop() {

  lastUpdate = millis();

  // Grab frame and write to serial
  if (cam.grabFrame(fb, 3000) == 0) {
    static FrameBuffer outfb(0x30000000);
    for (int i = 0; i < 320; i++) {
      for (int j = 0; j < 240; j++) {
        ((uint16_t*)outfb.getBuffer())[j + i * 240] = ((uint16_t*)fb.getBuffer())[i + j * 320];
        uint8_t lo_hi[] = { (uint8_t)((uint16_t*)outfb.getBuffer())[j + i * 240], (uint8_t)(((uint16_t*)outfb.getBuffer())[j + i * 240] >> 8) };
        ((uint16_t*)outfb.getBuffer())[j + i * 240] = (lo_hi[0] << 8) + lo_hi[1];
      }
    }
    //tft.drawRGBBitmap(0, 0, (uint16_t*)outfb.getBuffer(), 240, 320);
    tft.writeRect(0, 0, 240, 320, (uint16_t*)outfb.getBuffer());
    //delay(2000);
  } else {
    blinkLED(20);
  }
}