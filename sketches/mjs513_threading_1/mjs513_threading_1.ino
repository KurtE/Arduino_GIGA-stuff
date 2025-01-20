/* main.c - Hello World demo */

/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */


/* size of stack area used by each thread */
#define STACKSIZE 1024

/* scheduling priority used by each thread */
#define PRIORITY 7

/* delay between greetings (in ms) */
#define SLEEPTIME 500


K_THREAD_STACK_DEFINE(threadA_stack_area, STACKSIZE);
static struct k_thread threadA_data;

/* threadA is a static thread that is spawned automatically */

void threadA(void *dummy1, void *dummy2, void *dummy3)
{
	ARG_UNUSED(dummy1);
	ARG_UNUSED(dummy2);
	ARG_UNUSED(dummy3);

	Serial.print("thread_a: thread started \n");

	while (1)
	{
		Serial.print("thread_a: thread loop \n");
		k_msleep(SLEEPTIME);
	}

}


void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  while (!Serial && millis() < 5000) {}
  Serial.println("Threading 1 start");
	k_tid_t tid = k_thread_create(&threadA_data, threadA_stack_area,
			K_THREAD_STACK_SIZEOF(threadA_stack_area),
			threadA, NULL, NULL, NULL,
			PRIORITY, 0, K_FOREVER);
  Serial.print("Thread ID: ");
  Serial.println((uint32_t)tid, HEX);
	k_thread_name_set(&threadA_data, "thread_a");

	k_thread_start(&threadA_data);

  //k_tid_t our_tid = k_current_get();
  k_tid_t our_tid = k_sched_current_thread_query();
  int main_pri = k_thread_priority_get(our_tid);
  Serial.print("main TID: ");
  Serial.print((uint32_t)our_tid, HEX);
  Serial.print(" pri: ");
  Serial.println(main_pri);
  printk("main TID:%x pri:%d\n", (uint32_t)our_tid, main_pri);
  k_thread_priority_set(our_tid, PRIORITY+1);
  main_pri = k_thread_priority_get(our_tid);
  Serial.print("\tupdated pri: ");
  Serial.println(main_pri);
  printk("main TID:%x pri:%d\n", (uint32_t)our_tid, main_pri);
  #if defined(CONFIG_USERSPACE)
  Serial.println("CONFIG_USERSPACE is defined");
  #else
  Serial.println("CONFIG_USERSPACE is **NOT** defined");
  #endif
  Serial.println("End Setup");
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  k_msleep(SLEEPTIME);

}