#include <Arduino_USBHostMbed5.h>
#include <LibPrintf.h>
#include "USBHostSerialDevice.h"

REDIRECT_STDOUT_TO(Serial)

USBHostSerialDevice hser;


void onhserEvent(uint8_t x, uint8_t y, uint8_t z, uint8_t rz, uint16_t buttons) {
    printf("x: %02X, y: %02X, z: %02X, rz: %02X, buttons: %04X\r\n", x, y, z, rz, buttons);
}

void setup()
{
    Serial.begin(115200);
    while (!Serial && millis() < 5000) {}

    Serial.println("Starting hser test...");

    // Enable the USBHost 
    pinMode(PA_15, OUTPUT);
    digitalWrite(PA_15, HIGH);

    while (!hser.connect()) {
      Serial.println("No hser connected");        
        delay(5000);
    }

    Serial.println("hser connected try begin");
    hser.begin(4800);
//    hser.attachEvent(onhserEvent);

}

void loop()
{

}