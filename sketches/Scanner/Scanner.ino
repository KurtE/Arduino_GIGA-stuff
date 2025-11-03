// Wire Scanner - scans for I2C devices on all Wire ports
//
// This Wire library adapted is for Teensy boards
//
//   https://github.com/PaulStoffregen/Wire/
//
// Adapted from I2C Scanner originally published on Arduino Playground
// see comments below for link and credits for original authors.

#include <Wire.h>
#include "stm32h7xx_hal_dcmi.h"

#ifndef WIRE_INTERFACES_COUNT
#define WIRE_INTERFACES_COUNT 1
#endif

extern "C" {
extern int camera_extclk_config(int frequency);
}

void setup() {


#define DCMI_TIM                    (TIM1)
#define DCMI_TIM_PIN                (GPIO_PIN_1)
#define DCMI_TIM_PORT               (GPIOK)
#define DCMI_TIM_AF                 (GPIO_AF1_TIM1)
#define DCMI_TIM_CHANNEL            (TIM_CHANNEL_1)
#define DCMI_TIM_CLK_ENABLE()       __TIM1_CLK_ENABLE()
#define DCMI_TIM_CLK_DISABLE()      __TIM1_CLK_DISABLE()
#define DCMI_TIM_PCLK_FREQ()        HAL_RCC_GetPCLK2Freq()
#define DCMI_TIM_FREQUENCY          (6000000)
#define DCMI_RESET_PIN              (PC_13)

camera_extclk_config(DCMI_TIM_FREQUENCY);

	pinMode(PC_13, OUTPUT);
	digitalWrite(PC_13, LOW);
	delay(10);
	digitalWrite(PC_13, HIGH);
	delay(10);
  //analogWrite(PK_1, 128);


#if WIRE_INTERFACES_COUNT >= 1
  //Wire.setSCL(16);  // uncomment these to use alternate Wire pins
  //Wire.setSDA(17);
  Wire.begin();
#endif
#if WIRE_INTERFACES_COUNT >= 2
  Wire1.begin();
#endif
#if WIRE_INTERFACES_COUNT >= 3
  Wire2.begin();
#endif
#if WIRE_INTERFACES_COUNT >= 4
  Wire3.begin();
#endif
  Serial.begin(9600);
  while (!Serial);   // Wait for Arduino Serial Monitor
  Serial.println(F("\nI2C Scanner"));
}

void loop() {
  Serial.println();
#if WIRE_INTERFACES_COUNT >= 1
  Serial.println(F("Scanning Wire..."));
  scan(Wire);
  delay(500);
#endif
#if WIRE_INTERFACES_COUNT >= 2
  Serial.println(F("Scanning Wire1..."));
  scan(Wire1);
  delay(500);
#endif
#if WIRE_INTERFACES_COUNT >= 3
  Serial.println(F("Scanning Wire2..."));
  scan(Wire2);
  delay(500);
#endif
#if WIRE_INTERFACES_COUNT >= 4
  Serial.println(F("Scanning Wire3..."));
  scan(Wire3);
  delay(500);
#endif
  delay(5000);           // wait 5 seconds for next scan
}


void scan(TwoWire &myport) {
  int nDevices = 0;
  for (int address = 1; address < 127; address++) {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    myport.beginTransmission(address);
    int error = myport.endTransmission();

    if (error == 0) {
      Serial.print(F("Device found at address 0x"));
      if (address < 16) {
        Serial.print("0");
      }
      Serial.print(address,HEX);
      Serial.print("  (");
      printKnownChips(address);
      Serial.println(")");
      nDevices++;
    } else if (error==4) {
      Serial.print(F("Unknown error at address 0x"));
      if (address < 16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
    }
  }
  if (nDevices == 0) {
    Serial.println(F("No I2C devices found"));
  } else {
    Serial.println(F("done"));
  }
  Serial.println();
}


void printKnownChips(byte address)
{
  // Is this list missing part numbers for chips you use?
  // Please suggest additions here:
  // https://github.com/PaulStoffregen/Wire/issues/new
  switch (address) {
    case 0x00: Serial.print(F("AS3935")); break;
    case 0x01: Serial.print(F("AS3935")); break;
    case 0x02: Serial.print(F("AS3935")); break;
    case 0x03: Serial.print(F("AS3935")); break;
    case 0x04: Serial.print(F("ADAU1966")); break;
    case 0x0A: Serial.print(F("SGTL5000")); break; // MCLK required
    case 0x0B: Serial.print(F("SMBusBattery?")); break;
    case 0x0C: Serial.print(F("AK8963")); break;
    case 0x10: Serial.print(F("CS4272")); break;
    case 0x11: Serial.print(F("Si4713")); break;
    case 0x13: Serial.print(F("VCNL4000,AK4558")); break;
    case 0x18: Serial.print(F("LIS331DLH")); break;
    case 0x19: Serial.print(F("LSM303,LIS331DLH")); break;
    case 0x1A: Serial.print(F("WM8731")); break;
    case 0x1C: Serial.print(F("LIS3MDL")); break;
    case 0x1D: Serial.print(F("LSM303D,LSM9DS0,ADXL345,MMA7455L,LSM9DS1,LIS3DSH")); break;
    case 0x1E: Serial.print(F("LSM303D,HMC5883L,FXOS8700,LIS3DSH")); break;
    case 0x20: Serial.print(F("MCP23017,MCP23008,PCF8574,FXAS21002,SoilMoisture")); break;
    case 0x21: Serial.print(F("MCP23017,MCP23008,PCF8574")); break;
    case 0x22: Serial.print(F("MCP23017,MCP23008,PCF8574")); break;
    case 0x23: Serial.print(F("MCP23017,MCP23008,PCF8574")); break;
    case 0x24: Serial.print(F("MCP23017,MCP23008,PCF8574,ADAU1966,HM01B0")); break;
    case 0x25: Serial.print(F("MCP23017,MCP23008,PCF8574")); break;
    case 0x26: Serial.print(F("MCP23017,MCP23008,PCF8574")); break;
    case 0x27: Serial.print(F("MCP23017,MCP23008,PCF8574,LCD16x2,DigoleDisplay")); break;
    case 0x28: Serial.print(F("BNO055,EM7180,CAP1188")); break;
    case 0x29: Serial.print(F("TSL2561,VL6180,TSL2561,TSL2591,BNO055,CAP1188")); break;
    case 0x2A: Serial.print(F("SGTL5000,CAP1188")); break;
    case 0x2B: Serial.print(F("CAP1188")); break;
    case 0x2C: Serial.print(F("MCP44XX ePot")); break;
    case 0x2D: Serial.print(F("MCP44XX ePot")); break;
    case 0x2E: Serial.print(F("MCP44XX ePot")); break;
    case 0x2F: Serial.print(F("MCP44XX ePot")); break;
    case 0x30: Serial.print(F("Si7210")); break;
    case 0x31: Serial.print(F("Si7210")); break;
    case 0x32: Serial.print(F("Si7210")); break;
    case 0x33: Serial.print(F("MAX11614,MAX11615,Si7210,MLX90640,MLX90641")); break;
    case 0x34: Serial.print(F("MAX11612,MAX11613")); break;
    case 0x35: Serial.print(F("MAX11616,MAX11617")); break;
    case 0x38: Serial.print(F("RA8875,FT6206,MAX98390")); break;
    case 0x39: Serial.print(F("TSL2561, APDS9960")); break;
    case 0x3A: Serial.print(F("MLX90632")); break;
    case 0x3C: Serial.print(F("SSD1306,DigisparkOLED")); break;
    case 0x3D: Serial.print(F("SSD1306")); break;
    case 0x40: Serial.print(F("PCA9685,Si7021,MS8607")); break;
    case 0x41: Serial.print(F("STMPE610,PCA9685")); break;
    case 0x42: Serial.print(F("PCA9685")); break;
    case 0x43: Serial.print(F("PCA9685")); break;
    case 0x44: Serial.print(F("PCA9685, SHT3X, ADAU1966")); break;
    case 0x45: Serial.print(F("PCA9685, SHT3X")); break;
    case 0x46: Serial.print(F("PCA9685")); break;
    case 0x47: Serial.print(F("PCA9685")); break;
    case 0x48: Serial.print(F("ADS1115,PN532,TMP102,LM75,PCF8591,CS42448")); break;
    case 0x49: Serial.print(F("ADS1115,TSL2561,PCF8591,CS42448,TC74A1")); break;
    case 0x4A: Serial.print(F("ADS1115,Qwiic Keypad,CS42448")); break;
    case 0x4B: Serial.print(F("ADS1115,TMP102,BNO080,Qwiic Keypad,CS42448")); break;
    case 0x50: Serial.print(F("EEPROM,FRAM")); break;
    case 0x51: Serial.print(F("EEPROM")); break;
    case 0x52: Serial.print(F("Nunchuk,EEPROM")); break;
    case 0x53: Serial.print(F("ADXL345,EEPROM")); break;
    case 0x54: Serial.print(F("EEPROM")); break;
    case 0x55: Serial.print(F("EEPROM")); break;
    case 0x56: Serial.print(F("EEPROM")); break;
    case 0x57: Serial.print(F("EEPROM")); break;
    case 0x58: Serial.print(F("TPA2016,MAX21100")); break;
    case 0x5A: Serial.print(F("MPR121,MLX90614")); break;
    case 0x60: Serial.print(F("MPL3115,MCP4725,MCP4728,TEA5767,Si5351")); break;
    case 0x61: Serial.print(F("MCP4725,AtlasEzoDO")); break;
    case 0x62: Serial.print(F("LidarLite,MCP4725,AtlasEzoORP")); break;
    case 0x63: Serial.print(F("MCP4725,AtlasEzoPH")); break;
    case 0x64: Serial.print(F("AtlasEzoEC, ADAU1966")); break;
    case 0x66: Serial.print(F("AtlasEzoRTD")); break;
    case 0x68: Serial.print(F("DS1307,DS3231,MPU6050,MPU9050,MPU9250,ITG3200,ITG3701,LSM9DS0,L3G4200D")); break;
    case 0x69: Serial.print(F("MPU6050,MPU9050,MPU9250,ITG3701,L3G4200D")); break;
    case 0x6A: Serial.print(F("LSM9DS1")); break;
    case 0x6B: Serial.print(F("LSM9DS0")); break;
    case 0x6F: Serial.print(F("Qwiic Button")); break;
    case 0x70: Serial.print(F("HT16K33,TCA9548A")); break;
    case 0x71: Serial.print(F("SFE7SEG,HT16K33")); break;
    case 0x72: Serial.print(F("HT16K33")); break;
    case 0x73: Serial.print(F("HT16K33")); break;
    case 0x76: Serial.print(F("MS5607,MS5611,MS5637,BMP280")); break;
    case 0x77: Serial.print(F("BMP085,BMA180,BMP280,MS5611")); break;
    case 0x7C: Serial.print(F("FRAM_ID")); break;
    default: Serial.print(F("unknown chip"));
  }
}


// --------------------------------------
// i2c_scanner
// https://playground.arduino.cc/Main/I2cScanner/
//
// Version 1
//    This program (or code that looks like it)
//    can be found in many places.
//    For example on the Arduino.cc forum.
//    The original author is not know.
// Version 2, Juni 2012, Using Arduino 1.0.1
//     Adapted to be as simple as possible by Arduino.cc user Krodal
// Version 3, Feb 26  2013
//    V3 by louarnold
// Version 4, March 3, 2013, Using Arduino 1.0.3
//    by Arduino.cc user Krodal.
//    Changes by louarnold removed.
//    Scanning addresses changed from 0...127 to 1...119,
//    according to the i2c scanner by Nick Gammon
//    http://www.gammon.com.au/forum/?id=10896
// Version 5, March 28, 2013
//    As version 4, but address scans now to 127.
//    A sensor seems to use address 120.
// Version 6, November 27, 2015.
//    Added waiting for the Leonardo serial communication.
//
//
// This sketch tests the standard 7-bit addresses
// Devices with higher bit address might not be seen properly.
//

extern "C" {

void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == DCMI_TIM) {
        // Enable DCMI timer clock
        DCMI_TIM_CLK_ENABLE();

        // Timer GPIO configuration
        GPIO_InitTypeDef  hgpio;
        hgpio.Pin       = DCMI_TIM_PIN;
        hgpio.Pull      = GPIO_NOPULL;
        hgpio.Speed     = GPIO_SPEED_HIGH;
        hgpio.Mode      = GPIO_MODE_AF_PP;
        hgpio.Alternate = DCMI_TIM_AF;
        HAL_GPIO_Init(DCMI_TIM_PORT, &hgpio);
    }
}



static TIM_HandleTypeDef  htim  = {0};

int camera_extclk_config(int frequency)
{
    // TCLK (PCLK * 2).
    uint32_t tclk = DCMI_TIM_PCLK_FREQ() * 2;

    // Period should be even.
    uint32_t period = (tclk / frequency) - 1;

    if (htim.Init.Period && (htim.Init.Period != period)) {
        __HAL_TIM_SET_AUTORELOAD(&htim, period);
        __HAL_TIM_SET_COMPARE(&htim, DCMI_TIM_CHANNEL, period / 2);
        return 0;
    }

    // Timer base configuration.
    htim.Instance               = DCMI_TIM;
    htim.Init.Period            = period;
    htim.Init.Prescaler         = 0;
    htim.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim.Init.RepetitionCounter = 0;
    htim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

    // Timer channel configuration.
    TIM_OC_InitTypeDef TIMOCHandle;
    TIMOCHandle.Pulse           = period / 2;
    TIMOCHandle.OCMode          = TIM_OCMODE_PWM1;
    TIMOCHandle.OCPolarity      = TIM_OCPOLARITY_HIGH;
    TIMOCHandle.OCNPolarity     = TIM_OCNPOLARITY_HIGH;
    TIMOCHandle.OCFastMode      = TIM_OCFAST_DISABLE;
    TIMOCHandle.OCIdleState     = TIM_OCIDLESTATE_RESET;
    TIMOCHandle.OCNIdleState    = TIM_OCNIDLESTATE_RESET;

    // Init, config and start the timer.
    if ((HAL_TIM_PWM_Init(&htim) != HAL_OK)
    || (HAL_TIM_PWM_ConfigChannel(&htim, &TIMOCHandle, DCMI_TIM_CHANNEL) != HAL_OK)
    || (HAL_TIM_PWM_Start(&htim, DCMI_TIM_CHANNEL) != HAL_OK)) {
        return -1;
    }

    return 0;
}

} /* extern 'C'*/
