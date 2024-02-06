#include <RPC.h>
Stream *USERIAL = nullptr;

#include "SDRAM.h"

// Data constructors
struct IMU_DATA {
  float ax = 0.0;
  float ay = 0.0;
  float az = 0.0;
  float gx = 0.0;
  float gy = 0.0;
  float gz = 0.0;
  float mx = 0.0;
  float my = 0.0;
  float mz = 0.0;
} __attribute__((aligned(8)));

struct DATA_FRAME_SEND {
    IMU_DATA imu_data;
} __attribute__((aligned(8)));

struct DATA_FRAME_RETURN {
  char debugStringData[100];
} __attribute__((aligned(8)));

struct SEND_FRAME_READY_FLAG {
  volatile bool frame_ready = false;
} __attribute__((aligned(8)));

struct RETURN_FRAME_READY_FLAG {
  volatile bool frame_ready = false;
} __attribute__((aligned(8)));

//SDRAM Pointers
const uint32_t SDRAM_START_ADDRESS_4 = ((uint32_t)0x38000000);  //USING THE AHB SRAM4 DOMAIN SPACE
volatile uint32_t *sdramMemory = (uint32_t*)0x38000000;

SEND_FRAME_READY_FLAG* send_frame_ready_sdram = (SEND_FRAME_READY_FLAG*)(sdramMemory);
RETURN_FRAME_READY_FLAG* return_frame_ready_sdram = (RETURN_FRAME_READY_FLAG*)(sdramMemory + sizeof(SEND_FRAME_READY_FLAG) / sizeof(uint32_t));
DATA_FRAME_SEND* data_frame_send_sdram = (DATA_FRAME_SEND*)(sdramMemory + (sizeof(SEND_FRAME_READY_FLAG) + sizeof(RETURN_FRAME_READY_FLAG)) / sizeof(uint32_t));
DATA_FRAME_RETURN* data_frame_return_sdram = (DATA_FRAME_RETURN*)(sdramMemory + (sizeof(SEND_FRAME_READY_FLAG) + sizeof(RETURN_FRAME_READY_FLAG) + sizeof(DATA_FRAME_SEND)) / sizeof(uint32_t));

#include "icm20948.h"

/* Mpu9250 object */
bfs::Icm20948 imu;
volatile bool isrFired = false;

void setup() {
 if (HAL_GetCurrentCPUID() == CM7_CPUID) {
  while (!Serial && millis() < 5000) {}
    Serial.begin(115200);
    USERIAL = &Serial;
  } else {
    RPC.begin();
    USERIAL = &RPC;
    delay(1000);
  }

  SDRAM.begin(SDRAM_START_ADDRESS_4);

  pinMode(LED_RED, OUTPUT);
  for (uint8_t i = 0; i < 5; i++ ) {
    digitalWrite(LED_RED, HIGH);
    delay(250);
    digitalWrite(LED_RED, LOW);
    delay(250);
  }

  USERIAL->println("M4 Started\n - now to start M4");
  delay(100);

  Wire1.begin();
  Wire1.setClock(400000);
  //delay(5000);
  /* I2C bus,  0x68 address */
  imu.Config(&Wire1, bfs::Icm20948::I2C_ADDR_SEC);
  /* Initialize and configure IMU */
  if (!imu.Begin()) {
    USERIAL->println("Error initializing communication with IMU");
    while(1) {}
  }
  /* Set the sample rate divider for both accelerometer and gyroscope*/
  /* 
    rate = 1125/(SRD + 1) HZ
    ==========================
    SRD 8  => rate = 125 HZ
    SRD 9  => rate = 112.5 HZ
    SRD 10 => rate = 102.3 HZ
    SRD 11 => rate = 93.75 HZ
    SRD 19 => rate = 56.25 HZ
    SRD 20 => rate = 53,57 HZ
    SRD 21 => rate = 51,14 HZ
    SRD 22 => rate = 48.91 HZ
    when using SRD to set sampling rate magnetometer is 
    automatically set to 50Hz for SRD's > 10 otherwise it
    is set to 100Hz.
   */
  if (!imu.ConfigSrd(21)) {
    USERIAL->println("Error configured SRD");
    while(1) {}
  }
  // enabling the data ready interrupt
  // currently only the data ready interrupt is supported
  imu.EnableDrdyInt();
  // attaching the interrupt to microcontroller pin 1
  pinMode(2,INPUT);
  attachInterrupt(digitalPinToInterrupt(2), sensorISR, RISING);
}

void loop() {
  if (isrFired)
  { // If our isr flag is set then clear the interrupts on the ICM
    isrFired = false;
    getIMU();
  }
  
  receiveDataFromM4();

}


void sensorISR(void)
{
  isrFired = true; // Can't use I2C within ISR on 328p, so just set a flag to know that data is available
}

float imu_values[9];
void getIMU() {
  /* Check if data read */
 {
    imu.Read(imu_values);
    //Function used to clear interrupts if necessary
    imu.clearInterrupts();

    send_frame_ready_sdram->frame_ready = false;
    sendDataToM4();
  }

}

void sendDataToM4() {
  if (!send_frame_ready_sdram->frame_ready) { //if false it can be written to
    //USERIAL->print(send_frame_ready_sdram->frame_ready);
    DATA_FRAME_SEND dataForM4;
    memcpy(&dataForM4.imu_data, &imu_values, sizeof(struct IMU_DATA));
    *data_frame_send_sdram = dataForM4;
    send_frame_ready_sdram->frame_ready = true; //if true M4 can read from it
    //USERIAL->println(send_frame_ready_sdram->frame_ready);
  }
}

void receiveDataFromM4() {
  if (return_frame_ready_sdram->frame_ready) {  //true indicates it can read by M7
    DATA_FRAME_RETURN dataFromM4 = *data_frame_return_sdram;
    //telemetry.printTelemetry(String(dataFromM4.debugStringData), TELEMETRY_LOW_PRIORITY);
    USERIAL->println(String(dataFromM4.debugStringData));
    return_frame_ready_sdram->frame_ready = false;  //false indicates data has been read by M7
  }
}