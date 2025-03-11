 #include "zephyrInternal.h"
 static const struct gpio_dt_spec arduino_pins[] = {DT_FOREACH_PROP_ELEM_SEP(
   DT_PATH(zephyr_user), digital_pin_gpios, GPIO_DT_SPEC_GET_BY_IDX, (, ))};


  void pinModeAnalog(pin_size_t pinNumber) {
     gpio_pin_configure_dt(&arduino_pins[pinNumber],
                           GPIO_INPUT | GPIO_ACTIVE_HIGH | GPIO_MODE_ANALOG);
 }
 



void print_gpio_regs(const char *name, GPIO_TypeDef *port) {
  //printk("GPIO%s(%p) %08X %08X %08x\n", name, port, port->MODER, port->AFR[0], port->AFR[1]);
  Serial.print("GPIO");
  Serial.print(name);
  Serial.print(" ");
  Serial.print(port->MODER, HEX);
  Serial.print(" ");
  Serial.print(port->AFR[0], HEX);
  Serial.print(" ");
  Serial.println(port->AFR[1], HEX);
}

void show_all_gpio_regs() {
  print_gpio_regs("A", (GPIO_TypeDef *)GPIOA_BASE);
  print_gpio_regs("B", (GPIO_TypeDef *)GPIOB_BASE);
  print_gpio_regs("C", (GPIO_TypeDef *)GPIOC_BASE);
  print_gpio_regs("D", (GPIO_TypeDef *)GPIOD_BASE);
  print_gpio_regs("E", (GPIO_TypeDef *)GPIOE_BASE);
  print_gpio_regs("F", (GPIO_TypeDef *)GPIOF_BASE);
  print_gpio_regs("G", (GPIO_TypeDef *)GPIOG_BASE);
  print_gpio_regs("H", (GPIO_TypeDef *)GPIOH_BASE);
  print_gpio_regs("I", (GPIO_TypeDef *)GPIOI_BASE);
  print_gpio_regs("J", (GPIO_TypeDef *)GPIOJ_BASE);
  print_gpio_regs("K", (GPIO_TypeDef *)GPIOK_BASE);
}


void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 5000) {}
  //Serial.println((uint32_t)A0);
  //Serial.println((uint32_t)A1);
  //Serial.println((uint32_t)A2);
  //Serial.println((uint32_t)A3);
  Serial.println((uint32_t)A4);
  Serial.println((uint32_t)A5);
  Serial.println((uint32_t)A6);
  Serial.println((uint32_t)A7);
  
  // hack to see if we need to set these pins to Analog mode

  //pinModeAnalog(8);
  //pinModeAnalog(10);
  // lets try setting C2/C3 to analog
  uint32_t moder = GPIOC->MODER;
  moder |= (3 << (2*2)) | (3 << (3 * 2)); // C2/C3 to analog mode
  GPIOC->MODER = moder;
  printk("MODER: %x %p %p %x\n", moder, &(GPIOC->MODER), (void*)GPIOC_BASE, GPIOC->MODER);
  moder = GPIOA->MODER;
  moder |= (3 << (4*2)) | (3 << (6 * 2)); // C2/C3 to analog mode
  GPIOA->MODER = moder;
  printk("MODER: %x %p %p %x\n", moder, &(GPIOA->MODER), (void*)GPIOA_BASE, GPIOA->MODER);

//  pinModeAnalog(21);
  show_all_gpio_regs();
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.print("A0: "); Serial.print(analogRead(A0));  Serial.print(" "); Serial.print(analogRead(PA_0C));
  Serial.print(" - A1: ");Serial.print(analogRead(A1)); Serial.print(" "); Serial.print(analogRead(PA_1C));
  Serial.print(" - A2: ");Serial.print(analogRead(A2)); Serial.print(" "); Serial.print(analogRead(PC_2C));
  Serial.print(" - A3: ");Serial.print(analogRead(A3)); Serial.print(" "); Serial.print(analogRead(PC_3C));
  Serial.print(" - A4: ");Serial.print(analogRead(A4)); Serial.print(" "); Serial.print(analogRead(PC_2)); Serial.print(" "); Serial.print(analogRead(19));
  Serial.print(" - A5: ");Serial.print(analogRead(A5)); Serial.print(" "); Serial.print(analogRead(PC_3)); Serial.print(" "); Serial.print(analogRead(20));
  Serial.print(" - A6: ");Serial.print(analogRead(A6)); Serial.print(" "); Serial.print(analogRead(PA_4));
  Serial.print(" - A7: ");Serial.print(analogRead(A7)); Serial.print(" "); Serial.print(analogRead(PA_6));
  Serial.println();
  delay(250);
  if (Serial.available()) {
    while (Serial.read() != -1 ) ;
    while (Serial.read() == -1 ) ;
    while (Serial.read() != -1 ) ;
  }
}