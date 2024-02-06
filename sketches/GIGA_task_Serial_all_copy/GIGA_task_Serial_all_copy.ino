//connect  Serial1 TX -> Serial2 RX, Serial2 TX -> Serial3 RX, Serial3 TX -> Serial4 RX....
// Include Scheduler since we want to manage multiple tasks.
#include <Scheduler.h>


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

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  while (!Serial && millis() < 4000)
    ;
  Serial.begin(115200);
  delay(800);
  Serial.println("Test all Serials");
  Serial.print("Baud rate: ");
  Serial.println(SPD, DEC);
  Serial1.begin(SPD);
  Serial2.begin(SPD);
  Serial3.begin(SPD);
  Serial4.begin(SPD);

  // Start the other tasks
  Scheduler.startLoop(loop2);
  Scheduler.startLoop(loop3);
  Scheduler.startLoop(loop4);
}

#define MIN(a, b) ((a) <= (b) ? (a) : (b))


void CopyFromTo(Stream &SerialF, Stream &SerialT, uint8_t buffer_index) {
#if 0
  int available;
  int cb;
  BufferInfoClass *buf = &buffers[buffer_index];
  if ((available = SerialF.available()) != 0) {
    // available for write not implementd.
    //available_for_write = SerialT.availableForWrite();
    //Serial.print(" ");
    //Serial.println(available_for_write, DEC);
    cb = MIN(available, BUFFER_SIZE);
    if (cb) {
      SerialF.readBytes(&buf->buffer[buf->cb_buffer], cb);
      SerialT.write(&buf->buffer[buf->cb_buffer], cb);
      buf->cb_buffer += cb;
      buf->cb_copy[buf->cb_copy_cnt] = cb;
      buf->cb_copy_cnt++;
      millis_last_input = millis();
    }
  }
#else
  if (SerialF.available() != 0) {
    BufferInfoClass *buf = &buffers[buffer_index];
    int ch = SerialF.read();
    SerialT.write(ch);
    buf->buffer[buf->cb_buffer++] = ch;
    buf->cb_copy[buf->cb_copy_cnt] = 1;
    buf->cb_copy_cnt++;
    millis_last_input = millis();
  }
#endif
  yield();
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

void loop1() {
  CopyFromTo(Serial1, Serial, 1);
}
void loop2() {
  CopyFromTo(Serial2, Serial2, 2);
}
void loop3() {
  CopyFromTo(Serial3, Serial3, 3);
}
void loop4() {
  CopyFromTo(Serial4, Serial4, 4);
}
