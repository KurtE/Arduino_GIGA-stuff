#include <GIGA_digitalWriteFast.h>
#include <RPC.h>
#define LED_PIN 86
void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  for (uint8_t i = 0; i < 5; i++) {
    digitalWriteFast(LED_PIN, HIGH);
    delay(250);
    digitalWriteFast(LED_PIN, LOW);
    delay(250);
  }
  while (!Serial && millis() < 5000) {}
  Serial.println("M7 Started");

  // Lets try quick and dirty test of pin mapping.
  // Lets try quick and dirty test of pin mapping.
  Serial.print("Builtin: ");
  Serial.print(LED_BUILTIN, DEC);
  Serial.print(" Red: ");
  Serial.print(LED_RED, DEC);
  Serial.print(" Green: ");
  Serial.print(LED_GREEN, DEC);
  Serial.print(" Blue: ");
  Serial.println(LED_BLUE, DEC);
  for (pin_size_t pin = 0; pin < PINS_COUNT; pin++) {
    PinName pinname = g_APinDescription[pin].name;
    int pinindex = PinNameToIndex(pinname);
    Serial.print(pin, DEC);
    Serial.print(" ");
    Serial.print(pinname, HEX);
    Serial.print(" P");
    uint8_t port_name = ((pinname >> 4) & 0xf) + 'A';
    Serial.write(port_name);
    Serial.print(pinname & 0xf, DEC);
    Serial.print(" ");
    Serial.println(pinindex, DEC);
  }

  Serial.println("Now Start M4");
  delay(100);
  RPC.begin();
}

char buffer[1024];
uint16_t buffer_count = 0;
uint32_t last_blink = 0;
uint8_t blink_state = 0;

void loop() {
  if ((millis() - last_blink) > 1000) {
    blink_state++;
    digitalToggleFast(LED_PIN);
    last_blink = millis();
  }
  while (RPC.available()) {
    buffer[buffer_count++] = (char)RPC.read();  // Fill the buffer with characters
  }
  if (buffer_count > 0) {
    Serial.write(buffer, buffer_count);
    buffer_count = 0;
  }
}