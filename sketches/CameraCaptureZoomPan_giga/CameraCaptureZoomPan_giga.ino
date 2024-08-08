/*
 * This example shows how to use the Nicla Vision to capture images from the camera
 * with a zoom window and send them over the serial port.
 * The zoom window will move from left to right and top to bottom 
 * in the predefined steps of pixels (ZOOM_X_STEP and ZOOM_Y_STEP).
 * 
 * Whenever the board sends a frame over the serial port, the blue LED will blink.
 * 
 * Instructions:
 * 1. Upload this sketch to Nicla Vision.
 * 2. Open the CameraRawBytesVisualizer.pde Processing sketch and change `useGrayScale` to `false`.
 * 3. Adjust the serial port in the Processing sketch to match the one used by Nicla Vision.
 * 4. Run the Processing sketch.
 * 
 * Initial author: Sebastian Romero @sebromero
 */

// This is a modified version that runs on the Arduino GIGA with the GC2145 camera
// Instead of doing Serial outputs, it instead shows the image on the display.
// Entering anything in the Serial Monitor, advances the display to the next step


#include "arducam_dvp.h"
#include "Arduino_H7_Video.h"
#include "dsi.h"
#include "SDRAM.h"

#define ARDUCAM_CAMERA_GC2145

#include "GC2145/gc2145.h"
GC2145 galaxyCore;
Camera cam(galaxyCore);
uint32_t palette[256];


#define IMAGE_MODE CAMERA_RGB565

#define CHUNK_SIZE 512                // Size of chunks in bytes
#define RESOLUTION CAMERA_R1600x1200  // Zoom in from the highest supported resolution
#define ZOOM_WINDOW_RESOLUTION CAMERA_R320x240

constexpr uint16_t ZOOM_WINDOW_WIDTH = 320;
constexpr uint16_t ZOOM_WINDOW_HEIGHT = 240;
constexpr uint16_t ZOOM_X_STEP = 100;
constexpr uint16_t ZOOM_Y_STEP = 100;

FrameBuffer fb;
FrameBuffer outfb;
Arduino_H7_Video Display(800, 480, GigaDisplayShield);

uint32_t currentZoomX = 0;
uint32_t currentZoomY = 0;
uint32_t maxZoomX = 0;  // Will be calculated in setup()
uint32_t maxZoomY = 0;  // Will be calculated in setup()


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
  if (!Serial && millis() < 5000) {
    Serial.begin(115200);
    while (!Serial)
      ;
  }

  Serial.println("Before camera begin");
  // Init the cam QVGA, 30FPS
  if (!cam.begin(RESOLUTION, IMAGE_MODE, 30)) {
    blinkLED();
  }

  Serial.println("After camera begin");
  blinkLED(5);

  // Flips the image vertically
  cam.setVerticalFlip(true);

  // Mirrors the image horizontally
  cam.setHorizontalMirror(true);

  // Calculate the max zoom window position
  maxZoomX = cam.getResolutionWidth() - ZOOM_WINDOW_WIDTH;
  maxZoomY = cam.getResolutionHeight() - ZOOM_WINDOW_HEIGHT;

  Serial.print("Camera Width:");
  Serial.print(cam.getResolutionWidth());
  Serial.print(" Height:");
  Serial.println(cam.getResolutionHeight());

  Serial.print("Max Zoom X:");
  Serial.print(maxZoomX);
  Serial.print(" Y:");
  Serial.println(maxZoomY);

  // Set the zoom window to 0,0
  int ret = cam.zoomTo(ZOOM_WINDOW_RESOLUTION, currentZoomX, currentZoomY);
  Serial.print("After zoom:");
  Serial.println(ret);
  Display.begin();

  if (IMAGE_MODE == CAMERA_GRAYSCALE) {
    dsi_configueCLUT((uint32_t*)palette);
  }
  outfb.setBuffer((uint8_t*)SDRAM.malloc(1024 * 1024));

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

    // double the resolution and transpose (rotate by 90 degrees) in the same step
    // this only works if the camera feed is 320x240 and the area where we want to display is 640x480
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
    dsi_drawCurrentFrameBuffer();
  } else {
    blinkLED(20);
  }

  if (Serial.available()) {
    while (Serial.read() != -1) {}

    currentZoomX += ZOOM_X_STEP;

    if (currentZoomX > maxZoomX) {
      currentZoomX = 0;
      currentZoomY += ZOOM_Y_STEP;
      if (currentZoomY > maxZoomY) {
        currentZoomY = 0;
      }
    }
    cam.zoomTo(ZOOM_WINDOW_RESOLUTION, currentZoomX, currentZoomY);
  }
}
