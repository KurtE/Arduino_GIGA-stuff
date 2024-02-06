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

#include "USBHostSerialDevice.h"
#include <LibPrintf.h>



/************************************************************/
//  Define mapping VID/PID - to Serial Device type.
/************************************************************/
USBHostSerialDevice::product_vendor_mapping_t USBHostSerialDevice::pid_vid_mapping[] = {
  // FTDI mappings.
  { 0x0403, 0x6001, USBHostSerialDevice::FTDI, 0 },
  { 0x0403, 0x8088, USBHostSerialDevice::FTDI, 1 },  // 2 devices try to claim at interface level
  { 0x0403, 0x6010, USBHostSerialDevice::FTDI, 1 },  // Also Dual Serial, so claim at interface level

  // PL2303
  { 0x67B, 0x2303, USBHostSerialDevice::PL2303, 0 },

  // CH341
  { 0x4348, 0x5523, USBHostSerialDevice::CH341, 0 },
  { 0x1a86, 0x7523, USBHostSerialDevice::CH341, 0 },
  { 0x1a86, 0x5523, USBHostSerialDevice::CH341, 0 },

  // Silex CP210...
  { 0x10c4, 0xea60, USBHostSerialDevice::CP210X, 0 },
  { 0x10c4, 0xea70, USBHostSerialDevice::CP210X, 0 }
};


USBHostSerialDevice::USBHostSerialDevice() {
  host = USBHost::getHostInst();
  init();
}

void USBHostSerialDevice::init() {
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

bool USBHostSerialDevice::connected() {
  return dev_connected;
}

bool USBHostSerialDevice::connect() {
  USB_INFO(" USBHostSerialDevice::connect() called\r\n");
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
    USB_DBG("\tDev: %p\r\n", d);
    if (d != NULL) {
      USB_INFO("Device:%p\r\n", d);
      if (host->enumerate(d, this) != USB_TYPE_OK) {
        USB_INFO("Enumerate returned status not OK");
        continue;  //break;  what if multiple devices?
      }

      printf("\tconnect hser_device_found\n\r");

      bulk_in = d->getEndpoint(intf_SerialDevice, BULK_ENDPOINT, IN);
      USB_INFO("bulk in:%p", bulk_in);

      bulk_out = d->getEndpoint(intf_SerialDevice, BULK_ENDPOINT, OUT);
      USB_INFO(" out:%p\r\n", bulk_out);

      printf("\tAfter get end points\n\r");
      if (bulk_in && bulk_out) {
        dev = d;
        dev_connected = true;
        //        USB_INFO("New hser device: VID:%04x PID:%04x [dev: %p - intf: %d]", dev->getVid(), dev->getPid(), dev, intf_SerialDevice);
        printf("New hser device: VID:%04x PID:%04x [dev: %p - intf: %d]", dev->getVid(), dev->getPid(), dev, intf_SerialDevice);
        dev->setName("Serial", intf_SerialDevice);
        host->registerDriver(dev, intf_SerialDevice, this, &USBHostSerialDevice::init);
        size_bulk_in = bulk_in->getSize();
        size_bulk_out = bulk_out->getSize();

        bulk_in->attach(this, &USBHostSerialDevice::rxHandler);
        bulk_out->attach(this, &USBHostSerialDevice::txHandler);
        host->bulkRead(dev, bulk_in, buf, size_bulk_in, false);
        printf("\n\r>>>>>>>>>>>>>> connected returning true <<<<<<<<<<<<<<<<<<<<\n\r");

        // Each serial type might have their own init sequence required.
        switch (sertype_) {
          default: break;  // don't do anything for the rest of them
          case FTDI: initFTDI(); break;
          case PL2303: initPL2303(); break;
          case CH341: initCH341(); break;
          case CP210X: initCP210X(); break;
        }


        return true;
      }
    }
  }
  init();
  return false;
}

void USBHostSerialDevice::disconnect() {
  init();  // clear everything
}

void USBHostSerialDevice::rxHandler() {
  //printf("USBHostSerialDevice::rxHandler() called");
  if (bulk_in) {
    int len = bulk_in->getLengthTransferred();
    //MemoryHexDump(Serial, buf, len, true);
    _mut.lock();
    for (int i = 0; i < len; i++) {
      rxBuffer.store_char(buf[i]);
    }
    _mut.unlock();

    // Setup the next read.
    host->bulkRead(dev, bulk_in, buf, size_bulk_in, false);
  }
}

void USBHostSerialDevice::txHandler() {
  printf("USBHostSerialDevice::txHandler() called");
  if (bulk_out) {
  }
}

/*virtual*/ void USBHostSerialDevice::setVidPid(uint16_t vid, uint16_t pid) {
  // we don't check VID/PID for hser driver
  USB_INFO("VID: %X, PID: %X\n\r", vid, pid);
  printf("VID: %X, PID: %X", vid, pid);
  sertype_ = UNKNOWN;
  for (uint16_t i = 0; i < (sizeof(pid_vid_mapping) / sizeof(pid_vid_mapping[0])); i++) {
    if ((pid_vid_mapping[i].idVendor == vid) && (pid_vid_mapping[i].idProduct == pid)) {
      sertype_ = pid_vid_mapping[i].sertype;
      break;
    }
  }
  switch (sertype_) {
    default: printf(" Unknown\n\r"); break;
    case FTDI: printf(" FTDI\n\r"); break;
    case PL2303: printf(" PL2303\n\r"); break;
    case CH341: printf(" CH341\n\r"); break;
    case CP210X: printf(" Silex CP210X\n\r"); break;
  }
}



/*virtual*/ bool USBHostSerialDevice::parseInterface(uint8_t intf_nb, uint8_t intf_class, uint8_t intf_subclass, uint8_t intf_protocol)  //Must return true if the interface should be parsed
{
  // PL2303 nb:0, cl:255 isub:0 iprot:0
  USB_INFO("USBHostSerialDevice::parseInterface nb:%d, cl:%u isub:%u iprot:%u\n\r", intf_nb, intf_class, intf_subclass, intf_protocol);
  printf("USBHostSerialDevice::parseInterface nb:%d, cl:%u isub:%u iprot:%u\n\r", intf_nb, intf_class, intf_subclass, intf_protocol);
  if (sertype_ != UNKNOWN) {
    intf_SerialDevice = intf_nb;
    return true;
  }
  return false;
}

/*virtual*/ bool USBHostSerialDevice::useEndpoint(uint8_t intf_nb, ENDPOINT_TYPE type, ENDPOINT_DIRECTION dir)  //Must return true if the endpoint will be used
{
  USB_INFO("USBHostSerialDevice::useEndpoint(%u, %u, %u)\n\r", intf_nb, type, dir);
  printf("USBHostSerialDevice::useEndpoint(%u, %u, %u)\n\r", intf_nb, type, dir);
  if (intf_nb == intf_SerialDevice) {
    //if (type == INTERRUPT_ENDPOINT && dir == IN) return true; // see if we can ignore it later

    if (type == BULK_ENDPOINT && dir == IN) {
      hser_device_found = true;
      return true;
    }
    if (type == BULK_ENDPOINT && dir == OUT) {
      hser_device_found = true;
      return true;
    }
  }
  return false;
}

void USBHostSerialDevice::initFTDI() {
}

void USBHostSerialDevice::initPL2303() {
  // How Teensy USBHost sends the setup message for ths.
  // mk_setup(setup, 0xC0, 0x1, 0x8383, 0, 1)2400B1E0 - C0 01 83 83 00 00 01 00                           : ........
  // mk_setup(setup, 0xC0, 0x1, 0x8484, 0, 1)2400B1E0 - C0 01 84 84 00 00 01 00                           : ........
  // mk_setup(setup, 0x40, 1, 0x0404, 1, 0)2400B1E0 - 40 01 04 04 01 00 00 00                           : @.......
  // mk_setup(setup, 0xC0, 0x1, 0x8484, 0, 1)2400B1E0 - C0 01 84 84 00 00 01 00                           : ........
  // mk_setup(setup, 0xC0, 0x1, 0x8383, 0, 1)2400B1E0 - C0 01 83 83 00 00 01 00                           : ........
  // mk_setup(setup, 0x40, 1, 0, 1, 0)2400B1E0 - 40 01 00 00 01 00 00 00                           : @.......
  // mk_setup(setup, 0x40, 1, 1, 0, 0)2400B1E0 - 40 01 01 00 00 00 00 00                           : @.......
  // mk_setup(setup, 0x40, 1, 2, 0x24, 0)2400B1E0 - 40 01 02 00 24 00 00 00                           : @...$...
  // mk_setup(setup, 0x40, 1, 8, 0, 0)2400B1E0 - 40 01 08 00 00 00 00 00                           : @.......
  // mk_setup(setup, 0x40, 1, 9, 0, 0)2400B1E0 - 40 01 09 00 00 00 00 00                           : @.......
  // mk_setup(setup, 0xA1, 0x21, 0, 0, 7)2400B1E0 - A1 21 00 00 00 00 07 00                           : .!......
  // mk_setup(setup, 0x21, 0x20, 0, 0, 7)2400B1E0 - 21 20 00 00 00 00 07 00                           : ! ......
  // mk_setup(setup, 0x40, 1, 0, 0, 0)2400B1E0 - 40 01 00 00 00 00 00 00                           : @.......
  // mk_setup(setup, 0xA1, 0x21, 0, 0, 7)2400B1E0 - A1 21 00 00 00 00 07 00                           : .!......
  // mk_setup(setup, 0x21, 0x22, 3, 0, 0)2400B1E0 - 21 22 03 00 00 00 00 00                           : !"......
  // mk_setup(setup, 0x21, 0x22, 3, 0, 0)2400B1E0 - 21 22 03 00 00 00 00 00                           : !"......

  // SETUP ; 0x0 ; 0x1 ; [SET_CONFIGURATION I:0x0 L:0x0] ;  0x0 0x9 0x1 0x0 0x0 0x0 0x0 0x0
  // mk_setup(setup, 0xC0, 0x1, 0x8484, 0, 1)2400B1E0 - C0 01 84 84 00 00 01 00                           : ........
  // SETUP ; 0x0 ; 0x1 ; [RT:0xc0 R:0x1 I:0x0 L:0x1] ;  0xc0 0x1 0x84 0x84 0x0 0x0 0x1 0x0
  // IN ; 0x0 ; 0x1 ;  ;  0x2
  // mk_setup(setup, 0x40, 1, 0x0404, 0, 0)2400B1E0 - 40 01 04 04 00 00 00 00                           : @.......
  // SETUP ; 0x0 ; 0x1 ; [RT:0x40 R:0x1 I:0x0 L:0x0] ;  0x40 0x1 0x4 0x4 0x0 0x0 0x0 0x0
  // mk_setup(setup, 0xC0, 0x1, 0x8484, 0, 1)2400B1E0 - C0 01 84 84 00 00 01 00                           : ........
  // SETUP ; 0x0 ; 0x1 ; [RT:0xc0 R:0x1 I:0x0 L:0x1] ;  0xc0 0x1 0x84 0x84 0x0 0x0 0x1 0x0
  // IN ; 0x0 ; 0x1 ;  ;  0x2
  // SETUP ; 0x0 ; 0x1 ; [RT:0xc0 R:0x1 I:0x0 L:0x1] ;  0xc0 0x1 0x83 0x83 0x0 0x0 0x1 0x0
  // IN ; 0x0 ; 0x1 ;  ;  0xff
  // SETUP ; 0x0 ; 0x1 ; [RT:0xc0 R:0x1 I:0x0 L:0x1] ;  0xc0 0x1 0x84 0x84 0x0 0x0 0x1 0x0
  // IN ; 0x0 ; 0x1 ;  ;  0x2
  // SETUP ; 0x0 ; 0x1 ; [RT:0x40 R:0x1 I:0x1 L:0x0] ;  0x40 0x1 0x4 0x4 0x1 0x0 0x0 0x0
  // SETUP ; 0x0 ; 0x1 ; [RT:0xc0 R:0x1 I:0x0 L:0x1] ;  0xc0 0x1 0x84 0x84 0x0 0x0 0x1 0x0
  // IN ; 0x0 ; 0x1 ;  ;  0x2
  // SETUP ; 0x0 ; 0x1 ; [RT:0xc0 R:0x1 I:0x0 L:0x1] ;  0xc0 0x1 0x83 0x83 0x0 0x0 0x1 0x0
  // IN ; 0x0 ; 0x1 ;  ;  0xff
  // SETUP ; 0x0 ; 0x1 ; [RT:0x40 R:0x1 I:0x1 L:0x0] ;  0x40 0x1 0x0 0x0 0x1 0x0 0x0 0x0
  // SETUP ; 0x0 ; 0x1 ; [RT:0x40 R:0x1 I:0x0 L:0x0] ;  0x40 0x1 0x1 0x0 0x0 0x0 0x0 0x0
  // SETUP ; 0x0 ; 0x1 ; [RT:0x40 R:0x1 I:0x24 L:0x0] ;  0x40 0x1 0x2 0x0 0x24 0x0 0x0 0x0
  // SETUP ; 0x0 ; 0x1 ; [RT:0x40 R:0x1 I:0x0 L:0x0] ;  0x40 0x1 0x8 0x0 0x0 0x0 0x0 0x0
  // SETUP ; 0x0 ; 0x1 ; [RT:0x40 R:0x1 I:0x0 L:0x0] ;  0x40 0x1 0x9 0x0 0x0 0x0 0x0 0x0
  // SETUP ; 0x0 ; 0x1 ;  I:0x0 L:0x7] ;  0xa1 0x21 0x0 0x0 0x0 0x0 0x7 0x0
  // IN ; 0x0 ; 0x1 ;  ;  0x80 0x25 0x0 0x0 0x0 0x0 0x0
  // SETUP ; 0x0 ; 0x1 ;  I:0x0 L:0x7] ;  0x21 0x14 0x0 0x0 0x0 0x0 0x7 0x0

  printf("Init PL2303 - strange stuff\n\r");
  host->controlRead(dev, 0xc0, 1, 0x8484, 0, setupdata, 1);
  printf("PL2303: writeRegister(0x04, 0x00)\n\r");
  host->controlWrite(dev, 0x40, 1, 0x0404, 0, nullptr, 0);
  printf("PL2303: readRegister(0x04)\n\r");
  host->controlRead(dev, 0xc0, 1, 0x8484, 0, setupdata, 1);
  printf("PL2303: v1 = readRegister(0x03)\n\r");
  host->controlRead(dev, 0xc0, 1, 0x8383, 0, setupdata, 1);
  printf("PL2303: readRegister(0x04)\n\r");
  host->controlRead(dev, 0xc0, 1, 0x8484, 0, setupdata, 1);
  printf("PL2303: writeRegister(0x04, 0x01)\n\r");
  host->controlWrite(dev, 0x40, 1, 0x0404, 1, nullptr, 0);
  printf("PL2303: readRegister(0x04)\n\r");
  host->controlRead(dev, 0xc0, 1, 0x8484, 0, setupdata, 1);
  printf("PL2303: v2 = readRegister(0x03)\n\r");
  host->controlRead(dev, 0xc0, 1, 0x8383, 0, setupdata, 1);
  printf("PL2303: writeRegister(0, 1)\n\r");
  host->controlWrite(dev, 0x40, 1, 0, 1, nullptr, 0);
  printf("PL2303: writeRegister(1, 0)\n\r");
  host->controlWrite(dev, 0x40, 1, 1, 0, nullptr, 0);
  printf("PL2303: writeRegister(2, 24)\n\r");
  host->controlWrite(dev, 0x40, 1, 2, 0x24, nullptr, 0);
  printf("PL2303: writeRegister(8, 0)\n\r");
  host->controlWrite(dev, 0x40, 1, 8, 0, nullptr, 0);
  printf("PL2303: writeRegister(9, 0)\n\r");
  host->controlWrite(dev, 0x40, 1, 9, 0, nullptr, 0);
  printf("PL2303: Read current Baud/control\n\r");
  host->controlRead(dev, 0xA1, 0x21, 0, 0, setupdata, 7);
}

void USBHostSerialDevice::initCH341() {
}

void USBHostSerialDevice::initCP210X() {
  printf("CP210X:  0x41, 0x11, 0, 0, 0 - reset port\n\r");
  host->controlWrite(dev, 0x41, 0x11, 0, 0, nullptr, 0);

  // set data format
  uint16_t cp210x_format = (format_ & 0xf) << 8;  // This should give us the number of bits.

  // now lets extract the parity from our encoding bits 5-7 and in theres 4-7
  cp210x_format |= (format_ & 0xe0) >> 1;   // they encode bits 9-11
  if (format_ & 0x100) cp210x_format |= 2;  // See if two stop bits
  printf("CP210x setup, cp210x_format %x\n\r", cp210x_format);
  host->controlWrite(dev, 0x41, 3, cp210x_format, 0, nullptr, 0);  // data format 8N1

  // set baud rate
  setupdata[0] = (baudrate)&0xff;  // Setup baud rate 115200 - 0x1C200
  setupdata[1] = (baudrate >> 8) & 0xff;
  setupdata[2] = (baudrate >> 16) & 0xff;
  setupdata[3] = (baudrate >> 24) & 0xff;
  printf("CP210x Set Baud 0x40, 0x1e\n");
  host->controlWrite(dev, 0x40, 0x1e, 0, 0, setupdata, 4);

  // Appears to be an enable command
  memset(setupdata, 0, sizeof(setupdata));  // clear out the data
  printf("CP210x 0x41, 0, 1\n\r");
  host->controlWrite(dev, 0x41, 0, 1, 0, nullptr, 0);

  // MHS_REQUEST
  host->controlWrite(dev, 0x41, 7, 0x0303, 0, nullptr, 0);
  //dtr_rts_ = 3;
  return;
}



/*virtual */ int USBHostSerialDevice::available(void) {
  return rxBuffer.available();
}
/*virtual */ int USBHostSerialDevice::peek(void) {
  return rxBuffer.peek();
}
/*virtual */ int USBHostSerialDevice::read(void) {
  _mut.lock();
  auto ret = rxBuffer.read_char();
  _mut.unlock();
  return ret;
}

/*virtual */ int USBHostSerialDevice::availableForWrite() {
  return 0;
}
/*virtual */ size_t USBHostSerialDevice::write(uint8_t c) {
  return 0;
}
/*virtual */ void USBHostSerialDevice::flush(void) {}


void USBHostSerialDevice::begin(uint32_t baud, uint32_t format) {

  baudrate = baud;
  format_ = format;
  switch (sertype_) {
    default:
    case CDCACM: break;
    case FTDI:
      {
      }
      break;
    case PL2303:
      {
        MemoryHexDump(Serial, setupdata, 7, false, "baud/control before\n");
        setupdata[0] = (baudrate)&0xff;  // Setup baud rate 115200 - 0x1C200
        setupdata[1] = (baudrate >> 8) & 0xff;
        setupdata[2] = (baudrate >> 16) & 0xff;
        setupdata[3] = (baudrate >> 24) & 0xff;
        setupdata[4] = (format & 0x100) ? 2 : 0;  // 0 - 1 stop bit, 1 - 1.5 stop bits, 2 - 2 stop bits
        setupdata[5] = (format & 0xe0) >> 5;      // 0 - None, 1 - Odd, 2 - Even, 3 - Mark, 4 - Space
        setupdata[6] = format & 0x1f;             // Data bits (5, 6, 7, 8 or 16)
        MemoryHexDump(Serial, setupdata, 7, false, "baud/control after\n");
        printf("PL2303: Save out new baud and format\n\r");
        host->controlWrite(dev, 0x21, 0x20, 0, 0, setupdata, 7);

        printf("PL2303: writeRegister(0, 0)\n\r");
        host->controlWrite(dev, 0x40, 1, 0, 0, nullptr, 0);

        printf("PL2303: Read current Baud/control\n\r");
        memset(setupdata, 0, sizeof(setupdata));  // clear it to see if we read it...
        host->controlRead(dev, 0xA1, 0x21, 0, 0, setupdata, 7);
        MemoryHexDump(Serial, setupdata, 7, false, "baud/control read back\n");

        // This sets the control lines (0x1=DTR, 0x2=RTS)
        printf("PL2303: 0x21, 0x22, 0x3\n\r");
        host->controlWrite(dev, 0x21, 0x22, 3, 0, nullptr, 0);

        printf("PL2303: 0x21, 0x22, 0x3 again\n\r");
        host->controlWrite(dev, 0x21, 0x22, 3, 0, nullptr, 0);
      }
      break;  // set more stuff...

    case CH341: break;
    case CP210X: 
      initCP210X();
      break;
  }
}
