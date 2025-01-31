void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while (!Serial && millis() < 5000) {}
  Serial.println("\nTest millis and Micros");
  Serial.println(micros());
}

void loop() {
  unsigned long start_us = micros();
  unsigned long start_ms = millis();
  delay(1000);

  unsigned long end_us = micros();
  unsigned long end_ms = millis();

  unsigned long delta_us = end_us - start_us;
  unsigned long delta_ms = end_ms - start_ms;

  Serial.print("US: ");
  Serial.print(start_us);
  Serial.print(" ");
  Serial.print(end_us);
  Serial.print(" ");
  Serial.print(delta_us);
  Serial.print(" KC32: ");
  Serial.print(k_cycle_get_32());
  Serial.print(" KC64: ");
  Serial.print(k_cycle_get_64());


  Serial.print("\tMS: ");
  Serial.print(start_ms);
  Serial.print(" ");
  Serial.print(end_ms);
  Serial.print(" ");
  Serial.println(delta_ms);

  if (Serial.available()) {
    Serial.println("Paused");
    while (Serial.read() != -1) {}
    while (Serial.read() == -1) {}
    while (Serial.read() != -1) {}
  }
}