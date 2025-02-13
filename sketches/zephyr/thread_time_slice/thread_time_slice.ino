#define STACKSIZE 1024

/* STEP 5 - Change the priority of thread0 to 6 */
#define THREAD0_PRIORITY 7
#define THREAD1_PRIORITY 7

/*
 * TODO - how to change timeslice programatically
 * timeslice applies to threads of same priority
  */

#define time_slice 10

void thread0(void)
{
	while (1) {
		Serial.print("Hello, I am thread0\n");
		k_busy_wait(time_slice * 100000);
	}
}

void thread1(void)
{
	while (1) {
		Serial.print("Hello, I am thread1\n");
		k_busy_wait(time_slice * 100000);
	}
}

K_THREAD_DEFINE(thread0_id, STACKSIZE, thread0, NULL, NULL, NULL, THREAD0_PRIORITY, 0, 0);
K_THREAD_DEFINE(thread1_id, STACKSIZE, thread1, NULL, NULL, NULL, THREAD1_PRIORITY, 0, 0);

void setup() {
  while (!Serial && millis() < 5000) {};
  Serial.begin(115200);
  Serial.println("Threading time slice sketch  started");
k_tid_t our_tid = k_sched_current_thread_query();
  int main_pri = k_thread_priority_get(our_tid);
  Serial.print("main TID: ");
  Serial.print((uint32_t)our_tid, HEX);
  Serial.print(" pri: ");
  Serial.println(main_pri);
  printk("main TID:%x pri:%d\n", (uint32_t)our_tid, main_pri);
  //k_thread_priority_set(our_tid, THREAD0_PRIORITY+1);
  //main_pri = k_thread_priority_get(our_tid);
  //Serial.print("\tupdated pri: ");
  //Serial.println(main_pri);
  //printk("main TID:%x pri:%d\n", (uint32_t)our_tid, main_pri);
}

void loop() {
  // put your main code here, to run repeatedly:
  k_msleep(time_slice);
}
