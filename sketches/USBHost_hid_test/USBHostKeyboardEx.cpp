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

#include "USBHostKeyboardEx.h"

#if USBHOST_KEYBOARD

static const uint8_t keymap[4][0x39] = {
  { 0, 0, 0, 0, 'a', 'b' /*0x05*/,
    'c', 'd', 'e', 'f', 'g' /*0x0a*/,
    'h', 'i', 'j', 'k', 'l' /*0x0f*/,
    'm', 'n', 'o', 'p', 'q' /*0x14*/,
    'r', 's', 't', 'u', 'v' /*0x19*/,
    'w', 'x', 'y', 'z', '1' /*0x1E*/,
    '2', '3', '4', '5', '6' /*0x23*/,
    '7', '8', '9', '0', 0x0A /*enter*/,                                     /*0x28*/
    0x1B /*escape*/, 0x08 /*backspace*/, 0x09 /*tab*/, 0x20 /*space*/, '-', /*0x2d*/
    '=', '[', ']', '\\', '#',                                               /*0x32*/
    ';', '\'', 0, ',', '.',                                                 /*0x37*/
    '/' },

  /* CTRL MODIFIER */
  {
    0, 0, 0, 0, 0, 0 /*0x05*/,
    0, 0, 0, 0, 0 /*0x0a*/,
    0, 0, 0, 0, 0 /*0x0f*/,
    0, 0, 0, 0, 0 /*0x14*/,
    0, 0, 0, 0, 0 /*0x19*/,
    0, 0, 0, 0, 0 /*0x1E*/,
    0, 0, 0, 0, 0 /*0x23*/,
    0, 0, 0, 0, 0 /*enter*/, /*0x28*/
    0, 0, 0, 0, 0,           /*0x2d*/
    0, 0, 0, 0, 0,           /*0x32*/
    0, 0, 0, 0, 0,           /*0x37*/
    0 },

  /* SHIFT MODIFIER */
  {
    0, 0, 0, 0, 'A', 'B' /*0x05*/,
    'C', 'D', 'E', 'F', 'G' /*0x0a*/,
    'H', 'I', 'J', 'K', 'L' /*0x0f*/,
    'M', 'N', 'O', 'P', 'Q' /*0x14*/,
    'R', 'S', 'T', 'U', 'V' /*0x19*/,
    'W', 'X', 'Y', 'Z', '!' /*0x1E*/,
    '@', '#', '$', '%', '^' /*0x23*/,
    '&', '*', '(', ')', 0,   /*0x28*/
    0, 0, 0, 0, 0,           /*0x2d*/
    '+', '{', '}', '|', '~', /*0x32*/
    ':', '"', 0, '<', '>',   /*0x37*/
    '?' },

  /* ALT MODIFIER */
  {
    0, 0, 0, 0, 0, 0 /*0x05*/,
    0, 0, 0, 0, 0 /*0x0a*/,
    0, 0, 0, 0, 0 /*0x0f*/,
    0, 0, 0, 0, 0 /*0x14*/,
    0, 0, 0, 0, 0 /*0x19*/,
    0, 0, 0, 0, 0 /*0x1E*/,
    0, 0, 0, 0, 0 /*0x23*/,
    0, 0, 0, 0, 0 /*enter*/, /*0x28*/
    0, 0, 0, 0, 0,           /*0x2d*/
    0, 0, 0, 0, 0,           /*0x32*/
    0, 0, 0, 0, 0,           /*0x37*/
    0 }

};

#define KEY_CAPS_LOCK           (  0x39 )
#define KEY_SCROLL_LOCK         (  0x47 )
#define KEY_NUM_LOCK            (  0x53 )


USBHostKeyboardEx::USBHostKeyboardEx() {
  // Don't reset these each time...
  onKey = NULL;
  onKeyCode = NULL;
  onKeyRelease = NULL;
  init();
}


void USBHostKeyboardEx::init() {
  dev = NULL;
  int_in = NULL;
  report_id = 0;
  dev_connected = false;
  keyboard_intf = -1;
  keyboard_device_found = false;
}

bool USBHostKeyboardEx::connected() {
  return dev_connected;
}


bool USBHostKeyboardEx::connect() {

  if (dev_connected) {
    return true;
  }
  host = USBHost::getHostInst();

  for (uint8_t i = 0; i < MAX_DEVICE_CONNECTED; i++) {
    if ((dev = host->getDevice(i)) != NULL) {

      int ret = host->enumerate(dev, this);
      if (ret) {
        break;
      }


      if (keyboard_device_found) {
        {
          /* As this is done in a specific thread
                     * this lock is taken to avoid to process the device
                     * disconnect in usb process during the device registering */
          USBHost::Lock Lock(host);

          int_in = dev->getEndpoint(keyboard_intf, INTERRUPT_ENDPOINT, IN);

          if (!int_in) {
            break;
          }

          USB_INFO("New Keyboard device: VID:%04x PID:%04x [dev: %p - intf: %d]", dev->getVid(), dev->getPid(), dev, keyboard_intf);
          dev->setName("Keyboard", keyboard_intf);
          host->registerDriver(dev, keyboard_intf, this, &USBHostKeyboardEx::init);

          int_in->attach(this, &USBHostKeyboardEx::rxHandler);
        }
        host->interruptRead(dev, int_in, report, int_in->getSize(), false);

        dev_connected = true;
        return true;
      }
    }
  }
  init();
  return false;
}

static bool contains(uint8_t b, const uint8_t *data) {
  if (data[2] == b || data[3] == b || data[4] == b) return true;
  if (data[5] == b || data[6] == b || data[7] == b) return true;
  return false;
}


void USBHostKeyboardEx::rxHandler() {
  int len = int_in->getLengthTransferred();
  //int index = (len == 9) ? 1 : 0;
  int len_listen = int_in->getSize();
  uint8_t key = 0;
  if (len == 8 || len == 9) {
    // boot format: byte 0 mod, 1=skip, 2-7 keycodes
    if (memcmp(report, prev_report, len)) {
      //printf("USBHostKeyboardEx::rxHandler l:%u %u = ", len, len_listen);
      //for (uint8_t i = 0; i < len; i++) printf("%02X ", report[i]);
      //printf("\n");


      uint8_t modifier = report[0];
      // first check for new key presses
      for (uint8_t i = 2; i < 8; i++) {
        uint8_t keycode = report[i];
        if (keycode == 0) break;  // no more keys pressed
        if (!contains(keycode, prev_report)) {
          // new key press
          if (onKey) {
            // This is pretty lame... and buggy
            key = mapKeycodeToKey(modifier, keycode);
            //printf("key: %x(%c)\n", key, key);
            if (key) (*onKey)(key);
          }
          if (onKeyCode) (*onKeyCode)(report[i], modifier);
        }
      }
      // next check for releases.
      for (uint8_t i = 2; i < 8; i++) {
        uint8_t keycode = prev_report[i];
        if (keycode == 0) break;  // no more keys pressed
        if (!contains(keycode, report)) {
          if (onKeyRelease) {
            key = mapKeycodeToKey(prev_report[0], prev_report[i]);
            if (key) (*onKeyRelease)(key);
          }
          // Now see if this is one of the modifier keys
          if (keycode == KEY_NUM_LOCK) {
            numLock(!leds_.numLock);
            // Lets toggle Numlock
          } else if (keycode == KEY_CAPS_LOCK) {
            capsLock(!leds_.capsLock);

          } else if (keycode == KEY_SCROLL_LOCK) {
            scrollLock(!leds_.scrollLock);
          }
        }
      }
      memcpy(prev_report, report, 8);
    }
  }
  if (dev && int_in) {
    host->interruptRead(dev, int_in, report, len_listen, false);
  }
}

uint8_t USBHostKeyboardEx::mapKeycodeToKey(uint8_t modifier, uint8_t keycode) {
  modifier = (modifier | (modifier >> 4)) & 0xf;  // merge left and right modifiers.
  if (modifier >= 4) return 0;
  if (keycode >= 0x39) return 0;
  return keymap[modifier][keycode];
}
void USBHostKeyboardEx::numLock(bool f) {
  if (leds_.numLock != f) {
    leds_.numLock = f;
    updateLEDS();
  }
}

void USBHostKeyboardEx::capsLock(bool f) {
  if (leds_.capsLock != f) {
    leds_.capsLock = f;
    updateLEDS();
  }
}

void USBHostKeyboardEx::scrollLock(bool f) {
  if (leds_.scrollLock != f) {
    leds_.scrollLock = f;
    updateLEDS();
  }
}

void USBHostKeyboardEx::LEDS(uint8_t leds) {
  printf("Keyboard setLEDS %x\n", leds);
  leds_.byte = leds;
  updateLEDS();
}

void USBHostKeyboardEx::updateLEDS() {
  if (host && dev) {
    printf("$$$ updateLEDS: %x\n", leds_.byte);
    host->controlWrite(dev, 0x21, 9, 0x200, 0, (uint8_t *)&leds_.byte, sizeof(leds_.byte));
  }
}



/*virtual*/ void USBHostKeyboardEx::setVidPid(uint16_t vid, uint16_t pid) {
  // we don't check VID/PID for keyboard driver
}

/*virtual*/ bool USBHostKeyboardEx::parseInterface(uint8_t intf_nb, uint8_t intf_class, uint8_t intf_subclass, uint8_t intf_protocol)  //Must return true if the interface should be parsed
{
  //printf("intf_class: %d\n", intf_class);
  //printf("intf_subclass: %d\n", intf_subclass);
  //printf("intf_protocol: %d\n", intf_protocol);

  if ((keyboard_intf == -1) && (intf_class == HID_CLASS) && (intf_subclass == 0x01) && (intf_protocol == 0x01)) {
    keyboard_intf = intf_nb;
    return true;
  }
  return false;
}

/*virtual*/ bool USBHostKeyboardEx::useEndpoint(uint8_t intf_nb, ENDPOINT_TYPE type, ENDPOINT_DIRECTION dir)  //Must return true if the endpoint will be used
{
  printf("intf_nb: %d\n", intf_nb);
  if (intf_nb == keyboard_intf) {
    if (type == INTERRUPT_ENDPOINT && dir == IN) {
      keyboard_device_found = true;
      return true;
    }
  }
  return false;
}

#endif
