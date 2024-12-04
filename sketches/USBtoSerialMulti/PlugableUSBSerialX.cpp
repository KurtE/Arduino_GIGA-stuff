/*
 * Copyright (c) 2018-2019, Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
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

#include "Arduino.h"

#if DEVICE_USBDEVICE && defined(SERIAL_CDC)

#include "stdint.h"
#include "PluggableUSBSerialX.h"
#include "usb_phy_api.h"
#include "mbed.h"

using namespace arduino;

static rtos::EventFlags event[USBSerialX::MAX_OBJECTS];
uint8_t USBSerialX::s_count_objects = 0;
USBSerialX *USBSerialX::s_objects[MAX_OBJECTS] = {nullptr};


void USBSerialX::waitForPortClose() {
    USBSerialX::s_objects[0]->processWaitForPortClose();
}

void USBSerialX::waitForPortClose1() {
    USBSerialX::s_objects[1]->processWaitForPortClose();
}

void usbPortXChanged(int baud, int bits, int parity, int stop) {
    if (baud == 1200) {
        event[0].set(1);
    }
}

void usbPortXChanged1(int baud, int bits, int parity, int stop) {
    if (baud == 1200) {
        event[1].set(1);
    }
}

USBSerialX::USBSerialX(bool connect_blocking, const char* name, uint16_t vendor_id, uint16_t product_id, uint16_t product_release):
    USBCDCX(get_usb_phy(), name, vendor_id, product_id, product_release)
{
    // should double check we don't have too many
    object_num = s_count_objects;
    s_objects[s_count_objects++] = this;
    _settings_changed_callback = 0;

    if (connect_blocking) {
        wait_ready();
    }
}

USBSerialX::USBSerialX(USBPhy *phy, uint16_t vendor_id, uint16_t product_id, uint16_t product_release):
    USBCDCX(phy, NULL, vendor_id, product_id, product_release)
{
    // should double check we don't have too many
    object_num = s_count_objects;
    s_objects[s_count_objects++] = this;
    _settings_changed_callback = 0;
}

USBSerialX::~USBSerialX()
{
}

void USBSerialX::processWaitForPortClose() {

    event[object_num].wait_any(0xFF);
    // wait for DTR be 0 (port closed) and timeout to be over
    long start = millis();
    static const int WAIT_TIMEOUT = 200;
    while (s_objects[object_num]->connected() || (millis() - start) < WAIT_TIMEOUT) {
        // the delay is needed to handle other "concurrent" IRQ events
        delay(1);
    }
    _ontouch1200bps_();
}




void USBSerialX::begin(unsigned long) {
    if (object_num == 0) this->attach(usbPortXChanged);
    else this->attach(usbPortXChanged1);
    this->attach(::mbed::callback(this, &USBSerialX::onInterrupt));
    t = new rtos::Thread(osPriorityNormal, 256, nullptr, "USBevt");
    if (object_num == 0) t->start(waitForPortClose);
    else t->start(waitForPortClose1);
    onInterrupt();
}

int USBSerialX::_putc(int c)
{
    if (send((uint8_t *)&c, 1)) {
        return c;
    } else {
        return -1;
    }
}

int USBSerialX::_getc()
{
    uint8_t c = 0;
    if (receive(&c, sizeof(c))) {
        return c;
    } else {
        return -1;
    }
}

void USBSerialX::data_rx()
{
    USBCDCX::assert_locked();

    //call a potential handler
    for (size_t i = 0 ; i < _howManyCallbacks ; i++) {
        if (_rx[i]) {
            _rx[i].call();
        } else {
            break;
        }
    }
}

uint32_t USBSerialX::_available()
{
    USBCDCX::lock();

    uint32_t size = 0;
    if (!_rx_in_progress) {
        size = _rx_size > CDC_MAX_PACKET_SIZE ? CDC_MAX_PACKET_SIZE : _rx_size;
    }

    USBCDCX::unlock();
    return size;
}

bool USBSerialX::connected()
{
    return _terminal_connected;
}
#endif
