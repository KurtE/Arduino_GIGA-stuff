#include <elapsedMillis.h>
#define USBHOST_OTHER
#include <Arduino_USBHostMbed5.h>
REDIRECT_STDOUT_TO(Serial)

USBHostSerial userial;

void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  Serial.println("Starting Serial test...");

  // Enable the USBHost
  pinMode(PA_15, OUTPUT);
  digitalWrite(PA_15, HIGH);

  elapsedMillis em;
  while (!userial.connect()) {
    if (em > 5000 ) {
      Serial.println("No USB Serial device connected");
      em = 0;
    }
    delay(500);
  }
  Serial.println("Serial connected");
}

void loop() {
  // print characters received
  while (userial.available()) {
    Serial.write(userial.getc());
  }

  delay(1000);
}
