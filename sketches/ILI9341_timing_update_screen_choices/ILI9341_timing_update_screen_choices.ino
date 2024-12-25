

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

class wrapped_SPI : public arduino::ZephyrSPI {
  public:
    const struct device *SPIDevice() {return spi_dev;}
    struct spi_config *getConfig()  {return &config;}
    struct spi_config *getConfig16() {return &config16;}
};


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
    Serial.print("PSPI: 0x");
    Serial.println((uint32_t)p, HEX);
    spi_dev = (const struct device *)p[1];
    memset((void *)&config16, 0, sizeof(config16));
    config16.frequency = TFT_SPEED;
    config16.operation = SPI_WORD_SET(16) | SPI_TRANSFER_MSB;
    Serial.print("Get zspi and config by hack: 0x");
    Serial.print((uint32_t)spi_dev, HEX);
    Serial.print(" 0x");
    Serial.println((uint32_t)&p[2], HEX);

    Serial.print("Try SubClass: 0x");
    wrapped_SPI *pwspi = (wrapped_SPI*)&TFT_SPI;
    Serial.print((uint32_t)pwspi->SPIDevice(), HEX);
    Serial.print(" 0x");
    Serial.print((uint32_t)pwspi->getConfig(), HEX);
    Serial.print(" 0x");
    Serial.println((uint32_t)pwspi->getConfig16(), HEX);


    tx_buf.buf = pframeBuffer = tft.getFrameBuffer();
}

//****************************************************************************
// loop
//****************************************************************************
uint8_t update_mode = 0;

volatile bool spi_async_active = false;

void spi_cb(const struct device *dev, int result, void *data) {
  spi_async_active = false;
}

void loop() {
    switch (update_mode) {
        case 0: Serial.print("updateScreen: "); break;
        case 1: Serial.print("SPI.transfer: "); break;
        case 2: Serial.print("SPI.transfer16: "); break;
        case 3: Serial.print("Zephyr: "); break;
        case 4: Serial.print("Zephyr Async: "); break;
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
    if (update_mode > 4) {
        update_mode = 0;
        Serial.println();
    }
    delay(250);
    if (Serial.available()) {
      while (Serial.read() != -1);
      Serial.println("Paused");
      while (Serial.read() == -1);
      while (Serial.read() != -1);

    }
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
        } else if (update_mode == 3) {
            spi_transceive(spi_dev, &config16, &tx_buf_set, nullptr);
        } else {
            spi_async_active = true;
            int iRet;
            if ((iRet = spi_transceive_cb(spi_dev, &config16, &tx_buf_set, nullptr, &spi_cb, &tft)) < 0) {
              Serial.print("spi_transceiveCB failed:");
              Serial.println(iRet);
            } else {
              while (spi_async_active) {}
            }
        }
        tft.endSPITransaction();
    }
}
