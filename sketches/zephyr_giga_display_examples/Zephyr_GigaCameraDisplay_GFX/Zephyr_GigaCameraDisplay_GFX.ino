#include <elapsedMillis.h>

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

#define CANVAS_ROTATION 0
uint16_t color565(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

#define CAMERA_WIDTH 320
#define CAMERA_HEIGHT 240
#define SCALE 2

#include <camera.h>
 
Camera cam;

// The buffer used to capture the frame
FrameBuffer fb;

#ifdef ROTATE_CAMERA_IMAGE
uint16_t *rotate_buffer = nullptr;
#endif


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
  while (!Serial && millis() < 5000) {}
  Serial.begin(115200);

  // Init the cam QVGA, 30FPS
  Serial.println("Before camera start");
  Serial.flush();
  //cam.debug(Serial);

  if (!cam.begin(CAMERA_WIDTH, CAMERA_HEIGHT, CAMERA_RGB565, true)) {
    fatal_error("Camera begin failed");
  }
  //Serial.println("Before setRotation");
  //Serial.flush();

  display.begin();
  elapsedMicros em;
  display.setRotation(1);
  display.fillScreen(GC9A01A_BLUE);
  Serial.println(em, DEC);
  Serial.print("Camera Width: "); Serial.print(CAMERA_WIDTH);
  Serial.print(" Height: "); Serial.println(CAMERA_HEIGHT);
  Serial.print("Screen Width: "); Serial.print(display.width());
  Serial.print(" Height: "); Serial.println(display.height());
  Serial.print("Scale: "); Serial.println(SCALE);
  Serial.println("end setup");
  Serial.flush();
}

inline uint16_t HTONS(uint16_t x) {
  return ((x >> 8) & 0x00FF) | ((x << 8) & 0xFF00);
}
//#define HTONS(x) (((x >> 8) & 0x00FF) | ((x << 8) & 0xFF00))


uint32_t display_time_sum = 0;
uint8_t display_time_count = 0;

void loop() {

  // Grab frame and write to another framebuffer
  if (cam.grabFrame(fb)) {
    //Serial.println("Camera frame received");
    if (Serial.available()) {
      while (Serial.read() != -1) {}
//      cam.printRegs();
      //MemoryHexDump(Serial, fb.getBuffer(), 1024, true, "Start of Camera Buffer\n");
      Serial.println("*** Paused ***");
      while (Serial.read() == -1) {}
      while (Serial.read() != -1) {}
    }

    uint16_t *pixels = (uint16_t *)fb.getBuffer();
    elapsedMicros emDisplay;
//    for (int i = 0; i < CAMERA_WIDTH*CAMERA_HEIGHT; i++) pixels[i] = HTONS(pixels[i]);
    #if defined(SCALE) && SCALE > 1
    // Quick and dirty scale.
    int yDisplay = (display.height() - (CAMERA_HEIGHT * SCALE)) / 2;
    display.startBuffering();
    for (int yCamera = 0; yCamera < CAMERA_HEIGHT; yCamera++) {
      int xDisplay = (display.width() - (CAMERA_WIDTH * SCALE)) / 2;
      for (int xCamera = 0; xCamera < CAMERA_WIDTH; xCamera++) {
        display.fillRect(xDisplay, yDisplay, SCALE, SCALE, *pixels++);
        xDisplay += SCALE;
      }
      yDisplay += SCALE;
    }
    display.endBuffering();
    #else  
    display.drawRGBBitmap((display.width() - CAMERA_WIDTH) / 2, (display.height() - CAMERA_HEIGHT) / 2, pixels, CAMERA_WIDTH, CAMERA_HEIGHT);
    #endif

    cam.releaseFrame(fb);

    display_time_sum += emDisplay;
    display_time_count++;
    if (display_time_count == 128) {
      Serial.print("Avg display Time: ");
      Serial.print(display_time_sum / display_time_count);
      Serial.print(" fps:");
      Serial.println(128000000.0 / float(display_time_sum), 2);
      display_time_sum = 0;
      display_time_count = 0;
    }

  } else {

    digitalWrite(LED_BUILTIN, digitalRead(LED_BUILTIN)? LOW : HIGH);
  }
  delay(50);
}