

//=============================================================================
// Simple image (BMP optional JPEG and PNG) display program, which if the
// sketch is built with one of the USB Types which include MTP support
//=============================================================================
#include <SPI.h>

//-----------------------------------------------------------------------------
// ILI9341 displays
//-----------------------------------------------------------------------------
#include "ILI9341_GIGA_zephyr.h"
#include <elapsedMillis.h>
#define CS_SD 6
#define TFT_DC 9
#define TFT_RST 8
#define TFT_CS 10
ILI9341_GIGA_n tft(TFT_CS, TFT_DC, TFT_RST);
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
const struct device *spi_dev = nullptr;
struct spi_config config16;
struct spi_buf tx_buf = { .buf = nullptr, .len = 320 * 240 * 2 };
const struct spi_buf_set tx_buf_set = { .buffers = &tx_buf, .count = 1 };
uint16_t *pframeBuffer;

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

    tft.fillScreen(RED);
    delay(500);
    tft.fillScreen(GREEN);
    delay(500);
    tft.fillScreen(BLUE);
    delay(500);

    tft.useFrameBuffer(true);

    // setup to use zephyr spi
    uint32_t *p = (uint32_t *)&TFT_SPI;
    spi_dev = (const struct device *)p[1];
    memset((void *)&config16, 0, sizeof(config16));
    config16.frequency = TFT_SPEED;
    config16.operation = SPI_WORD_SET(16) | SPI_TRANSFER_MSB;
    tx_buf.buf = pframeBuffer = tft.getFrameBuffer();
}

//****************************************************************************
// loop
//****************************************************************************
uint8_t update_mode = 0;


void loop() {
    switch (update_mode) {
        case 0: Serial.print("updateScreen: "); break;
        case 1: Serial.print("SPI.transfer: "); break;
        case 2: Serial.print("SPI.transfer16: "); break;
        case 3: Serial.print("Zephyr: "); break;
    }
    elapsedMillis em;
    tft.fillScreen(BLUE);
    UpdateScreen();
    tft.fillScreen(BLACK);
    UpdateScreen();
    tft.fillScreen(WHITE);
    UpdateScreen();
    tft.fillScreen(GREEN);
    UpdateScreen();
    tft.fillScreen(RED);
    UpdateScreen();
    Serial.println((uint32_t)em);
    update_mode++;
    if (update_mode > 3) {
        update_mode = 0;
        Serial.println();
    }
    delay(250);
}



void UpdateScreen() {
    if (update_mode == 0) {
        tft.updateScreen();
    } else {
        tft.beginSPITransaction(TFT_SPEED);
        tft.setAddr(0, 0, tft.width() - 1, tft.height() - 1);
        tft.writecommand_cont(ILI9341_RAMWR);
        tft.setDataMode();
        uint32_t frame_size = tft.width() * tft.height();
        if (update_mode == 1) {
            for (uint32_t i = 0; i < frame_size; i++) {
                uint16_t color = pframeBuffer[i];
                TFT_SPI.transfer(color >> 8);
                TFT_SPI.transfer(color & 0xff);
            }
        } else if (update_mode == 2) {
            for (uint32_t i = 0; i < frame_size; i++) {
                TFT_SPI.transfer16(pframeBuffer[i]);
            }
        } else {
            spi_transceive(spi_dev, &config16, &tx_buf_set, nullptr);
        }
        tft.endSPITransaction();
    }
}
