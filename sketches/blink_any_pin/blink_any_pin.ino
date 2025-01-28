int blink_pin = 2;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  for (uint8_t i=0; i < 4; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
  }
  pinMode(LED_BUILTIN, INPUT);
  Serial.begin(115200);
  while (!Serial) {}
  Serial.println("Quick blink pin test");
  Serial.println (blink_pin);
  pinMode(blink_pin, OUTPUT);
  Serial.print("LED_BUILTIN: ");
  Serial.println(LED_BUILTIN);
}

void loop() {
  digitalWrite(blink_pin, HIGH);
  delay(250);
  digitalWrite(blink_pin, LOW);
  delay(250);
  if (Serial.available()) {
    int new_pin = Serial.parseInt();
    while (Serial.read() != -1) {
      pinMode(blink_pin, INPUT);
      blink_pin = new_pin;
      pinMode(blink_pin, OUTPUT);
      Serial.print("New pin: ");
      Serial.println(blink_pin);
    }
  }
}

