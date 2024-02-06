#include <Arduino_USBHostMbed5.h>
#include <LibPrintf.h>
#include "USBHostGamepad.h"

USBHostGamepad gamepad;


void onGamepadEvent(uint8_t x, uint8_t y, uint8_t z, uint8_t rz, uint16_t buttons) {
    printf("x: %02X, y: %02X, z: %02X, rz: %02X, buttons: %04X\r\n", x, y, z, rz, buttons);
}

void setup()
{
    Serial.begin(115200);
    while (!Serial) {}

    Serial.println("Starting Gamepad test...");

    // Enable the USBHost 
    pinMode(PA_15, OUTPUT);
    digitalWrite(PA_15, HIGH);

    while (!gamepad.connect()) {
      Serial.println("No gamepad connected");        
        delay(5000);
    }

    Serial.println("Gamepad connected");
    gamepad.attachEvent(onGamepadEvent);

}

void loop()
{

}