//=============================================================================
// Simple image (BMP optional JPEG and PNG) display program, which if the
// sketch is built with one of the USB Types which include MTP support
//=============================================================================
#include <FATFileSystem.h>
#include <Arduino_USBHostMbed5.h>
#include <SPI.h>

#ifdef ARDUINO_PORTENTA_H7_M7
#include "SDMMCBlockDevice.h"
#include "FATFileSystem.h"
#endif

#include <SdFat.h>
#include "sdios.h"
#include "SDRAM.h"

#include <elapsedMillis.h>

#include "WrapperFile.h"

#include "Arduino_H7_Video.h"
#include "ArduinoGraphics.h"
//#include "Arduino_H7_Video_Plus.h"

#define MAX_FILENAME_LEN 256

REDIRECT_STDOUT_TO(Serial)

/***************************************************
  Some of this code originated with the spitftbitmap.ino sketch
  that is part of the Adafruit_ILI9341 library. 
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/

// warning, this sketch uses libraries that are not installed as part of Teensyduino
// ILI9341_t3n - https://github.com/KurtE/ILI9341_t3n
// JPGDEC - https://github.com/bitbank2/JPEGDEC (also on arduino library manager)
// PNGdec - https://github.com/bitbank2/PNGdec (also on arduino library manager)

// optional JPEG support requires external library
// uncomment if you wish to use.
#include <JPEGDEC.h>

// optional PNG support requires external library
#include <PNGdec.h>


// support for ILI9341_t3n - that adds additional features
//#include <ILI9341_GIGA_n.h>
//#include "Arduino_GigaDisplay_GFX.h"




//-----------------------------------------------------------------------------
// displays
//-----------------------------------------------------------------------------
//Arduino_H7_Video_Plus Display(800, 480, GigaDisplayShield);
Arduino_H7_Video Display(800, 480, GigaDisplayShield);

//-----------------------------------------------------------------------------
// Some common things.
//-----------------------------------------------------------------------------

#ifndef BLUE
#define BLUE 0x001F
#define BLACK 0x0000
#define WHITE 0xFFFF
#define GREEN 0x07E0
#define RED 0xf800
#endif


//-----------------------------------------------------------------------------
// Other globals
//-----------------------------------------------------------------------------
int g_tft_width = 0;
int g_tft_height = 0;



// SD_FAT_TYPE = 0 for SdFat/File as defined in SdFatConfig.h,
// 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
#define SD_FAT_TYPE 3
#define CS_SD 52
// SDCARD_SS_PIN is defined for the built-in SD on some boards.
//#ifndef SDCARD_SS_PIN
//const uint8_t CS_SD_PIN = SS;
//#else   // SDCARD_SS_PIN
// Assume built-in SD is used.
//const uint8_t CS_SD_PIN = SDCARD_SS_PIN;
//#endif  // SDCARD_SS_PIN

// Try max SPI clock for an SD. Reduce SPI_CLOCK if errors occur.
#define SPI_CLOCK SD_SCK_MHZ(30)

// Try to select the best SD card configuration.
//#if HAS_SDIO_CLASS
//#define SD_CONFIG SdioConfig(FIFO_SDIO)
//#elif ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(CS_SD, DEDICATED_SPI, SPI_CLOCK)
//#else  // HAS_SDIO_CLASS
//#define SD_CONFIG SdSpiConfig(CS_SD, SHARED_SPI, SPI_CLOCK)
//#endif  // HAS_SDIO_CLASS


//-----------------------------------
// SD specific
//-----------------------------------
#ifdef ARDUINO_PORTENTA_H7_M7
SDMMCBlockDevice block_device;
mbed::FATFileSystem sdio("sd");
DIR *root_SDIO = nullptr;  //
#endif

#ifdef CS_SD
SdFs SD;
FsFile root_SD;
#endif

//-----------------------------------
// USB specific
//-----------------------------------
USBHostMSD msd;
mbed::FATFileSystem usb("usb");
DIR *root_USB = nullptr;  //



//-----------------------------------
//-----------------------------------
#define MAX_FILENAME_LEN 256


bool g_fast_mode = false;
bool g_picture_loaded = false;
bool g_use_canvas = true;


elapsedMillis emDisplayed;
#define DISPLAY_IMAGES_TIME 2500

// Options file information
static const PROGMEM char options_file_name[] = "/usb/PictureViewOptions.ini";
static const PROGMEM char options_file_name_sd[] = "/PictureViewOptions.ini";
int g_debug_output = 0;
int g_stepMode = 0;
int g_BMPScale = -1;
int g_JPGScale = 0;
int g_PNGScale = 1;
int g_image_Bpp = 0;  //
int g_center_image = 1;
int g_display_image_time = 2500;
int g_background_color = BLACK;
int g_max_scale_up = 4;


// scale boundaries {2, 4, 8, 16<maybe>}
enum { SCL_HALF = 0,
       SCL_QUARTER,
       SCL_EIGHTH,
       SCL_16TH };
int g_jpg_scale_x_above[4];
int g_jpg_scale_y_above[4];

// variables used in some of the display output functions
int g_image_width;
int g_image_height;
int g_image_offset_x = 0;
int g_image_offset_y = 0;
uint8_t g_image_scale = 1;
uint8_t g_image_scale_up = 0;
uint32_t g_WRCount = 0;  // debug count how many time writeRect called


#define USB_DRIVE 0x01
#define SDIO_DRIVE 0x02
#define SD_DRIVE 0x04

#ifdef ARDUINO_PORTENTA_H7_M7
#ifdef CS_SD
#define ALL_STARTED (USB_DRIVE | SDIO_DRIVE | SD_DRIVE)
#else
#define ALL_STARTED (USB_DRIVE | SDIO_DRIVE)
#endif
#else
#ifdef CS_SD
#define ALL_STARTED (USB_DRIVE | SD_DRIVE)
#else
#define ALL_STARTED (USB_DRIVE)
#endif
#endif

uint8_t g_devices_started = 0;  // no devices started
uint8_t g_current_device = 0;

void fillScreen(uint8_t r, uint8_t g, uint8_t b, bool begin_end = true) {
    if (begin_end) Display.beginDraw();

    Display.background(r, g, b);
    Display.clear();
    if (begin_end) Display.endDraw();
}

inline void fillScreen(uint16_t color, bool begin_end = true) {
    uint8_t r, g, b;
    Color565ToRGB(color, r, g, b);
    fillScreen(r, g, b, begin_end);
}




//****************************************************************************
// Setup
//****************************************************************************
void setup(void) {
    // Enable the USBHost
    pinMode(PA_15, OUTPUT);
    digitalWrite(PA_15, HIGH);

    SDRAM.begin();  // start sdram up.

    // Keep the SD card inactive while working the display.
    delay(20);

    Serial.begin(115200);
    while (!Serial && millis() < 3000)
        ;
        // give chance to debug some display startups...

        //-----------------------------------------------------------------------------
        // initialize display
        //-----------------------------------------------------------------------------


#ifdef CS_SD
    Serial.print(">>> CS_SD: ");
    Serial.println(CS_SD, DEC);
    pinMode(CS_SD, OUTPUT);
    digitalWrite(CS_SD, HIGH);
#endif


    Serial.println("*** start up display ***");
    Display.begin();

    //tft.setRotation(1);

    fillScreen(0xff, 0, 0, true);  // experiment going direct
    //tft.fillScreen(RED);
    delay(500);
    fillScreen(0, 0xff, 0, true);
    //tft.fillScreen(GREEN);
    delay(500);
    fillScreen(0, 0, 0xff, true);
    //tft.fillScreen(BLUE);
    delay(500);

    g_tft_width = Display.width();
    g_tft_height = Display.height();


    // Wait for at least 1 device to start and a little extra.
    //    tft.fillScreen(RED);
    //    tft.setTextColor(RED, WHITE);
    //    tft.setCursor(1, 1);
    //    tft.print("Waiting for SD or USB");
    show_updated_startup_status();
    elapsedMillis em;
    while (!g_devices_started || (em < 3000)) {
        if (g_devices_started == ALL_STARTED) break;
#ifdef CS_SD
        if (!(g_devices_started & SD_DRIVE)) {
            Serial.println("calling SDBEGIN");
            if (SD.begin(SD_CONFIG)) {
                g_devices_started |= SD_DRIVE;
                show_updated_startup_status();
                if (!root_SD.open("/")) {
                    //tft.print("Failed to open root_SD directory");
                }
                Serial.println("SD Started");
                em = 0;
            }
        }
#endif
#ifdef ARDUINO_PORTENTA_H7_M7
        if (!(g_devices_started & SDIO_DRIVE)) {
            int err = sdio.mount(&block_device);
            if (!err) {
                g_devices_started |= SDIO_DRIVE;
                tft.setCursor(1, 90);
                tft.setTextColor(RED, WHITE);
                tft.println("SDIO Started");
                root_SDIO = opendir("/sd");
                Serial.println("SDIO Started");
                em = 0;
            }
        }
#endif

        if (!(g_devices_started & USB_DRIVE)) {
            if (!msd.connected()) {
                if (msd.connect()) Serial.println("MSD Connect");
            }
            if (msd.connected()) {
                Serial.println("Try to mount USB device");
                if (usb.mount(&msd) == 0) {
                    g_devices_started |= USB_DRIVE;
                    show_updated_startup_status();
                    root_USB = opendir("/usb/");
                    Serial.print("USB Started");
                    em = 0;
                }
            }
        }
        if (em > 5000) {
            static uint8_t wait_count = 0;
            wait_count++;
            //tft.setCursor(300, 1);
            //tft.write((wait_count & 1) ? '*' : ' ');
            em = 0;
        }
    }
    Serial.println("Started.");

    if (g_devices_started & USB_DRIVE) g_current_device = USB_DRIVE;
    else if (g_devices_started & SDIO_DRIVE) g_current_device = SDIO_DRIVE;
    else g_current_device = SD_DRIVE;

    Serial.print("\n\n>>>> All started: ");
    Serial.print(g_devices_started, HEX);
    Serial.print(" Current: ");
    Serial.println(g_current_device, HEX);


    Serial.println("OK!");
    emDisplayed = g_display_image_time;

    Serial.println("Simple Serial commands:");
    Serial.println("  1 - USB Drive");
    Serial.println("  2 - SDFat Drive");
    Serial.println("  3 - SDIO Drive");
    Serial.println("  d - toggle debug on and off");
    Serial.println("  s - toggle step mode (Pause after each picture");
    Serial.println("  l - list files on current drive");
    Serial.println("  <Anything else> will pause");

    //-----------------------------------------------------------------------------
    // Initialize options and then read optional config file
    //-----------------------------------------------------------------------------
    g_jpg_scale_x_above[0] = (g_tft_width * 3) / 2;
    g_jpg_scale_x_above[1] = g_tft_width * 3;
    g_jpg_scale_x_above[2] = g_tft_width * 6;
    g_jpg_scale_x_above[3] = g_tft_width * 12;

    g_jpg_scale_y_above[0] = (g_tft_height * 3) / 2;
    g_jpg_scale_y_above[1] = g_tft_height * 3;
    g_jpg_scale_y_above[2] = g_tft_height * 6;
    g_jpg_scale_y_above[3] = g_tft_height * 12;
    FILE *optionsFile = fopen(options_file_name, "r+");
    if (optionsFile) {
        Serial.print("Opened Options file:");
        Serial.println(options_file_name);
        WrapperFile wfileOptions(optionsFile);
        ProcessOptionsFile(&wfileOptions);
        fclose(optionsFile);
    } else {
#ifdef CS_SD
        FsFile options_file;
        if (options_file.open(options_file_name_sd)) {
            Serial.print("Opened Options file:");
            Serial.println(options_file_name_sd);
            WrapperFile wfileOptions(options_file);
            ProcessOptionsFile(&wfileOptions);
            options_file.close();
        }

#endif
    }
    ShowAllOptionValues();
}

void show_updated_startup_status() {
    Display.beginDraw();
    fillScreen(0, 0, 0xff, false);
    Display.textFont(Font_5x7);
    //Display.textSize(5, 5);
    Display.setTextSize(5);
    Display.stroke(0xff, 0, 0);
    Display.background(0, 0, 0xff);
    Display.text("Waiting for SD or USB", 0, 0);

    if (g_devices_started & SD_DRIVE) {
        Display.text("SD Started", 100, 100);
    }

    if (g_devices_started & USB_DRIVE) {
        Display.text("USB Started", 100, 200);
    }

    Display.endDraw();
}

//****************************************************************************
// loop
//****************************************************************************
void loop() {
    // don't process unless time elapsed or g_fast_mode
    // Timing may depend on which type of display we are using...
    // if it has logical frame buffer, maybe as soon as we display an image,
    // try to load the next one, and then wait until the image time to
    // tell display to update...

    if (!g_fast_mode && !g_stepMode && (emDisplayed < (uint32_t)g_display_image_time)) {
        // so we are not step or fast and time < pcture time should we wait?
        // if we are not using canvas or we have already loaded the next one then return
        if (!g_use_canvas || g_picture_loaded) return;
    }

    //---------------------------------------------------------------------------
    // Find the next file to read in.
    //---------------------------------------------------------------------------
    if (!g_picture_loaded) {
        bool did_rewind = false;
        uint8_t name_len;
        bool bmp_file = false;
        bool jpg_file = false;
        bool png_file = false;

        Serial.println("\nLoop looking for image file");

        // BUGBUG: this is not overly clean, but tries to allow two different
        // types of file systems...
        WrapperFile wpfImage;
        FsFile imageFile;
        struct dirent *dir_entry;

        char file_name[MAX_FILENAME_LEN];
        for (;;) {
#ifdef ARDUINO_PORTENTA_H7_M7
            if (g_current_device == SDIO_DRIVE) {
                dir_entry = readdir(root_SDIO);
                if (!dir_entry) {
                    if (did_rewind) break;  // only go around once.
                    rewinddir(root_SDIO);
                    continue;  // try again
                }
                // maybe should check file name quick and dirty
                if (!dir_entry->d_name) continue;
                strcpy(file_name, "/sd/");
                strcat(file_name, dir_entry->d_name);
                name_len = strlen(file_name);
            } else
#endif
                if (g_current_device == SD_DRIVE) {
                imageFile = root_SD.openNextFile();
                if (!imageFile) {
                    if (did_rewind) break;  // only go around once.
                    root_SD.rewindDirectory();
                    did_rewind = true;
                    continue;  // try again
                }
                // maybe should check file name quick and dirty
                name_len = imageFile.getName(file_name, sizeof(file_name));
            } else {
                dir_entry = readdir(root_USB);
                if (!dir_entry) {
                    if (did_rewind) break;  // only go around once.
                    rewinddir(root_USB);
                    continue;  // try again
                }
                // maybe should check file name quick and dirty
                if (!dir_entry->d_name) continue;
                strcpy(file_name, "/usb/");
                strcat(file_name, dir_entry->d_name);
                name_len = strlen(file_name);
            }

            if ((strcmp(&file_name[name_len - 4], ".bmp") == 0) || (strcmp(&file_name[name_len - 4], ".BMP") == 0)) bmp_file = true;
            if ((strcmp(&file_name[name_len - 4], ".jpg") == 0) || (strcmp(&file_name[name_len - 4], ".JPG") == 0)) jpg_file = true;
            if (stricmp(&file_name[name_len - 4], ".bmp") == 0) bmp_file = true;
            if (stricmp(&file_name[name_len - 4], ".jpg") == 0) jpg_file = true;
            if (stricmp(&file_name[name_len - 4], ".png") == 0) png_file = true;
            //if (stricmp(name, options_file_name) == 0) ProcessOptionsFile(imageFile);
            if (bmp_file || jpg_file || png_file) break;
        }

//---------------------------------------------------------------------------
// Found a file so try to process it.
//---------------------------------------------------------------------------
#ifdef ARDUINO_PORTENTA_H7_M7
        if (g_current_device == SDIO_DRIVE) {
            FILE *bmpFile = fopen(file_name, "r+");
            wpfImage.setFile(bmpFile);
        }
#endif
        if (g_current_device == SD_DRIVE) {
            wpfImage.setFile(imageFile);
        } else {

            FILE *bmpFile = fopen(file_name, "r+");
            wpfImage.setFile(bmpFile);
        }

        if (wpfImage) {

            elapsedMillis emDraw = 0;
            g_WRCount = 0;
            if (bmp_file) {
                bmpDraw24(wpfImage, file_name, true);

#ifdef __JPEGDEC__
            } else if (jpg_file) {
                processJPGFile(wpfImage, file_name, true);
#endif

#ifdef __PNGDEC__
            } else if (png_file) {
                processPNGFile(wpfImage, file_name, true);
#endif
            }
            Serial.print("!!File:");
            Serial.print(file_name);
            Serial.print(" Time:");
            Serial.print((uint32_t)emDraw, DEC);
            Serial.print(" writeRect calls:");
            Serial.println(g_WRCount, DEC);
        } else {
            fillScreen(0, 0, 0xff, true);
            //pgfx->fillScreen(GREEN);
            //pgfx->setTextColor(WHITE);
            //pgfx->setTextSize(2);
            //pgfx->println(F("No Files Found"));
        }
        g_picture_loaded = true;
    }

    //---------------------------------------------------------------------------
    // If the display has a command to update the screen now, see if we should
    // do now or wait until proper time
    //---------------------------------------------------------------------------
    if (g_fast_mode || g_stepMode || (emDisplayed >= (uint32_t)g_display_image_time)) {
        if (g_picture_loaded) {
            Display.endDraw();  // lets update with new image...
        }
        //---------------------------------------------------------------------------
        // Process any keyboard input.
        //---------------------------------------------------------------------------
        if (g_stepMode) {
            int ch;
            Serial.println("Step Mode: enter anything to continue");
            while ((ch = Serial.read()) == -1) {}  // in case at startup...
            while (ch != -1) {
                if (ch == 'd') g_debug_output = !g_debug_output;
                else if (ch == 's') g_stepMode = !g_stepMode;
                else if (ch == 'l') listFiles();
                MaybeSwtichDrive(ch);

                ch = Serial.read();
            }
        } else if (Serial.available()) {
            int ch;
            while (Serial.read() != -1)
                ;
            Serial.println("Paused: enter anything to continue");
            while ((ch = Serial.read()) == -1) {}
            while (ch != -1) {
                if (ch == 'd') g_debug_output = !g_debug_output;
                else if (ch == 's') g_stepMode = !g_stepMode;
                else if (ch == 'l') listFiles();
                MaybeSwtichDrive(ch);
                ch = Serial.read();
            }
        }
        emDisplayed = 0;
        g_picture_loaded = false;
    }
}

void MaybeSwtichDrive(int ch) {
    switch (ch) {
        case '1':
            if (g_devices_started & USB_DRIVE) {
                g_current_device = USB_DRIVE;
                Serial.println("*** Switched to USB Drive ***");
            }
            break;
        case '2':
            if (g_devices_started & SD_DRIVE) {
                g_current_device = SD_DRIVE;
                Serial.println("*** Switched to SDFat Drive ***");
            }
            break;
        case '3':
            if (g_devices_started & SDIO_DRIVE) {
                g_current_device = SDIO_DRIVE;
                Serial.println("*** Switched to SDIO Drive ***");
            }
            break;
    }
}

int stricmp(const char *s1, const char *s2) {
    while (*s1 != 0 && *s2 != 0) {
        if (*s1 != *s2 && ::toupper(*s1) != ::toupper(*s2)) {
            return -1;
        }
        s1++;
        s2++;
    }
    return (*s1 == 0 && *s2 == 0) ? 0 : -1;
}

//****************************************************************************
// forward function definitions.
//****************************************************************************
//****************************************************************************
// forward function definitions.
//****************************************************************************

void writeClippedRect24(int16_t x, int16_t y, int16_t cx, int16_t cy, uint32_t *pixels) {
    /*    Serial.print("WCR24(");
    Serial.print(x);
    Serial.print(",");
    Serial.print(y);
    Serial.print(",");
    Serial.print(cx);
    Serial.print(",");
    Serial.print(cy);
    Serial.print(",");
    Serial.print((uint32_t)pixels, HEX);
    Serial.print("Off: ");
    Serial.print(g_image_offset_x);
    Serial.print(",");
    Serial.println(g_image_offset_y);
*/

    //Image img(ENCODING_RGB24, (uint8_t *)pixels, cx, cy);
    y += g_image_offset_y;
    x += g_image_offset_x;
#if 0
    Image img(ENCODING_RGB, (uint8_t *)pixels, cx, cy);
    Display.image(img, x, y, cx, cy);
#else
    //Display.image(img, x, y);
    for (int16_t iy = y; iy < (y + cy); iy++) {
        for (int16_t ix = x; ix < (x + cx); ix++) {
            uint32_t color = *pixels++;
            Display.set(ix, iy, color >> 16, color >> 8, color);
        }
    }
#endif
}



//=============================================================================
// Options file support - process only if file changed dates (Or first time)
//    example looking for update file.
// This is a real simple parser x=y where x is string y is int...
//=============================================================================
//DateTimeFields g_dtf_optFileLast = { 99 };  // not valid so change first time...
#define MAX_KEY_NAME 20
typedef struct {
    const char key_name[MAX_KEY_NAME];
    int *key_value_addr;
} key_name_value_t;

static const PROGMEM key_name_value_t keyNameValues[] = {
    { "Background", &g_background_color },
    { "debug", &g_debug_output },
    { "Step", &g_stepMode },
    { "BMPScale", &g_BMPScale },
    { "JPGScale", &g_JPGScale },
    { "PNGScale", &g_PNGScale },
    { "ScaleXAbove2", &g_jpg_scale_x_above[SCL_HALF] },
    { "ScaleXAbove4", &g_jpg_scale_x_above[SCL_QUARTER] },
    { "ScaleXAbove8", &g_jpg_scale_x_above[SCL_EIGHTH] },
    { "ScaleXAbove16", &g_jpg_scale_x_above[SCL_16TH] },
    { "ScaleYAbove2", &g_jpg_scale_y_above[SCL_HALF] },
    { "ScaleYAbove4", &g_jpg_scale_y_above[SCL_QUARTER] },
    { "ScaleYAbove8", &g_jpg_scale_y_above[SCL_EIGHTH] },
    { "ScaleYAbove16", &g_jpg_scale_y_above[SCL_16TH] },
    { "Center", &g_center_image },
    { "MaxScaleUp", &g_max_scale_up },
    { "ImageTimeMS", &g_display_image_time }
};


bool ReadOptionsLine(WrapperFile *optFile, char *key_name, uint8_t sizeof_key, int &key_value) {
    int ch;

    key_value = 0;
    // first lets get key name ignore all whitespace...
    while ((ch = optFile->read()) <= ' ') {
        if (ch < 0) return false;
    }

    uint8_t ich = 0;
    while (ich < (sizeof_key - 1)) {
        if (ch == '=') {
            ch = optFile->read();
            break;
        }
        key_name[ich++] = ch;
        ch = optFile->read();
        if (ch < 0) return false;  //
    }
    key_name[ich] = '\0';

    int sign_value = 1;
    if (ch == '-') {
        sign_value = -1;
        ch = optFile->read();
        if (ch == -1) return false;
    }

    while ((ch >= '0') && (ch <= '9')) {
        key_value = key_value * 10 + ch - '0';
        ch = optFile->read();
    }
    // should probably check for other stuff, but...
    key_value *= sign_value;

    // total hacky but allow hex value
    if ((key_value == 0) && ((ch == 'x') || (ch == 'X'))) {
        ch = optFile->read();
        for (;;) {
            if ((ch >= '0') && (ch <= '9')) key_value = key_value * 16 + ch - '0';
            else if ((ch >= 'a') && (ch <= 'f'))
                key_value = key_value * 16 + 10 + ch - 'a';
            else if ((ch >= 'A') && (ch <= 'F'))
                key_value = key_value * 16 + 10 + ch - 'A';
            else
                break;
            ch = optFile->read();
        }
    }

    return true;
}


bool ProcessOptionsFile(WrapperFile *optfile) {
    if (!optfile) return false;
    int key_value;
    char key_name[20];
#ifdef LATER
    DateTimeFields dtf;
    if (!optfile.getModifyTime(dtf)) return false;
    if (memcmp(&dtf, &g_dtf_optFileLast, sizeof(dtf)) == 0) return false;
    g_dtf_optFileLast = dtf;
    Serial.printf("Updated Options file found date: M: %02u/%02u/%04u %02u:%02u\n",
                  dtf.mon + 1, dtf.mday, dtf.year + 1900, dtf.hour, dtf.min);

// do simple scan through file
#endif
    bool found = false;
    while (ReadOptionsLine(optfile, key_name, sizeof(key_name), key_value)) {
        Serial.print("\t>>");
        Serial.print(key_name);
        Serial.print("=");
        Serial.print(key_value, DEC);
        for (uint8_t key_index = 0; key_index < (sizeof(keyNameValues) / sizeof(keyNameValues[0])); key_index++) {
            if (stricmp(key_name, keyNameValues[key_index].key_name) == 0) {
                Serial.print(" was: ");
                Serial.println(*(keyNameValues[key_index].key_value_addr), DEC);
                *(keyNameValues[key_index].key_value_addr) = key_value;
                found = true;
                break;
            }
        }
        if (!found) Serial.println(" ** Unknown Key **");
    }


    return true;
}

void ShowAllOptionValues() {
    Serial.println("\n----------------------------------");
    Serial.print("Sketch uses Option file: ");
    Serial.print(options_file_name);
    Serial.print(" or ");
    Serial.print(options_file_name_sd);
    Serial.println(" at the root of USB drive or SD Card");
    Serial.println("\t<All key names>=<current key value");
    for (uint8_t key_index = 0; key_index < (sizeof(keyNameValues) / sizeof(keyNameValues[0])); key_index++) {
        Serial.print("\t");
        Serial.print(keyNameValues[key_index].key_name);
        Serial.print("=");
        Serial.println(*(keyNameValues[key_index].key_value_addr), DEC);
    }
    Serial.println("----------------------------------\n");
}


inline uint16_t Color565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}
inline void Color565ToRGB(uint16_t color, uint8_t &r, uint8_t &g, uint8_t &b) {
    r = (color >> 8) & 0x00F8;
    g = (color >> 3) & 0x00FC;
    b = (color << 3) & 0x00F8;
}

inline uint32_t color888(uint8_t r, uint8_t g, uint8_t b) {
    return 0xff000000 | (r << 16) | (g << 8) | b;
}

inline void color888toRGB(uint32_t color, uint8_t &r, uint8_t &g, uint8_t &b) {
    r = color >> 16;
    g = color >> 8;
    b = color;
}
inline uint32_t color565To888(uint16_t color) {
    uint8_t r, g, b;
    Color565ToRGB(color, r, g, b);
    if (r) r |= 0x7;
    if (g) g |= 0x3;
    if (b) b |= 0x7;
    return 0xff000000 | (r << 16) | (g << 8) | b;
}



//=============================================================================
// BMP support
//=============================================================================
// This function opens a Windows Bitmap (BMP) file and
// displays it at the given coordinates.  It's sped up
// by reading many pixels worth of data at a time
// (rather than pixel by pixel).  Increasing the buffer
// size takes more of the Arduino's precious RAM but
// makes loading a little faster.  20 pixels seems a
// good balance for tiny AVR chips.

#define BUFFPIXEL 80
void bmpDraw24(WrapperFile &bmpFile, const char *filename, bool fErase) {

    //  File     bmpFile;
    int image_width, image_height;        // W+H in pixels
    uint8_t bmpDepth;                     // Bit depth (currently must be 24)
    uint32_t bmpImageoffset;              // Start of image data in file
    uint32_t rowSize;                     // Not always = image_width; may have padding
    uint8_t sdbuffer[3 * BUFFPIXEL];      // pixel buffer (R+G+B per pixel)
    uint16_t buffidx = sizeof(sdbuffer);  // Current position in sdbuffer
    boolean goodBmp = false;              // Set to true on valid header parse
    boolean flip = true;                  // BMP is stored bottom-to-top
    int row, col;
    uint8_t r, g, b;
    uint32_t pos = 0;

    uint32_t *usPixels = nullptr;


    Serial.println();
    Serial.print(F("Loading image '"));
    Serial.print(filename);
    Serial.println('\'');

    // Parse BMP header
    if (read16(bmpFile) == 0x4D42) {                                     // BMP signature
        uint32_t bmpFileSize __attribute__((unused)) = read32(bmpFile);  // Read & ignore creator bytes
        //Serial.print(F("File size: ")); Serial.println(bmpFileSize);
        (void)read32(bmpFile);             // Read & ignore creator bytes
        bmpImageoffset = read32(bmpFile);  // Start of image data
        //Serial.print(F("Image Offset: ")); Serial.println(bmpImageoffset, DEC);
        // Read DIB header
        uint32_t bmpHdrSize __attribute__((unused)) = read32(bmpFile);
        //Serial.print(F("Header size: ")); Serial.println(bmpHdrSize);
        g_image_width = image_width = read32(bmpFile);
        g_image_height = image_height = read32(bmpFile);

        if (read16(bmpFile) == 1) {      // # planes -- must be '1'
            bmpDepth = read16(bmpFile);  // bits per pixel
            //Serial.print(F("Bit Depth: ")); Serial.println(bmpDepth);
            if ((bmpDepth == 24) && (read32(bmpFile) == 0)) {  // 0 = uncompressed

                goodBmp = true;  // Supported BMP format -- proceed!
                Display.beginDraw();
                Serial.print("Image size: ");
                Serial.print(image_width);
                Serial.print("x");
                Serial.print(image_height);
                Serial.print(" depth:");
                Serial.print(bmpDepth, DEC);
                // BMP rows are padded (if needed) to 4-byte boundary
                rowSize = (image_width * 3 + 3) & ~3;

                // If image_height is negative, image is in top-down order.
                // This is not canon but has been observed in the wild.
                if (image_height < 0) {
                    image_height = -image_height;
                    flip = false;
                }

                g_image_scale = 1;
                g_image_scale_up = 0;
                if (g_BMPScale > 0) {
                    g_image_scale = g_BMPScale;  // use what they passed in
                } else if (g_BMPScale < 0) {
                    // bugbug experiement - try to scale up...
                    if ((image_width * 2 < g_tft_width) && (image_height * 2 < g_tft_height)) {
                        // image is less than half the screen...
                        // lets try simple scale up...
                        g_image_scale_up = g_tft_width / image_width;
                        int scale_up_height = g_tft_height / image_height;
                        if (scale_up_height < g_image_scale_up) g_image_scale_up = scale_up_height;
                        if (g_image_scale_up > g_max_scale_up) g_image_scale_up = g_max_scale_up;

                    } else {
                        if (image_width > g_tft_width) g_image_scale = (image_width + g_tft_width - 1) / g_tft_width;
                        if (image_height > g_tft_height) {
                            int yscale = (image_height + g_tft_height - 1) / g_tft_height;
                            if (yscale > g_image_scale) g_image_scale = yscale;
                        }
                    }
                } else {
                    if ((image_width > g_jpg_scale_x_above[SCL_16TH]) || (image_height > g_jpg_scale_y_above[SCL_16TH])) {
                        g_image_scale = 16;
                    } else if ((image_width > g_jpg_scale_x_above[SCL_EIGHTH]) || (image_height > g_jpg_scale_y_above[SCL_EIGHTH])) {
                        g_image_scale = 8;
                    } else if ((image_width > g_jpg_scale_x_above[SCL_QUARTER]) || (image_height > g_jpg_scale_y_above[SCL_QUARTER])) {
                        g_image_scale = 4;
                    } else if ((image_width > g_jpg_scale_x_above[SCL_HALF]) || (image_height > g_jpg_scale_y_above[SCL_HALF])) {
                        g_image_scale = 2;
                    }
                }
                if (g_image_scale_up) {
                    if (g_center_image) {
                        g_image_offset_x = (g_tft_width - (image_width * g_image_scale_up)) / 2;
                        g_image_offset_y = (g_tft_height - (image_height * g_image_scale_up)) / 2;
                    } else {
                        g_image_offset_x = 0;
                        g_image_offset_y = 0;
                    }

                    g_image_scale = 2;  // bugbug use this to know which row to read in to...
                    Serial.print("Scale: ");
                    Serial.print(g_image_scale_up, DEC);
                    Serial.print(" Image Offsets (");
                    Serial.print(g_image_offset_x, DEC);
                    Serial.print(", ");
                    Serial.print(g_image_offset_y, DEC);
                    Serial.println(")");

                    if (fErase && (((image_width * g_image_scale_up) < g_tft_width) || ((image_height * g_image_scale_up) < image_height))) {
                        fillScreen((uint16_t)g_background_color, false);
                    }

                    // now we will allocate large buffer for SCALE*width
                    // need 2 rows to work with, and resultant output will be an addition 2x
                    usPixels = (uint32_t *)malloc(image_width * (2 + g_image_scale_up * g_image_scale_up) * sizeof(uint32_t));

                } else {
                    if (g_center_image) {
                        g_image_offset_x = (g_tft_width - (image_width / g_image_scale)) / 2;
                        g_image_offset_y = (g_tft_height - (image_height / g_image_scale)) / 2;
                    } else {
                        g_image_offset_x = 0;
                        g_image_offset_y = 0;
                    }
                    Serial.print("Scale: 1/");
                    Serial.print(g_image_scale, DEC);
                    Serial.print(" Image Offsets (");
                    Serial.print(g_image_offset_x, DEC);
                    Serial.print(", ");
                    Serial.print(g_image_offset_y, DEC);
                    Serial.println(")");

                    if (fErase && (((image_width / g_image_scale) < g_tft_width) || ((image_height / g_image_scale) < image_height))) {
                        fillScreen((uint16_t)g_background_color, false);
                    }
                    // now we will allocate large buffer for SCALE*width
                    usPixels = (uint32_t *)malloc(image_width * g_image_scale * sizeof(uint32_t));
                }

                if (usPixels) {
                    for (row = 0; row < image_height; row++) {  // For each scanline...

                        // Seek to start of scan line.  It might seem labor-
                        // intensive to be doing this on every line, but this
                        // method covers a lot of gritty details like cropping
                        // and scanline padding.  Also, the seek only takes
                        // place if the file position actually needs to change
                        // (avoids a lot of cluster math in SD library).
                        if (flip)  // Bitmap is stored bottom-to-top order (normal BMP)
                            pos = bmpImageoffset + (image_height - 1 - row) * rowSize;
                        else  // Bitmap is stored top-to-bottom
                            pos = bmpImageoffset + row * rowSize;
                        if (bmpFile.position() != pos) {  // Need seek?
                            bmpFile.seek(pos);
                            buffidx = sizeof(sdbuffer);  // Force buffer reload
                        }

                        uint32_t *pusRow = usPixels + image_width * (row % g_image_scale);
                        for (col = 0; col < image_width; col++) {  // For each pixel...
                            // Time to read more pixel data?
                            if (buffidx >= sizeof(sdbuffer)) {  // Indeed
                                bmpFile.read(sdbuffer, sizeof(sdbuffer));
                                buffidx = 0;  // Set index to beginning
                            }

                            // Convert pixel from BMP to TFT format, push to display
                            b = sdbuffer[buffidx++];
                            g = sdbuffer[buffidx++];
                            r = sdbuffer[buffidx++];
                            pusRow[col] = color888(r, g, b);
                        }  // end pixel
                        if (g_image_scale_up) {
                            ScaleUpwriteClippedRect24(row, image_width, usPixels);
                        } else if (g_image_scale == 1) {
                            writeClippedRect24(0, row, image_width, 1, pusRow);
                        } else {
                            ScaleDownwriteClippedRect24(row, image_width, usPixels);
                        }
                    }                // end scanline
                    free(usPixels);  // free it after we are done
                    usPixels = nullptr;
                }  // malloc succeeded

            }  // end goodBmp
        }
    }
    //Display.endDraw();
    bmpFile.close();
    if (!goodBmp) Serial.println(F("BMP format not recognized."));
}
// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(WrapperFile &f) {
    uint16_t result;
    ((uint8_t *)&result)[0] = f.read();  // LSB
    ((uint8_t *)&result)[1] = f.read();  // MSB
    return result;
}

uint32_t read32(WrapperFile &f) {
    uint32_t result;
    ((uint8_t *)&result)[0] = f.read();  // LSB
    ((uint8_t *)&result)[1] = f.read();
    ((uint8_t *)&result)[2] = f.read();
    ((uint8_t *)&result)[3] = f.read();  // MSB
    return result;
}



//=============================================================================
// TFT Helper functions t
// which doe snot have offset/clipping support
//=============================================================================


void ScaleUpwriteClippedRect24(int row, int image_width, uint32_t *usPixels) {
    //--------------------------------------------------------------------
    // experiment scale up...
    //--------------------------------------------------------------------
    uint32_t *usRowOut = usPixels + 2 * image_width;  //
    uint8_t r_cur, r_prev, g_cur, g_prev, b_cur, b_prev;
    int red, green, blue;
    int image_width_out = (image_width - 1) * g_image_scale_up + 1;  // we don't fill out the last one.
    // if this is not our first row, then we need to compute the fill in row
    // first...
    // Our buffer has g_image_scale_up rows of data to send in one chunk
    uint32_t *puCol = usRowOut;
    uint32_t *puCurRow;
    uint32_t *puPrevRow;
    if (row & 1) {
        puCurRow = usPixels + image_width;
        puPrevRow = usPixels;
    } else {
        puCurRow = usPixels;
        puPrevRow = usPixels + image_width;
    }

    // First lets generate the one for the actual row;
    uint32_t *p = usRowOut + image_width_out * (g_image_scale_up - 1);
    uint32_t *ppixIn = puCurRow;
    for (int col = 0; col < image_width; col++) {
        // bug bug.. could be faster
        *p = *ppixIn++;  // copy the pixel in to pixel out
        if (col) {
            // Now lets fill in the columns between the prev and new...
            color888toRGB(*p, r_cur, g_cur, b_cur);
            color888toRGB(*(p - g_image_scale_up), r_prev, g_prev, b_prev);
            for (int j = 1; j < g_image_scale_up; j++) {
                red = (int)r_prev + (((int)r_cur - (int)r_prev) * j) / g_image_scale_up;
                green = (int)g_prev + (((int)g_cur - (int)g_prev) * j) / g_image_scale_up;
                blue = (int)b_prev + (((int)b_cur - (int)b_prev) * j) / g_image_scale_up;
                *(p - g_image_scale_up + j) = color888(red, green, blue);
            }
        }
        p += g_image_scale_up;
    }

    // except for the first row we now need to fill in the extra rows from the previous one
    if (row) {
        for (int col = 0; col < image_width; col++) {
            color888toRGB(*puCurRow++, r_cur, g_cur, b_cur);
            color888toRGB(*puPrevRow++, r_prev, g_prev, b_prev);
            for (int i = 1; i < g_image_scale_up; i++) {
                uint32_t *p = puCol + (i - 1) * image_width_out;  // so location for this item
                int red = (int)r_prev + (((int)r_cur - (int)r_prev) * i) / g_image_scale_up;
                int green = (int)g_prev + (((int)g_cur - (int)g_prev) * i) / g_image_scale_up;
                int blue = (int)b_prev + (((int)b_cur - (int)b_prev) * i) / g_image_scale_up;
                *p = color888(red, green, blue);
                // need to compute middle ones as well.
                if (col) {
                    // Now lets fill in the columns between the prev and new...
                    color888toRGB(*p, r_cur, g_cur, b_cur);
                    color888toRGB(*(p - g_image_scale_up), r_prev, g_prev, b_prev);
                    for (int j = 1; j < g_image_scale_up; j++) {
                        red = (int)r_prev + (((int)r_cur - (int)r_prev) * j) / g_image_scale_up;
                        green = (int)g_prev + (((int)g_cur - (int)g_prev) * j) / g_image_scale_up;
                        blue = (int)b_prev + (((int)b_cur - (int)b_prev) * j) / g_image_scale_up;
                        *(p - g_image_scale_up + j) = color888(red, green, blue);
                    }
                }
            }
            puCol += g_image_scale_up;
        }
        writeClippedRect24(0, 1 + (row - 1) * g_image_scale_up, image_width_out, g_image_scale_up, usRowOut);
    } else {
        // first row just output it's own data.
        writeClippedRect24(0, 0, image_width_out, 1, usRowOut + image_width_out * (g_image_scale_up - 1));
    }
}

void ScaleDownwriteClippedRect24(int row, int image_width, uint32_t *usPixels) {
    if ((row % g_image_scale) == (g_image_scale - 1)) {
        //--------------------------------------------------------------------
        // else scale down
        //--------------------------------------------------------------------
        uint32_t newx = 0;
        for (uint32_t pix_cnt = 0; pix_cnt < (uint32_t)image_width; pix_cnt += g_image_scale) {
            uint8_t red = 0;
            uint8_t green = 0;
            uint8_t blue = 0;
            float r = 0;
            float g = 0;
            float b = 0;
            for (uint8_t i = 0; i < g_image_scale; i++) {
                for (uint8_t j = 0; j < g_image_scale; j++) {
                    color888toRGB(usPixels[pix_cnt + i + (j * image_width)], red, green, blue);
                    // Sum the squares of components instead
                    r += red * red;
                    g += green * green;
                    b += blue * blue;
                }
            }
            // overwrite the start of our buffer with
            usPixels[newx++] = color888((uint8_t)sqrt(r / (g_image_scale * g_image_scale)), (uint8_t)sqrt(g / (g_image_scale * g_image_scale)), (uint8_t)sqrt(b / (g_image_scale * g_image_scale)));
        }
        writeClippedRect24(0, row / g_image_scale, image_width / g_image_scale, 1, usPixels);
    }
}


//=============================================================================
// JPeg support
//=============================================================================
//used for jpeg files primarily
#ifdef __JPEGDEC__
JPEGDEC jpeg;


void processJPGFile(WrapperFile &jpgFile, const char *name, bool fErase) {
    int image_size = jpgFile.size();
    jpgFile.seek(0);
    Serial.println();
    Serial.print((uint32_t)&jpgFile, HEX);
    Serial.print(F(" Loading JPG image '"));
    Serial.print(name);
    Serial.print("' ");
    Serial.println(image_size, DEC);
    uint8_t scale = 1;
    if (jpeg.open((void *)&jpgFile, image_size, nullptr, myReadJPG, mySeekJPG, JPEGDraw)) {
        int image_width = jpeg.getWidth();
        int image_height = jpeg.getHeight();
        int decode_options = 0;
        Serial.print("Image size: ");
        Serial.print(image_width);
        Serial.print("x");
        Serial.print(image_height);

        // Try setting to 24 bit mode
        jpeg.setPixelType(RGB8888);

        switch (g_JPGScale) {
            case 1:
                scale = 1;
                decode_options = 0;
                break;
            case 2:
                scale = 2;
                decode_options = JPEG_SCALE_HALF;
                break;
            case 4:
                scale = 4;
                decode_options = JPEG_SCALE_QUARTER;
                break;
            case 8:
                scale = 8;
                decode_options = JPEG_SCALE_EIGHTH;
                break;
            default:
                {
                    if ((image_width > g_jpg_scale_x_above[SCL_16TH]) || (image_height > g_jpg_scale_y_above[SCL_16TH])) {
                        decode_options = JPEG_SCALE_EIGHTH | JPEG_SCALE_HALF;
                        scale = 16;
                    } else if ((image_width > g_jpg_scale_x_above[SCL_EIGHTH]) || (image_height > g_jpg_scale_y_above[SCL_EIGHTH])) {
                        decode_options = JPEG_SCALE_EIGHTH;
                        scale = 8;
                    } else if ((image_width > g_jpg_scale_x_above[SCL_QUARTER]) || (image_height > g_jpg_scale_y_above[SCL_QUARTER])) {
                        decode_options = JPEG_SCALE_QUARTER;
                        scale = 4;
                    } else if ((image_width > g_jpg_scale_x_above[SCL_HALF]) || (image_height > g_jpg_scale_y_above[SCL_HALF])) {
                        decode_options = JPEG_SCALE_HALF;
                        scale = 2;
                    }
                }
        }
        Display.beginDraw();
        if (fErase && ((image_width / scale < g_tft_width) || (image_height / scale < g_tft_height))) {
            fillScreen((uint16_t)g_background_color, false);
        }

        if (g_center_image) {
            g_image_offset_x = (g_tft_width - image_width / scale) / 2;
            g_image_offset_y = (g_tft_height - image_height / scale) / 2;
        } else {
            g_image_offset_x = 0;
            g_image_offset_y = 0;
        }
        g_image_scale = scale;
        Serial.print("Scale: 1/");
        Serial.print(g_image_scale);
        Serial.print(" Image Offsets (");
        Serial.print(g_image_offset_x);
        Serial.print(", ");
        Serial.print(g_image_offset_y), Serial.println(")");

        jpeg.decode(0, 0, decode_options);
        jpeg.close();
        //Display.endDraw();
    } else {
        Serial.println("Was not a valid jpeg file");
    }
    jpgFile.close();
}


int32_t myReadJPG(JPEGFILE *pjpegfile, uint8_t *buffer, int32_t length) {
    if (!pjpegfile || !pjpegfile->fHandle) return 0;
    return ((WrapperFile *)(pjpegfile->fHandle))->read(buffer, length);
}
int32_t mySeekJPG(JPEGFILE *pjpegfile, int32_t position) {
    if (!pjpegfile || !pjpegfile->fHandle) return 0;
    return ((WrapperFile *)(pjpegfile->fHandle))->seek(position);
}

int JPEGDraw(JPEGDRAW *pDraw) {
    if (g_debug_output) {
        Serial.print("jpeg draw: x,y=");
        Serial.print(pDraw->x);
        Serial.print(",");
        Serial.print(pDraw->y);
        Serial.print(", cx,cy = ");
        Serial.print(pDraw->iWidth);
        Serial.print(",");
        Serial.println(pDraw->iHeight);
    }
    writeClippedRect24(pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight, (uint32_t *)pDraw->pPixels);
    return 1;
}
#endif

//=============================================================================
// PNG support
//=============================================================================
// used for png files primarily
#ifdef __PNGDEC__
PNG png;

void processPNGFile(WrapperFile &pngFile, const char *name, bool fErase) {
    int rc;

    Serial.println();
    Serial.print(F("Loading PNG image '"));
    Serial.print(name);
    Serial.println('\'');
    // hack pass pointer to file as the name...
    rc = png.open((const char *)(void *)&pngFile, myOpen, nullptr, myReadPNG, mySeekPNG, PNGDraw);
    if (rc == PNG_SUCCESS) {
        g_image_width = png.getWidth();
        g_image_height = png.getHeight();
        g_image_Bpp = png.getBpp();
        g_image_scale_up = 0;

        g_image_scale = 1;  // default...
        Serial.print("image specs: (");
        Serial.print(g_image_width);
        Serial.print(" x ");
        Serial.print(g_image_height);
        Serial.print("), ");
        Serial.print(png.getBpp());
        Serial.print(" bpp, pixel type: ");
        Serial.println(png.getPixelType());
        if (g_PNGScale > 0) {
            g_image_scale = g_PNGScale;  // use what they passed in
        } else if (g_PNGScale < 0) {
            if (g_image_width > g_tft_width)
                g_image_scale = (g_image_width + g_tft_width - 1) / g_tft_width;
            if (g_image_height > g_tft_height) {
                int yscale = (g_image_height + g_tft_height - 1) / g_tft_height;
                if (yscale > g_image_scale)
                    g_image_scale = yscale;
            }
        } else {
            if ((g_image_width > g_jpg_scale_x_above[SCL_16TH]) || (g_image_height > g_jpg_scale_y_above[SCL_16TH])) {
                g_image_scale = 16;
            } else if ((g_image_width > g_jpg_scale_x_above[SCL_EIGHTH]) || (g_image_height > g_jpg_scale_y_above[SCL_EIGHTH])) {
                g_image_scale = 8;
            } else if ((g_image_width > g_jpg_scale_x_above[SCL_QUARTER]) || (g_image_height > g_jpg_scale_y_above[SCL_QUARTER])) {
                g_image_scale = 4;
            } else if ((g_image_width > g_jpg_scale_x_above[SCL_HALF]) || (g_image_height > g_jpg_scale_y_above[SCL_HALF])) {
                g_image_scale = 2;
            }
        }
        Display.beginDraw();
        if (fErase && (((g_image_width / g_image_scale) < g_tft_width) || ((g_image_height / g_image_scale) < g_image_height))) {
            fillScreen((uint16_t)g_background_color, false);
        }

        if (g_center_image) {
            g_image_offset_x = (g_tft_width - (png.getWidth() / g_image_scale)) / 2;
            g_image_offset_y = (g_tft_height - (png.getHeight() / g_image_scale)) / 2;
        } else {
            g_image_offset_x = 0;
            g_image_offset_y = 0;
        }


        Serial.print("Scale: 1/");
        Serial.print(g_image_scale);
        Serial.print(" Image Offsets (");
        Serial.print(g_image_offset_x);
        Serial.print(", ");
        Serial.print(g_image_offset_y);
        Serial.println(")");

        uint32_t *usPixels = (uint32_t *)malloc(
            g_image_width * ((g_image_scale == 1) ? 16 : g_image_scale) * sizeof(uint32_t));
        if (usPixels) {
            rc = png.decode(usPixels, 0);

            if (rc != PNG_SUCCESS) {
                Serial.print("Ping Decode returned error: ");
                Serial.print(rc);
                Serial.print(" ");
                switch (rc) {
                    case PNG_INVALID_PARAMETER:
                        Serial.println("PNG_INVALID_PARAMETER");
                        break;
                    case PNG_DECODE_ERROR:
                        Serial.println("PNG_DECODE_ERROR");
                        break;
                    case PNG_MEM_ERROR:
                        Serial.println("PNG_MEM_ERROR");
                        break;
                    case PNG_NO_BUFFER:
                        Serial.println("PNG_NO_BUFFER");
                        break;
                    case PNG_UNSUPPORTED_FEATURE:
                        Serial.println("PNG_UNSUPPORTED_FEATURE");
                        break;
                    case PNG_INVALID_FILE:
                        Serial.println("PNG_INVALID_FILE");
                        break;
                    case PNG_TOO_BIG:
                        Serial.println("PNG_TOO_BIG");
                        break;
                }
            }
            png.close();
            free(usPixels);
        } else
            Serial.println("Error could not allocate line buffer");
    } else {

        Serial.print("Was not a valid PNG file RC:");
        Serial.println(rc, DEC);
    }
}

// Hack simply pass back pointer to file...
void *myOpen(const char *filename, int32_t *size) {
    WrapperFile *pngFile = (WrapperFile *)filename;
    *size = pngFile->size();
    return pngFile;
}

int32_t myReadPNG(PNGFILE *ppngfile, uint8_t *buffer, int32_t length) {
    if (!ppngfile || !ppngfile->fHandle) return 0;
    return ((WrapperFile *)(ppngfile->fHandle))->read(buffer, length);
}

int32_t mySeekPNG(PNGFILE *ppngfile, int32_t position) {
    if (!ppngfile || !ppngfile->fHandle) return 0;
    return ((WrapperFile *)(ppngfile->fHandle))->seek(position);
}

//
// Was PNGRGB8888 from PNG file.

void PNGRGB8888(PNGDRAW *pDraw, uint32_t *pPixels, int iHasAlpha) {
    int x, j;
    uint32_t usPixel, *pDest = pPixels;
    uint8_t c = 0, a, *pPal, *s = pDraw->pPixels;

    switch (pDraw->iPixelType) {
        case PNG_PIXEL_GRAY_ALPHA:
            for (x = 0; x < pDraw->iWidth; x++) {
                c = *s++;  // gray level
                a = *s++;
                j = (a * c) >> 8;  // multiply by the alpha
                usPixel = color888(j, j, j);
                *pDest++ = usPixel;
            }
            break;
        case PNG_PIXEL_GRAYSCALE:
            switch (pDraw->iBpp) {
                case 8:
                    for (x = 0; x < pDraw->iWidth; x++) {
                        c = *s++;
                        usPixel = color888(c, c, c);
                        *pDest++ = usPixel;
                    }
                    break;
                case 1:
                    for (x = 0; x < pDraw->iWidth; x++) {
                        if ((x & 7) == 0) {
                            c = *s++;
                        }
                        if (c & 0x80) {
                            usPixel = 0xffffffff;
                        } else {
                            usPixel = 0;
                        }
                        *pDest++ = usPixel;
                        c <<= 1;
                    }
                    break;
            }  // switch on bpp
            break;
        case PNG_PIXEL_TRUECOLOR:
            for (x = 0; x < pDraw->iWidth; x++) {
                usPixel = color888(s[0], s[1], s[2]);
                *pDest++ = usPixel;
                s += 3;
            }
            break;
        case PNG_PIXEL_INDEXED:                              // palette color (can be 1/2/4 or 8 bits per pixel)
            if (pDraw->pFastPalette && !pDraw->iHasAlpha) {  // faster RGB565 palette exists
                switch (pDraw->iBpp) {
                    case 8:
                        for (x = 0; x < pDraw->iWidth; x++) {
                            c = *s++;
                            usPixel = color565To888(pDraw->pFastPalette[c]);
                            *pDest++ = usPixel;
                        }
                        break;
                    case 4:
                        for (x = 0; x < pDraw->iWidth; x += 2) {
                            c = *s++;
                            usPixel = color565To888(pDraw->pFastPalette[c >> 4]);
                            *pDest++ = usPixel;
                            usPixel = color565To888(pDraw->pFastPalette[c & 0xf]);
                            *pDest++ = usPixel;
                        }
                        break;
                    case 2:
                        for (x = 0; x < pDraw->iWidth; x += 4) {
                            c = *s++;
                            for (j = 0; j < 4; j++) {  // work on pairs of bits
                                usPixel = color565To888(pDraw->pFastPalette[c >> 6]);
                                *pDest++ = usPixel;
                                c <<= 2;
                            }
                        }
                        break;
                    case 1:
                        for (x = 0; x < pDraw->iWidth; x++) {
                            if ((x & 7) == 0) {
                                c = *s++;
                            }
                            usPixel = color565To888(pDraw->pFastPalette[c >> 7]);
                            *pDest++ = usPixel;
                            c <<= 1;
                        }
                        break;
                }  // switch on bpp
                return;
            }
            switch (pDraw->iBpp) {
                case 8:                      // 8-bit palette also supports palette alpha
                    if (pDraw->iHasAlpha) {  // use the alpha to modify the palette
                        for (x = 0; x < pDraw->iWidth; x++) {
                            int a;
                            c = *s++;
                            a = pDraw->pPalette[768 + c];  // get alpha
                            pPal = &pDraw->pPalette[c * 3];
                            usPixel = color888(pPal[0], pPal[1], pPal[2]);
                            *pDest++ = usPixel;
                        }  // for x
                    } else {
                        for (x = 0; x < pDraw->iWidth; x++) {
                            c = *s++;
                            pPal = &pDraw->pPalette[c * 3];
                            usPixel = color888(pPal[0], pPal[1], pPal[2]);
                            *pDest++ = usPixel;
                        }  // for x
                    }      // not alpha palette
                    break;
                case 4:
                    for (x = 0; x < pDraw->iWidth; x += 2) {
                        c = *s++;
                        pPal = &pDraw->pPalette[(c >> 4) * 3];
                        usPixel = color888(pPal[0], pPal[1], pPal[2]);
                        *pDest++ = usPixel;
                        pPal = &pDraw->pPalette[(c & 0xf) * 3];
                        usPixel = color888(pPal[0], pPal[1], pPal[2]);
                        *pDest++ = usPixel;
                    }
                    break;
                case 2:
                    for (x = 0; x < pDraw->iWidth; x += 4) {
                        c = *s++;
                        for (j = 0; j < 4; j++) {  // work on pairs of bits
                            pPal = &pDraw->pPalette[(c >> 6) * 3];
                            usPixel = color888(pPal[0], pPal[1], pPal[2]);
                            *pDest++ = usPixel;
                            c <<= 2;
                        }
                    }
                    break;
                case 1:
                    for (x = 0; x < pDraw->iWidth; x++) {
                        if ((x & 7) == 0) {
                            c = *s++;
                        }
                        pPal = &pDraw->pPalette[(c >> 7) * 3];
                        usPixel = color888(pPal[0], pPal[1], pPal[2]);
                        *pDest++ = usPixel;
                        c <<= 1;
                    }
                    break;
            }  // switch on bits per pixel
            break;
        case PNG_PIXEL_TRUECOLOR_ALPHA:  // truecolor + alpha
            for (x = 0; x < pDraw->iWidth; x++) {
                usPixel = color888(s[0], s[1], s[2]);
                *pDest++ = usPixel;
                s += 4;  // skip alpha
            }
            break;
    }
}

// Function to draw pixels to the display
void PNGDraw(PNGDRAW *pDraw) {
    // Serial.print(".");
    uint32_t *usPixels = (uint32_t *)pDraw->pUser;
    if (g_image_scale == 1) {
        uint32_t *pusRow =
            usPixels + pDraw->iWidth * (pDraw->y & 0xf);  // we have 16 lines to work with

        PNGRGB8888(pDraw, pusRow, false);
        // but we will output 8 lines at time.
        if ((pDraw->y == g_image_height - 1) || ((pDraw->y & 0x7) == 0x7)) {
            //      WaitforWRComplete(); // make sure previous writes are done
            writeClippedRect24(0, pDraw->y & 0xfff8, pDraw->iWidth,
                               (pDraw->y & 0x7) + 1,
                               usPixels + (pDraw->y & 0x8) * pDraw->iWidth);
        }
    } else {
        uint32_t *pusRow = usPixels + pDraw->iWidth * (pDraw->y % g_image_scale);
        PNGRGB8888(pDraw, pusRow, false);
        ScaleDownwriteClippedRect24(pDraw->y, pDraw->iWidth, pusRow);
    }
    g_WRCount++;
}

#endif

//=============================================================================
// Touch screen support
//=============================================================================
#if 0
void ProcessTouchScreen() {
  // See if there's any  touch data for us
  //  if (ts.bufferEmpty()) {
  //    return;
  //  }
  // You can also wait for a touch
  if (!ts.touched()) {
    g_fast_mode = false;
    return;
  }

  // first hack, if screen pressed go very fast
  g_fast_mode = true;

  // Retrieve a point
  TS_Point p = ts.getPoint();

  // p is in ILI9341_t3 setOrientation 1 settings. so we need to map x and y differently.

  Serial.print("X = ");
  Serial.print(p.x);
  Serial.print("\tY = ");
  Serial.print(p.y);
  Serial.print("\tPressure = ");
  Serial.print(p.z);


  // Scale from ~0->4000 to pgfx->width using the calibration #'s
#if 1  // SCREEN_ORIENTATION_1
  p.x = map(p.x, TS_MINX, TS_MAXX, 0, g_tft_width);
  p.y = map(p.y, TS_MINY, TS_MAXY, 0, g_tft_height);
#else

    uint16_t px = map(p.y, TS_MAXY, TS_MINY, 0, g_tft_width);
    p.y = map(p.x, TS_MINX, TS_MAXX, 0, g_tft_height);
    p.x = px;
#endif
  Serial.print(" (");
  Serial.print(p.x);
  Serial.print(", ");
  Serial.print(p.y);
  Serial.println(")");
}
#endif
#if 0
char g_path[512];  // should not get this long
void listFiles() {
  strcpy(g_path, "/usb/");
#ifdef LATER
  Serial.print("\n Space Used = ");
  Serial.println(pfs->usedSize());
  Serial.print("Filesystem Size = ");
  Serial.println(pfs->totalSize());
#endif
  printDirectory(0);  // we are at the root_SD directory.
}

void printDirectory(int numSpaces) {
  DIR *d = opendir(g_path);
  if (!d) return;
  struct dirent *dir_entry;
  if (numSpaces > 0) strcat(g_path, "/");  // if not root_SD append /
  int path_len = strlen(g_path);
  while ((dir_entry = readdir(d)) != nullptr) {
    printSpaces(numSpaces);
    Serial.print(dir_entry->d_name);
    if (dir_entry->d_type == DT_DIR) {
      Serial.println("/");
      strcpy(&g_path[path_len], dir_entry->d_name);
      printDirectory(numSpaces + 2);
    } else {
      // files have sizes, directories do not
      Serial.println();
    }
  }
  closedir(d);
}

void printSpaces(int num) {
  for (int i = 0; i < num; i++) {
    Serial.print(" ");
  }
}
//#else
  void listFiles() {
    Serial.print("\n Space Used = ");
    uint64_t used_size = (SD.clusterCount() - SD.freeClusterCount())
                         * (uint64_t)SD.bytesPerCluster();

    Serial.println(used_size, DEC);
    Serial.print("Filesystem Size = ");
    uint64_t total_size = SD.clusterCount() * SD.bytesPerCluster();
    Serial.println(total_size);

    Serial.println("Directory\n---------");
    printDirectory(SD.open("/"), 0);
    Serial.println();
  }


  void printDirectory(FsFile dir, int numSpaces) {
    char file_name[MAX_FILENAME_LEN];
    while (true) {
      FsFile entry = dir.openNextFile();
      if (!entry) {
        //Serial.println("** no more files **");
        break;
      }
      printSpaces(numSpaces);
      size_t name_len = entry.getName(file_name, sizeof(file_name));
      Serial.print(file_name);
      if (entry.isDirectory()) {
        Serial.println("/");
        printDirectory(entry, numSpaces + 2);
      } else {
        // files have sizes, directories do not
        printSpaces(36 - numSpaces - name_len);
        Serial.print("  ");
        Serial.println(entry.size(), DEC);
      }
      entry.close();
    }
  }

  void printSpaces(int num) {
    for (int i = 0; i < num; i++) {
      Serial.print(" ");
    }
  }
#else
void printSpaces(int num) {
    for (int i = 0; i < num; i++) {
        Serial.print(" ");
    }
}

void listFiles() {
    Serial.println("\n*** List Files ***");
    if (g_current_device == SD_DRIVE) {
        FsFile root_file;
        root_file = SD.open("/");
        if (root_file) printDirectory(&root_file, 0);
    }
    Serial.println("*** Completed ***");
}
void printDirectory(FsFile *dir, int numSpaces) {
    while (true) {
        FsFile entry = dir->openNextFile();
        if (!entry) {
            //Serial.println("** no more files **");
            break;
        }
        printSpaces(numSpaces);
        char file_name[40];
        int cbName = entry.getName(file_name, sizeof(file_name));
        Serial.print(file_name);
        if (entry.isDirectory()) {
            Serial.println("/");
            printDirectory(&entry, numSpaces + 2);
        } else {
            // files have sizes, directories do not
            printSpaces(36 - numSpaces - cbName);
            Serial.print("  ");
            Serial.println(entry.size(), DEC);
        }
        entry.close();
    }
}

#endif
