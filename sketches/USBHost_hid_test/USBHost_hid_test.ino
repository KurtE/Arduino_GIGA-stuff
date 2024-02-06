/*
  USBHost Keyboard test


  The circuit:
   - Arduino GIGA

  This example code is in the public domain.
*/

#include <Arduino_USBHostMbed5.h>
#include "USBHostKeyboardEx.h"
REDIRECT_STDOUT_TO(Serial)

USBHostKeyboardEx kbd;

// If you are using a Portenta Machine Control uncomment the following line
// mbed::DigitalOut otg(PB_14, 0);

void setup()
{
    Serial.begin(115200);
    while (!Serial && millis() < 5000) {}

    Serial.println("Starting Keyboard test...");

    // Enable the USBHost 
    pinMode(PA_15, OUTPUT);
    digitalWrite(PA_15, HIGH);

    // if you are using a Max Carrier uncomment the following line
    // start_hub();

    while (!kbd.connect()) {
      Serial.println("No keyboard connected");        
        delay(5000);
    }

    kbd.attach(&kbd_key_cb);
    kbd.attach(&kbd_keycode_cb);
    kbd.attachRelease(&kbd_key_release_cb);
    Serial.println("End of Setup");
}

void loop()
{
    delay(1000);
}

void kbd_key_cb(uint8_t key) {
  Serial.print("Key pressed: ");
  Serial.print(key, HEX);
  Serial.print("(");
  if ((key >= ' ') && (key <= '~')) Serial.write(key);
  Serial.println(")");
}

void kbd_key_release_cb(uint8_t key) {
  Serial.print("Key released: ");
  Serial.print(key, HEX);
  Serial.print("(");
  if ((key >= ' ') && (key <= '~')) Serial.write(key);
  Serial.println(")");
}


void kbd_keycode_cb(uint8_t keycode, uint8_t mod) {
  Serial.print("Keycode: ");
  Serial.print(keycode, HEX);
  Serial.print(" mod: ");
  Serial.println(mod, HEX);
}
