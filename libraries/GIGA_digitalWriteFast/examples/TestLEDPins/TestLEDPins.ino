#include <RPC.h>
#include <GIGA_digitalWriteFast.h>

Stream *USERIAL = nullptr;

void setup() {
  // put your setup code here, to run once:
  if (HAL_GetCurrentCPUID() == CM7_CPUID) {

    while (!Serial && millis() < 5000) {}
    Serial.begin(115200);
    Serial.println("\n*** Test Led Pins M7 version ***");
    USERIAL = &Serial;
  } else {
    RPC.begin();
    USERIAL = &RPC;
    USERIAL->println("\n*** Test Led Pins M4 version ***");
  }

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(86, OUTPUT);
  pinMode(87, OUTPUT);
  pinMode(88, OUTPUT);
}

void test_pin(const char *name, pin_size_t pin) {
  USERIAL->print("Test Pin by number: ");
  USERIAL->print(name);
  USERIAL->print("(");
  USERIAL->print(pin, DEC);
  USERIAL->println(")");
  for (uint8_t i = 0; i < 2; i++) {
    digitalWriteFast(pin, HIGH);
    delay(250);
    digitalWriteFast(pin, LOW);
    delay(250);
  }
  for (uint8_t i = 0; i < 4; i++) {
    digitalToggleFast(pin);
    delay(500);
  }
  digitalWriteFast(pin, HIGH);
}

void test_pin(const char *name, PinName pin) {
  USERIAL->print("Test Pin by name: ");
  USERIAL->print(name);
  USERIAL->print("(");
  USERIAL->print(pin, DEC);
  USERIAL->print(" P");
  uint8_t port_name = ((pin >> 4) & 0xf) + 'A';
  USERIAL->write(port_name);
  USERIAL->write('_');
  USERIAL->print(pin & 0xf, DEC);

  USERIAL->println(")");

  for (uint8_t i = 0; i < 2; i++) {
    digitalWriteFast(pin, HIGH);
    delay(250);
    digitalWriteFast(pin, LOW);
    delay(250);
  }
  for (uint8_t i = 0; i < 4; i++) {
    digitalToggleFast(pin);
    delay(500);
  }
  digitalWriteFast(pin, HIGH);
}


void loop() {
  // put your main code here, to run repeatedly:
  test_pin("LED_BUILTIN", LED_BUILTIN);
  test_pin("LED_RED", LED_RED);
  test_pin("LED_GREEN", LED_GREEN);
  test_pin("LED_BLUE", LED_BLUE);
  test_pin("86", 86);
  test_pin("87", 87);
  test_pin("88", 88);
  test_pin("D86", D86);
  test_pin("D87", D87);
  test_pin("D88", D88);
  test_pin("PI_12", PI_12);  // RED
  test_pin("PJ_13", PJ_13);
  test_pin("PE_3", PE_3);

}
