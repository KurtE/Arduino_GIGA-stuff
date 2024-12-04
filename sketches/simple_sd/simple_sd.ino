/*
 * Example use of chdir(), ls(), mkdir(), and  rmdir().
 */
#include "SdFat.h"

// SD_FAT_TYPE = 0 for SdFat/File as defined in SdFatConfig.h,
// 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
#define SD_FAT_TYPE 3
/*
  Change the value of SD_CS_PIN if you are using SPI and
  your hardware does not use the default value, SS.
  Common values are:
  Arduino Ethernet shield: pin 4
  Sparkfun SD shield: pin 8
  Adafruit SD shields and modules: pin 10
*/

#define DEBUG_PIN 4

// SDCARD_SS_PIN is defined for the built-in SD on some boards.
const uint8_t SD_CS_PIN = 6;

// Try max SPI clock for an SD. Reduce SPI_CLOCK if errors occur.
#define SPI_CLOCK SD_SCK_MHZ(12)

// Try to select the best SD card configuration.
//#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SPI_CLOCK, &SPI1)
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SPI_CLOCK, &SPI1)

//------------------------------------------------------------------------------

#if SD_FAT_TYPE == 0
SdFat sd;
File file;
File root;
#elif SD_FAT_TYPE == 1
SdFat32 sd;
File32 file;
File32 root;
#elif SD_FAT_TYPE == 2
SdExFat sd;
ExFile file;
ExFile root;
#elif SD_FAT_TYPE == 3
SdFs sd;
FsFile file;
FsFile root;
#endif  // SD_FAT_TYPE

void setup() {
    Serial.begin(9600);
    delay(2000);
    //Serial.println("Type any character to start");
    pinMode(10, OUTPUT);
    digitalWrite(10, HIGH);  // make sure display does not interfere
    digitalWrite(10, LOW);  // make sure display does not interfere
    digitalWrite(10, HIGH);  // make sure display does not interfere
    pinMode(6, OUTPUT);
    digitalWrite(6, HIGH);  // make sure display does not interfere
    digitalWrite(6, LOW);  // make sure display does not interfere
    digitalWrite(6, HIGH);  // make sure display does not interfere
#if 1
    pinMode(DEBUG_PIN, OUTPUT);
    digitalWrite(DEBUG_PIN, LOW);  // make sure display does not interfere
    digitalWrite(DEBUG_PIN, HIGH);  // make sure display does not interfere
    digitalWrite(DEBUG_PIN, LOW);  // make sure display does not interfere
#endif

    // Initialize the SD card.
    Serial.println("Before SD Begin");

//    digitalWrite(DEBUG_PIN, HIGH);  // make sure display does not interfere
    if (!sd.cardBegin(SD_CONFIG)) {
//        digitalWrite(DEBUG_PIN, LOW);  // make sure display does not interfere
        Serial.print("Card Begin failed Error: ");
        Serial.println(sd.sdErrorCode());
        while (1) {}
    }
    //digitalWrite(DEBUG_PIN, LOW);  // make sure display does not interfere

    if (!sd.volumeBegin()) {
        Serial.println("Volume begin failed");
        while (1) {}
    }

    Serial.println("List of files on the SD.\n");
    //sd.ls(LS_R);
}

void loop() {
    // put your main code here, to run repeatedly:
}