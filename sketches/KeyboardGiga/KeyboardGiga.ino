#include "USBHostGiga.h"

//REDIRECT_STDOUT_TO(Serial)
Keyboard keyb;
HostSerial ser;

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
} __attribute__((packed)) KBDLeds_t;

KBDLeds_t leds_ = { 0 };

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while (!Serial)
    ;
  pinMode(PA_15, OUTPUT);
  keyb.begin();
  ser.begin();
}

extern "C" USBH_HandleTypeDef *g_phost_keyboard;

uint16_t last_request_state = 0xff;


void loop() {
  if (last_request_state != g_phost_keyboard->RequestState) {
    last_request_state = g_phost_keyboard->RequestState;
    Serial.print("RequestState: ");
    Serial.println(last_request_state, HEX);
  }

  if (keyb.available()) {
    auto _key = keyb.read();
    Serial.print(_key.keys[0], HEX);
    Serial.print("=");
    Serial.println(keyb.getAscii(_key));
    switch (_key.keys[0]) {
      case KEY_CAPS_LOCK: toggle_capsLock(); break;
      case KEY_SCROLL_LOCK: toggle_scrollLock(); break;
      case KEY_KEYPAD_NUM_LOCK_AND_CLEAR: toggle_numLock(); break;
    }
  }

  while (ser.available()) {
    auto _char = ser.read();
    Serial.write(_char);
  }
  //delay(1);
}

void toggle_numLock() {
  leds_.numLock = !leds_.numLock;
  updateLEDS();
}

void toggle_capsLock() {
  leds_.capsLock = !leds_.capsLock;
  updateLEDS();
}

void toggle_scrollLock() {
  leds_.scrollLock = !leds_.scrollLock;
  updateLEDS();
}

void LEDS(uint8_t leds) {
  //printf("Keyboard setLEDS %x\n", leds);
  leds_.byte = leds;
  updateLEDS();
}


void updateLEDS() {
  if (!g_phost_keyboard) return;
  Serial.print("Update LEDS ");
  Serial.println(leds_.byte, HEX);
  g_phost_keyboard->Control.setup.b.bmRequestType = 0x21;
  g_phost_keyboard->Control.setup.b.bRequest = 0x9;
  g_phost_keyboard->Control.setup.b.wValue.w = 0x200;
  g_phost_keyboard->Control.setup.b.wIndex.w = 0U;
  g_phost_keyboard->Control.setup.b.wLength.w = 0x1;

  USBH_CtlReq(g_phost_keyboard, (uint8_t *)&leds_.byte, 0x1);
}
