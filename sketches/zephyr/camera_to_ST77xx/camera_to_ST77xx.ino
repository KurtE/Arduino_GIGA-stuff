#include <ST77XX_zephyr.h>
#include "camera.h"

Camera cam;
#include <SPI.h>
#include <ST77XX_zephyr.h>
#include <ST77XX_zephyr_font_Arial.h>
#include <zephyr/device.h>
#include <zephyr/drivers/video.h>
#include <zephyr/drivers/video-controls.h>

#ifdef ARDUINO_PORTENTA_H7
#define TFT_DC 4
#define TFT_CS 2
#define TFT_RST 3
#define TFT_SPI SPI

#else
#define TFT_CS 3   // 10  // CS & DC can use pins 2, 6, 9, 10, 15, 20, 21, 22, 23
#define TFT_DC 4   //8  //  but certain pairs must NOT be used: 2+10, 6+9, 20+23, 21+22
#define TFT_RST 5  //9  // RST can use any pin
#define TFT_SPI SPI1

#endif

#define CAMERA_WIDTH 320
#define CAMERA_HEIGHT 240
// Use one or the other
//ST7735_zephyr tft = ST7735_zephyr(&SPI, TFT_CS, TFT_DC, TFT_RST);

// For 1.54" TFT with ST7789
//ST7789_zephyr tft = ST7789_zephyr(&SPI, TFT_CS, TFT_DC, TFT_RST);

// For 3.5" or 4.0" TFT with ST7796
ST7796_zephyr tft = ST7796_zephyr(&TFT_SPI, TFT_CS, TFT_DC, TFT_RST);

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

  Serial.println("\n*** start display camera image on ST77XX ***");
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

  Serial.println("call cam.begin");

  uint8_t camera_count = 1;
  if ((camera_count = cam.getDTCameraCount()) > 1) {
    Serial.print("Camera count: ");
    Serial.println(camera_count);
    Serial.print("Enter Camera index: ");
    uint8_t camera_index = 0;
    int ich;
    while ((ich = Serial.read()) == -1) {};
    if (ich >= '0' && ich <= '9') {
      camera_index = ich - '0';
    }
    while ((ich = Serial.read()) != -1) {};
    cam.setDTCameraIndex(camera_index);
  }
  
  if (!cam.begin(CAMERA_WIDTH, CAMERA_HEIGHT, CAMERA_RGB565, true)) {
    fatal_error("Camera begin failed");
  }
  Serial.println("Camera started");
  cam.setVerticalFlip(false);
  cam.setHorizontalMirror(false);

  // Quick and dirty test to see if we can talk to the new stuff in camera.
#if 0
  video_selection sel;
  sel.target = VIDEO_SEL_TGT_NATIVE_SIZE;
  int ret = video_get_selection(cam.videoDevice(), &sel);

  Serial.print("video_get_selection(TGT_NATIVE_SIZE)");
  Serial.print(ret);
  Serial.print(" ");
  Serial.print(sel.rect.width);
  Serial.print(" ");
  Serial.println(sel.rect.height);
#endif
}

volatile bool write_rect_complete = false;
void write_rect_complete_cb(int result) {
  UNUSED(result);
  write_rect_complete = true;
}

bool use_writeRectCB = true;

void loop() {
  // put your main code here, to run repeatedly:
  FrameBuffer fb;
  //Serial.print("Call grabFrame");
  bool frame_received = cam.grabFrame(fb);
  //Serial.println(frame_received? " true" : " false");
  if (frame_received) {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    uint32_t start_time = micros();
    if (use_writeRectCB) {
      write_rect_complete = false;
      tft.writeRectCB(0, 0, CAMERA_WIDTH, CAMERA_HEIGHT, (const uint16_t *)fb.getBuffer(), &write_rect_complete_cb);
      while (!write_rect_complete) delay(1);
      tft.writeRect(0, 0, CAMERA_WIDTH, CAMERA_HEIGHT, (const uint16_t *)fb.getBuffer());
    }
    Serial.println(micros() - start_time);
    //tft.writeSubImageRectBytesReversed(0, 0, 320, 240, 0, 0, 320, 240, (const uint16_t*)fb.getBuffer());
    //tft.writeSubImageRect(0, 0, 320, 240, 0, 0, 320, 240, (const uint16_t*)fb.getBuffer());
    cam.releaseFrame(fb);
  }
  if (Serial.available()) {
    while (Serial.read() != -1) {}
    use_writeRectCB = !use_writeRectCB;
  }
}
