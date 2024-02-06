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
#define ALIGN_PTR(p,a)   ((p & (a-1)) ?(((uintptr_t)p + a) & ~(uintptr_t)(a-1)) : p)


#include "arducam_dvp.h"
//#include "SDRAM.h"

// This example only works with Greyscale cameras (due to the palette + resize&rotate algo)
//#define ARDUCAM_CAMERA_HM01B0

#include "OV7670/ov767x.h"
OV7670 ov767x;
//OV7675 ov767x;
Camera cam(ov767x);
#define IMAGE_MODE CAMERA_RGB565

// The buffer used to capture the frame
FrameBuffer fb;

// The buffer used to rotate and resize the frame
GigaDisplay_GFX display;
uint16_t g_camera_width;
uint16_t g_camera_height;

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
  Serial.println("Before camera start"); Serial.flush();
  if (!cam.begin(CAMERA_R640x480 /*CAMERA_R320x240 */, IMAGE_MODE, 10)) {
    blinkLED();
  }

  Serial.println("Before setBuffer"); Serial.flush();

  uint8_t *fb_mem = (uint8_t *)SDRAM.malloc(640 * 480 * 2 + 32);
  fb.setBuffer((uint8_t *)ALIGN_PTR((uintptr_t)fb_mem, 32));
  printf("Frame buffer: %p\n", fb.getBuffer());

  // clear the display (gives a nice black background)
  Serial.println("Before setRotation"); Serial.flush();
  display.begin();
  display.setRotation(3);
  Serial.println("Before fillscreen"); Serial.flush();
  display.fillScreen(GC9A01A_BLUE);

  g_camera_width = 640; //320; //cam.getResolutionWidth();
  g_camera_height = 480; //240; //cam.getResolutionHeight();
  Serial.println("end setup"); Serial.flush();
}

#define HTONS(x) (((x >> 8) & 0x00FF) | ((x << 8) & 0xFF00))

void loop() {

  // Grab frame and write to another framebuffer
  if (cam.grabFrame(fb, 3000) == 0) {

    // We need to swap bytes.
    uint16_t *pixels = (uint16_t *)fb.getBuffer();
    for (int i = 0; i < g_camera_width * g_camera_height; i++) pixels[i] = HTONS(pixels[i]);
    display.drawRGBBitmap((display.width() - g_camera_width) / 2, (display.height() - g_camera_height) / 2, pixels, g_camera_width, g_camera_height);
  } else {
    blinkLED(20);
  }
}