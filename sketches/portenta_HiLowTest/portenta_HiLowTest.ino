
const char *pin_names[] = {
  // clang-format off
            "PH_15", "PK_1", "PJ_11", "PG_7", "PC_7", "PC_6", "PA_8", "PI_0", "PC_3", "PI_1",
            "PC_2", "PH_8", "PH_7", "PA_10", "PA_9", "PA_0C", "PA_1C", "PC_2C", "PC_3C", "PC_2_ALT0",
            "PC_3_ALT0", "PA_4", "PA_6", "PK_5", "PK_6", "PK_7",

            "PA_0", "PA_1", "PA_2", "PA_3","PA_4", "PA_5", "PA_6", "PA_7", 
            "PA_8", "PA_9", "PA_10", "PA_11", "PA_12", "PA_13", "PA_14", "PA_15", 
            "PB_0", "PB_1", "PB_2", "PB_3", "PB_4", "PB_5", "PB_6", "PB_7", 
            "PB_8", "PB_9", "PB_10", "PB_11", "PB_12", "PB_13", "PB_14", "PB_15",
            "PC_0", "PC_1", "PC_2", "PC_3", "PC_4","PC_5", "PC_6", "PC_7", 
            "PC_8", "PC_9", "PC_10", "PC_11", "PC_12", "PC_13", "PC_14", "PC_15",
            "PD_0", "PD_1", "PD_2", "PD_3", "PD_4", "PD_5", "PD_6", "PD_7", 
            "PD_8", "PD_9", "PD_10", "PD_11", "PD_12", "PD_13", "PD_14", "PD_15", 
            "PE_0", "PE_1", "PE_2", "PE_3", "PE_4", "PE_5", "PE_6", "PE_7", 
            "PE_8", "PE_9", "PE_10", "PE_11", "PE_12", "PE_13", "PE_14", "PE_15", 
            "PF_0", "PF_1", "PF_2", "PF_3", "PF_4", "PF_5", "PF_6", "PF_7", 
            "PF_8", "PF_9", "PF_10", "PF_11","PF_12", "PF_13", "PF_14", "PF_15", 
            "PG_0", "PG_1", "PG_2", "PG_3", "PG_4", "PG_5", "PG_6","PG_7", 
            "PG_8", "PG_9", "PG_10", "PG_11", "PG_12", "PG_13", "PG_14", "PG_15", 
            "PH_0", "PH_1", "PH_2", "PH_3", "PH_4", "PH_5", "PH_6", "PH_7", 
            "PH_8", "PH_9", "PH_10", "PH_11","PH_12", "PH_13", "PH_14", "PH_15", 
            "PI_0", "PI_1", "PI_2", "PI_3", "PI_4", "PI_5", "PI_6","PI_7", 
            "PI_8", "PI_9", "PI_10", "PI_11", "PI_12", "PI_13", "PI_14", "PI_15", 
            "PJ_0", "PJ_1", "PJ_2", "PJ_3", "PJ_4", "PJ_5", "PJ_6", "PJ_7", 
            "PJ_8", "PJ_9", "PJ_10", "PJ_11", "PJ_12","PJ_13", "PJ_14", "PJ_15", 
            "PK_0", "PK_1", "PK_2", "PK_3", "PK_4", "PK_5", "PK_6", "PK_7",

  // clang-format on
};

const uint8_t count_pin_names = sizeof(pin_names) / sizeof(pin_names[1]);
uint8_t pin_test_mode = 1;

// manually disabled 16-19/27-28
// 23-25 as LEDS
uint8_t pinLast[count_pin_names] = {
  // clang-format off
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,           // 0
    0, 0, 0, 0, 0, 0, 0xff, 0xff, 0xff, 0xff,           // 10
    0, 0, 0, 0xff, 0xff, 0xff, 0, 0xff, 0xff, 0xff,     // 20
    0, 0xff, 0, 0xff, 0, 0, 0, 0, 0, 0,     // 30
    0, 0xff, 0xff, 0xff, 0, 0, 0, 0xff, 0, 0, //40
    0, 0, 0xff, 0xff, 0xff, 0xff, 0, 0, 0xff, 0xff,
    0, 0, 0xff, 0xff, 0, 0, 0xff, 0xff, 0xff, 0xff,
    0xff, 0, 0xff, 0, 0xff, 0xff, 0xff, 0, 0, 0,
    0, 0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0, 0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0, 0xff, 0xff, 0xff, 0,
    0xff, 0, 0, 0xff, 0xff, 0xff, 0, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0xff, 0xff, 0, 0, 0, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0, 0, 0, 0, 0,
    0, 0xff, 0xff, 0xff, 0xff, 0xff, 0, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff
  // clang-format on
};

#ifndef NUM_DIGITAL_PINS
#define NUM_DIGITAL_PINS (22u)
//#if DT_PROP_LEN(DT_PATH(zephyr_user), digital_pin_gpios) > 0
//uint16_t PINS_COUNT = DT_PROP_LEN(DT_PATH(zephyr_user), digital_pin_gpios);
const uint8_t PINS_COUNT = sizeof(pinLast);
//#endif
#endif

bool pins_changed[count_pin_names];

extern void allPinTest();
extern void testForShorts();

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 10000)
    ;
  //Serial.println("Compile Time:: " __FILE__ " " __DATE__ " " __TIME__);
  Serial.print("NUM_DIGITAL_PINS: ");
  Serial.println(NUM_DIGITAL_PINS, DEC);
  Serial.print("PINS_COUNT: ");
  Serial.println(PINS_COUNT);
  Serial.println("Pins included in test:");
  int index_first_in_series = -1;
  for (int i = 0; i < PINS_COUNT; i++) {
    if (pinLast[i] != 0xff) {
      if (index_first_in_series == -1) index_first_in_series = i;
    } else {
      // end of series
      if (index_first_in_series != -1) {
        Serial.print(" ");
        Serial.print(index_first_in_series);
        if (index_first_in_series != (i - 1)) {
          Serial.print("-");
          Serial.print(i - 1);
        }
        index_first_in_series = -1;
      }
    }
  }
  Serial.println();
  Serial.flush();

  testForShorts();
}

void loop() {
  allPinTest();
}


void allPinTest() {
  uint32_t ii;
  for(ii=0; ii < PINS_COUNT; ii++) pins_changed[ii] = false;

  Serial.print("PULLUP Start Vals:\n  ");
  Serial.print("PULLUP :: TEST to GND\n  ");
  for (ii = 0; ii < PINS_COUNT; ii++) {
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
    for (ii = 0; ii < PINS_COUNT; ii++) {
      if (pinLast[ii] != 0xff) {
        jj = digitalRead(ii);
        if (jj != pinLast[ii]) {
          pins_changed[ii] = true;
          dd = 1;
          cc++;
          pinLast[ii] = jj;
          Serial.print("d#=");
          Serial.print(ii);
          if (ii < count_pin_names) {
            Serial.print("(");
            Serial.print(pin_names[ii]);
            Serial.print(")");
          }
          if (pinLast[ii]) Serial.print("\t");
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
      for(ii=0; ii < PINS_COUNT; ii++) {
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
      for (ii = 0; ii < PINS_COUNT; ii++) {
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
            if (ii < count_pin_names) {
              Serial.print("(");
              Serial.print(pin_names[ii]);
              Serial.print(")");
            }
            Serial.print(" val=");
            Serial.println(pinLast[ii]);
          }
        }
      }
    }
  }
}

void testForShorts() {
#ifdef LATER_MAYBE
  uint32_t ii;
  Serial.print("Quick Test for Shorts to adjacent pin");
  Serial.println("First pull pins down and see if the next one follows");
  for (ii = 0; ii < NUM_DIGITAL_PINS - 1; ii++) {
    pinMode(ii + 1, INPUT_PULLDOWN);
    pinMode(ii, OUTPUT);
    digitalWrite(ii, HIGH);
    delayMicroseconds(5);
    if (digitalRead(ii + 1)) {
      Serial.print(ii, DEC);
      Serial.print(":");
      Serial.println(ii + 1, DEC);
    }
  }
  Serial.println("\n Now try Pull up and see if setting low follow");
  for (ii = 0; ii < NUM_DIGITAL_PINS - 1; ii++) {
    pinMode(ii + 1, INPUT_PULLUP);
    pinMode(ii, OUTPUT);
    digitalWrite(ii, LOW);
    delayMicroseconds(5);
    if (!digitalRead(ii + 1)) {
      Serial.print(ii, DEC);
      Serial.print(":");
      Serial.println(ii + 1, DEC);
    }
  }
  Serial.println();
#endif
}
