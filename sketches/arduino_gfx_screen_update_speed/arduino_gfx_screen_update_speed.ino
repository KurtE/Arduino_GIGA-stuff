#include <elapsedMillis.h>
#include "Arduino_GigaDisplay_GFX.h"
#define USE_SDRAM
//#define USE_ARDUCAM_DVP
REDIRECT_STDOUT_TO(Serial)


#ifdef USE_SDRAM
#include "SDRAM.h"
uint16_t *fb_mem;
#endif

GigaDisplay_GFX display;
#define GC9A01A_CYAN 0x07FF
#define GC9A01A_RED 0xf800
#define GC9A01A_BLUE 0x001F
#define GC9A01A_GREEN 0x07E0
#define GC9A01A_MAGENTA 0xF81F
#define GC9A01A_WHITE 0xffff
#define GC9A01A_BLACK 0x0000
#define GC9A01A_YELLOW 0xFFE0



void setup() {
  while (!Serial && millis() < 5000) {}
  Serial.begin(115200);
  display.begin();
  Serial.println("\nArduino GIGA GFX Speed test");

#ifdef USE_SDRAM
  SDRAM.begin();
  fb_mem = (uint16_t *)SDRAM.malloc(800 * 480);
#endif
}


#define WIDTH 480
#define HEIGHT 800

void writeRect(GigaDisplay_GFX *pdisp, int16_t x, int16_t y, const uint16_t bitmap[],
               int16_t w, int16_t h) {
  uint8_t rotation = pdisp->getRotation();
  uint16_t *display_buffer = pdisp->getBuffer();


  if (rotation == 1) {
    pdisp->startWrite();
    // If yScreen = XIn and XScreen = WIDTH - 1 - yIn
    // Then xIn = yScreen and yIn = xScreen
    int yScreenStart = x;
    int xScreenEnd = (WIDTH - 1) - y;
    int yScreenEnd = x + w - 1;
    int xScreenStart = xScreenEnd - h + 1;
    //    printf("\nx:%d, y:%d w:%d, h:%d -> (%d %d) (%d %d)\n", x,y, w, h, xScreenStart, xScreenEnd, yScreenStart, yScreenEnd);

    if (xScreenStart < 0) xScreenStart = 0;
    if (yScreenStart < 0) yScreenStart = 0;
    if (xScreenEnd >= WIDTH) xScreenStart = WIDTH - 1;
    if (yScreenEnd >= HEIGHT) yScreenStart = HEIGHT - 1;
    //    printf("\t-> (%d %d) (%d %d)\n", xScreenStart, xScreenEnd, yScreenStart, yScreenEnd);
    // lets update the display buffer pointer to the first pixel we are updating
    display_buffer += (yScreenStart * WIDTH) + xScreenStart;
    const uint16_t *pbitRowStart = bitmap + ((((WIDTH - 1) - xScreenStart) - x) * w + (yScreenStart - y));
    for (int ys = yScreenStart; ys <= yScreenEnd; ys++) {
      //int xBitmap = ys - y;
      const uint16_t *pbit = pbitRowStart;
      uint16_t *bScreen = display_buffer;
      for (int xs = xScreenStart; xs <= xScreenEnd; xs++) {
        //int yBitmap = ((WIDTH - 1) - xs) - x;
        //*bScreen++ = bitmap[yBitmap * w + xBitmap];
        *bScreen++ = *pbit;
        pbit -= w;
      }
      display_buffer += WIDTH;
      pbitRowStart++;
    }
    pdisp->endWrite();
#if 0    
  } else  if (rotation == 3) {
    // rotation 3
    // xScreen = yIn ; and yScreen = HEIGHT - 1 - xIn

    int yScreenEnd = (HEIGHT - 1 - x;
    int xScreenStart = y;
    int yScreenStart = yScreenEnd - w;
    int xScreenStart = y + h;

    if (xScrenStart < 0) xScrenStart = 0;
    if (yScrenStart < 0) yScrenStart = 0;
    if (xScreenEnd >= WIDTH) xScrenStart = WIDTH - 1;
    if (yScreenEnd >= HEIGHT) yScrenStart = HEIGHT - 1;
    for (ys = yScreenStart; ys <= yScreenEnd; ys++) {
      int xBitmap = ys - y;
      for (xs = xScreenStart; xs <= xScreenEnd; xs++) {
        int yBitmap = ((WIDTH - 1) - xs) - x;
        pdisp->drawPixel(xs, ys);
      }
    }

#endif
  } else {
    pdisp->drawRGBBitmap(0, 0, bitmap, w, h);
  }
}


uint32_t fill_screen(uint16_t color) {
#ifdef USE_SDRAM
  for (int i = 0; i < (800 * 480); i++) fb_mem[i] = color;
  elapsedMicros em;
  //display.drawRGBBitmap(0, 0, fb_mem, display.width(), display.height());
  writeRect(&display, 0, 0, fb_mem, display.width(), display.height());
#else
  elapsedMicros em;
  display.fillScreen(color);
#endif
  uint32_t elapsed = em;
  Serial.print("(");
  Serial.print(color, HEX);
  Serial.print("):");
  Serial.print(elapsed);
  delay(500);
  return elapsed;
}

uint8_t screen_rotation = 0;
void loop() {
  display.setRotation(screen_rotation);
  Serial.print("Rot:");
  Serial.print(screen_rotation);
  uint32_t elapsed_sum = fill_screen(GC9A01A_RED);
  elapsed_sum += fill_screen(GC9A01A_GREEN);
  elapsed_sum += fill_screen(GC9A01A_BLUE);
  float fps = (3000000.0 / elapsed_sum);
  Serial.print(" fps:");
  Serial.println(fps, 2);
  if (Serial.available()) {
    while (Serial.read() != -1) {}
    Serial.println("Paused");
    while (Serial.read() == -1) {}
    while (Serial.read() != -1) {}
  }
  screen_rotation = (screen_rotation + 1) & 0x3;
}
