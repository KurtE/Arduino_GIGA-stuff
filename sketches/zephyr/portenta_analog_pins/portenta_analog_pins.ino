 #include "zephyrInternal.h"
 static const struct gpio_dt_spec arduino_pins[] = {DT_FOREACH_PROP_ELEM_SEP(
   DT_PATH(zephyr_user), digital_pin_gpios, GPIO_DT_SPEC_GET_BY_IDX, (, ))};


  void pinModeAnalog(pin_size_t pinNumber) {
     gpio_pin_configure_dt(&arduino_pins[pinNumber],
                           GPIO_INPUT /*| GPIO_ACTIVE_HIGH */ | GPIO_MODE_ANALOG);
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
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);
  //pinMode(10, OUTPUT);  // PC2
  Serial.begin(115200);
  while (!Serial && millis() < 4000) {}
  #if 0
  pinMode(19, INPUT);
  pinMode(20, INPUT);
  pinModeAnalog(19);
  pinModeAnalog(20);
  #endif
  show_all_gpio_regs();
}

void loop() {
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  //digitalWrite(10, !digitalRead(10));
  Serial.print(analogRead(A0));
  Serial.print("\t");
  Serial.print(analogRead(A1));
  Serial.print("\t");
  Serial.print(analogRead(A2));
  Serial.print("\t");
  Serial.print(analogRead(A3));
  Serial.print("\t");
  Serial.print(analogRead(A4));
  Serial.print("\t");
  Serial.print(analogRead(A5));
  Serial.print("\t");
  Serial.print(analogRead(A6));
  Serial.print("\t");
  Serial.print(analogRead(A7));
  Serial.println();
  delay(250);
  if (Serial.available()) {
    show_all_gpio_regs();
    while (Serial.read() != -1) {}
    Serial.println("Paused");
    while (Serial.read() == -1) {}
    while (Serial.read() != -1) {}
    //analogRead(A4);
  }
}