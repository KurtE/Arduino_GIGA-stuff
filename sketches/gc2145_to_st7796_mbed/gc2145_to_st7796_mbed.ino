#include <SDRAM.h>
#include <ram_internal.h>

//#define USE_ILI9341
#ifdef USE_ILI9341
#include <ILI9341_GIGA_n.h>
#else
#include <ST77XX_mbed.h>
#endif

#include "arducam_dvp.h"
#define ARDUCAM_CAMERA_GC2145
#include "GC2145/gc2145.h"

GC2145 galaxyCore;
Camera cam(galaxyCore);
FrameBuffer fb;

#include <SPI.h>

#ifdef ARDUINO_PORTENTA_H7_M7
#define TFT_DC 4
#define TFT_CS 2
#define TFT_RST 3
#define TFT_SPI SPI

#else
#define TFT_CS 10  // CS & DC can use pins 2, 6, 9, 10, 15, 20, 21, 22, 23
#define TFT_DC 8   //  but certain pairs must NOT be used: 2+10, 6+9, 20+23, 21+22
#define TFT_RST 9  // RST can use any pin
#define TFT_SPI SPI1

#endif

#define CAMERA_WIDTH 320   //320 480
#define CAMERA_HEIGHT 240  // 240  320
#define IMAGE_MODE CAMERA_RGB565
#define RESOLUTION CAMERA_R320x240  // Zoom in from the highest supported resolution
#define ZOOM_WINDOW_RESOLUTION CAMERA_R320x240

#ifdef USE_ILI9341
ILI9341_GIGA_n tft(&TFT_SPI, TFT_CS, TFT_DC, TFT_RST);
#else
// Use one or the other
//ST7735_zephyr tft = ST7735_zephyr(&SPI, TFT_CS, TFT_DC, TFT_RST);

// For 1.54" TFT with ST7789
//ST7789_zephyr tft = ST7789_zephyr(&SPI, TFT_CS, TFT_DC, TFT_RST);

// For 3.5" or 4.0" TFT with ST7796
ST7796_zephyr tft = ST7796_zephyr(&TFT_SPI, TFT_CS, TFT_DC, TFT_RST);
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

#define RGB565_BLACK 0x0000       /*   0,   0,   0 */
#define RGB565_NAVY 0x000F        /*   0,   0, 128 */
#define RGB565_DARKGREEN 0x03E0   /*   0, 128,   0 */
#define RGB565_DARKCYAN 0x03EF    /*   0, 128, 128 */
#define RGB565_MAROON 0x7800      /* 128,   0,   0 */
#define RGB565_PURPLE 0x780F      /* 128,   0, 128 */
#define RGB565_OLIVE 0x7BE0       /* 128, 128,   0 */
#define RGB565_LIGHTGREY 0xC618   /* 192, 192, 192 */
#define RGB565_DARKGREY 0x7BEF    /* 128, 128, 128 */
#define RGB565_BLUE 0x001F        /*   0,   0, 255 */
#define RGB565_GREEN 0x07E0       /*   0, 255,   0 */
#define RGB565_CYAN 0x07FF        /*   0, 255, 255 */
#define RGB565_RED 0xF800         /* 255,   0,   0 */
#define RGB565_MAGENTA 0xF81F     /* 255,   0, 255 */
#define RGB565_YELLOW 0xFFE0      /* 255, 255,   0 */
#define RGB565_WHITE 0xFFFF       /* 255, 255, 255 */
#define RGB565_ORANGE 0xFD20      /* 255, 165,   0 */
#define RGB565_GREENYELLOW 0xAFE5 /* 173, 255,  47 */
#define RGB565_PINK 0xF81F


void setup() {
  SDRAM.begin();
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  while (!Serial && millis() < 5000) {}

  Serial.println("\n*** start display camera image on ILI9341 ***");
  Serial.println(TFT_CS);
  TFT_SPI.begin();
#ifdef USE_ILI9341
  tft.begin();
#else
  //tft.init(240, 320);           // Init ST7789 320x240
  tft.init(320, 480);
#endif

  tft.setRotation(1);
  tft.fillScreen(RGB565_BLACK);
#if 1
  delay(500);
  tft.fillScreen(RGB565_RED);
  delay(500);
  tft.fillScreen(RGB565_GREEN);
  delay(500);
  tft.fillScreen(RGB565_BLUE);
  delay(500);
  tft.fillScreen(RGB565_BLACK);
  delay(500);
#endif
  Serial.println("call cam.begin");
  //  if (!cam.begin(CAMERA_WIDTH, CAMERA_HEIGHT, CAMERA_RGB565, true)) {
  //  pinMode(PE_3, INPUT_PULLUP);
  //  pinMode(PD_5, INPUT_PULLUP);
  //  cam.debug(Serial);
  if (!cam.begin(RESOLUTION, IMAGE_MODE, 30)) {

    fatal_error("Camera begin failed");
  }
  cam.setVerticalFlip(false);
  cam.setHorizontalMirror(false);
  Serial.println("Camera started");
  delay(1000);

  uint8_t *fb_buf = nullptr;
#if 1  // to allocate here
  uint32_t buffer_size = cam.frameSize();
#if 0 // to use malloc
  Serial.print(" Buffer Size:");
  Serial.println(buffer_size);
  Serial.println("Try malloc to allocate buffer");
  Serial.flush();
  fb_buf = (uint8_t *)malloc(buffer_size + 32);
#endif  
#if 1
  if (fb_buf == nullptr) {
    Serial.println("Falled, try SDRAM.malloc");
    fb_buf = (uint8_t *)SDRAM.malloc(buffer_size + 32);
    if (fb_buf == nullptr) {
      Serial.println("*** failed ***");
      fatal_error("Failed to allocate SDRAM");
    }
  }
#endif  
  fb_buf = (uint8_t *)((((uint32_t)fb_buf) + 32) & 0xffffffe0l);
  Serial.print("FB Buffer: ");
  Serial.println((uint32_t)fb_buf, HEX);

  fb.setBuffer(fb_buf);
#endif
  cam.debug(Serial);
}

volatile bool write_rect_complete = false;
void write_rect_complete_cb(int result) {
  UNUSED(result);
  write_rect_complete = true;
}

bool use_writeRectCB = true;

void loop() {
  // put your main code here, to run repeatedly:
  //Serial.print("Call grabFrame");
  int err = cam.grabFrame(fb);
  if (err == 0) {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    uint32_t start_time = micros();

#if 1
    uint16_t *pixels = (uint16_t *)fb.getBuffer();
    for (int i = 0; i < CAMERA_WIDTH * CAMERA_HEIGHT; i++) pixels[i] = __REVSH(pixels[i]);
    tft.writeRect(0, 0, CAMERA_WIDTH, CAMERA_HEIGHT, pixels);
#else
    if (use_writeRectCB) {
      write_rect_complete = false;
      tft.writeRectCB(0, 0, CAMERA_WIDTH, CAMERA_HEIGHT, (const uint16_t *)fb.getBuffer(), &write_rect_complete_cb);
      while (!write_rect_complete) delay(1);
    } else {
      tft.writeRect(0, 0, CAMERA_WIDTH, CAMERA_HEIGHT, (const uint16_t *)fb.getBuffer());
    }
#endif
    Serial.println(micros() - start_time);
    //tft.writeSubImageRectBytesReversed(0, 0, 320, 240, 0, 0, 320, 240, (const uint16_t*)fb.getBuffer());
    //tft.writeSubImageRect(0, 0, 320, 240, 0, 0, 320, 240, (const uint16_t*)fb.getBuffer());
    //cam.releaseFrame(fb);
  } else {
    Serial.println(err);
    delay(250);
  }
  if (Serial.available()) {
    Serial.println("Paused");
    while (Serial.read() != -1) {}
    while (Serial.read() == -1) {}
    while (Serial.read() != -1) {}
    //use_writeRectCB = !use_writeRectCB;
  }
}