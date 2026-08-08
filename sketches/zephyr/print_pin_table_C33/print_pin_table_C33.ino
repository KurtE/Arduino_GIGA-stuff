/*
 * Enumerate and print the addresses of all GPIO devices
 * for an STM32 board in Zephyr RTOS.
 *
 * This works for boards with "st,stm32-gpio" compatible nodes.
 * currently trying renesas... 
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include "wiring_private.h"
using namespace zephyr::arduino;


static const char *pin_names[] = {
  // clang-format off
    "P0_0", "P0_1", "P0_2", "P0_3","P0_4", "P0_5", "P0_6", "P0_7", 
    "P0_8", "P0_9", "P0_10", "P0_11", "P0_12", "P0_13", "P0_14", "P0_15", 
    "P1_0", "P1_1", "P1_2", "P1_3", "P1_4", "P1_5", "P1_6", "P1_7", 
    "P1_8", "P1_9", "P1_10", "P1_11", "P1_12", "P1_13", "P1_14", "P1_15",
    "P2_0", "P2_1", "P2_2", "P2_3", "P2_4","P2_5", "P2_6", "P2_7", 
    "P2_8", "P2_9", "P2_10", "P2_11", "P2_12", "P2_13", "P2_14", "P2_15",
    "P3_0", "P3_1", "P3_2", "P3_3", "P3_4", "P3_5", "P3_6", "P3_7", 
    "P3_8", "P3_9", "P3_10", "P3_11", "P3_12", "P3_13", "P3_14", "P3_15", 
    "P4_0", "P4_1", "P4_2", "P4_3", "P4_4", "P4_5", "P4_6", "P4_7", 
    "P4_8", "P4_9", "P4_10", "P4_11", "P4_12", "P4_13", "P4_14", "P4_15", 
    "P5_0", "P5_1", "P5_2", "P5_3", "P5_4", "P5_5", "P5_6", "P5_7", 
    "P5_8", "P5_9", "P5_10", "P5_11","P5_12", "P5_13", "P5_14", "P5_15", 
    "P6_0", "P6_1", "P6_2", "P6_3", "P6_4", "P6_5", "P6_6","P6_7", 
    "P6_8", "P6_9", "P6_10", "P6_11", "P6_12", "P6_13", "P6_14", "P6_15", 
    "P7_0", "P7_1", "P7_2", "P7_3", "P7_4", "P7_5", "P7_6", "P7_7", 
    "P7_8", "P7_9", "P7_10", "P7_11","P7_12", "P7_13", "P7_14", "P7_15", 
    "P8_0", "P8_1", "P8_2", "P8_3", "P8_4", "P8_5", "P8_6","P8_7", 
    "P8_8", "P8_9", "P8_10", "P8_11", "P8_12", "P8_13", "P8_14", "P8_15", 
    "P9_0", "P9_1", "P9_2", "P9_3", "P9_4", "P9_5", "P9_6", "P9_7", 
    "P9_8", "P9_9", "P9_10", "P9_11", "P9_12","P9_13", "P9_14", "P9_15", 
    "PA_0", "PA_1", "PA_2", "PA_3", "PA_4", "PA_5", "PA_6", "PA_7",
    "PA_8", "PA_9", "PA_10", "PA_11", "PA_12","PA_13", "PA_14", "PA_15", 
    "PB_0", "PB_1", "PB_2", "PB_3", "PB_4", "PB_5", "PB_6", "PB_7",
    "PB_8", "PB_9", "PB_10", "PB_11", "PB_12","PB_13", "PB_14", "PB_15",

  // clang-format on
};


/* Macro to iterate over all GPIO nodes in the devicetree */
#define ADD_GPIO_NODE(node_id) DEVICE_DT_GET(node_id),

const struct device *zephyr_gpio_ports[] = {
#if defined(STM32H7) || defined(STM32U5)
  DT_FOREACH_STATUS_OKAY(st_stm32_gpio, ADD_GPIO_NODE)
#elif defined(ARDUINO_PORTENTA_C33)
  DT_FOREACH_STATUS_OKAY(renesas_ra_gpio_ioport, ADD_GPIO_NODE)
#endif
};

#define COUNT_zephyr_gpio_ports (sizeof(zephyr_gpio_ports) / sizeof(zephyr_gpio_ports[0]))


uint8_t mapPinNameToPin(uint8_t pin_name) {
  const struct device *port_device = zephyr_gpio_ports[pin_name >> 4];
  uint8_t pin_on_port = pin_name & 0xf;

  for (uint8_t pin_num = 0; pin_num < NUM_OF_DIGITAL_PINS; pin_num++) {
    if ((arduino_pins[pin_num].port == port_device) && (arduino_pins[pin_num].pin == pin_on_port)) {

      return pin_num;
    }
  }

  return 0xff;  // pin name not in Arduino Pin list
}

uint8_t mapPinToPinName(uint8_t pin) {

  // now find this port in our Port list;
  for (uint8_t i = 0; i < COUNT_zephyr_gpio_ports; i++) {
    if (arduino_pins[pin].port == zephyr_gpio_ports[i]) {
      return (uint8_t)((i << 4) | arduino_pins[pin].pin);
    }
  }
  return 0xff;
}


void setup(void) {
  Serial.begin(115200);
  while (!Serial && millis() < 5000) {}
  delay(5000);
  Serial.println("Enumerating GPIO devices...");

  // Lets print a condensed GPIO Table
  Serial.println("\n*** GPIO PORT/Pin To Arduino Pin numbers mapping ***");
  Serial.println("\nPORTX: 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15");
  Serial.print("====== == == == == == == == == == == == == == == == ==");

  for (uint8_t i = 0; i < (COUNT_zephyr_gpio_ports * 16); i++) {
    uint8_t pin_number = mapPinNameToPin(i);
    if ((i & 0xf) == 0) {
      Serial.print("\nPORT");
      Serial.print((i >> 4), HEX);
      Serial.print(":");
    }
    Serial.print(" ");
    if (pin_number == 0xff) Serial.print("--");
    else {
      if (pin_number < 10) Serial.print(" ");
      Serial.print(pin_number);
    }
  }
  Serial.println();

  Serial.println("\n*** Arduino Pin To Pin Name mapping table ***");
  Serial.flush();
  Serial.println("\nPin   0     1     2     3     4     5     6     7     8     9");
  Serial.print    ("=== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====");

  // for the fun of it print out pin to PinName map as well
  for (uint8_t i = 0; i < NUM_OF_DIGITAL_PINS; i++) {
    if ((i % 10) == 0) {
      Serial.print("\n");
      if (i < 10) Serial.print("  ");
      else if (i < 100) Serial.print(" ");
      Serial.print(i);
    }

    uint8_t pn = mapPinToPinName(i);
    if (pn == 0xff) {
      Serial.print(" -----");
    } else {
      const char *sz = pin_names[pn];
      Serial.print(" ");
      Serial.print(sz);
      if (strlen(sz) < 5) Serial.print(" ");
    }
  }
  Serial.println("\n");




}


void loop() {
}
