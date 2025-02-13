#include "Arduino.h"
//#include "LibPrintf.h"

/******************  configure SDFAT *******************************/
#include "SdFat.h"

// SD_FAT_TYPE = 0 for SdFat/File as defined in SdFatConfig.h,
// 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
#define SD_FAT_TYPE 3

// SDCARD_SS_PIN is defined for the built-in SD on some boards.
const uint8_t SD_CS_PIN = 7;

// Try max SPI clock for an SD. Reduce SPI_CLOCK if errors occur.
#define SPI_CLOCK SD_SCK_MHZ(50)

#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SD_SCK_MHZ(50), &SPI1)

SdFs sd;
FsFile file;
//used for logger
FsFile dataFile;

/******************  configure mpu9250 *******************************/
//used for MPU9250 and data structure
#include "mpu9250.h"

/* Mpu9250 object, SPI bus, CS on pin 10 */
bfs::Mpu9250 imu(&SPI1, 4);
/*******************************************************************/
#define DBGSerial Serial

void logData();
void stopLogging();
void dumpLog();
void menu();

bool write_data = false;
uint32_t record_count = 0;
/*******************************************************************/

void setup()
{ 
  Serial.begin(115200);
 
  //while(!Serial.available()); // comment if you do not want to wait for terminal (otherwise press any key to continue)
  while(!Serial.available() && millis() < 5000); // or third option to wait up to 5 seconds and then continue

 //Init 9250 i2c
  // start communication with IMU 
  /* Start the SPI bus */
  SPI.begin();
  /* Initialize and configure IMU */
  if (!imu.Begin()) {
    Serial.println("Error initializing communication with IMU");
    while(1) {}
  }
  /* Set the sample rate divider */
  if (!imu.ConfigSrd(19)) {
    Serial.println("Error configuring SRD");
    while(1) {}
  }
    // Initialize the SD card.
    Serial.println("Before SD Begin");

    if (!sd.begin(SD_CONFIG)) {
        sd.initErrorHalt(&Serial);
        Serial.println("Card Begin failed");
        while (1) {}
    }


  Serial.println("Setup done");
  menu();
  
}


void loop()
{
  if ( DBGSerial.available() ) {
    uint8_t command = DBGSerial.read();
    switch (command) {
   case 's':
      DBGSerial.println("\nLogging Data!!!");
      write_data = true;   // sets flag to continue to write data until new command is received
      // opens a file or creates a file if not present,  FILE_WRITE will append data to
      // to the file created.
      dataFile = sd.open("datalog.txt", FILE_WRITE);
      logData();
      break;
    case 'x': stopLogging(); break;
    case 'd': dumpLog(); break;
    case '\r':
    case '\n':
    case 'h': menu(); break;
    }
    while (DBGSerial.read() != -1) ; // remove rest of characters.
  } 
  
  if (write_data) logData();
}


void logData()
{
  // if the file is available, write to it:
  if (dataFile) {
    if (imu.Read()) {
      dataFile.print(imu.new_imu_data());
      dataFile.print("\t");
      dataFile.print(imu.new_mag_data());
      dataFile.print("\t");
      dataFile.print(imu.accel_x_mps2());
      dataFile.print("\t");
      dataFile.print(imu.accel_y_mps2());
      dataFile.print("\t");
      dataFile.print(imu.accel_z_mps2());
      dataFile.print("\t");
      dataFile.print(imu.gyro_x_radps());
      dataFile.print("\t");
      dataFile.print(imu.gyro_y_radps());
      dataFile.print("\t");
      dataFile.print(imu.gyro_z_radps());
      dataFile.print("\t");
      dataFile.print(imu.mag_x_ut());
      dataFile.print("\t");
      dataFile.print(imu.mag_y_ut());
      dataFile.print("\t");
      dataFile.print(imu.mag_z_ut());
      dataFile.print("\t");
      dataFile.print(imu.die_temp_c());
      dataFile.println();
    }
    record_count += 1;
  } else {
    // if the file isn't open, pop up an error:
    DBGSerial.println("error opening datalog.txt");
  }
  delay(100); // run at a reasonable not-too-fast speed for testing

}

void stopLogging()
{
  DBGSerial.println("\nStopped Logging Data!!!");
  write_data = false;
  // Closes the data file.
  dataFile.close();
  printf("Records written = %d\n", record_count);
}


void dumpLog()
{
  DBGSerial.println("\nDumping Log!!!");
  // open the file.
  dataFile = sd.open("datalog.txt");

  // if the file is available, write to it:
  if (dataFile) {
    while (dataFile.available()) {
      DBGSerial.write(dataFile.read());
    }
    dataFile.close();
  }
  // if the file isn't open, pop up an error:
  else {
    DBGSerial.println("error opening datalog.txt");
  }
}

void menu()
{
  DBGSerial.println();
  DBGSerial.println("Menu Options:");
  DBGSerial.println("\ts - Start Logging data (Restarting logger will append records to existing log)");
  DBGSerial.println("\tx - Stop Logging data");
  DBGSerial.println("\td - Dump Log");
  DBGSerial.println("\th - Menu");
  DBGSerial.println();
}


