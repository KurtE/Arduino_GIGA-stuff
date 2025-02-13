
K_MUTEX_DEFINE(my_mutex);

//#include "SDRAM.h"
uint8_t *mySmallArray = nullptr;
uint8_t multiplier = 1;

void thread1(void)
{
    while (1) {
        k_mutex_lock(&my_mutex, K_FOREVER);
        Serial.print("Thread 1 is accessing the shared resource\n");
        mySmallArray = (uint8_t*)malloc(128);
        for (int i = 0; i<128; i++) {
            mySmallArray[i] =  i * multiplier;
        }
        k_sleep(K_MSEC(100));
        k_mutex_unlock(&my_mutex);
        k_sleep(K_MSEC(1000));
    }
}

void thread2(void)
{
    while (1) {
        k_mutex_lock(&my_mutex, K_FOREVER);
        Serial.print("Thread 2 is accessing the shared resource\n");
        for(uint8_t i = 0; i < 128; i++) {
          Serial.print(mySmallArray[i]); Serial.print(", ");
        }
        Serial.println();
        // free the memory when you don't need them anymore
        //SDRAM.free(mySmallArray);
        free(mySmallArray);
        if(multiplier == 1) {
          multiplier = 2;
        } else {
          multiplier = 1;
        }
        k_sleep(K_MSEC(100));
        k_mutex_unlock(&my_mutex);
        k_sleep(K_MSEC(1000));
    }
}

K_THREAD_DEFINE(thread1_id, 1024, thread1, NULL, NULL, NULL, 4, 0, 0);
K_THREAD_DEFINE(thread2_id, 1024, thread2, NULL, NULL, NULL, 4, 0, 0);


void setup() {
  Serial.begin(115200);
  while(!Serial && millis() < 5000);

  //SDRAM.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  k_msleep(100);
}
