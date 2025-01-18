#include <SPI.h>
#define SD_CS 6
void setup() {
  // put your setup code here, to run once:
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  Serial.begin(115200);
  while (!Serial && millis() < 5000) {}
  Serial.println("Before SPI Begin");
  SPI.begin();
  Serial.println("After");
  pinMode(LED_BUILTIN, OUTPUT);

}

uint8_t buffer[64];
void loop() {
  digitalWrite(SD_CS, LOW);
  digitalWrite(LED_BUILTIN, HIGH);
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  memset(buffer, 's', sizeof(buffer));
  SPI.transfer(buffer, sizeof(buffer));
  digitalWrite(SD_CS, HIGH);
  SPI.endTransaction();
  SPI.beginTransaction(SPISettings(60000000, MSBFIRST, SPI_MODE0));
  digitalWrite(SD_CS, LOW);
  memset(buffer, 'F', sizeof(buffer));
  SPI.transfer(buffer, sizeof(buffer));
  SPI.endTransaction();
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(SD_CS, HIGH);
  SPI.beginTransaction(SPISettings(60000000, MSBFIRST, SPI_MODE0));
  digitalWrite(SD_CS, LOW);
  memset(buffer, '2', sizeof(buffer));
  SPI.transfer(buffer, sizeof(buffer));
  SPI.endTransaction();
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(SD_CS, HIGH);
  delay(100);
}