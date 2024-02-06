#include <floatToString.h>

#include <RPC.h>
Stream *USERIAL = nullptr;

#include "SDRAM.h"

// Data constructors
struct IMU_DATA {
  float ax;
  float ay;
  float az;
  float gx;
  float gy;
  float gz;
  float mx;
  float my;
  float mz;
} __attribute__((aligned(8)));
IMU_DATA imu_data_recv;

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

bool hasReceivedDataFromM7 = false;

void setup() {
 if (HAL_GetCurrentCPUID() == CM7_CPUID) {
  while (!Serial && millis() < 5000) {}
    Serial.begin(115200);
    USERIAL = &Serial;
  } else {
    RPC.begin();
    USERIAL = &RPC;
  }

  delay(500);

  pinMode(LED_BLUE, OUTPUT);
  for (uint8_t i = 0; i < 5; i++ ) {
    digitalWrite(LED_BLUE, HIGH);
    delay(250);
    digitalWrite(LED_BLUE, LOW);
    delay(250);
  }

}

void loop() {
  digitalWrite(LED_BLUE, HIGH);
  delay(250);

  receiveDataFromM7();
  if(hasReceivedDataFromM7 == true){
    /*
    USERIAL->print("\t"); 
    USERIAL->print(imu_data_recv.ax); USERIAL->print(", ");
    USERIAL->print(imu_data_recv.ay); USERIAL->print(", ");
    USERIAL->print(imu_data_recv.az); USERIAL->print(", ");
    USERIAL->print(imu_data_recv.gx); USERIAL->print(", ");
    USERIAL->print(imu_data_recv.gy); USERIAL->print(", ");
    USERIAL->print(imu_data_recv.gz); USERIAL->print(", ");
    USERIAL->print(imu_data_recv.mx); USERIAL->print(", ");
    USERIAL->print(imu_data_recv.my); USERIAL->print(", ");
    USERIAL->print(imu_data_recv.mz); USERIAL->println();
    */
    //String buf = ""; char S[15];
    //buf = floatToString(imu_data_recv.ax, S, sizeof(S), 6); buf.concat(", ");
    //buf.concat(floatToString(imu_data_recv.ay, S, sizeof(S), 6));
    hasReceivedDataFromM7 = false;

    sendDataToM7("Received data", true);
  }
  digitalWrite(LED_BLUE, LOW);
  delay(250);
}

void receiveDataFromM7() {
  if (send_frame_ready_sdram->frame_ready) {  //data is available from M7
    digitalWrite(LED_BLUE, HIGH);
    DATA_FRAME_SEND dataFromM7 = *data_frame_send_sdram;
    memcpy(&imu_data_recv, &dataFromM7.imu_data, sizeof(struct IMU_DATA));
    send_frame_ready_sdram->frame_ready = false;  //set to false to let M7 know data has been read
    hasReceivedDataFromM7 = true;
    digitalWrite(LED_BLUE, LOW);
  }
}

void sendDataToM7(String debugStringData, bool blocking) {
  do {
    if (!return_frame_ready_sdram->frame_ready) {  //if 0 can send data
      DATA_FRAME_RETURN dataForM7;
      debugStringData.toCharArray(dataForM7.debugStringData, sizeof(dataForM7.debugStringData));
      *data_frame_return_sdram = dataForM7;
      return_frame_ready_sdram->frame_ready = true;  //1 lets m7 know it can read data
      return;                        //break out of do->while loop
    } else if (blocking) {
      delayMicroseconds(5);  //hang out and try again, in a little bit
    }
  } while (blocking);
}