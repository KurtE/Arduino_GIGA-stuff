#include "Arduino.h"
#include "Arduino_Giga_display.h"

Display display;

uint16_t frameBuffer2[160 * 160] __attribute__((aligned(32)));
uint32_t sizeof_framebuffer2 = sizeof(frameBuffer2);

inline uint16_t HTONS(uint16_t x) {
    return ((x >> 8) & 0x00FF) | ((x << 8) & 0xFF00);
}


void setup() {
  while(!Serial && millis() < 5000);
  Serial.begin(115200);

  if(!display.begin(DISPLAY_RGB565, 2)) {
    Serial.println("Failed to start display");
    while(1) {}
  };

  Serial.println("Display configured!!");


  //uint16_t color = 0xe070;
  memset(frameBuffer2, RED, sizeof_framebuffer2);

  display.setFrameDesc(160, 160, 160, sizeof_framebuffer2);
  display.setFrameComplete(true);

  //for (int idx1 = 0; idx1 < 160; idx1 += 1) {
    for (int idx = 0; idx < 160; idx += 1) {
      display.write8(0, idx, frameBuffer2);
    }
 //}

  display.setFrameComplete(false);

}

void loop() {
  // put your main code here, to run repeatedly:

}
