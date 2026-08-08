/*
 * Enumerate and print the addresses of all GPIO devices
 * for an STM32 board in Zephyr RTOS.
 *
 * This works for boards with "st,stm32-gpio" compatible nodes.
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

/* Macro to iterate over all GPIO nodes in the devicetree */
#define ADD_GPIO_NODE(node_id) DEVICE_DT_GET(node_id),

const struct device *zephyr_gpio_ports[] = {
#if defined(STM32H7) || defined(STM32U5)
    DT_FOREACH_STATUS_OKAY(st_stm32_gpio, ADD_GPIO_NODE)
#elif defined(ARDUINO_PORTENTA_C33)
    DT_FOREACH_STATUS_OKAY(renesas_ra_gpio_ioport, ADD_GPIO_NODE)
#include <soc.h>
#include <zephyr/dt-bindings/gpio/renesas-ra-gpio-ioport.h>
//R_PORT0_Type *port_table[] = { R_PORT0, R_PORT1, R_PORT2, R_PORT3, R_PORT4, R_PORT5, R_PORT6, R_PORT7 };
#ifdef R7FA6M5BH_H
#warning "R7FA6M5BH_H is defined"
#endif
#endif
};

#define COUNT_zephyr_gpio_ports (sizeof(zephyr_gpio_ports) / sizeof(zephyr_gpio_ports[0]))
struct gpio_stm32_config_head {
  /* gpio_driver_config needs to be first */
  struct gpio_driver_config common;
  /* port base address */
  uint32_t *base;
};



#define LIST_GPIO_NODE(node_id)                       \
do {                                                  \
  const struct device *dev = DEVICE_DT_GET(node_id);  \
  Serial.print("GPIO device: ");                      \
  Serial.print(dev->name);                            \
  if (device_is_ready(dev)) {                         \
    Serial.print(" Address: 0x");                     \
    Serial.print((uint32_t)dev, HEX);                \
    uintptr_t base = DT_REG_ADDR(node_id);                        \
    Serial.print(" Base addr: 0x");                   \
    Serial.print(base, HEX);                   \
    Serial.println();                                 \
  } else {                                            \
    Serial.println(" NOT READY");                     \
  }                                                   \
} while (0);

  void
  setup(void) {
  Serial.begin(115200);
  while (!Serial && millis() < 5000) {}
  Serial.println("Enumerating GPIO devices...");

  /*
     * DT_FOREACH_STATUS_OKAY takes a compatible string and calls a macro
     * for each matching node that is marked "okay" in the devicetree. q
     */
  //DT_FOREACH_OKAY_st_stm32_gpio(LIST_GPIO_NODE);
#if defined(STM32H7) || defined(STM32U5)
  DT_FOREACH_STATUS_OKAY(st_stm32_gpio, LIST_GPIO_NODE)
#elif defined(ARDUINO_PORTENTA_C33)
  DT_FOREACH_STATUS_OKAY(renesas_ra_gpio_ioport, LIST_GPIO_NODE)
#endif
  Serial.println("Enumeration complete.\n");
    Serial.println("\nPort List:");
    for (uint8_t i = 0; i <COUNT_zephyr_gpio_ports; i++) {
      Serial.print((uint32_t)zephyr_gpio_ports[i], HEX);
      #if defined(STM32H7) || defined(STM32U5)
      Serial.print(" ");

      const struct gpio_stm32_config_head *cfg = (gpio_stm32_config_head*)zephyr_gpio_ports[i]->config;  
      GPIO_TypeDef *portX = (GPIO_TypeDef *)cfg->base;
      Serial.println((uint32_t)portX, HEX);
      #else
      Serial.println();
      #endif
    }

}

void loop() {

}
