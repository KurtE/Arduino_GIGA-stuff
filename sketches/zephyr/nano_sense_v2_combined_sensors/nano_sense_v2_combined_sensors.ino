#include <Wire.h>
#include <stdint.h>
#include "SparkFun_BMI270_Arduino_Library.h"
#include <Arduino_LPS22HB.h>
#include "DFRobot_BMM150.h"

// Create a new sensor object
DFRobot_BMM150_I2C bmm150(&Wire1, 0x10);
BMI270 imu;

// I2C address selection
uint8_t i2cAddress = BMI2_I2C_PRIM_ADDR; // 0x68
//uint8_t i2cAddress = BMI2_I2C_SEC_ADDR; // 0x69

void setup()
{
    // Start serial
    Serial.begin(115200);
    Serial.println("BMI270 Example 1 - Basic Readings I2C");


    // Initialize the I2C library
    Wire1.begin();
    //Wire1.setClock(400000);

    // Check if sensor is connected and initialize
    // Address is optional (defaults to 0x68)
    while(imu.beginI2C(i2cAddress, Wire1) != BMI2_OK)
    {
        // Not connected, inform user
        Serial.println("Error: BMI270 not connected, check wiring and I2C address!");

        // Wait a bit to see if connection is established
        k_msleep(1000);
    }

    Serial.println("BMI270 connected!");

  if (!BARO.begin()) {
    Serial.println("Failed to initialize pressure sensor!");
    while (1);
  } else {
    Serial.println("LPS22 connected");
  }

  while(bmm150.begin()){
    Serial.println("bmm150 init failed, Please try again!");
    delay(1000);
  } Serial.println("bmm150 init success!");

  bmm150.setOperationMode(BMM150_POWERMODE_NORMAL);
  bmm150.setPresetMode(BMM150_PRESETMODE_HIGHACCURACY);
  bmm150.setRate(BMM150_DATA_RATE_25HZ);
  bmm150.setMeasurementXYZ();

}

void loop()
{
  // Get measurements from the sensor. This must be called before accessing
  // the sensor data, otherwise it will never update
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

  float mag[3];
  if(bmm150.getDataReadyState()) {
    bmm150.getMagData(mag);
    Serial.print("mag x/y/z = "); Serial.print(mag[0],2);
    Serial.print(", "); Serial.print(mag[1],2);
    Serial.print(", "); Serial.print(mag[2],2); Serial.println(" uT");
  }

  // print an empty line
  Serial.println();
  // Print 50x per second
  delay(250);
}