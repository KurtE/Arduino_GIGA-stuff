K_MUTEX_DEFINE(my_mutex);

void thread1(void)
{
    while (1) {
        k_mutex_lock(&my_mutex, K_FOREVER);
        printk("Thread 1 is accessing the shared resource\n");
        k_sleep(K_MSEC(1000));
        k_mutex_unlock(&my_mutex);
        k_sleep(K_MSEC(1000));
    }
}

void thread2(void)
{
    while (1) {
        k_mutex_lock(&my_mutex, K_FOREVER);
        printk("Thread 2 is accessing the shared resource\n");
        k_sleep(K_MSEC(1000));
        k_mutex_unlock(&my_mutex);
        k_sleep(K_MSEC(1000));
    }
}

K_THREAD_DEFINE(thread1_id, 1024, thread1, NULL, NULL, NULL, 7, 0, 0);
K_THREAD_DEFINE(thread2_id, 1024, thread2, NULL, NULL, NULL, 7, 0, 0);


void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:
  k_msleep(1);
}