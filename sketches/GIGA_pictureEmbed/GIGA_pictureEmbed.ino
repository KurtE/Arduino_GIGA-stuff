// ILI9341 example with embedded color bitmaps in sketch.
// WILL NOT FIT ON ARDUINO UNO OR OTHER AVR BOARDS;
// uses large bitmap image stored in array!

// Options for converting images to the format used here include:
//   http://www.rinkydinkelectronics.com/t_imageconverter565.php
// or
//  GIMP (https://www.gimp.org/) as follows:
//    1. File -> Export As
//    2. In Export Image dialog, use 'C source code (*.c)' as filetype.
//    3. Press export to get the export options dialog.
//    4. Type the desired variable name into the 'prefixed name' box.
//    5. Uncheck 'GLIB types (guint8*)'
//    6. Check 'Save as RGB565 (16-bit)'
//    7. Press export to save your image.
//  Assuming 'image_name' was typed in the 'prefixed name' box of step 4,
//  you can have to include the c file, then using the image can be done with:
//    tft.drawRGBBitmap(0, 0, image_name.pixel_data, image_name.width, image_name.height);
//  See also https://forum.pjrc.com/threads/35575-Export-for-ILI9341_t3-with-GIMP

#include "Arduino_GigaDisplay_GFX.h"
#include "dragon.h"

// For the Adafruit shield, these are the default.
//#define TFT_DC 9
//#define TFT_CS 10


GigaDisplay_GFX tft;
#define BLUE 0x001F
#define BLACK 0x0000
#define WHITE 0xFFFF
#define GREEN 0x07E0
#define RED 0xf800


void setup() {
  tft.begin();
}

void loop(void) {
  for(uint8_t r=0; r<4; r++) {
    tft.setRotation(r);
    tft.fillScreen(BLACK);
    for(uint8_t j=0; j<20; j++) {
      tft.drawRGBBitmap(
        random(-DRAGON_WIDTH , tft.width()),
        random(-DRAGON_HEIGHT, tft.height()),
        // Some non-AVR MCU's have a "flat" memory model and don't
        // distinguish between flash and RAM addresses.  In this case,
        // the RAM-resident-optimized drawRGBBitmap in the ILI9341
        // library can be invoked by forcibly type-converting the
        // PROGMEM bitmap pointer to a non-const uint16_t *.
        (uint16_t *)dragonBitmap,
        DRAGON_WIDTH, DRAGON_HEIGHT);
      delay(1); // Allow ESP8266 to handle watchdog & WiFi stuff
    }
    delay(3000);
  }
}
