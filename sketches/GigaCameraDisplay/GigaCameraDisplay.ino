#include "arducam_dvp.h"
#include "Arduino_H7_Video.h"
#include "dsi.h"
#include "SDRAM.h"

// This example only works with Greyscale cameras (due to the palette + resize&rotate algo)
//#define ARDUCAM_CAMERA_HM01B0
#define ARDUCAM_CAMERA_OV767X

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
 OV7670 ov767x;
//OV7675 ov767x;
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
Arduino_H7_Video Display(800, 480, GigaDisplayShield);

void blinkLED(uint32_t count = 0xFFFFFFFF)
{
  pinMode(LED_BUILTIN, OUTPUT);
  while (count--) {
    digitalWrite(LED_BUILTIN, LOW);  // turn the LED on (HIGH is the voltage level)
    delay(50);                       // wait for a second
    digitalWrite(LED_BUILTIN, HIGH); // turn the LED off by making the voltage LOW
    delay(50);                       // wait for a second
  }
}

void setup() {
  // Init the cam QVGA, 30FPS
  if (!cam.begin(CAMERA_R320x240, IMAGE_MODE, 30)) {
    blinkLED();
  }

  Display.begin();

  outfb.setBuffer((uint8_t*)SDRAM.malloc(1024 * 1024));

  // clear the display (gives a nice black background)
  dsi_lcdClear(0);
  dsi_drawCurrentFrameBuffer();
  dsi_lcdClear(0);
  dsi_drawCurrentFrameBuffer();
}

#define HTONS(x)    (((x >> 8) & 0x00FF) | ((x << 8) & 0xFF00))

void loop() {

  // Grab frame and write to another framebuffer
  if (cam.grabFrame(fb, 3000) == 0) {

    // double the resolution and transpose (rotate by 90 degrees) in the same step
    // this only works if the camera feed is 320x240 and the area where we want to display is 640x480
#if 0
    for (int i = 0; i < 320; i++) {
      for (int j = 0; j < 240; j++) {
        if (IMAGE_MODE == CAMERA_GRAYSCALE) {
          ((uint8_t*)outfb.getBuffer())[j * 2 + (i * 2) * 480] = ((uint8_t*)fb.getBuffer())[i + j * 320];
          ((uint8_t*)outfb.getBuffer())[j * 2 + (i * 2) * 480 + 1] = ((uint8_t*)fb.getBuffer())[i + j * 320];
          ((uint8_t*)outfb.getBuffer())[j * 2 + (i * 2 + 1) * 480] = ((uint8_t*)fb.getBuffer())[i + j * 320];
          ((uint8_t*)outfb.getBuffer())[j * 2 + (i * 2 + 1) * 480 + 1] = ((uint8_t*)fb.getBuffer())[i + j * 320];
        } else {
          ((uint16_t*)outfb.getBuffer())[j * 2 + (i * 2) * 480] = HTONS(((uint16_t*)fb.getBuffer())[i + j * 320]);
          ((uint16_t*)outfb.getBuffer())[j * 2 + (i * 2) * 480 + 1] = HTONS(((uint16_t*)fb.getBuffer())[i + j * 320]);
          ((uint16_t*)outfb.getBuffer())[j * 2 + (i * 2 + 1) * 480] = HTONS(((uint16_t*)fb.getBuffer())[i + j * 320]);
          ((uint16_t*)outfb.getBuffer())[j * 2 + (i * 2 + 1) * 480 + 1] = HTONS(((uint16_t*)fb.getBuffer())[i + j * 320]);
        }
      }
    }
    dsi_lcdDrawImage((void*)outfb.getBuffer(), (void*)dsi_getCurrentFrameBuffer(), 480, 640, IMAGE_MODE == CAMERA_GRAYSCALE ? DMA2D_INPUT_L8 : DMA2D_INPUT_RGB565);
#else
    uint16_t *pixels = (uint16_t*)fb.getBuffer();
    for (int i = 0; i < 320*240; i++) pixels[i] = HTONS(pixels[i]);
    dsi_lcdDrawImage((void*)pixels, (void*)dsi_getCurrentFrameBuffer(), 320, 240, DMA2D_INPUT_RGB565);
#endif    
    dsi_drawCurrentFrameBuffer();
  } else {
    blinkLED(20);
  }
}