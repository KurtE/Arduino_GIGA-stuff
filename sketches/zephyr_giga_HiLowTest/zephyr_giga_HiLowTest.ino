//#ifdef ESP_PLATFORM
// BUGBUG UNOR4 boards do not support PULL_DOWN

#define digitalWriteFast digitalWrite
#define digitalReadFast digitalRead

#define INTERRUPT_HELPER(n, p, i) 1
#define NUM_DIGITAL_PINS                                                        \
  DT_FOREACH_PROP_ELEM_SEP(DT_PATH(zephyr_user), digital_pin_gpios,            \
                           INTERRUPT_HELPER, (+))

extern void allPinTest();
extern void testForShorts();

//#endif

#undef NUM_DIGITAL_PINS
#define NUM_DIGITAL_PINS 15

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 10000 );
  //Serial.println("Compile Time:: " __FILE__ " " __DATE__ " " __TIME__);
  Serial.print("Num Digital Pins: ");
  Serial.println(NUM_DIGITAL_PINS, DEC);
  Serial.flush();

  testForShorts();
  
}

void loop() {
    allPinTest( );
}

uint32_t pinLast[NUM_DIGITAL_PINS];
void allPinTest() {
  uint32_t ii;
  Serial.print("PULLUP Start Vals:\n  ");
  Serial.print("PULLUP :: TEST to GND\n  ");
  for ( ii = 0; ii < NUM_DIGITAL_PINS; ii++) {
    pinMode( ii, INPUT_PULLUP );
    delayMicroseconds( 5 );
    pinLast[ii] = digitalReadFast( ii );
    if (!pinLast[ii]) {
      Serial.print("\nd#=");
      Serial.print( ii );
      Serial.print( " val=" );
    }
    Serial.print( pinLast[ii] );
    Serial.print(',');
  }
  Serial.println();
  Serial.println();
  while ( 1 ) {
    uint32_t jj, dd = 0, cc = 0, ee=4;
    cc = 0;
    for ( ii = 0; ii < NUM_DIGITAL_PINS; ii++) {
      jj = digitalReadFast( ii );
      if ( jj != pinLast[ii] ) {
        dd = 1;
        cc++;
        pinLast[ii] = jj;
        Serial.print("d#=");
        Serial.print( ii );
        if ( pinLast[ii] ) Serial.print( "\t" );
        Serial.print( " val=" );
        Serial.print( pinLast[ii] );
        Serial.print(',');
      }
      if ( cc > 1 && ee ) {
        Serial.println(">>> MULTI CHANGE !!");
        ee--;
      }
#if 0
      if ( Serial.available() ) {
        while ( Serial.available() ) Serial.read();
        if ( 0 == SET ) {
          SET = 1;
          Serial.print("PULLUP :: TEST TO GND\n  ");
        }
        else {
          SET = 0;
          Serial.print("PULLDOWN :: TEST to 3.3V\n  ");
        }
        for ( ii = 0; ii < NUM_DIGITAL_PINS; ii++) {
          if ( 0 == SET )
            pinMode( ii, INPUT_PULLDOWN );
          else
            pinMode( ii, INPUT_PULLUP );
          delayMicroseconds( 20 );
          pinLast[ii] = digitalReadFast( ii );
          if (SET != pinLast[ii]) {
            Serial.print("d#=");
            Serial.print( ii );
            Serial.print( " val=" );
            Serial.println( pinLast[ii] );
          }
        }
      }
#endif      
    }
    if ( dd ) {
      dd = 0;
      Serial.println();
      delay( 50 );
    }
  }
}

void testForShorts() {
  uint32_t ii;
  Serial.print("Quick Test for Shorts to adjacent pin");
#if 1
  Serial.println("UNOR4 does not support INPUT_PULLDOWN");
#else
  Serial.println("First pull pins down and see if the next one follows");
  for ( ii = 0; ii < NUM_DIGITAL_PINS-1; ii++) {
    pinMode( ii+1, INPUT_PULLDOWN );
    pinMode( ii, OUTPUT);
    digitalWrite(ii, HIGH);
    delayMicroseconds( 5 );
    if (digitalRead(ii+1)) {
      Serial.print(ii,DEC);
      Serial.print(":");
      Serial.println(ii+1, DEC);
    }
  }
#endif  
  Serial.println("\n Now try Pull up and see if setting low follow");
  for ( ii = 0; ii < NUM_DIGITAL_PINS-1; ii++) {
    pinMode( ii+1, INPUT_PULLUP );
    pinMode( ii, OUTPUT);
    digitalWrite(ii, LOW);
    delayMicroseconds( 5 );
    if (!digitalRead(ii+1)) {
      Serial.print(ii,DEC);
      Serial.print(":");
      Serial.println(ii+1, DEC);
    }
  }
  Serial.println();  
}
