/*
SoftwareSerial.cpp (formerly NewSoftSerial.cpp) - 
Multi-instance software serial library for Arduino/Wiring
-- Interrupt-driven receive and other improvements by ladyada
   (http://ladyada.net)
-- Tuning, circular buffer, derivation from class Print/Stream,
   multi-instance support, porting to 8MHz processors,
   various optimizations, PROGMEM delay tables, inverse logic and 
   direct port writing by Mikal Hart (http://www.arduiniana.org)
-- Pin change interrupt macros by Paul Stoffregen (http://www.pjrc.com)
-- 20MHz processor support by Garrett Mace (http://www.macetech.com)
-- ATmega1280/2560 support by Brett Hagman (http://www.roguerobotics.com/)

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

The latest version of this library can always be found at
http://arduiniana.org.
*/

// When set, _DEBUG co-opts pins 11 and 13 for debugging with an
// oscilloscope or logic analyzer.  Beware: it also slightly modifies
// the bit times, so don't rely on it too much at high baud rates
#define _DEBUG 1
#define _DEBUG_PIN_RX 11
#define _DEBUG_PIN_RX_PISR 12
#define _DEBUG_PIN_TX 13
//
// Includes
//
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <Arduino.h>
#include "SoftwareSerial.h"

#if _DEBUG
#include <GIGA_digitalWriteFast.h>
#define DBGdigitalWrite digitalWriteFast
#define DBGdigitalToggle digitalToggleFast
#else
inline void DBGdigitalWrite(uint8_t pin, uint8_t val) {}
inline void DBGdigitalToggle(uint8_t pin) {} 
#endif



//#if defined(__MK20DX128__) || defined(__MK20DX256__) || defined(__MKL26Z64__) || defined(__MK64FX512__) || defined(__MK66FX1M0__) || defined(__IMXRT1052__) || defined(__IMXRT1062__)
#if defined(ARDUINO_GIGA)

// BUGBUG = need to check which core.
static uint32_t F_CPU = 480000000ul;

#if !_DEBUG
// same table in the GIGA_digitalWriteFast.h
static  GPIO_TypeDef * const port_table[] = { GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, GPIOG, GPIOH, GPIOI, GPIOJ, GPIOK };
#endif

#ifdef USE_GIGA_PORTENTA_H7_INTERRUPT
    // need to see what works well for a timer here...
SoftwareSerial::SoftwareSerial(uint8_t rxPin, uint8_t txPin, bool inverse_logic, TIM_TypeDef* timer) :
    rx_timer(timer) {
                
#else
SoftwareSerial::SoftwareSerial(uint8_t rxPin, uint8_t txPin, bool inverse_logic /* = false */) {
#endif

    buffer_overflow = false;

    port = NULL;
    txpin = txPin;
    rxpin = rxPin;
    PinName pin_name = g_APinDescription[txPin].name;
    tx_port = port_table[pin_name >> 4];
    tx_bitmask = 1 << (pin_name & 0xf);

    pin_name = g_APinDescription[rxPin].name;
    rx_port = port_table[pin_name >> 4];
    rx_bitmask = 1 << (pin_name & 0xf);
    cycles_per_bit = 0;
}




void SoftwareSerial::begin(unsigned long speed) {
    if (port) {
        port->begin(speed);
    } else {
        rx_head = 0;
        rx_tail = 0;
        if (HAL_GetCurrentCPUID() != CM7_CPUID) F_CPU = 240000000ul;

        cycles_per_bit = (uint32_t)(F_CPU + speed / 2) / speed;
        microseconds_per_bit = (float)cycles_per_bit / (float)(F_CPU / 1000000);
        // TODO: latency estimate could be better tuned to each board
        const float latency = 13.0f; // 900.0f / (float)(F_CPU / 1000000);
        microseconds_start = microseconds_per_bit * 1.5f - latency;
        uint32_t core_debug = CoreDebug->DEMCR;
        CoreDebug->DEMCR = core_debug | CoreDebug_DEMCR_TRCENA_Msk;
        uint32_t dwt_ctrl = DWT->CTRL;
        DWT->CTRL = dwt_ctrl | DWT_CTRL_CYCCNTENA_Msk;
        pinMode(txpin, OUTPUT);
        digitalWrite(txpin, HIGH);
        pinMode(rxpin, INPUT_PULLUP);
        delayMicroseconds(5);  // allow time for pullup -> logic high
// lets try to turn of RX to start        
        attachInterrupt(digitalPinToInterrupt(rxpin), start_bit_falling_edge, FALLING);
        rx_int = digitalPinToInterruptObj(rxpin);

        active_object = nullptr;
        // Hacking. 
        if (_debug_stream) {
            rx_timer.hwtimer()->setOverflow(microseconds_per_bit, MICROSEC_FORMAT);
            _debug_stream->print("Baud: "); _debug_stream->print(speed);
            _debug_stream->print(" Ticks: "); 
            _debug_stream->print(rx_timer.hwtimer()->getOverflow());        
            rx_timer.hwtimer()->setOverflow(microseconds_start, MICROSEC_FORMAT);
            _debug_stream->print(" Ticks start: "); 
            _debug_stream->println(rx_timer.hwtimer()->getOverflow());        
            //_debug_stream->printf("begin bitbang, rxpin=%u, txpin=%u\n", rxpin, txpin);
            _debug_stream->print("rx_timer freq: ");
            _debug_stream->println(rx_timer.hwtimer()->getTimerClkFreq());

            _debug_stream->print("HAL Current CPU ID: 0x");
            _debug_stream->println(HAL_GetCurrentCPUID(), HEX);

        }

        active_object = this;
        rx_timer.hwtimer()->setPreloadEnable(0); // turn off the buffering...
    }

#if _DEBUG
    pinMode(_DEBUG_PIN_RX, OUTPUT);
    pinMode(_DEBUG_PIN_RX_PISR, OUTPUT);
    pinMode(_DEBUG_PIN_TX, OUTPUT);
#endif    
}

void SoftwareSerial::end() {
    if (port) {
        port->end();
        port = NULL;
    } else {
        active_object = NULL;
        detachInterrupt(rxpin);
#ifdef USE_GIGA_PORTENTA_H7_INTERRUPT
        rx_timer.detachInterrupt();
#else        
        ticker.detach();
#endif        
        pinMode(txpin, INPUT);
        pinMode(rxpin, INPUT);
    }
    cycles_per_bit = 0;
}

// The worst case expected length of any interrupt routines.  If an
// interrupt runs longer than this number of cycles, it can disrupt
// the transmit waveform.  Increasing this number causes SoftwareSerial
// to hog the CPU longer, delaying all interrupt response for other
// libraries, so this should be made as small as possible but still
// ensure accurate transmit waveforms.
#define WORST_INTERRUPT_CYCLES 360

static void wait_for_target(uint32_t begin, uint32_t target) {
    if (target - (DWT->CYCCNT - begin) > WORST_INTERRUPT_CYCLES + 20) {
        uint32_t pretarget = target - WORST_INTERRUPT_CYCLES;
        //digitalWriteFast(12, HIGH);
        interrupts();
        while (DWT->CYCCNT - begin < pretarget)
            ;  // wait
        noInterrupts();
        //digitalWriteFast(12, LOW);
        DBGdigitalToggle(_DEBUG_PIN_TX);
    }
    while (DWT->CYCCNT - begin < target)
        ;  // wait
}

size_t SoftwareSerial::write(uint8_t b) {
    //_debug_stream->print("Write: ");
    //_debug_stream->print(b, HEX);
    //_debug_stream->print("C PB: ");
    //_debug_stream->println(cycles_per_bit);

    uint32_t target;
    uint8_t mask;
    uint32_t begin_cycle;

    // use hardware serial, if possible
    if (port) return port->write(b);
    if (cycles_per_bit == 0) return 0;
//    ARM_DEMCR |= ARM_DEMCR_TRCENA;
//    DWT->CTRL |= DWT->CTRL_CYCCNTENA;
    // start bit
    target = cycles_per_bit;
    noInterrupts();
    begin_cycle = DWT->CYCCNT;
    tx0();
    wait_for_target(begin_cycle, target);
    // 8 data bits
    for (mask = 1; mask; mask <<= 1) {
        if (b & mask) {
            tx1();
        } else {
            tx0();
        }
        target += cycles_per_bit;
        wait_for_target(begin_cycle, target);
    }
    // stop bit
    tx1();
    interrupts();
    target += cycles_per_bit;
    while (DWT->CYCCNT - begin_cycle < target)
        ;  // wait
    return 1;
}

void SoftwareSerial::flush() {
    if (port) port->flush();
}



SoftwareSerial* SoftwareSerial::active_object = NULL;

void SoftwareSerial::start_bit_falling_edge() {
    if (active_object) active_object->start_bit_begin();
}

void SoftwareSerial::data_bit_sampling_timer() {
    if (active_object) active_object->data_bit_sample();
}


void SoftwareSerial::start_bit_begin() {
    //digitalWriteFast(12, HIGH);
    DBGdigitalToggle(_DEBUG_PIN_RX_PISR);
    DBGdigitalWrite(_DEBUG_PIN_RX, HIGH);
    //ticker.attach(mbed::callback(this, &SoftwareSerial::data_bit_sample, std::chrono::microseconds(ms_per_bit))); 
    rx_int->disable_irq();
    rxcount = 0;
    rxbyte = 0;
    uint32_t ms_start = microseconds_start;
#ifdef USE_GIGA_PORTENTA_H7_INTERRUPT
    rx_timer.setInterval(ms_start, &data_bit_sampling_timer);
    // see if it sets up the 2nd time interval
    //rx_timer.enableTimer();
#else
    ticker.attach(mbed::callback(this, &SoftwareSerial::data_bit_sample), std::chrono::microseconds(ms_start)); 
#endif    
    DBGdigitalWrite(_DEBUG_PIN_RX, LOW);
/*
    if (data_bit_timer.begin(data_bit_sampling_timer, microseconds_start)) {
        data_bit_timer.update(microseconds_per_bit);
        detachInterrupt(rxpin);
        rxcount = 0;
        rxbyte = 0;
    } else {
        // TODO: should we somehow report error allocating IntervalTimer?
    }
    //digitalWriteFast(12, LOW);
*/    
}

void SoftwareSerial::data_bit_sample() {
    //digitalWriteFast(12, HIGH);
    //if (digitalRead(rxpin) == HIGH) rxbyte |= (1 << rxcount);
    DBGdigitalWrite(_DEBUG_PIN_RX, HIGH);
    if (rx_port->IDR & rx_bitmask) rxbyte |= (1 << rxcount);

    rxcount = rxcount + 1;
    if (rxcount == 1) {
        // update the time interval
#ifdef USE_GIGA_PORTENTA_H7_INTERRUPT
        rx_timer.setInterval(microseconds_per_bit, &data_bit_sampling_timer);
#else
        //rx_timer.setInterval(ms_per_bit, &data_bit_sampling_timer);
        ticker.detach();
        uint32_t ms_per_bit = microseconds_per_bit;
        ticker.attach(mbed::callback(this, &SoftwareSerial::data_bit_sample), std::chrono::microseconds(ms_per_bit)); 
#endif        
    }

    if (rxcount == 8) {  // last data bit
        uint16_t head = rx_head + 1;
        if (head >= _SS_MAX_RX_BUFF) head = 0;
        if (head != rx_tail) {
            rx_buffer[head] = rxbyte;
            rx_head = head;
        }
    }
    else if (rxcount >= 9) {  // stop bit
#ifdef USE_GIGA_PORTENTA_H7_INTERRUPT
    rx_timer.disableTimer();
#else        
        ticker.detach();
#endif
        //attachInterrupt(digitalPinToInterrupt(rxpin), start_bit_falling_edge, FALLING);
        rx_int->enable_irq();
    }
    DBGdigitalWrite(_DEBUG_PIN_RX, LOW);
    //digitalWriteFast(12, LOW);
}



int SoftwareSerial::available() {
    if (port) return port->available();
    uint16_t head, tail;
    head = rx_head;
    tail = rx_tail;
    if (head >= tail) return head - tail;
    return _SS_MAX_RX_BUFF + head - tail;
}

int SoftwareSerial::peek() {
    if (port) return port->peek();
    uint16_t head, tail;
    head = rx_head;
    tail = rx_tail;
    if (head == tail) return -1;
    if (++tail >= _SS_MAX_RX_BUFF) tail = 0;
    return rx_buffer[tail];
}

int SoftwareSerial::read() {
    if (port) return port->read();
    uint16_t head, tail;
    head = rx_head;
    tail = rx_tail;
    if (head == tail) return -1;
    if (++tail >= _SS_MAX_RX_BUFF) tail = 0;
    uint8_t n = rx_buffer[tail];
    rx_tail = tail;
    return n;
}



#endif
