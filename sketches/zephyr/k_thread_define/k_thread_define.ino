/*
 * Copyright (c) 2017 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <zephyr/kernel.h>

#include "elapsedMillis.h"
elapsedMillis timer;

/* STEP 2 - Define stack size and scheduling priority used by each thread */
#define STACKSIZE 1024

#define THREAD0_PRIORITY 7
#define THREAD1_PRIORITY 7

void thread0(void)
{
  Serial.print("Hello Thread 0 Started\n");
  while (1) {
    /* STEP 3 - Call printk() to display a simple string "Hello, I am thread0" */
    Serial.print("Hello, I am thread0\n");
    /* STEP 6 - Make the thread yield */
    // k_yield();
    /* STEP 10 - Put the thread to sleep */
    k_msleep(5);
    /* Remember to comment out the line from STEP 6 */
  }
}

void thread1(void)
{
  Serial.println("Thread 1 Started...");
  while (1) {
    /* STEP 3 - Call printk() to display a simple string "Hello, I am thread1" */
    Serial.print("Hello, I am thread1\n");
    /* STEP 8 - Make the thread yield */
    // k_yield();
    /* STEP 10 - Put the thread to sleep */
    k_msleep(10);
    /* Remember to comment out the line from STEP 8 */
  }
}

/* STEP 4 - Define and initialize the two threads */
K_THREAD_DEFINE(thread0_id, STACKSIZE, thread0, NULL, NULL, NULL, THREAD0_PRIORITY, 0, 0);
K_THREAD_DEFINE(thread1_id, STACKSIZE, thread1, NULL, NULL, NULL, THREAD1_PRIORITY, 0, 0);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while (!Serial && millis() < 5000) {};
  Serial.println("Threading sketch #2 started");

  delay(100);

k_tid_t our_tid = k_sched_current_thread_query();
  int main_pri = k_thread_priority_get(our_tid);
  Serial.print("main TID: ");
  Serial.print((uint32_t)our_tid, HEX);
  Serial.print(" pri: ");
  Serial.println(main_pri);
  printk("main TID:%x pri:%d\n", (uint32_t)our_tid, main_pri);
  k_thread_priority_set(our_tid, THREAD0_PRIORITY+1);
  main_pri = k_thread_priority_get(our_tid);
  Serial.print("\tupdated pri: ");
  Serial.println(main_pri);
  printk("main TID:%x pri:%d\n", (uint32_t)our_tid, main_pri);

  Serial.println("End Setup");
}

void loop() {
  // put your main code here, to run repeatedly:
  if(timer > 5000){
    Serial.println("called from loop");
    timer = 0;
  }
  k_msleep(100);
}