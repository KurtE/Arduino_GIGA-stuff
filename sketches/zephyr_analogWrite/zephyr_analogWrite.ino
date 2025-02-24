uint8_t analog_pin = 2;

void print_gpio_regs(const char* name, GPIO_TypeDef* port) {
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
}

#if defined(__ZEPHYR__)
#include <cmsis_core.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/clock_control.h>
#include <zephyr/logging/log.h>
#endif


#define toggle_pin 12  //31
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  //while (!Serial) {}
  delay(500);

  Serial.println("Start of PWM test");
  pinMode(toggle_pin, OUTPUT);
  delay(1000);

#if 0 // defined(__ZEPHYR__)
  Serial.println("Enter something if you want to start cam colock");
  while (Serial.read() != -1) {}
  int ich;
  while ((ich = Serial.read()) == -1) {};
  while (Serial.read() != -1) {}
  show_all_gpio_regs();
  if (ich >= ' ') {
    Serial.println("*** Getting PWM clock ***");
    const struct device* cam_ext_clk_dev = DEVICE_DT_GET(DT_NODELABEL(pwmclock));

    if (!device_is_ready(cam_ext_clk_dev)) {
      Serial.println("Clock not ready");
    }
    int ret = clock_control_on(cam_ext_clk_dev, (clock_control_subsys_t)0);
    if (ret < 0) {
      Serial.print("clock on: ");
      Serial.println(ret);
    }
    uint32_t rate;
    ret = clock_control_get_rate(cam_ext_clk_dev, (clock_control_subsys_t)0, &rate);
    Serial.print("Rate: ");
    Serial.println(rate);
    show_all_gpio_regs();
  } else {
    Serial.println("Not updating PWM Clock");
  }
#endif
}

void maybe_pause() {
  if (Serial.available()) {
    int ch;
    uint8_t new_awPin = 0xff;
    while (Serial.read() != -1) {}
    Serial.println("Paused");
    while ((ch = Serial.read()) == -1) {}
    while ((ch >= '0') && (ch <= '9')) {
      if (new_awPin == 0xff) new_awPin = 0;
      new_awPin = new_awPin * 10 + ch - '0';
      ch = Serial.read();
    }

    while (Serial.read() != -1) {}

    if (new_awPin != 0xff) {
      analogWrite(analog_pin, 0);
      analog_pin = new_awPin;
      Serial.print("Now PWM to pin: ");
      Serial.println(analog_pin);
      //show_all_gpio_regs();
    }

    #if 0
    Serial.print("CR1: 0x");
    Serial.println(TIM1->CR1, HEX);
    Serial.print("CR2: 0x");
    Serial.println(TIM1->CR2, HEX);
    Serial.print("SMCR: 0x");
    Serial.println(TIM1->SMCR, HEX);
    Serial.print("DIER: 0x");
    Serial.println(TIM1->DIER, HEX);
    Serial.print("SR: 0x");
    Serial.println(TIM1->SR, HEX);
    Serial.print("EGR: 0x");
    Serial.println(TIM1->EGR, HEX);
    Serial.print("CCMR1: 0x");
    Serial.println(TIM1->CCMR1, HEX);
    Serial.print("CCMR2: 0x");
    Serial.println(TIM1->CCMR2, HEX);
    Serial.print("CCER: 0x");
    Serial.println(TIM1->CCER, HEX);
    Serial.print("CNT: 0x");
    Serial.println(TIM1->CNT, HEX);
    Serial.print("PSC: 0x");
    Serial.println(TIM1->PSC, HEX);
    Serial.print("ARR: 0x");
    Serial.println(TIM1->ARR, HEX);
    Serial.print("RCR: 0x");
    Serial.println(TIM1->RCR, HEX);
    Serial.print("CCR1: 0x");
    Serial.println(TIM1->CCR1, HEX);
    Serial.print("CCR2: 0x");
    Serial.println(TIM1->CCR2, HEX);
    Serial.print("CCR3: 0x");
    Serial.println(TIM1->CCR3, HEX);
    Serial.print("CCR4: 0x");
    Serial.println(TIM1->CCR4, HEX);
    Serial.print("BDTR: 0x");
    Serial.println(TIM1->BDTR, HEX);
    Serial.print("DCR: 0x");
    Serial.println(TIM1->DCR, HEX);
    Serial.print("DMAR: 0x");
    Serial.println(TIM1->DMAR, HEX);
    Serial.print("CCMR3: 0x");
    Serial.println(TIM1->CCMR3, HEX);
    Serial.print("CCR5: 0x");
    Serial.println(TIM1->CCR5, HEX);
    Serial.print("CCR6: 0x");
    Serial.println(TIM1->CCR6, HEX);
    Serial.print("AF1: 0x");
    Serial.println(TIM1->AF1, HEX);
    Serial.print("AF2: 0x");
    Serial.println(TIM1->AF2, HEX);
    Serial.print("TISEL: 0x");
    Serial.println(TIM1->TISEL, HEX);
    #endif
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(toggle_pin, !digitalRead(toggle_pin));
  delay(5);
  digitalWrite(toggle_pin, !digitalRead(toggle_pin));
  for (int i = 0; i < 256; i += 32) {
    analogWrite(analog_pin, i);
    digitalWrite(toggle_pin, !digitalRead(toggle_pin));
    maybe_pause();
    delay(100);
  }

  digitalWrite(toggle_pin, !digitalRead(toggle_pin));
  delay(1);
  digitalWrite(toggle_pin, !digitalRead(toggle_pin));
  for (int i = 255; i >= 0; i -= 32) {
    analogWrite(analog_pin, i);
    digitalWrite(toggle_pin, !digitalRead(toggle_pin));
    maybe_pause();
    delay(100);
  }
  delay(50);
}