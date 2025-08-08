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

#define CAMERA_WIDTH 800
#define CAMERA_HEIGHT 600
#define CAMERA_VIEW_WIDTH 400
#define CAMERA_VIEW_HEIGHT 240
#define SCALE 2

#include <camera.h>

Camera cam;

// The buffer used to capture the frame
FrameBuffer fb;

#ifdef ROTATE_CAMERA_IMAGE
uint16_t *rotate_buffer = nullptr;
#endif

struct video_selection vselPan = { VIDEO_BUF_TYPE_OUTPUT, VIDEO_SEL_TGT_CROP, { 0, 0, 0, 0 } };
struct video_selection vselNativeSize = { VIDEO_BUF_TYPE_OUTPUT, VIDEO_SEL_TGT_NATIVE_SIZE, { 0, 0, 0, 0 } };
int pan_delta_x = 0;




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

  printk("Before setSnapshotMode call\n");
  cam.setSnapshotMode(true);
  printk("CONFIG_VIDEO_BUFFER_POOL_NUM_MAX: %u\n", CONFIG_VIDEO_BUFFER_POOL_NUM_MAX);
  printk("Before cam.begin call\n");
  if (!cam.begin(CAMERA_WIDTH, CAMERA_HEIGHT, CAMERA_VIEW_WIDTH, CAMERA_VIEW_HEIGHT, CAMERA_RGB565, true)) {
    fatal_error("Camera begin failed");
  }



  //Serial.println("Before setRotation");
  //Serial.flush();

  display.begin();
  elapsedMicros em;
  display.setRotation(1);
  display.fillScreen(GC9A01A_BLUE);
  Serial.println(em, DEC);
  Serial.print("Camera Width: ");
  Serial.print(CAMERA_WIDTH);
  Serial.print(" Height: ");
  Serial.println(CAMERA_HEIGHT);
  Serial.print("Screen Width: ");
  Serial.print(display.width());
  Serial.print(" Height: ");
  Serial.println(display.height());
  Serial.print("Scale: ");
  Serial.println(SCALE);
  Serial.println("end setup");
  Serial.flush();

  int err;
  if ((err = cam.getSelection(&vselPan)) == 0) {
    Serial.print("Crop Rect:(");
    Serial.print(vselPan.rect.left);
    Serial.print(",");
    Serial.print(vselPan.rect.top);
    Serial.print(") ");
    Serial.print(vselPan.rect.width);
    Serial.print("x");
    Serial.println(vselPan.rect.height);
  }

  if ((err = cam.getSelection(&vselNativeSize)) == 0) {
    Serial.print("Native Size Rect:(");
    Serial.print(vselNativeSize.rect.left);
    Serial.print(",");
    Serial.print(vselNativeSize.rect.top);
    Serial.print(") ");
    Serial.print(vselNativeSize.rect.width);
    Serial.print("x");
    Serial.print(vselNativeSize.rect.height);
  }
  pan_delta_x = (vselNativeSize.rect.width - vselPan.rect.width) / 4;
}



uint32_t display_time_sum = 0;
uint8_t display_time_count = 0;

void loop() {

  //Serial.println("Camera frame received");
  if (Serial.available()) {
    while (Serial.read() != -1) {}
    //      cam.printRegs();
    //MemoryHexDump(Serial, fb.getBuffer(), 1024, true, "Start of Camera Buffer\n");
    Serial.println("*** Paused ***");
    while (Serial.read() == -1) {}
    while (Serial.read() != -1) {}
  }

  // Grab frame and write to another framebuffer
  if (cam.grabFrame(fb, 250)) {
    uint16_t *pixels = (uint16_t *)fb.getBuffer();
    elapsedMicros emDisplay;
#if defined(SCALE) && SCALE > 1
    // Quick and dirty scale.
    int yDisplay = (display.height() - (CAMERA_VIEW_HEIGHT * SCALE)) / 2;
    display.startBuffering();
    for (int yCamera = 0; yCamera < CAMERA_VIEW_HEIGHT; yCamera++) {
      int xDisplay = (display.width() - (CAMERA_VIEW_WIDTH * SCALE)) / 2;
      for (int xCamera = 0; xCamera < CAMERA_VIEW_WIDTH; xCamera++) {
        display.fillRect(xDisplay, yDisplay, SCALE, SCALE, *pixels++);
        xDisplay += SCALE;
      }
      yDisplay += SCALE;
    }
    display.endBuffering();
#else
    display.drawRGBBitmap((display.width() - CAMERA_VIEW_WIDTH) / 2, (display.height() - CAMERA_VIEW_HEIGHT) / 2, pixels, CAMERA_VIEW_WIDTH, CAMERA_VIEW_HEIGHT);
#endif

    cam.releaseFrame(fb);

    display_time_sum += emDisplay;
    display_time_count++;
    if (display_time_count == 128) {
      Serial.print("Avg display Time: ");
      Serial.print(display_time_sum / display_time_count);
      Serial.print(" fps:");
      Serial.println(128000000.0 / double(display_time_sum), 2);
      display_time_sum = 0;
      display_time_count = 0;
    }
    if ((vselPan.rect.left == 0) && (pan_delta_x < 0)) pan_delta_x = -pan_delta_x;
    if ((vselPan.rect.left + vselPan.rect.width) == vselNativeSize.rect.width) pan_delta_x = -pan_delta_x;

    vselPan.rect.left += pan_delta_x;
    if (cam.setSelection(&vselPan) != 0) {
      Serial.print("Eror setting Crop Rect:(");
      Serial.print(vselPan.rect.left);
      Serial.print(",");
      Serial.print(vselPan.rect.top);
      Serial.print(") ");
      Serial.print(vselPan.rect.width);
      Serial.print("x");
      Serial.print(vselPan.rect.height);
    }


  } else {

    digitalWrite(LED_BUILTIN, digitalRead(LED_BUILTIN) ? LOW : HIGH);
  }
  delay(50);
}