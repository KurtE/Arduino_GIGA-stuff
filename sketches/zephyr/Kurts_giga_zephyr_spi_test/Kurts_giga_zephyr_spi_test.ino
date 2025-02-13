#include <SPI.h>
#define WSPI SPI
uint8_t CS = 10;
#include "simple_spi_user_class.h"
SimpleSPIUserClass *pspic = nullptr;
void setup() {
    // put your setup code here, to run once:
    while (!Serial && millis() < 4000) {}
    Serial.begin(115200);
    Serial.println("after Serial begin");
    WSPI.begin();
    Serial.println("after SPIx begin");
    WSPI.beginTransaction(SPISettings(24000000, MSBFIRST, SPI_MODE0));
    Serial.println("After begin Transaction");
    pinMode(CS, OUTPUT);
    digitalWrite(CS, HIGH);

    for (uint8_t i = 0; i < 5; i++) {
      digitalWrite(CS, LOW);
      digitalWrite(CS, HIGH);
    }
    pspic = new SimpleSPIUserClass(&WSPI);
}

void loop() {
    static uint8_t loop_count = 0;
    delay(250);
    loop_count++;
    digitalWrite(CS, LOW);
    pspic->transfer(loop_count);
    digitalWrite(CS, HIGH);

}