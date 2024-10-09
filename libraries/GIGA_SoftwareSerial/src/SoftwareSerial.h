#include "InterruptIn.h"
/*
SoftwareSerial.h (formerly NewSoftSerial.h) - 
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
-- Giga version by: KurtE

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

#ifndef SoftwareSerial_h
#define SoftwareSerial_h

#include <inttypes.h>
#include <Stream.h>
#include <HardwareSerial.h>

#include "Arduino.h"
#include "pinDefinitions.h"
#include "mbed.h"

#define USE_GIGA_PORTENTA_H7_INTERRUPT
#ifdef USE_GIGA_PORTENTA_H7_INTERRUPT
#include <Portenta_H7_TimerInterrupt.h>
#endif

using namespace std::chrono_literals;
using namespace std::chrono;

/******************************************************************************
* Definitions
******************************************************************************/

#define _SS_MAX_RX_BUFF 64  // RX buffer size
#ifndef GCC_VERSION
#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif

//#if defined(__MK20DX128__) || defined(__MK20DX256__) || defined(__MKL26Z64__) || defined(__MK64FX512__) || defined(__MK66FX1M0__) || defined(__IMXRT1052__) || defined(__IMXRT1062__)
#if defined(ARDUINO_GIGA)
class SoftwareSerial : public Stream {
  public:
#ifdef USE_GIGA_PORTENTA_H7_INTERRUPT
    // need to see what works well for a timer here...
    SoftwareSerial(uint8_t rxPin, uint8_t txPin, bool inverse_logic = false, TIM_TypeDef* timer=TIM15);
#else
    SoftwareSerial(uint8_t rxPin, uint8_t txPin, bool inverse_logic = false);
#endif    
    ~SoftwareSerial() {
        end();
    }
    void begin(unsigned long speed);
    void end();
    bool listen() {
        return true;
    }
    bool isListening() {
        return true;
    }
    bool overflow() {
        bool ret = buffer_overflow;
        buffer_overflow = false;
        return ret;
    }
    virtual int available();
    virtual int read();
    int peek();
    virtual void flush();
    virtual size_t write(uint8_t byte);
    operator bool() {
        return true;
    }

    void setDebugStream(Stream *pstream) {_debug_stream = pstream;} 

    using Print::write;
  private:
    HardwareSerial *port;
    uint32_t cycles_per_bit;
    float microseconds_per_bit;
    float microseconds_start;

    GPIO_TypeDef  *  tx_port;
    uint32_t tx_bitmask;
    inline void tx0() {
        tx_port->BSRR = (uint32_t)(tx_bitmask << 16);
    }
    inline void tx1() {
        tx_port->BSRR = tx_bitmask;
    }

    GPIO_TypeDef * rx_port;
    uint32_t rx_bitmask;

    bool buffer_overflow;
    uint8_t txpin;
    uint8_t rxpin;
    uint8_t rxbyte;
    uint8_t rxcount;
    static void start_bit_falling_edge();
    static void data_bit_sampling_timer();
    static SoftwareSerial *active_object;
    void start_bit_begin();
    void data_bit_sample();
    mbed::InterruptIn *rx_int;
#ifdef USE_GIGA_PORTENTA_H7_INTERRUPT
    Portenta_H7_Timer rx_timer;
#else
    mbed::Ticker ticker; // calls a callback repeatedly with a timeout
#endif
    //IntervalTimer data_bit_timer;
    uint16_t rx_head;
    uint16_t rx_tail;
    uint8_t rx_buffer[_SS_MAX_RX_BUFF];
    Stream *_debug_stream = &Serial; // by default
};


#endif

#endif
