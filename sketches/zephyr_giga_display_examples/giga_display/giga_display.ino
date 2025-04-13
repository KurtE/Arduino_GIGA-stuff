#include "Arduino.h"
#include "Arduino_Giga_display.h"

Display display;

uint8_t frameBuffer2[2 * 160 * 160] __attribute__((aligned(32)));
uint32_t sizeof_framebuffer2 = sizeof(frameBuffer2);

uint16_t color_table[] = {
  RGB565_RED, RGB565_MAGENTA, RGB565_YELLOW, RGB565_BLACK, RGB565_GREEN,
  RGB565_WHITE, RGB565_ORANGE, RGB565_GREENYELLOW, RGB565_PINK
};
uint8_t color_index = 0;

void fillScreen(uint16_t color) {
  for (int x = 0; x < 480; x += 160) {
    for (int y = 0; y < 800; y += 160) {
      uint16_t *pb = (uint16_t *)frameBuffer2;
      uint32_t count = sizeof_framebuffer2 / 2;
      while(count--) *pb++ = color;
      display.write8(x, y, frameBuffer2);
    }
  }
}

void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  uint8_t frameBuffer2[2 * w * h] __attribute__((aligned(32)));
  uint32_t sizeof_framebuffer2 = sizeof(frameBuffer2);

  uint16_t *pb = (uint16_t *)frameBuffer2;
  uint32_t count = sizeof_framebuffer2 / 2;

  for (int x1 = 0; x1 < w; x1 += w) {
    for (int y1 = 0; y1 < w; y1 += h) {
      while(count--) *pb++ = color;
      display.write8(x+x1, y+y1, frameBuffer2);
    }
  }

}

void setup() {
  while (!Serial && millis() < 5000)
    ;
  Serial.begin(115200);

  if (!display.begin()) {
    Serial.println("Failed to start display");
  };

  Serial.println("Display configured!!");

  //uint8_t color = 0x11u;
  //memset(frameBuffer2, color, sizeof_framebuffer2);

  //display.setFrameDesc(160, 160, 160, sizeof_framebuffer2);
  display.startFrameBuffering();

  fillScreen(RGB565_DARKGREY);

  for (int x = 0; x < 320; x += 160) {
    for (int y = 0; y < 480; y += 160) {
      uint16_t *pb = (uint16_t *)frameBuffer2;
      uint32_t count = sizeof_framebuffer2 / 2;
      while(count--) *pb++ = color_table[color_index];
      display.write8(x, y, frameBuffer2);
      color_index++;
      if (color_index == (sizeof(color_table)/sizeof(color_table[0]))) color_index = 0;
    }
  }

  delay(2000);

  fillScreen(RGB565_DARKGREY);

  display.endFrameBuffering();

  delay(2000);

  display.startFrameBuffering();
  fillScreen(RGB565_DARKGREY);
  //fillRect(160, 160, 160, 160, RGB565_RED);
  display.endFrameBuffering();
}

void loop() {
  // put your main code here, to run repeatedly:
}
