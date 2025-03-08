#include <Arduino.h>
#include "PinNames.h"
void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 5000) {}
  if (Serial) {
    Serial.print("Red Pin: ");
    Serial.println(PinNameToIndex(LED_RED));
    Serial.print("Green Pin: ");
    Serial.println(PinNameToIndex(LED_GREEN));
    Serial.print("Blue Pin: ");
    Serial.println(PinNameToIndex(LED_BLUE));

    for (uint8_t i = 0; i < 26; i++) {
      PinName pn = digitalPinToPinName(i);
      if (pn != NC) {
        Serial.print("\t");
        Serial.print(i);
        Serial.print(" = P");
        Serial.write('A' + ((uint8_t)pn >> 4));
        Serial.write('_');
        Serial.println((uint8_t)pn & 0xf);
      }
    }
  }
  pinMode(PJ_11, INPUT_PULLUP);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
}

uint8_t loop_count = 0;
PinStatus pj11_state_prev = HIGH;
void loop() {
  loop_count = (loop_count + 1) & 0x7;
  digitalWrite(LED_RED, (loop_count & 1) ? HIGH : LOW);
  digitalWrite(LED_BLUE, (loop_count & 2) ? HIGH : LOW);
  digitalWrite(LED_GREEN, (loop_count & 4) ? HIGH : LOW);
#if 1
  PinStatus pj11_state = digitalRead(PJ_11);
  if (pj11_state != pj11_state_prev) {
    pj11_state_prev = pj11_state;
    Serial.print("PJ_11(D2): ");
    Serial.println(pj11_state? "HIGH" : "LOW");
  }
  #endif
  delay(250);
}
