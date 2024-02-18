#include <elapsedMillis.h>
#include "Arduino_H7_Video.h"
#include "ArduinoGraphics.h"

Arduino_H7_Video Display(800, 480, GigaDisplayShield);
//Arduino_H7_Video Display(1024, 768, USBCVideo);

void setup() {
  while (!Serial && millis() < 5000) {}
  Serial.begin(115200);
  Display.begin();
  Serial.println("\nArduino GIGA ArduinoGraphics Speed test");
}

uint32_t fill_screen(uint8_t r, uint8_t g, uint8_t b) {
  elapsedMicros em;
  Display.beginDraw();
  Display.background(r, g, b);
  Display.clear();
  Display.endDraw();
  uint32_t elapsed = em;
  Serial.print("(");
  Serial.print(r, DEC);
  Serial.print(",");
  Serial.print(g, DEC);
  Serial.print(",");
  Serial.print(b, DEC);
  Serial.print("):");
  Serial.print(elapsed);
  delay(500);
  return elapsed;
}

void loop() {
  uint32_t elapsed_sum = fill_screen(255, 0, 0);
  elapsed_sum += fill_screen(0, 255, 0);
  elapsed_sum += fill_screen(0, 0, 255);
  float fps = (3000000.0 / elapsed_sum);
  Serial.print(" fps:");
  Serial.println(fps, 2);
  if (Serial.available()) {
    while (Serial.read() != -1) {}
    Serial.println("Paused");
    while (Serial.read() == -1) {}
    while (Serial.read() != -1) {}

  }
}