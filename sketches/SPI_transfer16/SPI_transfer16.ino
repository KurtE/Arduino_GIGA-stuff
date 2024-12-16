#include <SPI.h>
void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);
    SPI.begin();

    static SPI_TypeDef *DBGSPIx;
    DBGSPIx = (SPI_TypeDef *)0x40013000ul;
    Serial.print("S:");
    Serial.print((uint32_t)DBGSPIx, HEX);
    Serial.print(" ");
    Serial.print(DBGSPIx->CFG1, HEX);
    Serial.print(" ");
    Serial.print(DBGSPIx->CFG2, HEX);
    Serial.print(" ");
    Serial.print(DBGSPIx->CR1, HEX);
    Serial.print(" ");
    Serial.println(DBGSPIx->CR2, HEX);
    delay(50);

    // Check before we do begin transaction.
    pinMode(53, OUTPUT);
    digitalWrite(53, HIGH);
    digitalWrite(53, LOW);
    for (uint8_t i = 0; i < 10; i++) SPI.transfer(i);
    digitalWrite(53, HIGH);
    delay(5);
    digitalWrite(53, LOW);
    for (uint16_t i = 0; i < 64 * 10; i += 64) SPI.transfer16(i);
    digitalWrite(53, HIGH);


    SPI.beginTransaction(SPISettings(30000000, MSBFIRST, SPI_MODE0));
}

uint16_t loop_count = 0;

void loop() {
    loop_count++;
    digitalWrite(53, LOW);
    SPI.transfer(loop_count >> 8);
    SPI.transfer(loop_count & 0xff);
    SPI.transfer16(loop_count);
    digitalWrite(53, HIGH);
    delayMicroseconds(5);
}
