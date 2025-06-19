/*
  Scan

  This example scans for Bluetooth® Low Energy peripherals and prints out their advertising details:
  address, local name, advertised service UUID's.

  The circuit:
  - Arduino MKR WiFi 1010, Arduino Uno WiFi Rev2 board, Arduino Nano 33 IoT,
    Arduino Nano 33 BLE, or Arduino Nano 33 BLE Sense board.

  This example code is in the public domain.
*/

#include <ArduinoBLE.h>

void print_gpio_regs(const char *name, GPIO_TypeDef *port) {
  //printk("GPIO%s(%p) %08X %08X %08x\n", name, port, port->MODER, port->AFR[0], port->AFR[1]);
  Serial.print("GPIO");
  Serial.print(name);
  Serial.print(" ");
  Serial.print(port->MODER, HEX);
  Serial.print(" ");
  Serial.print(port->AFR[0], HEX);
  Serial.print(" ");
  Serial.print(port->AFR[1], HEX);
  Serial.print(" ");
  Serial.print(port->IDR, HEX);
  Serial.print(" ");
  Serial.print(port->ODR, HEX);
  Serial.print(" ");
  uint32_t pupdr = port->PUPDR;
  Serial.print(pupdr, HEX);
  Serial.print(" : ");
  for (uint8_t i = 0; i < 16; i++) {
    switch (pupdr & 0xC0000000) {
      case 0x00000000ul: Serial.print("-"); break;
      case 0x40000000ul: Serial.print("U"); break;
      case 0x80000000ul: Serial.print("D"); break;
      default: Serial.print("?"); break;
    }
    pupdr <<= 2;
  }
  Serial.println();
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

void show_uart_registers(USART_TypeDef *uart) {
  Serial.print(" CR1:");Serial.print(uart->CR1, HEX);
  Serial.print(" CR2:");Serial.print(uart->CR2, HEX);
  Serial.print(" CR3:");Serial.print(uart->CR3, HEX);
  Serial.print(" BRR:");Serial.print(uart->BRR, HEX); 
  Serial.print("(");Serial.print(120000000ul / uart->BRR, DEC);
  Serial.print(") GTPR:");Serial.print(uart->GTPR, HEX);
  Serial.print(" RTOR:");Serial.print(uart->RTOR, HEX);
  Serial.print(" RQR:");Serial.print(uart->RQR, HEX);
  Serial.print(" ISR:");Serial.print(uart->ISR, HEX);
  Serial.print(" ICR:");Serial.print(uart->ICR, HEX);
  Serial.print(" RDR:");Serial.print(uart->RDR, HEX);
  Serial.print(" TDR:");Serial.print(uart->TDR, HEX);
  Serial.println(" PRESC:");Serial.print(uart->PRESC, HEX);
}



void setup() {
  Serial.begin(9600);
  while (!Serial);



  // begin initialization
  show_all_gpio_regs();
  Serial.print("UART7: ");show_uart_registers((USART_TypeDef *) UART7_BASE);
  BLE.debug(Serial);
  
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    show_all_gpio_regs();
    Serial.print("UART7: ");show_uart_registers((USART_TypeDef *) UART7_BASE);
    while (1);
  }

  Serial.println("Bluetooth® Low Energy Central scan");

  // start scanning for peripheral
  BLE.scan();
}

void loop() {
  // check if a peripheral has been discovered
  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    // discovered a peripheral
    Serial.println("Discovered a peripheral");
    Serial.println("-----------------------");

    // print address
    Serial.print("Address: ");
    Serial.println(peripheral.address());

    // print the local name, if present
    if (peripheral.hasLocalName()) {
      Serial.print("Local Name: ");
      Serial.println(peripheral.localName());
    }

    // print the advertised service UUIDs, if present
    if (peripheral.hasAdvertisedServiceUuid()) {
      Serial.print("Service UUIDs: ");
      for (int i = 0; i < peripheral.advertisedServiceUuidCount(); i++) {
        Serial.print(peripheral.advertisedServiceUuid(i));
        Serial.print(" ");
      }
      Serial.println();
    }

    // print the RSSI
    Serial.print("RSSI: ");
    Serial.println(peripheral.rssi());

    Serial.println();
  }
}
