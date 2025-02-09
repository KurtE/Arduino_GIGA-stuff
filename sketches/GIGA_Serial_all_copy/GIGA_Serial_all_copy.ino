//connect  Serial1 TX -> Serial2 RX, Serial2 TX -> Serial3 RX, Serial3 TX -> Serial4 RX....
#define USE_SERIAL2
#define USE_SERIAL3
//#define USE_SERIAL4

#define SPD 115200
int loop_count = 0;

#define BUFFER_SIZE 80

class BufferInfoClass {
public:
  BufferInfoClass() {
    clear();
  }
  char buffer[BUFFER_SIZE];
  uint8_t cb_buffer;
  uint8_t cb_copy[BUFFER_SIZE];
  uint8_t cb_copy_cnt;
  void clear() {
    cb_buffer = 0;
    cb_copy_cnt = 0;
  }
};

BufferInfoClass buffers[5];
uint32_t millis_last_input = 0;
extern void wait_for_user_input();

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  while (!Serial && millis() < 4000)
    ;
  Serial.begin(SPD);
  delay(800);
  Serial.println("Test all Serials");
  Serial.print("Baud rate: ");
  Serial.println(SPD, DEC);
  wait_for_user_input();
  Serial1.begin(SPD);
  Serial1.println("Serial1 started");
#ifdef USE_SERIAL2
  //wait_for_user_input();
  Serial2.begin(SPD);
  Serial2.println("Serial2 started");
#endif
#ifdef USE_SERIAL3
  //wait_for_user_input();
  Serial3.begin(SPD);
  Serial3.println("Serial3 started");
#endif
#ifdef USE_SERIAL4
  //wait_for_user_input();
  Serial4.begin(SPD);
  Serial4.println("Serial4 started");
#endif
  wait_for_user_input();
}

void wait_for_user_input() {
  Serial.println("Waiting...");
#if 0
  while (!Serial.available()) {}
  while (Serial.available()) {Serial.read();}
#else
  while (Serial.read() == -1) {};
  while (Serial.read() != -1) {};
#endif
}

#define MIN(a, b) ((a) <= (b) ? (a) : (b))


void CopyFromTo(Stream &SerialF, Stream &SerialT, uint8_t buffer_index) {
  int available;
  int available_for_write;
  int cb;
  BufferInfoClass *buf = &buffers[buffer_index];
  if ((available = SerialF.available()) != 0) {
    Serial.print(buffer_index, DEC);
    Serial.print(":Avail: ");
    Serial.print(available, DEC);
    available_for_write = SerialT.availableForWrite();
    Serial.print(" ");
    Serial.println(available_for_write, DEC);
    cb = MIN(MIN(available, available_for_write), BUFFER_SIZE);
    if (cb) {
      SerialF.readBytes(&buf->buffer[buf->cb_buffer], cb);
      SerialT.write(&buf->buffer[buf->cb_buffer], cb);
      buf->cb_buffer += cb;
      buf->cb_copy[buf->cb_copy_cnt] = cb;
      buf->cb_copy_cnt++;
      millis_last_input = millis();
    }
  }
}

void memory_dump(const char *pb, uint8_t cb) {
  const char *pbA = pb;
  uint8_t cbA = cb;

  Serial.print("\t");
  for (uint8_t i = 0; i < cb; i++) {
    if (*pb < 0x10) Serial.write('0');
    Serial.print(*pb, HEX);
    Serial.print(" ");
    pb++;
  }

  Serial.print("\n\t ");
  for (uint8_t i = 0; i < cbA; i++) {
    if (*pbA >= ' ') Serial.write(*pbA);
    else Serial.write(' ');
    Serial.print("  ");
    pbA++;
  }
  Serial.println();
}

void print_buffer_header(uint8_t index) {
  BufferInfoClass *buf = &buffers[index];
  Serial.print("  ");
  Serial.print(buf->cb_buffer, DEC);
  Serial.print(" (");
  for (uint8_t i = 0; i < buf->cb_copy_cnt; i++) {
    if (i != 0) Serial.print(",");
    Serial.print(buf->cb_copy[i], DEC);
  }
  Serial.print(")");
}

void CompareBuffers(uint8_t index1, uint8_t index2) {
  if (buffers[index1].cb_buffer == buffers[index2].cb_buffer) {
    if (memcmp(buffers[index1].buffer, buffers[index2].buffer, buffers[index1].cb_buffer) == 0) {
      Serial.println(" ** Match **");
      return;
    } else {
      Serial.println(" ** different **");
    }
  } else {
    Serial.println(" ** counts different **");
  }
  memory_dump(buffers[index1].buffer, buffers[index1].cb_buffer);
  memory_dump(buffers[index2].buffer, buffers[index2].cb_buffer);
}

void loop() {
  CopyFromTo(Serial, Serial1, 0);
  CopyFromTo(Serial1, Serial, 1);
#ifdef USE_SERIAL2
  CopyFromTo(Serial2, Serial2, 2);
#endif
#ifdef USE_SERIAL3
  CopyFromTo(Serial3, Serial3, 3);
#endif
#ifdef USE_SERIAL4
  CopyFromTo(Serial4, Serial4, 4);
#endif

  // now see if we should compare the data yet or not
  if (buffers[0].cb_buffer && ((millis() - millis_last_input) > 100)) {
    Serial.println("Check buffers: ");

    print_buffer_header(0);
    Serial.println();

    for (uint8_t i = 1; i < 5; i++) {
      print_buffer_header(i);
      CompareBuffers(0, i);
    }
    for (uint8_t i = 0; i < 5; i++) buffers[i].clear();
  }

  digitalWrite(LED_BUILTIN, HIGH);
  delayMicroseconds(100);  // give time form things to propagate
  digitalWrite(LED_BUILTIN, LOW);
}
