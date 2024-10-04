#include <SoftwareSerial.h>
SoftwareSerial SerialSoft(2, 3);  //RX, TX
#define BAUD 115200  // start off slow
void setup() {
    // put your setup code here, to run once:
    while (!Serial && millis() < 4000) {}
    Serial.begin(BAUD);
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
