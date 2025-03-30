void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 4000) {}

  for (int pin_index = 0; pin_index < PINS_COUNT; pin_index++) {
    Serial.print(pin_index);
    Serial.print(", 0x");
    Serial.print((uint32_t)g_pin_cfg[pin_index].pin, HEX);
    const uint16_t *plist = g_pin_cfg[pin_index].list;
    while (plist) {
      uint16_t list_item = *plist;
      Serial.print(", 0x");
      Serial.print(list_item, HEX);
      Serial.print(", ");
      if (list_item & SCI_CHANNEL) {
        Serial.print("SCI:");
      }
      switch (list_item & PIN_USE_MASK) {
        case PIN_UNAVAILABLE: Serial.print("UNAVAILABLE"); break;
        case PIN_INTERRUPT: Serial.print("INTERRUPT"); break;
        case PIN_SCL: Serial.print("SCL"); break;
        case PIN_SDA: Serial.print("SDA"); break;
        case PIN_RX_MISO_SCL: Serial.print("RX_MISO_SCL"); break;
        case PIN_TX_MOSI_SDA: Serial.print("TX_MOSI_SDA"); break;
        case PIN_MISO: Serial.print("MISO"); break;
        case PIN_MOSI: Serial.print("MOSI"); break;
        case PIN_SCK: Serial.print("SCK"); break;
        case PIN_PWM: Serial.print("PWM"); break;
        case PIN_ANALOG: Serial.print("ANALOG"); break;
        case PIN_CTS_RTS_SS: Serial.print("CTS_RTS_SS"); break;
        case PIN_PWM_AGT: Serial.print("PWM_AGT"); break;
        case PIN_CAN_TX: Serial.print("CAN_TX"); break;
        case PIN_CAN_RX: Serial.print("CAN_RX"); break;
        case PIN_DAC: Serial.print("DAC"); break;
      }
      Serial.print("[");
      Serial.print(GET_CHANNEL(list_item));
      Serial.print("]");
      if (list_item & LAST_ITEM_MASK) break;
      plist++;
    }
    Serial.println();
  }
}

void loop() {
  // put your main code here, to run repeatedly:
}
