#include "Arduino.h"
#include "GFX.h"

GigaDisplay_GFX tft;

unsigned long  testTriangles();

void setup() {
  Serial.begin(115200);
  tft.fillScreen(RGB565_BLACK);
  delay(3000);

}

void loop() {
  
  Serial.println(millis());
  tft.fillScreen(RGB565_RED);
  tft.startBuffering();
  tft.fillRect(120, 180, 120, 180, RGB565_BLUE);
  tft.endBuffering();

  tft.print(false);
  delay(2000);

  Serial.print(F("Triangles (outline)      "));
  Serial.println(testTriangles());
  delay(2000);
}

unsigned long testTriangles() {
  unsigned long start;
  int           n, i, cx = tft.width()  / 2 - 1,
                      cy = tft.height() / 2 - 1;

  tft.fillScreen(RGB565_BLACK);
  n     = min(cx, cy);
  start = micros();
  tft.startBuffering();
  for (i = 0; i < n; i += 5) {
    tft.drawTriangle(
      cx    , cy - i, // peak
      cx - i, cy + i, // bottom left
      cx + i, cy + i, // bottom right
      tft.color565(i, i, i));
  }
  tft.endBuffering();

  return micros() - start;
}
