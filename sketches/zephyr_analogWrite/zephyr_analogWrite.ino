uint8_t analog_pin = 2;
#define toggle_pin 31
void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);
    //while (!Serial) {}
    delay(500);

    Serial.println("Start of PWM test");
    pinMode(toggle_pin, OUTPUT);
    delay(1000);
}

void maybe_pause() {
    if (Serial.available()) {
        int ch;
        uint8_t new_awPin = 0;
        while (Serial.read() != -1) {}
        Serial.println("Paused");
        while ((ch = Serial.read()) == -1) {}
        while ((ch >= '0') && (ch <= '9')) {
          new_awPin = new_awPin * 10 + ch - '0';
          ch = Serial.read();
        }

        while (Serial.read() != -1) {}

        if (new_awPin != 0) {
          analogWrite(analog_pin, 0);
          analog_pin = new_awPin;
        }

        Serial.print("CR1: 0x"); Serial.println (TIM1->CR1, HEX);
        Serial.print("CR2: 0x"); Serial.println (TIM1->CR2, HEX);
        Serial.print("SMCR: 0x"); Serial.println (TIM1->SMCR, HEX);
        Serial.print("DIER: 0x"); Serial.println (TIM1->DIER, HEX);
        Serial.print("SR: 0x"); Serial.println (TIM1->SR, HEX);
        Serial.print("EGR: 0x"); Serial.println (TIM1->EGR, HEX);
        Serial.print("CCMR1: 0x"); Serial.println (TIM1->CCMR1, HEX);
        Serial.print("CCMR2: 0x"); Serial.println (TIM1->CCMR2, HEX);
        Serial.print("CCER: 0x"); Serial.println (TIM1->CCER, HEX);
        Serial.print("CNT: 0x"); Serial.println (TIM1->CNT, HEX);
        Serial.print("PSC: 0x"); Serial.println (TIM1->PSC, HEX);
        Serial.print("ARR: 0x"); Serial.println (TIM1->ARR, HEX);
        Serial.print("RCR: 0x"); Serial.println (TIM1->RCR, HEX);
        Serial.print("CCR1: 0x"); Serial.println (TIM1->CCR1, HEX);
        Serial.print("CCR2: 0x"); Serial.println (TIM1->CCR2, HEX);
        Serial.print("CCR3: 0x"); Serial.println (TIM1->CCR3, HEX);
        Serial.print("CCR4: 0x"); Serial.println (TIM1->CCR4, HEX);
        Serial.print("BDTR: 0x"); Serial.println (TIM1->BDTR, HEX);
        Serial.print("DCR: 0x"); Serial.println (TIM1->DCR, HEX);
        Serial.print("DMAR: 0x"); Serial.println (TIM1->DMAR, HEX);
        Serial.print("CCMR3: 0x"); Serial.println (TIM1->CCMR3, HEX);
        Serial.print("CCR5: 0x"); Serial.println (TIM1->CCR5, HEX);
        Serial.print("CCR6: 0x"); Serial.println (TIM1->CCR6, HEX);
        Serial.print("AF1: 0x"); Serial.println (TIM1->AF1, HEX);
        Serial.print("AF2: 0x"); Serial.println (TIM1->AF2, HEX);
        Serial.print("TISEL: 0x"); Serial.println (TIM1->TISEL, HEX);
    }
}

void loop() {
    // put your main code here, to run repeatedly:
    digitalWrite(toggle_pin, !digitalRead(toggle_pin));
    digitalWrite(toggle_pin, !digitalRead(toggle_pin));
    for (int i = 0; i < 256; i += 32) {
        analogWrite(analog_pin, i);
        digitalWrite(toggle_pin, !digitalRead(toggle_pin));
        maybe_pause();
        delay(100);
    }

    digitalWrite(toggle_pin, !digitalRead(toggle_pin));
    digitalWrite(toggle_pin, !digitalRead(toggle_pin));
    for (int i = 255; i >= 0; i -= 32) {
        analogWrite(analog_pin, i);
        digitalWrite(toggle_pin, !digitalRead(toggle_pin));
        maybe_pause();
        delay(100);
    }
    delay(50);
}