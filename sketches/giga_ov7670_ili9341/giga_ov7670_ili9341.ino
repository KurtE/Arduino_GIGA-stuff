REDIRECT_STDOUT_TO(Serial)

#include "camera.h"
#include "MemoryHexDump.h"
#include <GIGA_digitalWriteFast.h>
#include <ILI9341_GIGA_n.h>

#if defined(ARDUINO_GIGA)
#include "ov767x.h"
// uncomment the correct camera in use
OV7670 ov767x;
// OV7675 ov767x;
Camera cam(ov767x);
#define IMAGE_MODE CAMERA_RGB565
#else
#error "This board is unsupported."
#endif

//-----------------------------------------------------------------------------
// ILI9341 displays
//-----------------------------------------------------------------------------

// This is calibration data for the raw touch data to the screen coordinates
// Warning, These may need to be tweeked.
#define TS_MINX 337
#define TS_MINY 529
#define TS_MAXX 3729
#define TS_MAXY 3711
//#define USE_SPI1
#ifdef USE_SPI1
#define TFT_DC 9
#define TFT_RST 8
#define TFT_CS 7
ILI9341_GIGA_n tft(&SPI1, TFT_CS, TFT_DC, TFT_RST);
#else
#define TFT_DC 24
#define TFT_RST 26
#define TFT_CS 22
ILI9341_GIGA_n tft(&SPI, TFT_CS, TFT_DC, TFT_RST, DMA1_Stream3);
#endif

#define SUPPORTS_XPT2046_TOUCH
#define TFT_USE_FRAME_BUFFER

#ifndef BLUE
#define BLUE 0x001F
#define BLACK 0x0000
#define WHITE 0xFFFF
#define GREEN 0x07E0
#define RED 0xf800
#endif



/*
Other buffer instantiation options:
  FrameBuffer fb(0x30000000);
  FrameBuffer fb(320,240,2);

If resolution higher than 320x240 is required, please use external RAM via
  #include "SDRAM.h"
  FrameBuffer fb(SDRAM_START_ADDRESS);
  ...
  // and adding in setup()
  SDRAM.begin();
*/
FrameBuffer fb;

unsigned long lastUpdate = 0;


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
  // Init the cam QVGA, 30FPS
  while (!Serial && millis() < 5000) {}
  Serial.begin(115200);
  pinMode(TFT_CS, OUTPUT);
  digitalWrite(TFT_CS, HIGH);
  Serial.println("*** start up ILI9341 ***");
  tft.begin(10000000u);
  tft.setRotation(1);

  tft.fillScreen(RED);
  delay(500);
  tft.fillScreen(GREEN);
  delay(500);
  tft.fillScreen(BLUE);
  delay(500);

  Serial.println("Before cam.begin");
  if (!cam.begin(CAMERA_R320x240, IMAGE_MODE, 30)) {
    blinkLED();
  }

  Serial.println("After cam.begin");

  blinkLED(5);
}

void loop() {
  if (!Serial) {
    Serial.begin(115200);
    while (!Serial)
      ;
  }

  while (Serial.read() == -1)
    ;
  while (Serial.read() != -1)
    ;


  // Grab frame and write to serial
  while (cam.grabFrame(fb, 3000) != 0)
    ;

  Serial.print("Frame Size: ");
  Serial.println(cam.frameSize(), DEC);
  MemoryHexDump(Serial, fb.getBuffer(), 320*4/*cam.frameSize()*/, true);
  tft.writeRect(0, 0, tft.width(), tft.height(), (uint16_t *)fb.getBuffer());
}
