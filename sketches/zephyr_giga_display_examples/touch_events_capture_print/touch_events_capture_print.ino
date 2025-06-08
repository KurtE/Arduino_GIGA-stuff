#include <zephyr/input/input.h>

typedef struct {
  uint32_t timeUS;
  struct input_event evt;  
} touch_events_t;

#define MAX_EVENTS 100
touch_events_t events[MAX_EVENTS];
int event_count = 0;

void touch_event_callback(struct input_event *evt, void *user_data) {
    UNUSED(user_data);

  if (event_count < MAX_EVENTS) {
    events[event_count].timeUS = micros();
    memcpy(&events[event_count].evt, evt, sizeof(struct input_event));
    event_count++;
  }
}


extern "C" void registerGigaTouchCallback(void (*cb)(struct input_event *evt, void *user_data));

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while (!Serial && millis() < 5000) {}
  Serial.println("GIGA Zephyr touch test");
  Serial.print("Max Touch Points: ");
  Serial.println(CONFIG_INPUT_GT911_MAX_TOUCH_POINTS, DEC);
  registerGigaTouchCallback(&touch_event_callback);

}

void loop() {
  if(Serial.available()) {
    while(Serial.read() != -1) {};

    if (event_count) {
      uint32_t last_time = events[0].timeUS;
      for (int i=0; i < event_count; i++) {
        Serial.print(" "); Serial.print(events[i].timeUS - last_time);
        Serial.print(": "); Serial.print(events[i].evt.type); 
        Serial.print(" "); Serial.print(events[i].evt.code); 
        Serial.print(" ");  Serial.print(events[i].evt.value);
        if (events[i].evt.sync) {
          Serial.println(" $");
        }
        last_time = events[i].timeUS;
      }
      event_count = 0;
      Serial.println();
    }

  }

  delay(5);

}
