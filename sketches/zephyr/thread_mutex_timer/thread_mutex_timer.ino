
K_MUTEX_DEFINE(my_mutex);

void thread1(void)
{
    while (1) {
        if (k_mutex_lock(&my_mutex, K_MSEC(500)) == 0) {
            Serial.print("Thread 1 is accessing the shared resource\n");
            k_sleep(K_MSEC(1000));
            k_mutex_unlock(&my_mutex);
        } else {
            Serial.print("Thread 1 could not acquire the mutex\n");
        }
        k_sleep(K_MSEC(1000));
    }
}

void thread2(void)
{
    while (1) {
        if (k_mutex_lock(&my_mutex, K_MSEC(500)) == 0) {
            Serial.print("Thread 2 is accessing the shared resource\n");
            k_sleep(K_MSEC(1000));
            k_mutex_unlock(&my_mutex);
        } else {
            Serial.print("Thread 2 could not acquire the mutex\n");
        }
        k_sleep(K_MSEC(1000));
    }
}

K_THREAD_DEFINE(thread1_id, 1024, thread1, NULL, NULL, NULL, 7, 0, 0);
K_THREAD_DEFINE(thread2_id, 1024, thread2, NULL, NULL, NULL, 7, 0, 0);

void setup() {
  while(!Serial && millis() < 5000);

}

void loop() {
  // put your main code here, to run repeatedly:
  k_msleep(100);
}
