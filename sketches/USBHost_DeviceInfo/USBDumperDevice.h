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

#ifndef USBDumperDevice_H
#define USBDumperDevice_H

#include <Arduino_USBHostMbed5.h>
#include "USBHost/USBHost.h"

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
class USBDumperDevice : public IUSBEnumerator {
public:

  /**
    * Constructor
    */
  USBDumperDevice();

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

  uint16_t idVendor() { return (dev != nullptr) ? dev->getVid() : 0; }
  uint16_t idProduct() { return (dev != nullptr) ? dev->getPid() : 0; }
  bool manufacturer(uint8_t *buffer, size_t len);
  bool product(uint8_t *buffer, size_t len);
  bool serialNumber(uint8_t *buffer, size_t len);

protected:
  //From IUSBEnumerator
  virtual void setVidPid(uint16_t vid, uint16_t pid);
  virtual bool parseInterface(uint8_t intf_nb, uint8_t intf_class, uint8_t intf_subclass, uint8_t intf_protocol);  //Must return true if the interface should be parsed
  virtual bool useEndpoint(uint8_t intf_nb, ENDPOINT_TYPE type, ENDPOINT_DIRECTION dir);                           //Must return true if the endpoint will be used


private:
  USBHost* host;
  USBDeviceConnected* dev;
  USBEndpoint* int_in;
  USBEndpoint* bulk_in;
  USBEndpoint* bulk_out;
  uint32_t size_bulk_in;
  uint32_t size_bulk_out;

  uint8_t buf[64];
  uint8_t setupdata[16];

  bool dev_connected;
  bool hser_device_found;
  uint8_t intf_SerialDevice;
  int ports_found;

  uint32_t baudrate = 115200;  // lets give it a default in case begin is not called
  uint32_t format_ = USBHOST_SERIAL_8N1;

  void rxHandler();
  void txHandler();
  void (*onUpdate)(uint8_t x, uint8_t y, uint8_t z, uint8_t rz, uint16_t buttons);
  int report_id;
  void init();
  // String indexes 
  uint8_t iManufacturer_ = 0xff;
  uint8_t iProduct_ = 0xff;
  uint8_t iSerialNumber_ = 0xff;
  uint16_t wLanguageID_ = 0x409; // english US
  // should be part of higher level stuff...
  bool getStringDesc(uint8_t index, uint8_t *buffer, size_t len);
  bool cacheStringIndexes();
  bool getHIDDesc(uint8_t index, uint8_t* buffer, size_t len);
  
  uint16_t getConfigurationDescriptor(uint8_t * buf, uint16_t max_len_buf);

  void dumpHIDReportDescriptor(uint8_t iInterface, uint16_t descsize);
  void printUsageInfo(uint8_t usage_page, uint16_t usage);
  void print_input_output_feature_bits(uint8_t val);
  enum {MAX_FEATURE_REPORTS=20};
  uint8_t feature_report_ids_[MAX_FEATURE_REPORTS];
  uint8_t cnt_feature_reports_ = 0;



};

#endif
