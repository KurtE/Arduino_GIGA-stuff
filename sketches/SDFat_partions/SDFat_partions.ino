#include "SdFat.h"
#include "sdios.h"


//=======================================================================
//  setup for SDFat
//=======================================================================

/*
  Set DISABLE_CS_PIN to disable a second SPI device.
  For example, with the Ethernet shield, set DISABLE_CS_PIN
  to 10 to disable the Ethernet controller.
*/
const int8_t DISABLE_CS_PIN = -1;
/*
  Change the value of SD_CS_PIN if you are using SPI
  and your hardware does not use the default value, SS.
  Common values are:
  Arduino Ethernet shield: pin 4
  Sparkfun SD shield: pin 8
  Adafruit SD shields and modules: pin 10
*/

// SDCARD_SS_PIN is defined for the built-in SD on some boards.
#ifndef SDCARD_SS_PIN
const uint8_t SD_CS_PIN = 10;
#else  // SDCARD_SS_PIN
// Assume built-in SD is used.
const uint8_t SD_CS_PIN = SDCARD_SS_PIN;
#endif  // SDCARD_SS_PIN

// Try max SPI clock for an SD. Reduce SPI_CLOCK if errors occur.
#define SPI_CLOCK SD_SCK_MHZ(50)

// Try to select the best SD card configuration.
#if HAS_SDIO_CLASS
#define SD_CONFIG SdioConfig(FIFO_SDIO)
#elif  ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SPI_CLOCK,&SPI1)
#else  // HAS_SDIO_CLASS
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SPI_CLOCK,&SPI1)
#endif  // HAS_SDIO_CLASS
//==============================================================================
// Serial output stream
ArduinoOutStream cout(Serial);
//------------------------------------------------------------------------------
uint32_t cardSectorCount = 0;
uint8_t  sectorBuffer[512];
//------------------------------------------------------------------------------
// SdCardFactory constructs and initializes the appropriate card.
SdCardFactory cardFactory;
// Pointer to generic SD card.
SdCard* sd = nullptr;

csd_t m_csd;
uint32_t m_eraseSize;

//------------------------------------------------------------------------------
#define sdError(msg) {cout << F("error: ") << F(msg) << endl; sdErrorHalt();}
//------------------------------------------------------------------------------
FsVolume partVol;
//FsFile dataFile;
ExFatVolume expartVol;

FsFile dataFile, myFile;  // Specifes that dataFile is of File type
int record_count = 0;
bool write_data = false;
uint8_t fileSysCount = 0;
uint8_t partitionTable[4];
int32_t dataStart[4];
//------------------------------------------------------------------------------

void setup() {
  char c;
  Serial.begin(9600);
  // Wait for USB Serial
  while (!Serial && millis() < 5000) {  }

 menu();

}

uint8_t storage_index = 0;
uint8_t current_store = 0;
void loop() {
  if ( Serial.available() ) {
    uint8_t command = Serial.read();
    int ch = Serial.read();
    if ('2'==command) storage_index = CommandLineReadNextNumber(ch, 0);
    while (ch == ' ') ch = Serial.read();

    uint32_t fsCount;
    switch (command) {
    case 'I':
      InitializeSdCard();
      break;
    case '1':
      mbrDmp();
      break;
    case '2':
      if (storage_index <= fileSysCount) {
        Serial.printf("Storage Index %d Selected of %d\n", storage_index, fileSysCount );
        Serial.println();
        if(partitionTable[storage_index - 1] == 1) {
            partVol.begin(sd, true, storage_index);
        } else if(partitionTable[storage_index - 1] == 2) {
            partVol.begin(sd, true, storage_index);
        } else if(partitionTable[storage_index - 1] == 3) {
            expartVol.begin(sd, true, storage_index);
        }
        current_store = storage_index;
      } else {
        Serial.printf("Storage Index %u out of range\n", storage_index);
      }
      break;
    case 'L':
     //lets get a dump of the files on available partitions
     for(uint8_t part = 0; part < 4; part++) {
        Serial.println();
        if(partitionTable[part] == 1) {
          getFat16partition(part + 1);
        } else if(partitionTable[part] == 2) {
          getFat32partition(part + 1);
        } else if(partitionTable[part] == 3) {
          getExFatpartition(part + 1);
        }
     }
      break;
    case 'l': listFiles(); break;
    case 's':
      Serial.println("\nLogging Data!!!");
      write_data = true;   // sets flag to continue to write data until new command is received
      // opens a file or creates a file if not present,  FILE_WRITE will append data to
      // to the file created.
      dataFile.open("datalog.txt", FILE_WRITE);
      logData();
      break;
    case 'x': stopLogging(); break;
    case 'd': dumpLog(); break;
/*
    case 't':
      bigFile2MB( 0 ); // CREATE
      command = 0;
      break;
    case 'S':
      bigFile2MB( 1 ); // CREATE
      command = 0;
     break;
    case 'w':
      test_write_file(ch);
      break;
*/
    case '\r':
    case '\n':
    case 'h': menu(); break;
    }
    while (Serial.read() != -1) ; // remove rest of characters.
  } 

  if (write_data) logData();

}
 
 
void menu()
{
  Serial.println();
  Serial.println("Menu Options:");
  Serial.println("\tI - Initialize Drive");

  Serial.println("\t1 - List Partitions (Step 1)");
  Serial.println("\t2# - Select Drive # for Logging (Step 2)");
  Serial.println("\tl - List files on selected partition");
  Serial.println("\tL - List files on all partitions");

  Serial.println("\ts - Start Logging data (Restarting logger will append records to existing log)");
  Serial.println("\tx - Stop Logging data");
  Serial.println("\td - Dump Log");
  Serial.println("\tr - Reset ");
  Serial.printf("\n\t%s","'S, or t': Make 2MB file , or remove all 2MB files");

  Serial.println("\th - Menu");
  Serial.println();
}


uint32_t CommandLineReadNextNumber(int &ch, uint32_t default_num) {
  while (ch == ' ') ch = Serial.read();
  if ((ch < '0') || (ch > '9')) return default_num;

  uint32_t return_value = 0;
  while ((ch >= '0') && (ch <= '9')) {
    return_value = return_value * 10 + ch - '0';
    ch = Serial.read();
  }
  return return_value;
}

void InitializeSdCard() 
{

  // Select and initialize proper card driver.
  sd = cardFactory.newCard(SD_CONFIG);
  if (!sd || sd->errorCode()) {
    sdError("card init failed.");
    return;
  }

  cardSectorCount = sd->sectorCount();
  if (!cardSectorCount) {
    sdError("Get sector count failed.");
    return;
  }

  cout << F("\nCard size: ") << cardSectorCount*5.12e-7;
  cout << F(" GB (GB = 1E9 bytes)\n");
  cout << F("Card size: ") << cardSectorCount/2097152.0;
  cout << F(" GiB (GiB = 2^30 bytes)\n\n");

  if (!sd->readCSD(&m_csd) ) {
    cout << F("readInfo failed\n");
  }

  printCardType();
  csdDmp();
  //if (!mbrDmp()) {
  //  return;
  //}
}


void listFiles()
{
/* tbd
  Serial.print("\n Space Used = ");
  Serial.println(myfs->usedSize());
  Serial.print("Filesystem Size = ");
  Serial.println(myfs->totalSize());
  printDirectory(myfs);
 */
 Serial.printf("Listing files on storage Index: %d\n", current_store );
  if(partitionTable[current_store - 1] == 1) {
      partVol.ls(&Serial, LS_R | LS_SIZE | LS_DATE);;
  } else if(partitionTable[current_store - 1] == 2) {
      partVol.ls(&Serial, LS_R | LS_SIZE | LS_DATE);;
  } else if(partitionTable[current_store - 1] == 3) {
      expartVol.ls(LS_SIZE | LS_DATE | LS_R);
  }
}


void logData()
{
  // make a string for assembling the data to log:
  String dataString = "";

  // read three sensors and append to the string:
  for (int analogPin = 0; analogPin < 3; analogPin++) {
    int sensor = analogRead(analogPin);
    dataString += String(sensor);
    if (analogPin < 2) {
      dataString += ",";
    }
  }

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    // print to the serial port too:
    Serial.println(dataString);
    record_count += 1;
  } else {
    // if the file isn't open, pop up an error:
    Serial.println("error opening datalog.txt");
  }
  delay(100); // run at a reasonable not-too-fast speed for testing
}

void stopLogging()
{
  Serial.println("\nStopped Logging Data!!!");
  write_data = false;
  // Closes the data file.
  dataFile.close();
  Serial.printf("Records written = %d\n", record_count);
}

void dumpLog()
{
  Serial.println("\nDumping Log!!!");
  // open the file.
  dataFile.open("datalog.txt");

  // if the file is available, write to it:
  if (dataFile) {
    while (dataFile.available()) {
      Serial.write(dataFile.read());
    }
    dataFile.close();
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }
}
