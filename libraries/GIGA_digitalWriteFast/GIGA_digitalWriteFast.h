/*
 *    UNOR4_digitalWriteFast.h - A quick and dirty digitalWriteFast
 *    and digitalToggleFast for Arduino UNO R4.  There are better
 *    versions out there, but this good enough for my testing
 * 
 *    Permission is hereby granted, free of charge, to any person
 *    obtaining a copy of this software and associated documentation
 *    files (the "Software"), to deal in the Software without
 *    restriction, including without limitation the rights to use,
 *    copy, modify, merge, publish, distribute, sublicense, and/or sell
 *    copies of the Software, and to permit persons to whom the
 *    Software is furnished to do so, subject to the following
 *    conditions:
 * 
 *    This permission notice shall be included in all copies or 
 *    substantial portions of the Software.
 * 
 *    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 *    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *    OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef _GIGA_DIGITALWRITEFAST_H_
#define _GIGA_DIGITALWRITEFAST_H_
#include <Arduino.h>
#include "pinDefinitions.h"
#if !defined(ARDUINO_GIGA) && !defined(ARDUINO_PORTENTA_H7_M7)

#error "Only works on Arduino GIGA or Portenta H7 boards"
#endif


static  GPIO_TypeDef * const port_table[] = { GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, GPIOG, GPIOH, GPIOI, GPIOJ, GPIOK };
static const uint16_t mask_table[] = { 1 << 0, 1 << 1, 1 << 2, 1 << 3, 1 << 4, 1 << 5, 1 << 6, 1 << 7,
                                       1 << 8, 1 << 9, 1 << 10, 1 << 11, 1 << 12, 1 << 13, 1 << 14, 1 << 15 };

// quick and dirty digitalWriteFast

// Sets the state of an IO pin
// Two versions - this version you takes in a pin number as pin_size_t type - uint8_t
static inline void digitalWriteFast(pin_size_t pin, PinStatus val) __attribute__((always_inline, unused));
static inline void digitalWriteFast(pin_size_t pin, PinStatus val) {
  PinName pin_name = g_APinDescription[pin].name;
  uint16_t mask = mask_table[pin_name & 0xf];
  GPIO_TypeDef  * const port = port_table[pin_name >> 4];
  if (val) port->BSRR = mask;
  else port->BSRR = (uint32_t)(mask << 16);
}

// This version you takes in a pin name (PinName) like LED_RED
static inline void digitalWriteFast(PinName pin_name, PinStatus val) __attribute__((always_inline, unused));
static inline void digitalWriteFast(PinName pin_name, PinStatus val) {
  uint16_t mask = mask_table[pin_name & 0xf];
  GPIO_TypeDef  * const port = port_table[pin_name >> 4];
  if (val) port->BSRR = mask;
  else port->BSRR = (uint32_t)(mask << 16);
}



// Toggles the state of an IO pin - pin number version
static inline void digitalToggleFast(pin_size_t pin) __attribute__((always_inline, unused));
static inline void digitalToggleFast(pin_size_t pin) {
  PinName pin_name = g_APinDescription[pin].name;
  uint16_t pin_mask = mask_table[pin_name & 0xf];
  GPIO_TypeDef  * const portX = port_table[pin_name >> 4];

  if (portX->ODR & pin_mask) portX->BSRR = (uint32_t)(pin_mask << 16);
  else portX->BSRR = pin_mask;
}

// Toggles the state of an IO pin - pin name version
static inline void digitalToggleFast(PinName pin_name) __attribute__((always_inline, unused));
static inline void digitalToggleFast(PinName pin_name) {
  uint16_t pin_mask = mask_table[pin_name & 0xf];
  GPIO_TypeDef  * const portX = port_table[pin_name >> 4];

  if (portX->ODR & pin_mask) portX->BSRR = (uint32_t)(pin_mask << 16);
  else portX->BSRR = pin_mask;
}


// Reads the state of an IO pin - pin number version
static inline uint16_t digitalReadFast(pin_size_t pin) __attribute__((always_inline, unused));
static inline uint16_t digitalReadFast(pin_size_t pin) {
  PinName pin_name = g_APinDescription[pin].name;
  uint16_t pin_mask = mask_table[pin_name & 0xf];
  GPIO_TypeDef  * const portX = port_table[pin_name >> 4];

  return (portX->IDR & pin_mask);
}

// Reads the state of an IO pin - pin name version
static inline uint16_t digitalReadFast(PinName pin_name) __attribute__((always_inline, unused));
static inline uint16_t digitalReadFast(PinName pin_name) {
  uint16_t pin_mask = mask_table[pin_name & 0xf];
  GPIO_TypeDef  * const portX = port_table[pin_name >> 4];

  return (portX->IDR & pin_mask);
}


#endif