/* mbed USBHost Library
 * Copyright (c) 2006-2013 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef USBHostKeyboardEx_H
#define USBHostKeyboardEx_H

#include "USBHost/USBHostConf.h"

#if USBHOST_KEYBOARD

#include "USBHost/USBHost.h"

/**
 * A class to communicate a USB keyboard
 */
class USBHostKeyboardEx : public IUSBEnumerator
{
public:

    /**
    * Constructor
    */
    USBHostKeyboardEx();

    /**
     * Try to connect a keyboard device
     *
     * @return true if connection was successful
     */
    bool connect();

    /**
    * Check if a keyboard is connected
    *
    * @returns true if a keyboard is connected
    */
    bool connected();

    /**
     * Attach a callback called when a keyboard event is received
     *
     * @param ptr function pointer
     */
    inline void attach(void (*ptr)(uint8_t key))
    {
        if (ptr != NULL) {
            onKey = ptr;
        }
    }

    /**
     * Attach a callback called when a keyboard event is received
     *
     * @param ptr function pointer
     */
    inline void attachRelease(void (*ptr)(uint8_t key))
    {
        if (ptr != NULL) {
            onKeyRelease = ptr;
        }
    }


    /**
     * Attach a callback called when a keyboard event is received
     *
     * @param ptr function pointer
     */
    inline void attach(void (*ptr)(uint8_t keyCode, uint8_t modifier))
    {
        if (ptr != NULL) {
            onKeyCode = ptr;
        }
    }

    typedef union {
        struct {
            uint8_t numLock : 1;
            uint8_t capsLock : 1;
            uint8_t scrollLock : 1;
            uint8_t compose : 1;
            uint8_t kana : 1;
            uint8_t reserved : 3;
        };
        uint8_t byte;
    }  __attribute__((packed)) KBDLeds_t;

    KBDLeds_t leds_ = {0};
    void     LEDS(uint8_t leds);
    uint8_t  LEDS() {return leds_.byte;}
    void     updateLEDS(void);
    bool     numLock() {return leds_.numLock;}
    bool     capsLock() {return leds_.capsLock;}
    bool     scrollLock() {return leds_.scrollLock;}
    void     numLock(bool f);
    void     capsLock(bool f);
    void     scrollLock(bool f);

protected:
    //From IUSBEnumerator
    virtual void setVidPid(uint16_t vid, uint16_t pid);
    virtual bool parseInterface(uint8_t intf_nb, uint8_t intf_class, uint8_t intf_subclass, uint8_t intf_protocol); //Must return true if the interface should be parsed
    virtual bool useEndpoint(uint8_t intf_nb, ENDPOINT_TYPE type, ENDPOINT_DIRECTION dir); //Must return true if the endpoint will be used

private:
    USBHost * host;
    USBDeviceConnected * dev;
    USBEndpoint * int_in;
    uint8_t report[9];
    uint8_t prev_report[9];
    int keyboard_intf;
    bool keyboard_device_found;

    bool dev_connected;

    void rxHandler();
    uint8_t mapKeycodeToKey(uint8_t modifier, uint8_t keycode);

    void (*onKey)(uint8_t key);
    void (*onKeyRelease)(uint8_t key);
    void (*onKeyCode)(uint8_t key, uint8_t modifier);

    int report_id;

    void init();

};

#endif

#endif
