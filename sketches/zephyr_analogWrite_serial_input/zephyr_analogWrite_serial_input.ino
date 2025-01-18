int  analog_pin = 10;
extern void print_pwm_structures();
void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);
    while (!Serial && millis() < 4000) {}
    delay(500);

    Serial.println("Start of PWM test");
    Serial.println("Enter: <pin> value - to change ");
    Serial.println("   #N - to print out information about timer N");
    delay(1000);
}

#include <zephyr/devicetree.h>
extern const struct pwm_dt_spec arduino_pwm[];
extern const pin_size_t arduino_pwm_pins[];
void print_pwm_structures() {
  Serial.print("\narduino_pwm table:");
  int cnt_pwms = DT_PROP_LEN(DT_PATH(zephyr_user), pwms);

  for (int i = 0; i < cnt_pwms; i++) {
    Serial.print("\t"); Serial.print(i);
    Serial.print(" 0x"); Serial.print((uint32_t)arduino_pwm[i].dev, HEX);
      Serial.print("("); Serial.print(arduino_pwm[i].dev->name);
      Serial.print(",0x"); Serial.print(arduino_pwm[i].dev->config);
      Serial.print(",0x"); Serial.print(arduino_pwm[i].dev->api);
      Serial.print(",0x"); Serial.print(arduino_pwm[i].dev->state);
      Serial.print(",0x"); Serial.print(arduino_pwm[i].dev->data);
    Serial.print(") "); Serial.print(arduino_pwm[i].channel);
    Serial.print(" "); Serial.print(arduino_pwm[i].period);
    Serial.print(" "); Serial.println(arduino_pwm[i].flags);
  }
  Serial.println("\nPWM Pin table:");
  for (int i = 0; i < (sizeof(arduino_pwm_pins) / sizeof(arduino_pwm_pins[0])); i++) {
    Serial.print("\t"); Serial.print(i);
    Serial.print(" "); Serial.println(arduino_pwm_pins[i]);
}

void print_timer_info(int iTimer) {
    TIM_TypeDef *ptmr = nullptr;
    switch (iTimer) {
        case 1: ptmr = TIM1; break;
        case 3: ptmr = TIM3; break;
        case 4: ptmr = TIM4; break;
        case 15: ptmr = TIM15; break;
        default: return;
    };
    Serial.print("\nTimer: ");
    Serial.println(iTimer);
    Serial.print("\tCR1: 0x");
    Serial.println(ptmr->CR1, HEX);
    Serial.print("\tCR2: 0x");
    Serial.println(ptmr->CR2, HEX);
    Serial.print("\tSMCR: 0x");
    Serial.println(ptmr->SMCR, HEX);
    Serial.print("\tDIER: 0x");
    Serial.println(ptmr->DIER, HEX);
    Serial.print("\tSR: 0x");
    Serial.println(ptmr->SR, HEX);
    Serial.print("\tEGR: 0x");
    Serial.println(ptmr->EGR, HEX);
    Serial.print("\tCCMR1: 0x");
    Serial.println(ptmr->CCMR1, HEX);
    Serial.print("\tCCMR2: 0x");
    Serial.println(ptmr->CCMR2, HEX);
    Serial.print("\tCCER: 0x");
    Serial.println(ptmr->CCER, HEX);
    Serial.print("\tCNT: 0x");
    Serial.println(ptmr->CNT, HEX);
    Serial.print("\tPSC: 0x");
    Serial.println(ptmr->PSC, HEX);
    Serial.print("\tARR: 0x");
    Serial.println(ptmr->ARR, HEX);
    Serial.print("\tRCR: 0x");
    Serial.println(ptmr->RCR, HEX);
    Serial.print("\tCCR1: 0x");
    Serial.println(ptmr->CCR1, HEX);
    Serial.print("\tCCR2: 0x");
    Serial.println(ptmr->CCR2, HEX);
    Serial.print("\tCCR3: 0x");
    Serial.println(ptmr->CCR3, HEX);
    Serial.print("\tCCR4: 0x");
    Serial.println(ptmr->CCR4, HEX);
    Serial.print("\tBDTR: 0x");
    Serial.println(ptmr->BDTR, HEX);
    Serial.print("\tDCR: 0x");
    Serial.println(ptmr->DCR, HEX);
    Serial.print("\tDMAR: 0x");
    Serial.println(ptmr->DMAR, HEX);
    Serial.print("\tCCMR3: 0x");
    Serial.println(ptmr->CCMR3, HEX);
    Serial.print("\tCCR5: 0x");
    Serial.println(ptmr->CCR5, HEX);
    Serial.print("\tCCR6: 0x");
    Serial.println(ptmr->CCR6, HEX);
    Serial.print("\tAF1: 0x");
    Serial.println(ptmr->AF1, HEX);
    Serial.print("\tAF2: 0x");
    Serial.println(ptmr->AF2, HEX);
    Serial.print("\tTISEL: 0x");
    Serial.println(ptmr->TISEL, HEX);
}

int get_next_cmd_num(int &ich) {
  int num = -1;
  while (ich == ' ') ich = Serial.read();
  if (ich < ' ') return -1; //assume we are done...
  if ((ich >= '0') && (ich <'9')) {
    num = ich - '0';
    for(;;) {
      ich = Serial.read();
      if ((ich < '0') || (ich > '9')) break;
      num = num * 10 + ich - '0';
    }
  }
  return num;
}

void loop() {
  while (Serial.available() == 0) {}

  
  int ich = Serial.read();
  if (ich == '#') {
    ich = Serial.read();
    print_timer_info(get_next_cmd_num(ich));
  } else {
    int p1 = get_next_cmd_num(ich);
    int p2 = get_next_cmd_num(ich);
    
    if (p2 != -1) {
      analog_pin = p1;
      Serial.print("analogWrite(");
      Serial.print(analog_pin);
      Serial.print(",");
      Serial.print(p2);
      Serial.println(")");
      analogWrite(analog_pin, p2);
    } else if (p1 != -1) {
      Serial.print("analogWrite(");
      Serial.print(analog_pin);
      Serial.print(",");
      Serial.print(p1);
      Serial.println(")");
      analogWrite(analog_pin, p1);
    }
  }
}
