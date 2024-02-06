#include <MemoryHexDump.h>

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

#include "USBDumperDevice.h"
#include <LibPrintf.h>



USBDumperDevice::USBDumperDevice() {
  host = USBHost::getHostInst();
  init();
}

void USBDumperDevice::init() {
  dev = NULL;
  bulk_in = NULL;
  bulk_out = NULL;
  int_in = NULL;
  onUpdate = NULL;
  dev_connected = false;
  hser_device_found = false;
  intf_SerialDevice = -1;
  ports_found = 0;
}

bool USBDumperDevice::connected() {
  return dev_connected;
}

bool USBDumperDevice::connect() {
  printf(" USBDumperDevice::connect() called\r\n");
  if (dev) {
    for (uint8_t i = 0; i < MAX_DEVICE_CONNECTED; i++) {
      USBDeviceConnected* d = host->getDevice(i);
      if (dev == d)
        return true;
    }
    disconnect();
  }
  host = USBHost::getHostInst();
  for (uint8_t i = 0; i < MAX_DEVICE_CONNECTED; i++) {
    USBDeviceConnected* d = host->getDevice(i);
    printf("\tDev: %p\r\n", d);
    if (d != NULL) {
      printf("Device:%p\r\n", d);
      if (host->enumerate(d, this) != USB_TYPE_OK) {
        printf("Enumerate returned status not OK");
        continue;  //break;  what if multiple devices?
      }

      printf("\tconnect Device found\n\r");

      bulk_in = d->getEndpoint(intf_SerialDevice, BULK_ENDPOINT, IN);
      printf("bulk in:%p", bulk_in);

      bulk_out = d->getEndpoint(intf_SerialDevice, BULK_ENDPOINT, OUT);
      printf(" out:%p\r\n", bulk_out);

      printf("\tAfter get end points\n\r");
      if (1 /*bulk_in && bulk_out*/) {
        dev = d;
        dev_connected = true;
        //        printf("New hser device: VID:%04x PID:%04x [dev: %p - intf: %d]", dev->getVid(), dev->getPid(), dev, intf_SerialDevice);
        printf("New Debug device: VID:%04x PID:%04x [dev: %p - intf: %d]\n\r", dev->getVid(), dev->getPid(), dev, intf_SerialDevice);
        //printf(" Report Desc Size: %u\n\r", dev->getLngthReportDescr());
        dev->setName("Debug", intf_SerialDevice);
        host->registerDriver(dev, intf_SerialDevice, this, &USBDumperDevice::init);
#if 0
        size_bulk_in = bulk_in->getSize();
        size_bulk_out = bulk_out->getSize();

        bulk_in->attach(this, &USBDumperDevice::rxHandler);
        bulk_out->attach(this, &USBDumperDevice::txHandler);
        host->bulkRead(dev, bulk_in, buf, size_bulk_in, false);
#endif
        uint16_t size_config = getConfigurationDescriptor(buf, 0);
        printf("Size of configuration Descriptor: %u\n\r", size_config);
        uint8_t* config_buffer = (uint8_t*)malloc(size_config);
        if (config_buffer) {
          size_config = getConfigurationDescriptor(config_buffer, size_config);
          MemoryHexDump(Serial, config_buffer, size_config, true, "Configuration Descriptor\n");

          // lets walk the configuration buffer:
          uint8_t interface_number = 0xff;
          for (uint16_t i = 0; i < size_config; i += config_buffer[i]) {
            switch (config_buffer[i + 1]) {
              case 2:  // Configuration
                {
                  Serial.println("Config:");
                  ConfigurationDescriptor* pconf = (ConfigurationDescriptor*)&config_buffer[i];
                  printf(" wTotalLength: %u\n\r", pconf->wTotalLength);
                  printf(" bNumInterfaces: %u\n\r", pconf->bNumInterfaces);
                  printf(" bConfigurationValue: %u\n\r", pconf->bConfigurationValue);
                  printf(" iConfiguration: %u\n\r", pconf->iConfiguration);
                  printf(" bmAttributes: %u\n\r", pconf->bmAttributes);
                  printf(" bMaxPower: %u\n\r", pconf->bMaxPower);
                }
                break;
              case 4:  // Interface
                {
                  Serial.println("Interface:");
                  InterfaceDescriptor* pintf = (InterfaceDescriptor*)&config_buffer[i];
                  interface_number = pintf->bInterfaceNumber;
                  printf("  bInterfaceNumber: %u\n\r", pintf->bInterfaceNumber);
                  printf("  bAlternateSetting: %u\n\r", pintf->bAlternateSetting);
                  printf("  bNumEndpoints: %u\n\r", pintf->bNumEndpoints);
                  printf("  bInterfaceClass: %u\n\r", pintf->bInterfaceClass);
                  printf("  bInterfaceSubClass: %u\n\r", pintf->bInterfaceSubClass);
                  printf("  bInterfaceProtocol: %u\n\r", pintf->bInterfaceProtocol);
                  printf("  iInterface: %u\n\r", pintf->iInterface);
                }
                break;
              case 5:  // Endpoint
                {
                  Serial.println("Endpoint:");
                  EndpointDescriptor* pendp = (EndpointDescriptor*)&config_buffer[i];
                  printf("  bEndpointAddress: %u\n\r", pendp->bEndpointAddress);
                  printf("  bmAttributes: %u\n\r", pendp->bmAttributes);
                  printf("  wMaxPacketSize: %u\n\r", pendp->wMaxPacketSize);
                  printf("  bInterval: %u\n\r", pendp->bInterval);
                }
                break;
              case 0x21:  // HID
                {
                  Serial.println("HID:");
                  // trying to remember the format of the subunits...
                  uint8_t j;
                  for (j = 6; j < config_buffer[i]; j += 3) {
                    if (config_buffer[i + j] == 0x22) {
                      uint16_t descsize = config_buffer[i + j + 1] + (uint16_t)(config_buffer[i + j + 2] << 8);
                      printf("  HID Descriptor size: %u\n\r", descsize);
                      dumpHIDReportDescriptor(interface_number, descsize);
                      break;
                    }
                  }
                }
                break;

              default:
                Serial.print("Unknown: ");
                MemoryHexDump(Serial, &config_buffer[i], config_buffer[i], false);
                break;
            }
          }


          //Configuration Descriptor
          //2400E8E0 - 09 02 3B 00 02 01 00 A0  32 09 04 00 00 01 03 01  : ..;..... 2.......
          //2400E8F0 - 01 00 09 21 11 01 00 01  22 41 00 07 05 81 03 08  : ...!.... "A......
          //2400E900 - 00 18 09 04 01 00 01 03  00 00 00 09 21 11 01 00  : ........ ....!...
          //2400E910 - 01 22 6D 00 07 05 82 03  08 00 30                 : ."m..... ..0

          // Config:    09 02 3B 00 02 01 00 A0 32
          // Interface: 09 04 00 00 01 03 01 01 00
          //  HID:      09 21 11 01 00 01 22 41 00
          // Endpoint:  07 05 81 03 08
          // Interface: 09 04 01 00 01 03 00 00 0
          // HID:       09 21 11 01 00 01 22 6D 00
          // Endpoint:  07 05 82 03  08 00 30
          free(config_buffer);
        }




        return true;
      }
    }
  }
  init();
  return false;
}

void USBDumperDevice::disconnect() {
  init();  // clear everything
}

void USBDumperDevice::rxHandler() {
  printf("USBDumperDevice::rxHandler() called");
  if (bulk_in) {
    int len = bulk_in->getLengthTransferred();
    MemoryHexDump(Serial, buf, len, true);
    // Setup the next read.
    host->bulkRead(dev, bulk_in, buf, size_bulk_in, false);
  }
}

void USBDumperDevice::txHandler() {
  printf("USBDumperDevice::txHandler() called");
  if (bulk_out) {
  }
}

/*virtual*/ void USBDumperDevice::setVidPid(uint16_t vid, uint16_t pid) {
  // we don't check VID/PID for hser driver
  printf("VID: %X, PID: %X", vid, pid);
}



/*virtual*/ bool USBDumperDevice::parseInterface(uint8_t intf_nb, uint8_t intf_class, uint8_t intf_subclass, uint8_t intf_protocol)  //Must return true if the interface should be parsed
{
  // PL2303 nb:0, cl:255 isub:0 iprot:0
  printf("parseInterface nb:%d\n\r", intf_nb);
  //printf(" bInterfaceNumber = %u\n\r",  descriptors[2]);
  printf(" bInterfaceClass = %u\n\r", intf_class);
  printf(" bInterfaceSubClass = %u\n\r", intf_subclass);

  switch (intf_class) {
    case 2: Serial.println("    Communications and CDC"); break;
    case 3:
      if (intf_subclass == 1) Serial.println("    HID (BOOT)");
      else Serial.println("    HID");
      break;
    case 0xa: Serial.println("    CDC-Data"); break;
  }

  printf(" bProtocol = %u\n\r", intf_protocol);
  if (intf_class == 3) {
    switch (intf_protocol) {
      case 0: Serial.println("    None"); break;
      case 1: Serial.println("    Keyboard"); break;
      case 2: Serial.println("    Mouse"); break;
    }
  }

  return true;
}

/*virtual*/ bool USBDumperDevice::useEndpoint(uint8_t intf_nb, ENDPOINT_TYPE type, ENDPOINT_DIRECTION dir)  //Must return true if the endpoint will be used
{
  printf("useEndpoint(%u, %u, %u)\n\r", intf_nb, type, dir);
  return false;
}

bool USBDumperDevice::manufacturer(uint8_t* buffer, size_t len) {
  cacheStringIndexes();
  return getStringDesc(iManufacturer_, buffer, len);
}

bool USBDumperDevice::product(uint8_t* buffer, size_t len) {
  cacheStringIndexes();
  return getStringDesc(iProduct_, buffer, len);
}

bool USBDumperDevice::serialNumber(uint8_t* buffer, size_t len) {
  cacheStringIndexes();
  return getStringDesc(iSerialNumber_, buffer, len);
}


#define STRING_DESCRIPTOR (3)

bool USBDumperDevice::cacheStringIndexes() {
  if (iManufacturer_ != 0xff) return true;  // already done

  //printf(">>>>> USBDumperDevice::cacheStringIndexes() called <<<<< \n\r");
  DeviceDescriptor device_descriptor;

  USB_TYPE res = host->controlRead(dev,
                                   USB_DEVICE_TO_HOST | USB_RECIPIENT_DEVICE,
                                   GET_DESCRIPTOR,
                                   (DEVICE_DESCRIPTOR << 8) | (0),
                                   0, (uint8_t*)&device_descriptor, DEVICE_DESCRIPTOR_LENGTH);

  if (res != USB_TYPE_OK) {
    //printf("\t Read device descriptor failed: %u\n\r",  res);
    return false;
  }

  iManufacturer_ = device_descriptor.iManufacturer;
  iProduct_ = device_descriptor.iProduct;
  iSerialNumber_ = device_descriptor.iSerialNumber;
  //printf("\tiMan:%u iProd:%u iSer:%u\n\r", iManufacturer_, iProduct_, iSerialNumber_);

  // Now lets try to get the default language ID:
  uint8_t read_buffer[64];
  //printf(">>>>> Get Language ID <<<<<<\n\r");
  res = host->controlRead(dev,
                          USB_DEVICE_TO_HOST | USB_RECIPIENT_DEVICE,
                          GET_DESCRIPTOR,
                          0x300,
                          0, read_buffer, sizeof(read_buffer));
  if (res != USB_TYPE_OK) {
    //printf("\tFailed default to  0x0409 English");
    wLanguageID_ = 0x409;
  } else {
    //MemoryHexDump(Serial, read_buffer, sizeof(read_buffer), true);
    wLanguageID_ = read_buffer[2] | (read_buffer[3] << 8);
    //printf("\tLanguage ID: %x\n\r", wLanguageID_);
  }

  return true;
}

uint16_t USBDumperDevice::getConfigurationDescriptor(uint8_t* buf, uint16_t max_len_buf) {
  USB_TYPE res;
  uint16_t total_conf_descr_length = 0;

  // fourth step: get the beginning of the configuration descriptor to have the total length of the conf descr
  res = host->controlRead(dev,
                          USB_DEVICE_TO_HOST | USB_RECIPIENT_DEVICE,
                          GET_DESCRIPTOR,
                          (CONFIGURATION_DESCRIPTOR << 8) | (0),
                          0, buf, CONFIGURATION_DESCRIPTOR_LENGTH);

  if (res != USB_TYPE_OK) {
    USB_ERR("GET CONF 1 DESCR FAILED");
    return 0;
  }
  total_conf_descr_length = buf[2] | (buf[3] << 8);
  if (max_len_buf == 0) return total_conf_descr_length;

  total_conf_descr_length = min(max_len_buf, total_conf_descr_length);

  USB_DBG("TOTAL_LENGTH: %d \t NUM_INTERF: %d", total_conf_descr_length, buf[4]);

  res = host->controlRead(dev,
                          USB_DEVICE_TO_HOST | USB_RECIPIENT_DEVICE,
                          GET_DESCRIPTOR,
                          (CONFIGURATION_DESCRIPTOR << 8) | (0),
                          0, buf, total_conf_descr_length);
  if (res != USB_TYPE_OK) {
    USB_ERR("controlRead FAILED(%u)", res);
    return 0;
  }
  return total_conf_descr_length;
}


bool USBDumperDevice::getStringDesc(uint8_t index, uint8_t* buffer, size_t len) {

  //printf(">>>>> USBDumperDevice::getStringDesc(%u) called <<<<< \n\r", index);
  if ((index == 0xff) || (index == 0)) return false;


  // Lets reserve space on stack to read in the string, note it is Unicode. so twice the len +
  uint8_t read_len = len * 2 + 2;
  uint8_t read_buffer[read_len];  // will probably give compiler warning about variable length...

  USB_TYPE res = host->controlRead(dev,
                                   USB_DEVICE_TO_HOST | USB_RECIPIENT_DEVICE,
                                   GET_DESCRIPTOR,
                                   (STRING_DESCRIPTOR << 8) | (index),
                                   wLanguageID_, read_buffer, read_len);

  if (res != USB_TYPE_OK) {
    //printf("\t Read string descriptor failed: %u\n\r",  res);
    return false;
  }
  //MemoryHexDump(Serial, read_buffer, read_len, true);

  if (read_buffer[1] != 0x03) return false;

  if (read_buffer[0] > (read_len)) read_buffer[0] = read_len;

  for (uint8_t i = 2; i < read_buffer[0]; i += 2) {
    *buffer++ = read_buffer[i];
  }
  *buffer = '\0';
  return true;
}

// Lets try to read in the HID Descriptor
bool USBDumperDevice::getHIDDesc(uint8_t index, uint8_t* buffer, size_t len) {

  printf(">>>>> USBDumperDevice::getHIDDesc(%u) called <<<<< \n\r", index);
  if (index == 0xff)  return false;

  USB_TYPE res = host->controlRead(dev, 0x81, 6, 0x2200, index, buffer, len);  // get report desc

  if (res != USB_TYPE_OK) {
    //printf("\t Read string descriptor failed: %u\n\r",  res);
    return false;
  }
  MemoryHexDump(Serial, buffer, len, true);
  return true;
}
