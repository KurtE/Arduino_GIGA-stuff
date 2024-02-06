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

#include "USBHostGamepad.h"
#include <LibPrintf.h>

#if USBHOST_GAMEPAD

USBHostGamepad::USBHostGamepad()
{
    host = USBHost::getHostInst();
    init();
}

void USBHostGamepad::init()
{
    dev = NULL;
    int_in = NULL;
    onUpdate = NULL;
    report_id = 0;
    dev_connected = false;
    gamepad_device_found = false;
    gamepad_intf = -1;
}

bool USBHostGamepad::connected() {
    return dev_connected;
}

bool USBHostGamepad::connect()
{
    int len_listen;

    if (dev_connected) {
        return true;
    }
    USB_INFO(" USBHostGamepad::connect() called\r\n");
    for (uint8_t i = 0; i < MAX_DEVICE_CONNECTED; i++) {

        if (dev = host->getDevice(i)) {
            USB_INFO("\t Device:%p\r\n",dev);
            if(host->enumerate(dev, this)) {
                break;
            }
            if (gamepad_device_found) {
                {
                    /* As this is done in a specific thread
                     * this lock is taken to avoid to process the device
                     * disconnect in usb process during the device registering */
                    USBHost::Lock  Lock(host);

                    int_in = dev->getEndpoint(gamepad_intf, INTERRUPT_ENDPOINT, IN);
                    if (!int_in) {
                        break;
                    }

                    USB_INFO("New Gamepad device: VID:%04x PID:%04x [dev: %p - intf: %d]", dev->getVid(), dev->getPid(), dev, gamepad_intf);
                    printf("New Gamepad device: VID:%04x PID:%04x [dev: %p - intf: %d]", dev->getVid(), dev->getPid(), dev, gamepad_intf);
                    dev->setName("Gamepad", gamepad_intf);
                    host->registerDriver(dev, gamepad_intf, this, &USBHostGamepad::init);

                    int_in->attach(this, &USBHostGamepad::rxHandler);
                    len_listen = int_in->getSize();
                    if (len_listen > sizeof(report)) {
                        len_listen = sizeof(report);
                    }
                }
                int ret=host->interruptRead(dev, int_in, report, len_listen, false);
                MBED_ASSERT((ret==USB_TYPE_OK) || (ret ==USB_TYPE_PROCESSING) || (ret == USB_TYPE_FREE));
                if ((ret==USB_TYPE_OK) || (ret ==USB_TYPE_PROCESSING)) {
                    dev_connected = true;
                    printf("Dev connected\n");
                }
                if (ret == USB_TYPE_FREE) {
                    printf("Dev not connected\n");
                    dev_connected = false;
                }
                return true;
            }
        }
    }
    init();
    return false;
}

void USBHostGamepad::rxHandler()
{
    int len_listen = int_in->getLengthTransferred();
    if (len_listen !=0) {
        uint16_t buttons = 0x0;
        if (onUpdate) {
            buttons |= ((report[5] & 0xF0) >> 4) | (report[6] << 4);
            (*onUpdate)(report[0], report[1], report[3], report[4], buttons);
        }
    }

    /*  set again the maximum value */
    len_listen = int_in->getSize();

    if (len_listen > sizeof(report)) {
        len_listen = sizeof(report);
    }

    if (dev) {
        host->interruptRead(dev, int_in, report, len_listen, false);
    }
}

/*virtual*/ void USBHostGamepad::setVidPid(uint16_t vid, uint16_t pid)
{
    // we don't check VID/PID for Gamepad driver
    USB_INFO("VID: %X, PID: %X\n\r", vid, pid);
    printf("VID: %X, PID: %X\n\r", vid, pid);
}

/*virtual*/ bool USBHostGamepad::parseInterface(uint8_t intf_nb, uint8_t intf_class, uint8_t intf_subclass, uint8_t intf_protocol) //Must return true if the interface should be parsed
{
    USB_INFO("USBHostGamepad::parseInterface nb:%d, cl:%u isub:%u iprot:%u\n\r", intf_nb, intf_class, intf_subclass, intf_protocol);
    printf("USBHostGamepad::parseInterface nb:%d, cl:%u isub:%u iprot:%u\n\r", intf_nb, intf_class, intf_subclass, intf_protocol);
    if ((gamepad_intf == -1) &&
            (intf_class == HID_CLASS) &&
            (intf_subclass == 0x00) &&
            (intf_protocol == 0x00)) {
        gamepad_intf = intf_nb;
        return true;
    }
    return false;
}

/*virtual*/ bool USBHostGamepad::useEndpoint(uint8_t intf_nb, ENDPOINT_TYPE type, ENDPOINT_DIRECTION dir) //Must return true if the endpoint will be used
{
    USB_INFO("USBHostGamepad::useEndpoint(%u, %u, %u\n\r", intf_nb, type, dir);
    printf("USBHostGamepad::useEndpoint(%u, %u, %u\n\r", intf_nb, type, dir);
    if (intf_nb == gamepad_intf) {
        if (type == INTERRUPT_ENDPOINT && dir == IN) {
            gamepad_device_found = true;
            return true;
        }
    }
    return false;
}

#endif