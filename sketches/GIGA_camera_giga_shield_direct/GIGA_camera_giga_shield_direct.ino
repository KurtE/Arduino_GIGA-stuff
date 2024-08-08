//=============================================================================
// Simple Camera to Arduino Giga Display SHield display.  
// This sketch only uses one frame buffer and if the image is > QVGA it
// truncates the image to 480x480
//=============================================================================
#include "arducam_dvp.h"
#include "Arduino_H7_Video.h"
#include "dsi.h"
#include "SDRAM.h"

//=============================================================================
// Define which camera - uncomment only one.
//=============================================================================
//#define ARDUCAM_CAMERA_HM01B0
//#define ARDUCAM_CAMERA_HM0360
//#define ARDUCAM_CAMERA_OV767X
#define ARDUCAM_CAMERA_GC2145

//=============================================================================
// Define which resolution
//=============================================================================

  // CAMERA_R160x120     = 0,   /* QQVGA Resolution   */
  // CAMERA_R320x240     = 1,   /* QVGA Resolution    */
  // CAMERA_R320x320     = 2,   /* 320x320 Resolution */
  // CAMERA_R640x480     = 3,   /* VGA                */
  // CAMERA_R800x600     = 5,   /* SVGA               */
  // CAMERA_R1600x1200   = 6,   /* UXGA               */
  // CAMERA_RMAX                /* Sentinel value */
#define RESOLUTION CAMERA_R320x240

//=============================================================================
#ifdef ARDUCAM_CAMERA_HM01B0
#include "Himax_HM01B0/himax.h"
HM01B0 himax;
Camera cam(himax);
#define IMAGE_MODE CAMERA_GRAYSCALE

#elif defined(ARDUCAM_CAMERA_HM0360)
#include "Himax_HM0360/hm0360.h"
HM0360 himax;
Camera cam(himax);
#define IMAGE_MODE CAMERA_GRAYSCALE

#elif defined(ARDUCAM_CAMERA_OV767X)
#include "OV7670/ov767x.h"
// OV7670 ov767x;
OV7675 ov767x;
Camera cam(ov767x);
#define IMAGE_MODE CAMERA_RGB565

#elif defined(ARDUCAM_CAMERA_GC2145)
#include "GC2145/gc2145.h"
GC2145 galaxyCore;
Camera cam(galaxyCore);
#define IMAGE_MODE CAMERA_RGB565
#endif

// The buffer used to capture the frame
FrameBuffer fb;

// The buffer used to rotate and resize the frame
Arduino_H7_Video Display(800, 480, GigaDisplayShield);

void blinkLED(uint32_t count = 0xFFFFFFFF) {
  pinMode(LED_BUILTIN, OUTPUT);
  while (count--) {
    digitalWrite(LED_BUILTIN, LOW);   // turn the LED on (HIGH is the voltage level)
    delay(50);                        // wait for a second
    digitalWrite(LED_BUILTIN, HIGH);  // turn the LED off by making the voltage LOW
    delay(50);                        // wait for a second
  }
}

uint32_t palette[256];

//#define CAMERA_WIDTH 640
//#define CAMERA_HEIGHT 480

uint32_t CAMERA_WIDTH;
uint32_t CAMERA_HEIGHT;


void setup() {
  // Init the cam QVGA, 30FPS
  while (!Serial && millis() < 4000) {}
  Serial.begin(115200);
  cam.debug(Serial);

  
  Serial.println("Giga Camera shield direct start");
  if (!cam.begin(RESOLUTION, IMAGE_MODE, 15)) {
    Serial.println("Camera Begin failed!!!");
    blinkLED();
  }
  CAMERA_WIDTH = cam.getResolutionWidth();
  CAMERA_HEIGHT = cam.getResolutionHeight();

  Serial.print("Camera Width:");
  Serial.print(CAMERA_WIDTH, DEC);
  Serial.print(" Height:");
  Serial.print(CAMERA_HEIGHT, DEC);

  uint32_t buffer_size = cam.frameSize();
  Serial.print(" Buffer Size:");
  Serial.println(buffer_size);

  cam.printRegs();

#if defined(ARDUCAM_CAMERA_HM0360) || defined(ARDUCAM_CAMERA_HM01B0)
  cam.setVerticalFlip(true);
  // Mirrors the image horizontally
  cam.setHorizontalMirror(true);
#endif
  // Setup the palette to convert 8 bit greyscale to 32bit greyscale
  for (int i = 0; i < 256; i++) {
    palette[i] = 0xFF000000 | (i << 16) | (i << 8) | i;
  }


  Display.begin();

  if (IMAGE_MODE == CAMERA_GRAYSCALE) {
    dsi_configueCLUT((uint32_t *)palette);
  }
  // big enough for full screen.
  Serial.println("Try malloc to allocate buffer");
  Serial.flush();
  uint8_t *fb_buf = (uint8_t *)malloc(buffer_size + 32);
  if (fb_buf == nullptr) {
    Serial.println("Falled, try SDRAM.malloc");
    fb_buf = (uint8_t *)SDRAM.malloc(buffer_size + 32);
    if (fb_buf == nullptr) {
      Serial.println("*** failed ***");
      blinkLED();
    }
  }
  fb_buf = (uint8_t *)((((uint32_t)fb_buf) + 32) & 0xffffffe0l);
  Serial.print("FB Buffer: ");
  Serial.println((uint32_t)fb_buf, HEX);

  fb.setBuffer(fb_buf);

  // clear the display (gives a nice black background)
  dsi_lcdClear(0);
  dsi_drawCurrentFrameBuffer();
  dsi_lcdClear(0);
  dsi_drawCurrentFrameBuffer();
}

inline uint16_t HTONS(uint16_t x) {
  return (((x >> 8) & 0x00FF) | ((x << 8) & 0xFF00));
}

void loop() {

  // Grab frame and write to another framebuffer
  if (cam.grabFrame(fb, 3000) == 0) {

    // double the resolution and transpose (rotate by 90 degrees) in the same step
    // this only works if the camera feed is 320x240 and the area where we want to display is 640x480
    SCB_CleanInvalidateDCache();
    if (CAMERA_WIDTH < 480) {
      if (IMAGE_MODE == CAMERA_RGB565) {
        uint16_t *frame_in = (uint16_t *)fb.getBuffer();
        for (uint32_t ii = 0; ii < (CAMERA_WIDTH * CAMERA_HEIGHT); ii++) frame_in[ii] = HTONS(frame_in[ii]);
      }
      dsi_lcdDrawImage((void *)fb.getBuffer(), (void *)dsi_getCurrentFrameBuffer(), CAMERA_WIDTH, CAMERA_HEIGHT, IMAGE_MODE == CAMERA_GRAYSCALE ? DMA2D_INPUT_L8 : DMA2D_INPUT_RGB565);
    } else {
      if (IMAGE_MODE == CAMERA_GRAYSCALE) {
        uint8_t *frame_in = (uint8_t *)fb.getBuffer();
        for (int y = 0; y < 480; y++) {
          for (int x = 0; x < 480; x++) {
            frame_in[y * 480 + x] = frame_in[y * CAMERA_WIDTH + x];
          }
        }
      } else {
        uint16_t *frame_in = (uint16_t *)fb.getBuffer();
        for (int y = 0; y < 480; y++) {
          for (int x = 0; x < 480; x++) {
            frame_in[y * 480 + x] = HTONS(frame_in[y * CAMERA_WIDTH + x]);
          }
        }
      }
      dsi_lcdDrawImage((void *)fb.getBuffer(), (void *)dsi_getCurrentFrameBuffer(), 480, 480, IMAGE_MODE == CAMERA_GRAYSCALE ? DMA2D_INPUT_L8 : DMA2D_INPUT_RGB565);
    }
    dsi_drawCurrentFrameBuffer();
  } else {
    blinkLED(20);
    static uint8_t error_count = 10;
    if (error_count) {
      Serial.println("Failed to read frame");
      error_count--;
    }
  }
}