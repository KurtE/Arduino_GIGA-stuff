K_MUTEX_DEFINE(my_mutex);

#include <Wire.h>
#include "SparkFun_BMI270_Arduino_Library.h"
#include <Arduino_LPS22HB.h>

// Create a new sensor object
BMI270 imu;

// I2C address selection
uint8_t i2cAddress = BMI2_I2C_PRIM_ADDR; // 0x68
//uint8_t i2cAddress = BMI2_I2C_SEC_ADDR; // 0x69

void bmi270_read(void)
{
    Wire1.begin();
    //Wire1.setClock(400000);
    // Check if sensor is connected and initialize
    // Address is optional (defaults to 0x68)
    while(imu.beginI2C(i2cAddress, Wire1) != BMI2_OK)
    {
        // Not connected, inform user
        Serial.println("Error: BMI270 not connected, check wiring and I2C address!");

        // Wait a bit to see if connection is established
        k_msleep(100);
    }
    Serial.println("BMI270 connected!");

    while (1) {
        k_mutex_lock(&my_mutex, K_FOREVER);
        imu.getSensorData();
        // Print acceleration data
        Serial.print("Acceleration in g's");
        Serial.print("\t");
        Serial.print("X: ");
        Serial.print(imu.data.accelX, 3);
        Serial.print("\t");
        Serial.print("Y: ");
        Serial.print(imu.data.accelY, 3);
        Serial.print("\t");
        Serial.print("Z: ");
        Serial.print(imu.data.accelZ, 3);
        Serial.print("\t");
        // Print rotation data
        Serial.print("Rotation in deg/sec");
        Serial.print("\t");
        Serial.print("X: ");
        Serial.print(imu.data.gyroX, 3);
        Serial.print("\t");
        Serial.print("Y: ");
        Serial.print(imu.data.gyroY, 3);
        Serial.print("\t");
        Serial.print("Z: ");
        Serial.println(imu.data.gyroZ, 3);
        k_sleep(K_MSEC(1));
        k_mutex_unlock(&my_mutex);
        k_yield();
    }
}

void lps22_read(void)
{

  if (!BARO.begin()) {
    Serial.println("Failed to initialize pressure sensor!");
    while (1);
  } else {
    Serial.println("LPS22 connected");
  }
    while (1) {
      k_mutex_lock(&my_mutex, K_FOREVER);
      // read the sensor value
      float pressure = BARO.readPressure();

      // print the sensor value
      Serial.print("Pressure = ");
      Serial.print(pressure);
      Serial.println(" kPa");

      float temperature = BARO.readTemperature();

      // print the sensor value
      Serial.print("Temperature = ");
      Serial.print(temperature);
      Serial.println(" C");

      // print an empty line
      Serial.println();
      k_sleep(K_MSEC(5));
      k_mutex_unlock(&my_mutex);
      k_yield();
    }
}


K_THREAD_DEFINE(thread1_id, 1024, bmi270_read, NULL, NULL, NULL, 6, 0, 1500);
K_THREAD_DEFINE(thread2_id, 1024, lps22_read, NULL, NULL, NULL, 6, 0, 2000);

void setup() {
  // Start serial
  Serial.begin(115200);
  Serial.println("BMI270 Example 1 - Basic Readings I2C");


}

void loop() {
  // put your main code here, to run repeatedly:
  k_msleep(1000);
}
