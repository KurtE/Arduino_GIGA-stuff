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

#ifndef USBHOSTGAMEPAD_H
#define USBHOSTGAMEPAD_H

#include <Arduino_USBHostMbed5.h>
#include "USBHost/USBHost.h"

#define USBHOST_GAMEPAD 1

#if USBHOST_GAMEPAD

#include "USBHost/USBHostConf.h"

/**
 * A class to communicate a USB Gamepad
 */
class USBHostGamepad : public IUSBEnumerator
{
public:

    /**
    * Constructor
    */
    USBHostGamepad();

    /**
     * Try to connect a Gamepad device
     *
     * @return true if connection was successful
     */
    bool connect();

    /**
    * Check if a Gamepad is connected
    *
    * @returns true if a Gamepad is connected
    */
    bool connected();

    /**
     * Attach a callback called when a Gamepad event is received
     *
     * @param ptr function pointer
     */
    inline void attachEvent(void (*ptr)(uint8_t x, uint8_t y, uint8_t z, uint8_t rz, uint16_t buttons))
    {
        if (ptr != NULL) {
            onUpdate = ptr;
        }
    }

protected:
    //From IUSBEnumerator
    virtual void setVidPid(uint16_t vid, uint16_t pid);
    virtual bool parseInterface(uint8_t intf_nb, uint8_t intf_class, uint8_t intf_subclass, uint8_t intf_protocol); //Must return true if the interface should be parsed
    virtual bool useEndpoint(uint8_t intf_nb, ENDPOINT_TYPE type, ENDPOINT_DIRECTION dir); //Must return true if the endpoint will be used

private:
    USBHost * host;
    USBDeviceConnected * dev;
    USBEndpoint * int_in;
    uint8_t report[8];
    bool dev_connected;
    bool gamepad_device_found;
    int gamepad_intf;

    void rxHandler();
    void (*onUpdate)(uint8_t x, uint8_t y, uint8_t z, uint8_t rz, uint16_t buttons);
    int report_id;
    void init();
};

#endif

#endif