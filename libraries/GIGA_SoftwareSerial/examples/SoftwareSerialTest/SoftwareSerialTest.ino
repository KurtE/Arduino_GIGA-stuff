#include <SoftwareSerial.h>
#include <RPC.h>
SoftwareSerial SerialSoft(2, 3, false, TIM15);  //RX, TX
Stream *host_serial = nullptr;

#define BAUD 9600                              // start off slow
void setup() {
    pinMode(LEDB, OUTPUT);
    pinMode(LEDG, OUTPUT);

    if (RPC.cpu_id() == CM7_CPUID) {
        Serial.begin(BAUD);
        host_serial = &Serial;
        blink(LEDB, 100);  //blink blue LED (M7 core)
        while (!Serial && millis() < 4000) {}
    } else {
        host_serial = &RPC;
        blink(LEDG, 100);  //blink green LED (M4 core)
    }

    RPC.begin();
    // put your setup code here, to run once:
    //    debug_CNCCNT();
//    delay(1000);
    host_serial->println("Before Serial1 begin");
//    host_serial->flush();
    Serial1.begin(BAUD);  //
    host_serial->println("Before SerialSoft begin");
//    host_serial->flush();
    SerialSoft.setDebugStream(host_serial);
    SerialSoft.begin(BAUD);
    host_serial->println("End of Setup");
//    host_serial->flush();
}
/*
GIGA R1 WiFi - Core identify sketch.

This simple sketch blinks an LED on boot.
You will need to upload it to both the M7 and M4 core.

It checks whether current CPU is M7 or M4, and blinks either 
the blue LED or the green LED, 10 times. 

As the M4 is booted when invoking RPC.begin() on the M7,
the M4 sketch will run as soon as the blink() function
finishes on the M7. 
*/

void blink(int led, int delaySeconds) {
    host_serial->println("Start Blink");
    for (int i=0; i < 10; i++) {
        digitalWrite(led, LOW);
        delay(delaySeconds);
        digitalWrite(led, HIGH);
        delay(delaySeconds);
    }
    host_serial->println("End Blink");
}

void loop() {
    // first of simple TX only...
    int ich = host_serial->read();
    if (ich != -1) {
        Serial1.write(ich);
#if 0
        static bool first_write = true;
        if (first_write) {
            print_timer_regs(15, TIM15);
            first_write = false;
        }
#endif
    }
    ich = SerialSoft.read();
    if (ich != -1) SerialSoft.write(ich);

    // See if We received anything on Serial1...
    ich = Serial1.read();
    if (ich != -1) host_serial->write(ich);
}

void print_timer_regs(uint8_t timer_num, TIM_TypeDef *ptimer) {
    host_serial->print("Timer(");
    host_serial->print(timer_num);
    host_serial->print("): ");
    host_serial->println((uint32_t)ptimer, HEX);
    host_serial->print("\tCR1: ");
    host_serial->print(ptimer->CR1, HEX);
    host_serial->println("\tcontrol register 1");
    host_serial->print("\tCR2: ");
    host_serial->print(ptimer->CR2, HEX);
    host_serial->println("\tcontrol register 2");
    host_serial->print("\tSMCR: ");
    host_serial->print(ptimer->SMCR, HEX);
    host_serial->println("\tslave mode control register");
    host_serial->print("\tDIER: ");
    host_serial->print(ptimer->DIER, HEX);
    host_serial->println("\tDMA/interrupt enable register");
    host_serial->print("\tSR: ");
    host_serial->print(ptimer->SR, HEX);
    host_serial->println("\tstatus register");
    host_serial->print("\tEGR: ");
    host_serial->print(ptimer->EGR, HEX);
    host_serial->println("\tevent generation register");
    host_serial->print("\tCCMR1: ");
    host_serial->print(ptimer->CCMR1, HEX);
    host_serial->println("\tcapture/compare mode register 1");
    host_serial->print("\tCCMR2: ");
    host_serial->print(ptimer->CCMR2, HEX);
    host_serial->println("\tcapture/compare mode register 2");
    host_serial->print("\tCCER: ");
    host_serial->print(ptimer->CCER, HEX);
    host_serial->println("\tcapture/compare enable register");
    host_serial->print("\tCNT: ");
    host_serial->print(ptimer->CNT, HEX);
    host_serial->println("\tcounter register");
    host_serial->print("\tPSC: ");
    host_serial->print(ptimer->PSC, HEX);
    host_serial->println("\tprescaler");
    host_serial->print("\tARR: ");
    host_serial->print(ptimer->ARR, HEX);
    host_serial->println("\tauto-reload register");
    host_serial->print("\tRCR: ");
    host_serial->print(ptimer->RCR, HEX);
    host_serial->println("\trepetition counter register");
    host_serial->print("\tCCR1: ");
    host_serial->print(ptimer->CCR1, HEX);
    host_serial->println("\tcapture/compare register 1");
    host_serial->print("\tCCR2: ");
    host_serial->print(ptimer->CCR2, HEX);
    host_serial->println("\tcapture/compare register 2");
    host_serial->print("\tCCR3: ");
    host_serial->print(ptimer->CCR3, HEX);
    host_serial->println("\tcapture/compare register 3");
    host_serial->print("\tCCR4: ");
    host_serial->print(ptimer->CCR4, HEX);
    host_serial->println("\tcapture/compare register 4");
    host_serial->print("\tBDTR: ");
    host_serial->print(ptimer->BDTR, HEX);
    host_serial->println("\tbreak and dead-time register");
    host_serial->print("\tDCR: ");
    host_serial->print(ptimer->DCR, HEX);
    host_serial->println("\tDMA control register");
    host_serial->print("\tDMAR: ");
    host_serial->print(ptimer->DMAR, HEX);
    host_serial->println("\tDMA address for full transfer");
    host_serial->print("\tCCMR3: ");
    host_serial->print(ptimer->CCMR3, HEX);
    host_serial->println("\tcapture/compare mode register 3");
    host_serial->print("\tCCR5: ");
    host_serial->print(ptimer->CCR5, HEX);
    host_serial->println("\tcapture/compare register5");
    host_serial->print("\tCCR6: ");
    host_serial->print(ptimer->CCR6, HEX);
    host_serial->println("\tcapture/compare register6");
    host_serial->print("\tAF1: ");
    host_serial->print(ptimer->AF1, HEX);
    host_serial->println("\talternate function option register 1");
    host_serial->print("\tAF2: ");
    host_serial->print(ptimer->AF2, HEX);
    host_serial->println("\talternate function option register 2");
    host_serial->print("\tTISEL: ");
    host_serial->print(ptimer->TISEL, HEX);
    host_serial->println("\tInput Selection register");
}
