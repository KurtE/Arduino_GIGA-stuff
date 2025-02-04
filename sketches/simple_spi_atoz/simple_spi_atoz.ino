#include <SPI.h>
#define SPIX SPI1
#define CS_PIN 10
void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 5000) {}
  SPIX.begin();
  SPIX.beginTransaction(SPISettings(3000000, MSBFIRST, SPI_MODE0));
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  digitalWrite(CS_PIN, LOW);
  digitalWrite(LED_BUILTIN, HIGH);
  for(uint8_t i = 'a'; i <='f'; i++) SPIX.transfer(i);
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(CS_PIN, HIGH);
  delay(50);
  Serial.print("paused");
  while(Serial.read() == -1){}
  while(Serial.read() != -1) {}

}
