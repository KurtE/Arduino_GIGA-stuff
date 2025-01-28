/*
  HS300x - Read Sensors Imperial

  This example reads data from the on-board HS300x sensor of the
  Nano 33 BLE Sense then, prints the temperature and humidity sensor
  values in imeprial units to the Serial Monitor once a second.

  The circuit:
  - Arduino Nano 33 BLE Sense R2

  This example code is in the public domain.
*/

#include <Arduino_HS300x.h>

void print_nrf_gpio(NRF_GPIO_Type *pX) {
  Serial.print("\tOUT:"); Serial.print(pX->OUT, HEX);
  Serial.print(" IN:"); Serial.print(pX->IN, HEX);
  Serial.print(" DIR:"); Serial.print(pX->DIR, HEX);
  Serial.print(" LATCH:"); Serial.print(pX->LATCH, HEX);
  Serial.print(" DETECTMODE:"); Serial.print(pX->DETECTMODE, HEX);
  for(uint8_t i = 0; i < 32; i++) {
    if ((i & 0x7) == 0)Serial.print("\n\t");
    Serial.print(pX->PIN_CNF[i]);
    Serial.print(" ");
  }
  Serial.println();

}

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // BUGBUG: lets try to see the initial state of the pins:
  // port0 0x50000000
  // port1 0x50000300
  Serial.println("\nGPIO P0 registers");
  print_nrf_gpio(NRF_P0);
  Serial.println("\nGPIO P1 registers");
  print_nrf_gpio(NRF_P1);
  #ifdef __ZEPHYR__
  pinMode(32, OUTPUT); //I2C_PULL
  digitalWrite(32, HIGH);
  pinMode(33, OUTPUT); //VDD_ENV_ENABLE
  digitalWrite(33, HIGH);
  // Hack to set HIGH output on P1[22]
  uint32_t pin22_cnf = NRF_P0->PIN_CNF[22];
  //pin22_cnf |= (GPIO_PIN_CNF_DRIVE_H0H1 << GPIO_PIN_CNF_DRIVE_Pos);
  NRF_P0->PIN_CNF[22] = 0x73; //pin22_cnf;
  delay(500);

  Serial.println("Registers after our modes");
  Serial.println("\nGPIO P0 registers");
  print_nrf_gpio(NRF_P0);
  Serial.println("\nGPIO P1 registers");
  print_nrf_gpio(NRF_P1);
  #endif



  Wire1.begin();
  Wire1.setClock(400000);

  if (!HS300x.begin()) {
    Serial.println("Failed to initialize humidity temperature sensor!");
    while (1);
  }
  Serial.println("Humidity temperature sensor initialized!");
}

void loop() {
  // Passing in FAHRENHEIT as the unit parameter to ENV.readTemperature(...),
  // allows you to read the sensor values in imperial units
  Serial.println("Before readTemperature");
  int temperature = HS300x.readTemperature(FAHRENHEIT);
  
  Serial.println("Before readHumidity");
  int humidity    = HS300x.readHumidity();
  Serial.println("After reads");

  // print each of the sensor values
  Serial.print("Temperature = ");
  Serial.print(temperature);
  Serial.println(" °F");

  Serial.print("Humidity    = ");
  Serial.print(humidity);
  Serial.println(" %");

  // print an empty line
  Serial.println();

  // wait 1 second to print again
  delay(1000);
}
