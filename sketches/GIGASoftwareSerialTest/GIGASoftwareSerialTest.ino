#include "SoftwareSerial.h"
SoftwareSerial SerialSoft(2, 3);  //RX, TX
#define BAUD 9600  // start off slow
void setup() {
    // put your setup code here, to run once:
    while (!Serial && millis() < 4000) {}
    Serial.begin(9600);
//    debug_CNCCNT();
    delay(1000);
    Serial.println("Before Serial1 begin"); Serial.flush();
    Serial1.begin(BAUD);  // 
    Serial.println("Before SerialSoft begin"); Serial.flush();
    SerialSoft.begin(BAUD);
    Serial.println("End of Setup"); Serial.flush();
}


void loop() {
    // first of simple TX only... 
    int ich = Serial.read();
    if (ich != -1) Serial1.write(ich);

    ich = SerialSoft.read();
    if (ich != -1) SerialSoft.write(ich);

    // See if We received anything on Serial1...
    ich = Serial1.read();
    if (ich != -1) Serial.write(ich);

}

void debug_CNCCNT() {
    // BUGBUG::: first some CYCNT testing
    Serial.println(DWT->CTRL);
    Serial.println(DWT->CYCCNT);
    pinMode(LED_BUILTIN, OUTPUT);
    uint32_t core_debug = CoreDebug->DEMCR;
    CoreDebug->DEMCR = core_debug | CoreDebug_DEMCR_TRCENA_Msk;

    uint32_t dwt_ctrl = DWT->CTRL;
    DWT->CTRL = dwt_ctrl | DWT_CTRL_CYCCNTENA_Msk;
    Serial.println(DWT->CTRL);

    // Lets get a general idea of the speed of the DWT timer
    #define COUNT_CYCLES_TEST 1000000000ul
    for (uint8_t i = 0; i < 5; i++) {
        uint32_t start_time = micros();
        uint32_t start_cycles = DWT->CYCCNT;

        while ((DWT->CYCCNT - start_cycles) < COUNT_CYCLES_TEST) {}
        uint32_t delta_time = micros() - start_time;
        Serial.print("DT: ");
        Serial.print(delta_time);
        Serial.print(" ");
        Serial.print(DWT->CYCCNT);
        Serial.print(" CPS: ");
        Serial.println((float(COUNT_CYCLES_TEST) / (float)delta_time) * 1000000.0, 2);
    }
}
