#include <GIGA_digitalWriteFast.h>
//#include "pinDefinitions.h"

#define PIN 2

//static inline void digitalWriteFast(uint8_t pin, PinStatus val) __attribute__((always_inline, unused));


void setup() {
  Serial.begin(115200);
  Serial.println("\n\nTest");
  while (!Serial && millis() < 5000)
    ;

  pinMode(PIN, OUTPUT);
}


void do_digitalWrite() {

  uint32_t start_time = micros();
  for (int i = 0; i < 1000; i++) {
    digitalWrite(PIN, HIGH);
    digitalWrite(PIN, LOW);
  }
  uint32_t delta_time = micros() - start_time;
  Serial.print("digitalWrite: ");
  Serial.println(delta_time, DEC);
}

void do_digitalWriteFast() {
  uint32_t start_time = micros();
  for (int i = 0; i < 1000; i++) {
    digitalWriteFast(PIN, HIGH);
    digitalWriteFast(PIN, LOW);
  }
  uint32_t delta_time = micros() - start_time;
  Serial.print("digitalWriteFast: ");
  Serial.println(delta_time, DEC);
}

void do_digitalToggleFast() {
  uint32_t start_time = micros();
  for (int i = 0; i < 1000; i++) {
    digitalToggleFast(PIN);
    digitalToggleFast(PIN);
  }
  uint32_t delta_time = micros() - start_time;
  Serial.print("digitalToggleFast: ");
  Serial.println(delta_time, DEC);
}


void loop() {
  do_digitalWrite();
  do_digitalWriteFast();
  do_digitalToggleFast();
  delay(1000);
}

#if 0
static  GPIO_TypeDef * const port_table[] = { GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOE, GPIOF, GPIOG, GPIOH, GPIOI, GPIOJ, GPIOK };
static const uint16_t mask_table[] = { 1 << 0, 1 << 1, 1 << 2, 1 << 3, 1 << 4, 1 << 5, 1 << 6, 1 << 7,
                                       1 << 8, 1 << 9, 1 << 10, 1 << 11, 1 << 12, 1 << 13, 1 << 14, 1 << 15 };
static inline void digitalWriteFast(pin_size_t pin, PinStatus val) {
  PinName hardware_port_pin = g_APinDescription[pin].name;
  //uint16_t mask = 1 << (hardware_port_pin & 0xf);
  uint16_t mask = mask_table[hardware_port_pin & 0xf];
  GPIO_TypeDef  * const port = port_table[hardware_port_pin >> 8];
  if (val) port->BSRR = mask;
  else port->BSRR = (uint32_t)(mask << 16);
}
#endif