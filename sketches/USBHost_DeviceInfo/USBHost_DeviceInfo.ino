#include <Arduino_USBHostMbed5.h>
#include <LibPrintf.h>
#include "USBDumperDevice.h"

REDIRECT_STDOUT_TO(Serial)

USBDumperDevice hser;

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 5000) {}

  Serial.println("Starting USB host Serial device test...");

  // Enable the USBHost
  pinMode(PA_15, OUTPUT);
  digitalWrite(PA_15, HIGH);

  while (!hser.connect()) {
    Serial.println("No USB host device connected");
    delay(5000);
  }

  printf("USB host device(%x:%x) connected\n\r", hser.idVendor(), hser.idProduct());

  uint8_t string_buffer[80];
  if (hser.manufacturer(string_buffer, sizeof(string_buffer))) {
    Serial.print("Manufacturer: ");
    Serial.println((char*)string_buffer);
  }

  if (hser.product(string_buffer, sizeof(string_buffer))) {
    Serial.print("Product: ");
    Serial.println((char*)string_buffer);
  }
  if (hser.serialNumber(string_buffer, sizeof(string_buffer))) {
    Serial.print("Serial Number: ");
    Serial.println((char*)string_buffer);
  }


}

void loop() {
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  delay(1000);
}
