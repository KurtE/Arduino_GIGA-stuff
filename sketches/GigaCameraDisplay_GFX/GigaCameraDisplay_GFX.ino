REDIRECT_STDOUT_TO(Serial)
#include "Arduino_GigaDisplay_GFX.h"
#define GC9A01A_CYAN 0x07FF
#define GC9A01A_RED 0xf800
#define GC9A01A_BLUE 0x001F
#define GC9A01A_GREEN 0x07E0
#define GC9A01A_MAGENTA 0xF81F
#define GC9A01A_WHITE 0xffff
#define GC9A01A_BLACK 0x0000
#define GC9A01A_YELLOW 0xFFE0
#define ALIGN_PTR(p, a) ((p & (a - 1)) ? (((uintptr_t)p + a) & ~(uintptr_t)(a - 1)) : p)

uint16_t color565(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}


#include "arducam_dvp.h"
//#include "SDRAM.h"

// This example only works with Greyscale cameras (due to the palette + resize&rotate algo)
//#define ARDUCAM_CAMERA_HM01B0

//#include "OV7670/ov767x.h"

//#define ARDUCAM_CAMERA_HM0360
#define ARDUCAM_CAMERA_OV767X
#define CAMERA_WIDTH 640
#define CAMERA_HEIGHT 480


#ifdef ARDUCAM_CAMERA_HM01B0
#include "Himax_HM01B0/himax.h"
HM01B0 himax;
Camera cam(himax);
#define IMAGE_MODE CAMERA_GRAYSCALE
#define CANVAS_ROTATION 3

#elif defined(ARDUCAM_CAMERA_HM0360)
#include "Himax_HM0360/hm0360.h"
HM0360 himax;
Camera cam(himax);
#define IMAGE_MODE CAMERA_GRAYSCALE
#define CANVAS_ROTATION 3

#elif defined(ARDUCAM_CAMERA_OV767X)
#include "OV7670/ov767x.h"
#warning OV767x
// OV7670 ov767x;
OV7675 ov767x;
Camera cam(ov767x);
#define IMAGE_MODE CAMERA_RGB565
#define CANVAS_ROTATION 1

#elif defined(ARDUCAM_CAMERA_GC2145)
#include "GC2145/gc2145.h"
#define CANVAS_ROTATION 1
GC2145 galaxyCore;
Camera cam(galaxyCore);
#define IMAGE_MODE CAMERA_RGB565
#endif

// The buffer used to capture the frame
FrameBuffer fb;
uint16_t palette[256];


// The buffer used to rotate and resize the frame
GigaDisplay_GFX display;

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
  while (!Serial && millis() < 5000) {}
  Serial.begin(115200);

  // Init the cam QVGA, 30FPS
  SDRAM.begin();
  Serial.println("Before camera start");
  Serial.flush();
#if CAMERA_WIDTH == 640
  if (!cam.begin(CAMERA_R640x480, IMAGE_MODE, 15)) {
    blinkLED();
  }
#else
  if (!cam.begin(CAMERA_R320x240, IMAGE_MODE, 30)) {
    blinkLED();
  }
#endif
  // Setup the palette to convert 8 bit greyscale to 32bit greyscale
  for (int i = 0; i < 256; i++) {
    palette[i] = color565(i, i, i);
  }

  Serial.println("Before setBuffer");
  Serial.flush();

  uint8_t *fb_mem = (uint8_t *)SDRAM.malloc(CAMERA_WIDTH * CAMERA_HEIGHT * 2 + 32);
  fb.setBuffer((uint8_t *)ALIGN_PTR((uintptr_t)fb_mem, 32));
  printf("Frame buffer: %p\n", fb.getBuffer());

  // clear the display (gives a nice black background)
  Serial.println("Before setRotation");
  Serial.flush();
  display.begin();
  display.setRotation(CANVAS_ROTATION);
  Serial.println("Before fillscreen");
  Serial.flush();
  display.fillScreen(GC9A01A_BLUE);
  Serial.println("end setup");
  Serial.flush();
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
}

#define HTONS(x) (((x >> 8) & 0x00FF) | ((x << 8) & 0xFF00))


void writeRect8BPP(GigaDisplay_GFX *pdisp, int16_t x, int16_t y, const uint8_t bitmap[],
                   int16_t w, int16_t h, const uint16_t *palette) {
  pdisp->startWrite();
  for (int16_t j = 0; j < h; j++, y++) {
    for (int16_t i = 0; i < w; i++) {
      pdisp->writePixel(x + i, y, palette[bitmap[j * w + i]]);
    }
  }
  pdisp->endWrite();
}


void loop() {

  // Grab frame and write to another framebuffer
  digitalWrite(2, HIGH);
  if (cam.grabFrame(fb, 3000) == 0) {
    digitalWrite(2, LOW);

    // We need to swap bytes.
    uint16_t *pixels = (uint16_t *)fb.getBuffer();
  digitalWrite(3, HIGH);
#if defined(ARDUCAM_CAMERA_HM01B0) || defined(ARDUCAM_CAMERA_HM0360)
    writeRect8BPP(&display, (display.width() - CAMERA_WIDTH) / 2, (display.height() - CAMERA_HEIGHT) / 2, (uint8_t *)pixels, CAMERA_WIDTH, CAMERA_HEIGHT, palette);
#else
    for (int i = 0; i < CAMERA_WIDTH * CAMERA_HEIGHT; i++) pixels[i] = HTONS(pixels[i]);
    display.drawRGBBitmap((display.width() - CAMERA_WIDTH) / 2, (display.height() - CAMERA_HEIGHT) / 2, pixels, CAMERA_WIDTH, CAMERA_HEIGHT);
#endif
  digitalWrite(3, LOW);

  } else {
    digitalWrite(2, LOW);
    blinkLED(20);
  }
}