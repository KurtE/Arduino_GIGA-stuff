#include "SDRAM.h"
SDRAMClass ram;

void print_gpio_regs(const char *name, GPIO_TypeDef* port) {
  printk("GPIO%s(%p) %08X %08X %08x\n", name, port, port->MODER, port->AFR[0], port->AFR[1]);
  Serial.print("GPIO"); Serial.print(name);
  Serial.print(" "); Serial.print(port->MODER, HEX); 
  Serial.print(" "); Serial.print(port->AFR[0], HEX); 
  Serial.print(" "); Serial.println(port->AFR[1], HEX); 
} 

void setup() {
  printk("Setup called\n");
    Serial.begin(115200);
  while (!Serial)
    ;
  uint8_t *b;
  Serial.println("Before Ram begin");
  Serial.flush();
  printk("before Ram begin\n");
  print_gpio_regs("A", GPIOA);
  print_gpio_regs("B", GPIOB);
  print_gpio_regs("C", GPIOC);
  print_gpio_regs("D", GPIOD);
  print_gpio_regs("E", GPIOE);
  print_gpio_regs("F", GPIOF);
  print_gpio_regs("G", GPIOG);
  print_gpio_regs("H", GPIOH);
  print_gpio_regs("I", GPIOI);
  print_gpio_regs("J", GPIOJ);
  print_gpio_regs("K", GPIOK);

  ram.begin();
  printk("after Ram begin\n");
  Serial.println("After Ram begin");
  Serial.flush();
  b = (uint8_t *)ram.malloc(320 * 240 * sizeof(uint8_t));
  Serial.println("After malloc begin");
  Serial.flush();
}

void loop() {

}