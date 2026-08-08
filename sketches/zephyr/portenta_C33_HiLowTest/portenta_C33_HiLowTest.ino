uint8_t pin_test_mode = 1;


// manually disabled 16-19/27-28
// 23-25 as LEDS
uint8_t pinLast[NUM_OF_DIGITAL_PINS] = {0};


bool pins_changed[NUM_OF_DIGITAL_PINS];

extern void allPinTest();

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 10000)
    ;
  //Serial.println("Compile Time:: " __FILE__ " " __DATE__ " " __TIME__);
  Serial.print("NUM_OF_DIGITAL_PINS: ");
  Serial.println(NUM_OF_DIGITAL_PINS, DEC);
  Serial.println("Pins included in test:");

}

void loop() {
  allPinTest();
}


void allPinTest() {
  uint32_t ii;
  for(ii=0; ii < NUM_OF_DIGITAL_PINS; ii++) pins_changed[ii] = false;

  Serial.print("PULLUP Start Vals:\n  ");
  Serial.print("PULLUP :: TEST to GND\n  ");
  for (ii = 0; ii < NUM_OF_DIGITAL_PINS; ii++) {
    if (pinLast[ii] != 0xff) {
      if ((ii == 0) || (pinLast[ii - 1] == 0xff)) {
        Serial.print("\n(");
        Serial.print(ii);
        Serial.print(") ");
        Serial.flush();
      }
      pinMode(ii, INPUT_PULLUP);
      delayMicroseconds(5);
      pinLast[ii] = digitalRead(ii);
      if (!pinLast[ii]) {
        Serial.print("\nd#=");
        Serial.print(ii);
        Serial.print(" val=");
      }
      Serial.print(pinLast[ii]);
      Serial.print(',');
    }
  }
  Serial.println();
  Serial.println();
  while (1) {
    uint32_t jj, dd = 0, cc = 0, ee = 4;
    cc = 0;
    for (ii = 0; ii < NUM_OF_DIGITAL_PINS; ii++) {
      if (pinLast[ii] != 0xff) {
        jj = digitalRead(ii);
        if (jj != pinLast[ii]) {
          pins_changed[ii] = true;
          dd = 1;
          cc++;
          pinLast[ii] = jj;
          Serial.print("d#=");
          Serial.print(ii);
          Serial.print(" val=");
          Serial.print(pinLast[ii]);
          Serial.print(',');
          //if (cc > 1 && ee) {
          //  Serial.println(">>> MULTI CHANGE !!");
          //  ee--;
          //}
        }
      }
    }
    if (dd) {
      dd = 0;
      Serial.println();
      delay(50);
    }

    if (Serial.available()) {
      while (Serial.available()) Serial.read();
    
      Serial.println("Pins that were touched: ");
      bool changed_found = false;
      for(ii=0; ii < NUM_OF_DIGITAL_PINS; ii++) {
        if (pins_changed[ii]) {
          pins_changed[ii] = false;
          if (!changed_found) {
            changed_found = true;
            Serial.print(" ");
            Serial.print(ii);
          }
        } else if (changed_found) {
          Serial.print("-");
          Serial.print(ii-1);
          changed_found = false;
        }
      }
      if (changed_found) {
        Serial.print("-");
        Serial.print(-1);
        changed_found = false;
      }
      Serial.println();

      if (0 == pin_test_mode) {
        pin_test_mode = 1;
        Serial.print("PULLUP :: TEST TO GND\n  ");
      } else {
        pin_test_mode = 0;
        Serial.print("PULLDOWN :: TEST to 3.3V\n  ");
      }
      for (ii = 0; ii < NUM_OF_DIGITAL_PINS; ii++) {
        if (pinLast[ii] != 0xff) {
          if (0 == pin_test_mode)
            pinMode(ii, INPUT_PULLDOWN);
          else
            pinMode(ii, INPUT_PULLUP);
          delayMicroseconds(20);
          pinLast[ii] = digitalRead(ii);
          if (pin_test_mode != pinLast[ii]) {
            Serial.print("d#=");
            Serial.print(ii);
            Serial.print(" val=");
            Serial.println(pinLast[ii]);
          }
        }
      }
    }
  }
}

