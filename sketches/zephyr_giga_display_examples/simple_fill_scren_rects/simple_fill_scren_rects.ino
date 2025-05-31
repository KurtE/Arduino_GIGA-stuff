#include "Arduino_GigaDisplay.h"
Display display;

uint16_t *initialFB;

void setup() {
  while(!Serial && millis()< 5000){}
  Serial.begin(9600);
  Serial.println("Zephyr simple Giga Display Test!");

  display.begin();
  display.setFrameDesc(480, 800, 480, 480*800*2);
  initialFB = (uint16_t*)display.getFrameBuffer();
    pinMode(74, OUTPUT);
    Serial.print("Pin74: ");
    Serial.println(digitalRead(74), DEC);
    digitalWrite(74, HIGH);

}

void fillScreen(uint16_t color) {
  uint16_t *fbCur = (uint16_t*)display.getFrameBuffer();
  uint16_t *fb = (fbCur == initialFB)? &initialFB[480*800] : initialFB;

  for (int i = 0; i < 480*800; i++) fb[i] = color;
  display.write8(0, 0, fb);

}

void loop(void) {
  fillScreen(RGB565_WHITE);
  delay(500);
  fillScreen(RGB565_RED);
  delay(500);
  fillScreen(RGB565_GREEN);
  delay(500);
  fillScreen(RGB565_BLUE);
  delay(500);
  fillScreen(RGB565_BLACK);
  delay(500);
}
