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

//#if DEVICE_USBDEVICE && defined(SERIAL_CDC)

#include "stdint.h"
#include "PluggableUSBMTP.h"
#include "EndpointResolver.h"
#include "AsyncOp.h"
#include "usb_phy_api.h"
using namespace arduino;

#define FLAG_WRITE_DONE (1 << 0)
#define FLAG_DISCONNECT (1 << 1)
#define FLAG_CONNECT (1 << 2)
#define DEFAULT_CONFIGURATION (1)

USBMTP::USBMTP(bool connect_blocking, const char *name, uint16_t vendor_id, uint16_t product_id, uint16_t product_release)
    : internal::PluggableUSBModule(1), extraDescriptor(name) {
    PluggableUSBD().plug(this);
}

USBMTP::USBMTP(USBPhy *phy, const char *name, uint16_t vendor_id, uint16_t product_id, uint16_t product_release)
    : internal::PluggableUSBModule(1), extraDescriptor(name) {
    PluggableUSBD().plug(this);
}



USBMTP::~USBMTP() {
    PluggableUSBD().deinit();
}

void USBMTP::init(EndpointResolver &resolver) {
    _bulk_in = resolver.endpoint_in(USB_EP_TYPE_BULK, MTP_MAX_PACKET_SIZE);
    _bulk_out = resolver.endpoint_out(USB_EP_TYPE_BULK, MTP_MAX_PACKET_SIZE);
    _int_in = resolver.endpoint_in(USB_EP_TYPE_INT, MTP_EVENT_SIZE);
    MBED_ASSERT(resolver.valid());
}

bool USBMTP::ready() {
    return _flags.get() & FLAG_CONNECT ? true : false;
}

void USBMTP::wait_ready() {
    _flags.wait_any(FLAG_CONNECT, osWaitForever, false);
}


void USBMTP::callback_reset() {
    assert_locked();
    /* Called in ISR context */

    _change_mtp_connected(false);
};


void USBMTP::callback_state_change(USBDevice::DeviceState new_state) {
    assert_locked();

    if (new_state == USBDevice::Configured) {
        //_flags.set(FLAG_CONNECT);
        _flags.clear(FLAG_DISCONNECT);
    } else {
        _flags.set(FLAG_DISCONNECT);
        _flags.clear(FLAG_CONNECT | FLAG_WRITE_DONE);
        _change_mtp_connected(false);

    }
}

uint32_t USBMTP::callback_request(const USBDevice::setup_packet_t *setup, USBDevice::RequestResult *result, uint8_t **data) {
    assert_locked();

    *result = USBDevice::PassThrough;
    uint32_t size = 0;

    // Process additional standard requests
    if (setup->wIndex != pluggedInterface) {
        return size;
    }

    // notes on how they split up that request type byte.
    // packet->bmRequestType.dataTransferDirection = (data[0] & 0x80) >> 7;
    // packet->bmRequestType.Type = (data[0] & 0x60) >> 5;
    // packet->bmRequestType.Recipient = data[0] & 0x1f;
    // packet->bRequest = data[1];
    // packet->wValue = (data[2] | (uint16_t)data[3] << 8);
    // packet->wIndex = (data[4] | (uint16_t)data[5] << 8);
    // packet->wLength = (data[6] | (uint16_t)data[7] << 8);

  #define CANCEL_REQUEST 0x64
  #define Get_Extended_Event_Data 0x65
  #define Device_Reset_Request 0x66
  #define Get_Device_Status 0x67

    if (setup->bmRequestType.Type == CLASS_TYPE) {
      switch (setup->bRequest) {
      case CANCEL_REQUEST: // Cancel Request, Still Image Class 1.0, 5.2.1, page 8
        if (setup->wLength == 6) {
          //endpoint0_setupdata.bothwords = setupdata;
          //endpoint0_receive(endpoint0_buffer, setup.wLength, 1);
          //return;
        }
        break;
      case Get_Extended_Event_Data: // Get Extended Event Data, Still Image Class 1.0, 5.2.2, page 9
        break;
      case Device_Reset_Request: // Device Reset, Still Image Class 1.0, 5.2.3 page 10
        break;
      case Get_Device_Status: // Get Device Status, Still Image Class 1.0, 5.2.4, page 10
        if (setup->wLength >= 4) {
          static uint8_t s_mtp_status[4] = {4, 0, 0, 0x20};
          s_mtp_status[2] = _mtp_connected;
          *result = USBDevice::Send;
          *data = s_mtp_status;
          size = 4;
        }
       break;
      }
    }

    //complete_request(result, data, size);
    return size;
}

uint8_t USBMTP::getProductVersion() {
    return 1;
}

bool USBMTP::callback_request_xfer_done(const USBDevice::setup_packet_t *setup, bool aborted) {
    assert_locked();
    if (setup->wIndex != pluggedInterface) {
        return false;
    }
    return true;
}

bool USBMTP::callback_set_configuration(uint8_t configuration) {
    assert_locked();

    if (configuration == DEFAULT_CONFIGURATION) {
        //complete_set_configuration(false);
    }
    PluggableUSBD().endpoint_add(_int_in, MTP_EVENT_SIZE, USB_EP_TYPE_INT);
    PluggableUSBD().endpoint_add(_bulk_in, MTP_MAX_PACKET_SIZE, USB_EP_TYPE_BULK, mbed::callback(this, &USBMTP::_send_isr));
    PluggableUSBD().endpoint_add(_bulk_out, MTP_MAX_PACKET_SIZE, USB_EP_TYPE_BULK, mbed::callback(this, &USBMTP::_receive_isr));

    PluggableUSBD().read_start(_bulk_out, _rx_buf, sizeof(_rx_buf));

    //complete_set_configuration(true);
    return true;
}

void USBMTP::callback_set_interface(uint16_t interface, uint8_t alternate) {
    assert_locked();

    //complete_set_interface(true);
}

void USBMTP::_change_mtp_connected(bool connected) {
    assert_locked();

    _mtp_connected = connected;
    if (!_mtp_connected) {
        // Abort TX
        if (_tx_in_progress) {
            PluggableUSBD().endpoint_abort(_bulk_in);
            _tx_in_progress = false;
        }
        _tx_buf = _tx_buffer;
        _tx_size = 0;
        _tx_list.process();
        MBED_ASSERT(_tx_list.empty());

        // Abort RX
        if (_rx_in_progress) {
            PluggableUSBD().endpoint_abort(_bulk_in);
            _rx_in_progress = false;
        }
        _rx_buf = _rx_buffer;
        _rx_size = 0;
        _rx_list.process();
        MBED_ASSERT(_rx_list.empty());
    }
    _connected_list.process();
}

void USBMTP::_send_isr_start() {
    assert_locked();

    if (!_tx_in_progress && _tx_size) {
        if (PluggableUSBD().write_start(_bulk_in, _tx_buffer, _tx_size)) {
            _tx_in_progress = true;
        }
    }
}

/*
* Called by when CDC data is sent
* Warning: Called in ISR
*/
void USBMTP::_send_isr() {
    assert_locked();

    PluggableUSBD().write_finish(_bulk_in);
    _tx_buf = _tx_buffer;
    _tx_size = 0;
    _tx_in_progress = false;

    _tx_list.process();
    if (!_tx_in_progress) {
        data_tx();
    }
}


void USBMTP::_receive_isr_start() {
    if ((_rx_size == 0) && !_rx_in_progress) {
        // Refill the buffer
        PluggableUSBD().read_start(_bulk_out, _rx_buffer, sizeof(_rx_buffer));
        _rx_in_progress = true;
    }
}

/*
* Called by when CDC data is received
* Warning: Called in ISR
*/
void USBMTP::_receive_isr() {
    assert_locked();

    MBED_ASSERT(_rx_size == 0);
    _rx_buf = _rx_buffer;
    _rx_size = PluggableUSBD().read_finish(_bulk_out);
    _rx_in_progress = false;
    _rx_list.process();
    if (!_rx_in_progress) {
        data_rx();
    }
}



const uint8_t *USBMTP::string_iinterface_desc() {
    static const uint8_t string_iinterface_descriptor[] = {
        0x08,                    //bLength
        STRING_DESCRIPTOR,       //bDescriptorType 0x03
        'M', 0, 'T', 0, 'P', 0,  //bString iInterface - MTP
    };
    return string_iinterface_descriptor;
}

const uint8_t *USBMTP::string_iproduct_desc() {
    static const uint8_t string_iproduct_descriptor[] = {
        0x12,                                                            //bLength
        STRING_DESCRIPTOR,                                               //bDescriptorType 0x03
        'M', 0, 'b', 0, 'e', 0, 'd', 0, ' ', 0, 'M', 0, 'T', 0, 'P', 0,  //bString iProduct - Mbed MTP
    };
    return string_iproduct_descriptor;
}

const uint8_t *USBMTP::configuration_desc(uint8_t index) {
    if (index != 0) {
        return NULL;
    }
    //#define CONFIG1_DESC_SIZE (9 + 9 + ENDPOINT_DESCRIPTOR_LENGTH + ENDPOINT_DESCRIPTOR_LENGTH + ENDPOINT_DESCRIPTOR_LENGTH)
    if (index != 0) return nullptr;

    uint8_t config_descriptor_temp[] = {
        // configuration descriptor
        9,                       // bLength
        2,                       // bDescriptorType
        LSB(CONFIG1_DESC_SIZE),  // wTotalLength
        MSB(CONFIG1_DESC_SIZE),
        1,     // bNumInterfaces
        1,     // bConfigurationValue
        0,     // iConfiguration
        0x80,  // bmAttributes
        50,    // bMaxPower

        // interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
        9,                 // bLength
        4,                 // bDescriptorType
        pluggedInterface,  // bInterfaceNumber
        0,                 // bAlternateSetting
        3,                 // bNumEndpoints
        0x06,              // bInterfaceClass (0x06 = still image)
        0x01,              // bInterfaceSubClass
        0x01,              // bInterfaceProtocol
        4,                 // iInterface

        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
        ENDPOINT_DESCRIPTOR_LENGTH,  // bLength
        ENDPOINT_DESCRIPTOR,         // bDescriptorType
        _bulk_out,                   // bEndpointAddress
        E_BULK,                      // bmAttributes (0x02=bulk)
        LSB(MTP_MAX_PACKET_SIZE),    // wMaxPacketSize (LSB)
        MSB(MTP_MAX_PACKET_SIZE),    // wMaxPacketSize (MSB)
        0,                           // bInterval
        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
        ENDPOINT_DESCRIPTOR_LENGTH,  // bLength
        ENDPOINT_DESCRIPTOR,         // bDescriptorType
        _bulk_in,                    // bEndpointAddress
        E_BULK,                      // bmAttributes (0x02=bulk)
        LSB(MTP_MAX_PACKET_SIZE),    // wMaxPacketSize (LSB)
        MSB(MTP_MAX_PACKET_SIZE),    // wMaxPacketSize (MSB)
        0,                           // bInterval

        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
        ENDPOINT_DESCRIPTOR_LENGTH,  // bLength
        ENDPOINT_DESCRIPTOR,         // bDescriptorType
        _int_in,                     // bEndpointAddress
        E_INTERRUPT,                 // bmAttributes (0x03=intr)
        LSB(MTP_EVENT_SIZE),         // wMaxPacketSize (LSB)
        MSB(MTP_EVENT_SIZE),         // wMaxPacketSize (MSB)
        MTP_EVENT_INTERVAL           // bInterval
    };
    MBED_ASSERT(sizeof(config_descriptor_temp) == sizeof(_config_descriptor));
    memcpy(_config_descriptor, config_descriptor_temp, sizeof(config_descriptor_temp));
    return _config_descriptor;
}
