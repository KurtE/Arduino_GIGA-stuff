#include <MemoryHexDump.h>

#include <GIGA_digitalWriteFast.h>

#include <ILI9341_GIGA_n.h>
#include <ili9341_GIGA_n_font_Arial.h>
#include <ili9341_GIGA_n_font_ArialBold.h>
#include <elapsedMillis.h>
#include "DMADefines.h"
#define BLACK ILI9341_BLACK
#define WHITE ILI9341_WHITE
#define YELLOW ILI9341_YELLOW
#define GREEN ILI9341_GREEN
#define RED ILI9341_RED

#define LIGHTGREY 0xC618 /* 192, 192, 192 */
#define DARKGREY 0x7BEF  /* 128, 128, 128 */
#define BLUE 0x001F      /*   0,   0, 255 */

#define TFT_DC 9
#define TFT_RST 8
#define TFT_CS 7

uint16_t colors[] = { 0, ILI9341_RED, ILI9341_BLUE, ILI9341_GREEN, ILI9341_WHITE, ILI9341_YELLOW, ILI9341_BLACK };
const char *color_names[] = { "Stripes", "Red", "Blue", "Green", "White", "Yellow", "Black" };
#define CNT_COLORS (sizeof(colors) / sizeof(colors[0]))

ILI9341_GIGA_n tft(&SPI1, TFT_CS, TFT_DC, TFT_RST);

uint16_t frame_buffer[320 * 240] __attribute__((aligned(16)));
uint16_t dummy_read_buffer[1] __attribute__((aligned(16)));


void setup() {
  Serial1.begin(2000000);
  while (!Serial && millis() < 3000)
    ;  // wait for Arduino Serial Monitor
  Serial.println("\nTFT DMA Test");
  tft.begin(20000000);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  for (uint8_t i = 0; i < CNT_COLORS; i++) {
    if (i == 0) draw_stripes();
    else tft.fillScreen(colors[i]);
    delay(500);
  }

  tft.setFrameBuffer(frame_buffer);
  tft.useFrameBuffer(true);

  for (uint8_t i = 0; i < CNT_COLORS; i++) {
    if (i == 0) draw_stripes();
    else tft.fillScreen(colors[i]);
    tft.updateScreen();
    delay(500);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  static uint8_t color_index = 0;
  static bool do_async = true;
  if (color_index == 0) draw_stripes();
  else tft.fillScreen(colors[color_index]);
  Serial.print("Hit enter to try next color ");
  Serial.print(color_names[color_index]);
  color_index++;
  if (color_index >= CNT_COLORS) color_index = 0;
  if (do_async) Serial.println(" Async");
  else Serial.println(" Normal");

  while (Serial.read() == -1) {}
  while (Serial.read() != -1) {}

  if (do_async) {
    updateScreenAsync();
  } else {
    tft.updateScreen();
  }
  do_async = !do_async;
}

void draw_stripes() {
  uint8_t color_index = 1;
  for (uint16_t x = 0; x < tft.width(); x++) {
    tft.drawLine(x, 0, x, tft.height(), colors[color_index]);
    color_index++;
    if (color_index >= CNT_COLORS) color_index = 1;
  }
}

// try dma stuff
enum { DMA_STATE_CLEAR = 0,
       DMA_STATE_FIRST_TRANSFER,
       DMA_STATE_SECOND_TRANSFER,
       DMA_STATE_COMPLETE };
volatile uint8_t dma_transfer_state = DMA_STATE_CLEAR;

volatile uint32_t txIRQCount = 0;
uint32_t M0AR_at_irq[2];

void updateScreenAsync() {
  txIRQCount = 0;
  initialSetupDMA();
  StartDMATransfer();

  // wait for transmit to complete
  Serial.print(">>>> TxIRQCount: ");
  Serial.println(txIRQCount, DEC);
  MemoryHexDump(Serial, frame_buffer, sizeof(frame_buffer), true, "Frame Buffer\n", 40);
  dump_dma_and_spi_settings(tft._pgigaSpi);
  elapsedMillis em;
  while ((dma_transfer_state != DMA_STATE_COMPLETE) && (em < 2000)) {
  }
  Serial.print(">>>> TxIRQCount: ");
  Serial.println(txIRQCount, DEC);
  Serial.print("M0AR deltas:");
  Serial.println(M0AR_at_irq[1] - M0AR_at_irq[0]);

  if (dma_transfer_state != DMA_STATE_COMPLETE) {
    Serial.println("*** DMA transfer timed out ***");
    dump_dma_and_spi_settings(tft._pgigaSpi);
    tft.endSPITransaction();
    restoreFromDMA();
  }
  dma_transfer_state = DMA_STATE_CLEAR;
}

void initialSetupDMA() {
  static bool dma_has_been_init = false;
  if (dma_has_been_init) return;
  dma_has_been_init = true;

  // Enable DMA1
  SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMA1EN_Msk);
  delay(1000);

  DMA1_Stream1->M0AR = (uint32_t)frame_buffer;
  DMA1_Stream1->PAR = (uint32_t)&tft._pgigaSpi->TXDR;


  uint32_t cr = 0;
  cr |= 1 << DMA_SxCR_DIR_Pos;      // Set Memory to Peripheral
  cr |= (2 << DMA_SxCR_MSIZE_Pos);  // try 32 set 16 bit mode
  cr |= DMA_SxCR_MINC;              // Memory Increment
  cr |= 1 << DMA_SxCR_PSIZE_Pos;    // Peripheral size 16 bits
  //cr |= DMA_SxCR_PFCTRL;            // Peripheral in control of flow contrl
  cr |= DMA_SxCR_TCIE;            // interrupt on completion
  cr |= DMA_SxCR_TEIE;            // Interrupt on error.
  cr |= (3u << DMA_SxCR_PL_Pos);  // Very high priority
  cr |= (0x1 << DMA_SxCR_MBURST_Pos); // Incr 4...
  DMA1_Stream1->CR = cr;

  // Experiment with FIFO not direct
  DMA1_Stream1->FCR |= (0x3 << DMA_SxFCR_FTH_Pos) | DMA_SxFCR_DMDIS;  // disable Direct mode
  DMA1_Stream1->FCR |= DMA_SxFCR_FEIE;  // Enable interrupt on FIFO error

  /******************MEM Address to buffer**************/
  /*******************Number of data transfer***********/
  DMA1_Stream1->NDTR = 38400;  // (32*240/2);  //0 - 65536

  /******************Periph request**********************/
  // Point to our SPI
  DMAMUX1_Channel1->CCR = (DMAMUX1_Channel1->CCR & ~(DMAMUX_CxCR_DMAREQ_ID_Msk))
                          | (DMAMUX1_CxCR_DMAREQ_ID::SPI5_TX_DMA << DMAMUX_CxCR_DMAREQ_ID_Pos);

  // transfer complete and error interupt
  DMA1->LIFCR = DMA_LIFCR_CTCIF1;  //Clear IT in LISR Register
  NVIC_SetVector(DMA1_Stream1_IRQn, (uint32_t)&DMA1_Stream1_IRQHandler);
  NVIC_EnableIRQ(DMA1_Stream1_IRQn);

  // configure FIFO?
}

void restoreFromDMA() {
  CLEAR_BIT(tft._pgigaSpi->CFG1, SPI_CFG1_RXDMAEN);  // disable SPI RX
  CLEAR_BIT(tft._pgigaSpi->CFG1, SPI_CFG1_TXDMAEN);  // disable SPI TX

  setSPIDataSize(8);  // restore back to 8 bits
}

// TX handler
void DMA1_Stream1_IRQHandler(void) {
  txIRQCount++;
  digitalToggleFast(LED_BLUE);
  if (DMA1->LISR & DMA_LISR_TCIF1) {
    DMA1->LIFCR = DMA_LIFCR_CTCIF1;
    if (dma_transfer_state == DMA_STATE_FIRST_TRANSFER) {
      dma_transfer_state = DMA_STATE_SECOND_TRANSFER;
      M0AR_at_irq[0] = (uint32_t)DMA1_Stream1->M0AR;
      DMA1_Stream1->M0AR = (uint32_t)(&frame_buffer[38400]);
      DMA1_Stream1->NDTR = 38400;                      // (32*240/2);  //0 - 65536
      SET_BIT(tft._pgigaSpi->CFG1, SPI_CFG1_RXDMAEN);  // enable SPI RX

      // enable the two streams
      SET_BIT(DMA1_Stream1->CR, DMA_SxCR_EN_Msk);
      SET_BIT(tft._pgigaSpi->CR1, SPI_CR1_SPE);  // enable SPI


      // Enable TX
      SET_BIT(tft._pgigaSpi->CFG1, SPI_CFG1_TXDMAEN);  // enable SPI TX
    } else if (dma_transfer_state == DMA_STATE_SECOND_TRANSFER) {
      M0AR_at_irq[1] = (uint32_t)DMA1_Stream1->M0AR;
      dma_transfer_state = DMA_STATE_COMPLETE;  // tell caller we are done.
      tft.endSPITransaction();

      CLEAR_BIT(tft._pgigaSpi->CR1, SPI_CR1_SPE);        // disable SPI
      CLEAR_BIT(tft._pgigaSpi->CFG1, SPI_CFG1_RXDMAEN);  // disable SPI RX
      CLEAR_BIT(tft._pgigaSpi->CFG1, SPI_CFG1_TXDMAEN);  // disable SPI TX

      tft._pgigaSpi->CFG1 = (tft._pgigaSpi->CFG1 & ~(SPI_CFG1_DSIZE_Msk | SPI_CFG1_CRCSIZE_Msk))
                            | (7 << SPI_CFG1_DSIZE_Pos) | (7 << SPI_CFG1_CRCSIZE_Pos);
      tft._pgigaSpi->CFG1 = (tft._pgigaSpi->CFG1 & ~(SPI_CFG1_FTHLV_Msk))
                            | (7 << SPI_CFG1_FTHLV_Pos);
      SET_BIT(tft._pgigaSpi->CR1, SPI_CR1_SPE);  // enable SPI

    } else {
      // ??? error
    }
  }
  if (DMA1->LISR & DMA_LISR_TEIF1) {
    digitalToggleFast(LED_RED);
    DMA1->LIFCR = DMA_LIFCR_CTEIF1;
  }
  if (DMA1->LISR & DMA_LISR_FEIF1) {
    digitalToggleFast(LED_RED);
    DMA1->LIFCR = DMA_LIFCR_CFEIF1;
  }
}


void setSPIDataSize(uint8_t datasize) {

  datasize--;  // decrement by 1
  // lets disable SPI and set to 16 bit mode
  CLEAR_BIT(tft._pgigaSpi->CR1, SPI_CR1_SPE);  // disable SPI
  tft._pgigaSpi->CFG1 = (tft._pgigaSpi->CFG1 & ~(SPI_CFG1_DSIZE_Msk | SPI_CFG1_CRCSIZE_Msk))
                        | (datasize << SPI_CFG1_DSIZE_Pos) | (datasize << SPI_CFG1_CRCSIZE_Pos);
  tft._pgigaSpi->CFG1 = (tft._pgigaSpi->CFG1 & ~(SPI_CFG1_FTHLV_Msk))
                        | (1 << SPI_CFG1_FTHLV_Pos);

  SET_BIT(tft._pgigaSpi->CR1, SPI_CR1_SPE);  // enable SPI
}

void StartDMATransfer() {
  // reset the buffers.
  DMA1_Stream1->M0AR = (uint32_t)frame_buffer;
  DMA1_Stream1->NDTR = 38400;  // (32*240/2);  //0 - 65536

  // Lets setup the transfer ... everything before the fill screen.
  tft.beginSPITransaction(tft._SPI_CLOCK);
  // Doing full window.
  tft.setAddr(0, 0, tft.width() - 1, tft.height() - 1);
  tft.writecommand_cont(ILI9341_RAMWR);
  tft.setDataMode();

  setSPIDataSize(16);

  // enable RX
  SET_BIT(DMA1_Stream1->CR, DMA_SxCR_EN_Msk);

  // Enable TX
  SET_BIT(tft._pgigaSpi->CFG1, SPI_CFG1_TXDMAEN);  // enable SPI TX
  // finally enable SPI
  dma_transfer_state = DMA_STATE_FIRST_TRANSFER;
  SET_BIT(tft._pgigaSpi->CR1, SPI_CR1_SPE);     // enable SPI
  SET_BIT(tft._pgigaSpi->CR1, SPI_CR1_CSTART);  // enable SPI
}
