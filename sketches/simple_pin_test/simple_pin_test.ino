
uint32_t time_last_led_change;
uint8_t led_state = LOW;
uint8_t other_pin = 0xff;
#define BLINK_SPEED 250

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); 
  while (!Serial && millis() < 5000) {};
  pinMode (LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  time_last_led_change = millis();
  Serial.print("LED PIN: "); Serial.println(LED_BUILTIN, DEC);
}

void loop() {
  if ((millis() - time_last_led_change) > BLINK_SPEED) {
    led_state = (led_state == LOW)? HIGH : LOW;
    digitalWrite(LED_BUILTIN, led_state);
    if (other_pin !=  0xff) digitalWrite(other_pin, led_state);
    time_last_led_change = millis();
  }
  if (Serial.available()) {
    uint8_t new_pin = 0;
    int ch;
    while ((ch = Serial.read()) >= ' ') {
      if ((ch >= '0') && (ch <= '9')) new_pin = new_pin * 10 + ch - '0';
    }
    while (Serial.available()) Serial.read(); // clear rest
    Serial.print("New pin: "); Serial.println(new_pin);
    Serial.flush();
    if (other_pin != 0xff) {
      digitalWrite(other_pin, LOW);
      pinMode(other_pin, INPUT);
    }
    other_pin = new_pin;
    pinMode(other_pin, OUTPUT);
    if ((other_pin !=  0xff) && (other_pin != LED_BUILTIN)) digitalWrite(other_pin, led_state);
    time_last_led_change = millis();
  }
}
