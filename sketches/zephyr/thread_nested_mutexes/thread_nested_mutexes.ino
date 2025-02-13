
K_MUTEX_DEFINE(mutex1);
K_MUTEX_DEFINE(mutex2);

void thread1(void)
{
    Serial.print("Thread 1 Started\n");
    while (1) {
        k_mutex_lock(&mutex1, K_FOREVER);
        Serial.print("Thread 1 locked mutex1\n");
        //k_msleep(500);

        k_mutex_lock(&mutex2, K_FOREVER);
        Serial.print("Thread 1 locked mutex2\n");
        //k_msleep(500);

        Serial.print("Thread 1 is accessing the shared resource\n");
        k_msleep(100);  //was 1000

        k_mutex_unlock(&mutex2);
        Serial.print("Thread 1 unlocked mutex2\n");

        k_mutex_unlock(&mutex1);
        Serial.print("Thread 1 unlocked mutex1\n");

        k_msleep(100);  //was 1000
    }
}

void thread2(void)
{
    Serial.print("Thread 2 Started\n");
    while (1) {
        k_mutex_lock(&mutex2, K_FOREVER);
        Serial.print("Thread 2 locked mutex2\n");
        //k_msleep(500);

        k_mutex_lock(&mutex1, K_FOREVER);
        Serial.print("Thread 2 locked mutex1\n");
        //k_msleep(500);

        Serial.print("Thread 2 is accessing the shared resource\n");
        k_msleep(100);  //was 1000

        k_mutex_unlock(&mutex1);
        Serial.print("Thread 2 unlocked mutex1\n");

        k_mutex_unlock(&mutex2);
        Serial.print("Thread 2 unlocked mutex2\n");

        k_msleep(100);  //was 1000
    }
}

K_THREAD_DEFINE(thread1_id, 1024, thread1, NULL, NULL, NULL, 7, 0, 0);
K_THREAD_DEFINE(thread2_id, 1024, thread2, NULL, NULL, NULL, 7, 0, 0);

void setup() {
  while(!Serial && millis()<5000);
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  k_msleep(250);
}
