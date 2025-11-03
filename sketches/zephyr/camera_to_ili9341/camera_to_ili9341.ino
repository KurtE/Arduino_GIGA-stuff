#include <ILI9341_GIGA_zephyr.h>
#include "camera.h"

Camera cam;

#ifdef ARDUINO_PORTENTA_H7
#ifdef ZEPHYR_PINNAMES_H
#define TFT_DC PC_6
#define TFT_RST PC_7
#define TFT_CS PG_7
#else
#define TFT_DC 5
#define TFT_RST 4
#define TFT_CS 3
#endif
ILI9341_GIGA_n tft(&SPI, TFT_CS, TFT_DC, TFT_RST);
#else
#define TFT_DC 5
#define TFT_RST 4
#define TFT_CS 3
//#define TFT_DC 8
//#define TFT_RST 9
//#define TFT_CS 7
ILI9341_GIGA_n tft(&SPI1, TFT_CS, TFT_DC, TFT_RST);
#endif

void fatal_error(const char *msg) {
  Serial.println(msg);
  pinMode(LED_BUILTIN, OUTPUT);
  while (1) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
  }
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

void print_PinConfig(const char *name, GPIO_TypeDef *port, const char *regName) {
  uint32_t reg = 0;
  uint8_t numPins = 0;
  uint8_t numBits = 0;
  uint8_t hack = 0;
  Serial.print("GPIO");
  Serial.print(name);
  Serial.print(" ");
  if(strcmp(regName, "M") == 0) {
    Serial.print("MODER: ");
    numPins = 16;
    numBits = 2;
    hack = 0;
    reg = port->MODER;
  } else if(strcmp (regName , "AL") == 0) {
    Serial.print("AFRL");
    numPins = 8;
    numBits = 4;
    hack = 0;
    reg = port->AFR[0];
  } else {
    Serial.print("AFRH");
    numPins = 8;
    numBits = 4;
    hack = 1;
    reg = port->AFR[1];
  }


  for(uint8_t i = 0; i < numPins; i++) {
    unsigned  mask;
    //mask = ((1 << numBits2Extract) << startBit)
    mask = ((1 << numBits) - 1) << (i*numBits);
    //extractedBits = (value & mask) >> startBit
    uint8_t extractedBits = (reg & mask) >> (i*numBits);
    Serial.print("("); Serial.print(i+(hack*8)); Serial.print(")"); 
    Serial.print(extractedBits); Serial.print(", ");
  }
  Serial.println();
}

void show_all_gpio_regs() {
  print_gpio_regs("A", (GPIO_TypeDef *)GPIOA_BASE);
  print_PinConfig("A", (GPIO_TypeDef *)GPIOC_BASE, "M");
  print_PinConfig("A", (GPIO_TypeDef *)GPIOC_BASE, "AL");
  print_PinConfig("A", (GPIO_TypeDef *)GPIOC_BASE, "AH");

  print_gpio_regs("B", (GPIO_TypeDef *)GPIOB_BASE);
  print_gpio_regs("C", (GPIO_TypeDef *)GPIOC_BASE);
  print_PinConfig("C", (GPIO_TypeDef *)GPIOC_BASE, "M");
  print_PinConfig("C", (GPIO_TypeDef *)GPIOC_BASE, "AL");
  print_PinConfig("C", (GPIO_TypeDef *)GPIOC_BASE, "AH");

  print_gpio_regs("D", (GPIO_TypeDef *)GPIOD_BASE);
  print_gpio_regs("E", (GPIO_TypeDef *)GPIOE_BASE);
  print_gpio_regs("F", (GPIO_TypeDef *)GPIOF_BASE);
  print_gpio_regs("G", (GPIO_TypeDef *)GPIOG_BASE);
  print_gpio_regs("H", (GPIO_TypeDef *)GPIOH_BASE);
  print_PinConfig("H", (GPIO_TypeDef *)GPIOH_BASE, "M");
  print_PinConfig("H", (GPIO_TypeDef *)GPIOH_BASE, "AL");
  print_PinConfig("H", (GPIO_TypeDef *)GPIOH_BASE, "AH");
  print_gpio_regs("I", (GPIO_TypeDef *)GPIOI_BASE);
  print_gpio_regs("J", (GPIO_TypeDef *)GPIOJ_BASE);
  print_PinConfig("J", (GPIO_TypeDef *)GPIOJ_BASE, "M");
  print_PinConfig("J", (GPIO_TypeDef *)GPIOJ_BASE, "AL");
  print_PinConfig("J", (GPIO_TypeDef *)GPIOJ_BASE, "AH");
  print_gpio_regs("K", (GPIO_TypeDef *)GPIOK_BASE);
}



void setup() {
  printk("Setup called\n");
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  while (!Serial && millis() < 5000) {}
  printk("After Serial begin and wait\n");

  // put your setup code here, to run once:
  Serial.println("\n*** start display camera image on ILI9341 ***");
  if (Serial) show_all_gpio_regs();
  printk("Before tft.begin\n");
  tft.begin(16000000);
  Serial.println("After tft.begin");
  tft.setRotation(1);
  Serial.println("After Set rotation");
  tft.fillScreen(ILI9341_BLACK);
  Serial.println("After first fill Screen");
  delay(500);
  tft.fillScreen(ILI9341_RED);
  delay(500);
  tft.fillScreen(ILI9341_GREEN);
  delay(500);
  tft.fillScreen(ILI9341_BLUE);
  delay(500);
  tft.fillScreen(ILI9341_BLACK);
  delay(500);
  Serial.println("Start Camera");
  if (!cam.begin(320, 240, CAMERA_RGB565, false)) {
    fatal_error("Camera begin failed");
  }
  cam.setVerticalFlip(false);
  Serial.println("After set verical flip false");
  cam.setHorizontalMirror(false);
  Serial.println("After set Horizontal Mirror false");
}

void loop() {
  // put your main code here, to run repeatedly:
  FrameBuffer fb;
  if (cam.grabFrame(fb)) {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    tft.writeRect(0, 0, 320, 240, (const uint16_t*)fb.getBuffer());
    //tft.writeSubImageRectBytesReversed(0, 0, 320, 240, 0, 0, 320, 240, (const uint16_t*)fb.getBuffer());
    //tft.writeSubImageRect(0, 0, 320, 240, 0, 0, 320, 240, (const uint16_t*)fb.getBuffer());
    cam.releaseFrame(fb);
  }
}
