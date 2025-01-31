#include <SPI.h>
#define CS_PIN 6
void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 5000) {}
  SPI.begin();
  SPI.beginTransaction(SPISettings(3000000, MSBFIRST, SPI_MODE0));
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  digitalWrite(CS_PIN, LOW);
  digitalWrite(LED_BUILTIN, HIGH);
  for(uint8_t i = 'a'; i <='z'; i++) SPI.transfer(i);
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(CS_PIN, HIGH);
  delay(50);
}
