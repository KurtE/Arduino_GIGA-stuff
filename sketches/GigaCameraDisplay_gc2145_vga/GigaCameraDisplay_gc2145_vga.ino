#include "arducam_dvp.h"
#include "Arduino_H7_Video.h"
#include "dsi.h"
#include "SDRAM.h"

// This example only works with Greyscale cameras (due to the palette + resize&rotate algo)
#define ARDUCAM_CAMERA_GC2145

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
FrameBuffer outfb;

// The buffer used to rotate and resize the frame
#define VGA_RES
#ifdef VGA_RES
#define CAMERA_WIDTH 640
#define CAMERA_HEIGHT 480
#define CAMERA_RESOLUTION CAMERA_R640x480
#else
#define CAMERA_WIDTH 320
#define CAMERA_HEIGHT 240
#define CAMERA_RESOLUTION CAMERA_R320x240
#endif
Arduino_H7_Video Display(800, 480, GigaDisplayShield);
#define OUTFB_WIDTH 480
#define OUTFB_HEIGHT 640
#define RED 0xf800

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

void setup() {
  // Init the cam QVGA, 30FPS
  if (!cam.begin(CAMERA_RESOLUTION, IMAGE_MODE, 30)) {
    blinkLED();
  }

  // Setup the palette to convert 8 bit greyscale to 32bit greyscale
  for (int i = 0; i < 256; i++) {
    palette[i] = 0xFF000000 | (i << 16) | (i << 8) | i;
  }
  galaxyCore.debug(Serial);
  Display.begin();
  galaxyCore.printRegs();

  if (IMAGE_MODE == CAMERA_GRAYSCALE) {
    dsi_configueCLUT((uint32_t *)palette);
  }

  uint8_t *fb_buf = (uint8_t *)((((uint32_t)SDRAM.malloc(CAMERA_WIDTH * CAMERA_HEIGHT * 2 + 32)) + 32) & 0xffffffe0l);
  fb.setBuffer(fb_buf);
  Serial.print("FB Buffer: ");
  Serial.println((uint32_t)fb_buf, HEX);

  #ifdef USE_OUTFB
  outfb.setBuffer((uint8_t *)SDRAM.malloc(OUTFB_WIDTH * OUTFB_HEIGHT * 2));
  Serial.print("OutFB Buffer: ");
  Serial.print((uint32_t)outfb.getBuffer(), HEX);
  #endif

  // clear the display (gives a nice black background)
  dsi_lcdClear(0);
  dsi_drawCurrentFrameBuffer();
  dsi_lcdClear(0);
  dsi_drawCurrentFrameBuffer();
}

#define HTONS(x) (((x >> 8) & 0x00FF) | ((x << 8) & 0xFF00))

void loop() {

  // Grab frame and write to another framebuffer
  if (cam.grabFrame(fb, 3000) == 0) {

    #ifdef USE_OUTFB
    // Try simple rotate of the image

    uint16_t *frame_in = (uint16_t *)fb.getBuffer();
    uint16_t *frame_out = (uint16_t *)outfb.getBuffer();

    for (int ii = 0; ii < (OUTFB_WIDTH * OUTFB_HEIGHT); ii++) frame_out[ii] = RED;

    for (int y = 0; y < CAMERA_HEIGHT; y++) {
      for (int x = 0; x < CAMERA_WIDTH; x++ ) {
        // frame_out[y + (CAMERA_WIDTH - 1 - x) * OUTFB_WIDTH ] = HTONS(*frame_in);
        frame_out[(CAMERA_HEIGHT - 1 - y) +  x * OUTFB_WIDTH ] = HTONS(*frame_in);
        frame_in++;
      }
    }
    dsi_lcdDrawImage((void *)outfb.getBuffer(), (void *)dsi_getCurrentFrameBuffer(), 480, 640, IMAGE_MODE == CAMERA_GRAYSCALE ? DMA2D_INPUT_L8 : DMA2D_INPUT_RGB565);
    #else
    uint16_t *frame_in = (uint16_t *)fb.getBuffer();
    #if CAMERA_WIDTH > 320 
    for (int y = 0; y < 480; y++) {
      for (int x = 0; x < 48; x++) {
        frame_in[y * 480 + x] = HTONS(frame_in[y * 640 + x]);
      }
    }
    dsi_lcdDrawImage((void *)fb.getBuffer(), (void *)dsi_getCurrentFrameBuffer(), 480, 480, IMAGE_MODE == CAMERA_GRAYSCALE ? DMA2D_INPUT_L8 : DMA2D_INPUT_RGB565);
    #else
    dsi_lcdDrawImage((void *)fb.getBuffer(), (void *)dsi_getCurrentFrameBuffer(), CAMERA_WIDTH, CAMERA_HEIGHT, IMAGE_MODE == CAMERA_GRAYSCALE ? DMA2D_INPUT_L8 : DMA2D_INPUT_RGB565);
    #endif
    #endif
    dsi_drawCurrentFrameBuffer();
  } else {
    blinkLED(20);
  }
}