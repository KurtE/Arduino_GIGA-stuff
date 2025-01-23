void setup() {
  Serial.begin(115200);
  while(!Serial && (millis() < 4000)) {}
  Serial.println("Quick and dirty test");
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_BUILTIN+1, OUTPUT);
  pinMode(LED_BUILTIN+2, OUTPUT);
}

int loop_count = 0;
void loop() {
  Serial.print("Loop count: ");
  Serial.println(++loop_count);
  Serial.flush();
  digitalWrite(LED_BUILTIN, (loop_count & 1)? HIGH : LOW);
  digitalWrite(LED_BUILTIN+1, (loop_count & 2)? HIGH : LOW);
  digitalWrite(LED_BUILTIN+2, (loop_count & 4)? HIGH : LOW);
  delay(250);
}