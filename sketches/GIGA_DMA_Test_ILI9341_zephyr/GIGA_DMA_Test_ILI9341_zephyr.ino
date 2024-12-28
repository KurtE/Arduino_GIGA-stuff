#include <MemoryHexDump.h>
#include "SDRAM.h"

#include <ILI9341_GIGA_zephyr.h>
//#include <ili9341_GIGA_n_font_Arial.h>
//#include <ili9341_GIGA_n_font_ArialBold.h>
#include <elapsedMillis.h>
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
#define TFT_CS 10

#define LED_RED (LED_BUILTIN + 0)
#define LED_GREEN (LED_BUILTIN + 1)
#define LED_BLUE (LED_BUILTIN + 2)


uint16_t colors[] = { 0, ILI9341_RED, ILI9341_BLUE, ILI9341_GREEN, ILI9341_WHITE, ILI9341_YELLOW, ILI9341_BLACK };
const char *color_names[] = { "Stripes", "Red", "Blue", "Green", "White", "Yellow", "Black" };
#define CNT_COLORS (sizeof(colors) / sizeof(colors[0]))

ILI9341_GIGA_n tft(&SPI1, TFT_CS, TFT_DC, TFT_RST);

//uint16_t frame_buffer[320 * 240] __attribute__((aligned(16)));
uint16_t dummy_read_buffer[1] __attribute__((aligned(16)));


inline void digitalToggleFast(uint8_t pin) {
    digitalWrite(pin, !digitalRead(pin));
}

extern void draw_stripes();
extern void UpdateScreenAsync();

void setup() {
    Serial1.begin(2000000);
    while (!Serial && millis() < 3000)
        ;  // wait for Arduino Serial Monitor
    Serial.println("\nTFT DMA Test");
    Serial.print("Address TFT: 0x");
    Serial.println((uint32_t)&tft, HEX);
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

    uint16_t *pframe_buffer = (uint16_t*)SDRAM.malloc(320*240*2);
    Serial.print("Frame Buffer address: 0x");
    Serial.println((uint32_t)pframe_buffer);
    tft.setFrameBuffer(pframe_buffer);
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
        UpdateScreenAsync();
    } else {
        tft.updateScreen();
    }
    do_async = !do_async;
}

void draw_stripes() {
    uint8_t color_index = 1;
    for (uint16_t x = 0; x < tft.width(); x+=8) {
        tft.fillRect(x, 0, 8, tft.height(), colors[color_index]);
        color_index++;
        if (color_index >= CNT_COLORS) color_index = 1;
    }
}

void UpdateScreenAsync() {
    if (!tft.updateScreenAsync()) {
        Serial.println("Failed to updateScreen Async");
    } else {
        tft.waitUpdateAsyncComplete();
    }
}
