

//=============================================================================
// Simple image (BMP optional JPEG and PNG) display program, which if the
// sketch is built with one of the USB Types which include MTP support
//=============================================================================
#include <SPI.h>

//-----------------------------------------------------------------------------
// ILI9341 displays
//-----------------------------------------------------------------------------
//#include "ILI9341_GIGA_n.h"
#include <ILI9341_GIGA_zephyr.h>

#include <elapsedMillis.h>
#define CS_SD 6
#define TFT_DC 9
#define TFT_RST 8
#define TFT_CS 10
ILI9341_GIGA_n tft(TFT_CS, TFT_DC, TFT_RST);
//ILI9341_GIGA_n tft(&SPI1, TFT_CS, TFT_DC, TFT_RST);

#define TFT_SPI SPI1
#define TFT_SPEED 20000000u



//-----------------------------------------------------------------------------
// Some common things.
//-----------------------------------------------------------------------------

#define BLUE 0x001F
#define BLACK 0x0000
#define WHITE 0xFFFF
#define GREEN 0x07E0
#define RED 0xf800

//****************************************************************************
// Setup
//****************************************************************************
extern void UpdateScreen();

void setup(void) {
    // Keep the SD card inactive while working the display.
    delay(20);

    Serial.begin(115200);
    while (!Serial && millis() < 3000)
        ;
    // give chance to debug some display startups...

    //-----------------------------------------------------------------------------
    // initialize display
    //-----------------------------------------------------------------------------

    pinMode(TFT_CS, OUTPUT);
    pinMode(CS_SD, OUTPUT);
    digitalWrite(TFT_CS, HIGH);

    Serial.println("*** start up ILI9341 ***");
    tft.setSPI(TFT_SPI);  // temporary...
    tft.begin(TFT_SPEED);
    tft.setRotation(1);
}
void loop()
{
    tft.fillScreen(RED);
    delay(500);
    tft.fillScreen(GREEN);
    delay(500);
    tft.fillScreen(BLUE);
    delay(500);
}
