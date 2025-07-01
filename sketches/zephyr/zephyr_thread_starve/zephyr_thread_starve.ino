#define STACKSIZE 4096
#define SLEEPTIME 500
#define PRIORITY 2

K_THREAD_STACK_DEFINE(blink_pins_thread_stack, STACKSIZE);

static struct k_thread blink_pins_thread_data;

void blink_pins_thread(void *dummy1, void *dummy2, void *dummy3)
{
	ARG_UNUSED(dummy1);
	ARG_UNUSED(dummy2);
	ARG_UNUSED(dummy3);

	Serial.print("thread_a: thread started \n");

	while (1)
	{
    static uint8_t pin_state = 0;
    pin_state ^=1;
    digitalWrite(LED_BUILTIN, pin_state);
  	k_msleep(SLEEPTIME);
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 5000)
    ;

  Serial.println("Thread Test started");
  pinMode(LED_BUILTIN, OUTPUT);
  
  // Start a thread:
    k_thread_create(&blink_pins_thread_data, blink_pins_thread_stack,
      K_THREAD_STACK_SIZEOF(blink_pins_thread_stack),
      blink_pins_thread, NULL, NULL, NULL,
      PRIORITY, 0, K_FOREVER);
  k_thread_name_set(&blink_pins_thread_data, "blink pins");

  Serial.print("Sketch Priority: ");
  Serial.println(k_thread_priority_get(k_sched_current_thread_query()));
  Serial.print("Cooperative: ");
  Serial.println(CONFIG_NUM_COOP_PRIORITIES);
  Serial.print("Preemptive: ");
  Serial.println(CONFIG_NUM_PREEMPT_PRIORITIES);

	k_thread_start(&blink_pins_thread_data);
}

void loop() {
  //k_yield();
  delay(1);
}
