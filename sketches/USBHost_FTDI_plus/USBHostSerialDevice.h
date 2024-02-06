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

#ifndef USBHostSerialDevice_H
#define USBHostSerialDevice_H

#include <Arduino_USBHostMbed5.h>
#include "USBHost/USBHost.h"

#define USBHOST_hser 1

#if USBHOST_hser

#include "USBHost/USBHostConf.h"

// USBSerial formats - Lets encode format into bits
// Bits: 0-4 - Number of data bits
// Bits: 5-7 - Parity (0=none, 1=odd, 2 = even)
// bits: 8-9 - Stop bits. 0=1, 1=2


#define USBHOST_SERIAL_7E1 0x047
#define USBHOST_SERIAL_7O1 0x027
#define USBHOST_SERIAL_8N1 0x08
#define USBHOST_SERIAL_8N2 0x108
#define USBHOST_SERIAL_8E1 0x048
#define USBHOST_SERIAL_8O1 0x028



/**
 * A class to communicate a USB hser
 */
class USBHostSerialDevice : public IUSBEnumerator, public Stream
{
public:

    /**
    * Constructor
    */
    USBHostSerialDevice();

    /**
     * Try to connect a hser device
     *
     * @return true if connection was successful
     */
    bool connect();
    void disconnect();

    /**
    * Check if a hser is connected
    *
    * @returns true if a hser is connected
    */
    bool connected();

    /**
     * Attach a callback called when a hser event is received
     *
     * @param ptr function pointer
     */
    inline void attachEvent(void (*ptr)(uint8_t x, uint8_t y, uint8_t z, uint8_t rz, uint16_t buttons))
    {
        if (ptr != NULL) {
            onUpdate = ptr;
        }
    }

    void begin(uint32_t baud, uint32_t format = USBHOST_SERIAL_8N1);

    // from Stream
    virtual int available(void);
    virtual int peek(void);
    virtual int read(void);
    virtual int availableForWrite();
    virtual size_t write(uint8_t c);
    virtual void flush(void);

protected:
    //From IUSBEnumerator
    virtual void setVidPid(uint16_t vid, uint16_t pid);
    virtual bool parseInterface(uint8_t intf_nb, uint8_t intf_class, uint8_t intf_subclass, uint8_t intf_protocol); //Must return true if the interface should be parsed
    virtual bool useEndpoint(uint8_t intf_nb, ENDPOINT_TYPE type, ENDPOINT_DIRECTION dir); //Must return true if the endpoint will be used


    void initFTDI();
    void initPL2303();
    void initCH341();
    void initCP210X();


private:
    USBHost * host;
    USBDeviceConnected * dev;
    USBEndpoint * int_in;
    USBEndpoint * bulk_in;
    USBEndpoint * bulk_out;
    uint32_t size_bulk_in;
    uint32_t size_bulk_out;

    uint8_t buf[64];
    uint8_t setupdata[16];

    bool dev_connected;
    bool hser_device_found;
    uint8_t intf_SerialDevice;
    int ports_found;


    void rxHandler();
    void txHandler();
    void (*onUpdate)(uint8_t x, uint8_t y, uint8_t z, uint8_t rz, uint16_t buttons);
    int report_id;
    void init();

    // The current know serial device types
    typedef enum { UNKNOWN = 0, CDCACM, FTDI, PL2303, CH341, CP210X } sertype_t;

    typedef struct {
        uint16_t    idVendor;
        uint16_t    idProduct;
        sertype_t   sertype;
        int         claim_at_type;
    } product_vendor_mapping_t;
    static product_vendor_mapping_t pid_vid_mapping[];

    sertype_t sertype_ = UNKNOWN;

};

#endif

#endif