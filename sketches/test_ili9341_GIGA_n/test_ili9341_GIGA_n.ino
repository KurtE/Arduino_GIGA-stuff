#include <ILI9341_GIGA_n.h>
#include <GIGA_digitalWriteFast.h>
#include <elapsedMillis.h>
#include <LibPrintf.h>
#include <RPC.h>

#define TFT_DC 9
#define TFT_RST 8
#define TFT_CS 10

#define DEBUG_PIN 3

ILI9341_GIGA_n tft(&SPI1, TFT_CS, TFT_DC, TFT_RST);
Stream *USERIAL = nullptr;

void setup() {
  // put your setup code here, to run once:
  pinMode(DEBUG_PIN, OUTPUT);
  if (HAL_GetCurrentCPUID() == CM7_CPUID) {
    Serial.begin(115200);
    while (!Serial && millis() < 5000) {};
    USERIAL = &Serial;
    USERIAL->println("ILI9341 Test! <CM7>");
  } else {
    RPC.begin();
    USERIAL = &RPC;
    USERIAL->println("ILI9341 Test! <CM4>");
    printf_init(RPC);
  }

  delay(50);
  tft.begin(20000000);
  USERIAL->println("after tft.begin");
  printSPIRegisters();
  delay(50);

  Serial1.begin(1000000);
}

void printSPIRegisters() {
  if (tft._pgigaSpi) {
    printf("\nHardware SPI registers\n");
    printf("\tCR1:  %lx\n", tft._pgigaSpi->CR1);         /*!< SPI/I2S Control register 1,                      Address offset: 0x00 */
    printf("\tCR2:  %lx\n", tft._pgigaSpi->CR2);         /*!< SPI Control register 2,                          Address offset: 0x04 */
    printf("\tCFG1:  %lx\n", tft._pgigaSpi->CFG1);       /*!< SPI Configuration register 1,                    Address offset: 0x08 */
    printf("\tCFG2:  %lx\n", tft._pgigaSpi->CFG2);       /*!< SPI Configuration register 2,                    Address offset: 0x0C */
    printf("\tIER:  %lx\n", tft._pgigaSpi->IER);         /*!< SPI/I2S Interrupt Enable register,               Address offset: 0x10 */
    printf("\tSR:  %lx\n", tft._pgigaSpi->SR);           /*!< SPI/I2S Status register,                         Address offset: 0x14 */
    printf("\tIFCR:  %lx\n", tft._pgigaSpi->IFCR);       /*!< SPI/I2S Interrupt/Status flags clear register,   Address offset: 0x18 */
    printf("\tTXDR:  %lx\n", tft._pgigaSpi->TXDR);       /*!< SPI/I2S Transmit data register,                  Address offset: 0x20 */
    printf("\tRXDR:  %lx\n", tft._pgigaSpi->RXDR);       /*!< SPI/I2S Receive data register,                   Address offset: 0x30 */
    printf("\tCRCPOLY:  %lx\n", tft._pgigaSpi->CRCPOLY); /*!< SPI CRC Polynomial register,                     Address offset: 0x40 */
    printf("\tTXCRC:  %lx\n", tft._pgigaSpi->TXCRC);     /*!< SPI Transmitter CRC register,                    Address offset: 0x44 */
    printf("\tRXCRC:  %lx\n", tft._pgigaSpi->RXCRC);     /*!< SPI Receiver CRC register,                       Address offset: 0x48 */
    printf("\tUDRDR:  %lx\n", tft._pgigaSpi->UDRDR);     /*!< SPI Underrun data register,                      Address offset: 0x4C */
    printf("\tI2SCFGR:  %lx\n", tft._pgigaSpi->I2SCFGR); /*!< I2S Configuration register,                      Address offset: 0x50 */
  }
}

const static uint16_t colors[] = { ILI9341_BLACK, ILI9341_RED, ILI9341_WHITE, ILI9341_GREEN, ILI9341_BLUE };
#define COUNT_COLORS (sizeof(colors) / sizeof(colors[0]))
bool use_frame_buffer = false;
void loop() {
  // put your main code here, to run repeatedly:
  tft.useFrameBuffer(use_frame_buffer);
  if (use_frame_buffer) USERIAL->println("Using Frame Buffer");
  else USERIAL->println("Not useing frame buffer");
  for (int i = 0; i < (sizeof(colors) / sizeof(colors[0])); i++) {
    if (use_frame_buffer)Serial1.write('f');
    Serial1.print(i, DEC);
    digitalWriteFast(DEBUG_PIN, HIGH);
    tft.fillScreen(colors[i]);
    if (use_frame_buffer) tft.updateScreen();
    digitalWriteFast(DEBUG_PIN, LOW);
    pauseMS(2000);
  }
  for (int i = 0; i < COUNT_COLORS; i++) {
    if (use_frame_buffer)Serial1.write('f');
    Serial1.write('a'+i);
    digitalWriteFast(DEBUG_PIN, HIGH);
    tft.fillRect(i * 20, i * 30, tft.width() - i * 40, tft.height() - i * 60, colors[i]);
    if (use_frame_buffer) tft.updateScreen();
    digitalWriteFast(DEBUG_PIN, LOW);
    pauseMS(1000);
  }
  for (int i = 0; i < COUNT_COLORS; i++) {
    if (!use_frame_buffer) digitalWriteFast(DEBUG_PIN, HIGH);
    if (!use_frame_buffer) Serial1.write('A'+i);
    tft.fillRect(i * 20, i * 30, tft.width() - i * 40, tft.height() - i * 60, colors[COUNT_COLORS - (i + 1)]);
    if (!use_frame_buffer) digitalWriteFast(DEBUG_PIN, HIGH);
    if (!use_frame_buffer) pauseMS(250);
  }
  if (use_frame_buffer) {
    Serial1.print("FX");
    digitalWriteFast(DEBUG_PIN, HIGH);
    tft.updateScreen();
    digitalWriteFast(DEBUG_PIN, LOW);
  }
  pauseMS(2000);

  use_frame_buffer = (use_frame_buffer) ? false : true;
}

void pauseMS(uint32_t ms) {
  elapsedMillis em;
  while (em < ms) {
    if (USERIAL->available()) {
      digitalWriteFast(DEBUG_PIN, HIGH);
      digitalWriteFast(DEBUG_PIN, LOW);
      digitalWriteFast(DEBUG_PIN, HIGH);
      digitalWriteFast(DEBUG_PIN, LOW);
      int ch = USERIAL->read();

      if (ch == 'd') {
        printSPIRegisters();
        ch = USERIAL->read();
      }
      if (ch == 'p') {
        USERIAL->println("Paused");
        while (USERIAL->read() != -1) {}
        while (USERIAL->read() == -1) {}
      }
      while (USERIAL->read() != -1) {}
    }
  }
}
