/***************************************************
  This is our GFX example for the Adafruit ILI9341 Breakout and Shield
  ----> http://www.adafruit.com/products/1651

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/

#include <RPC.h>
#include <GIGA_digitalWriteFast.h>

#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

// For the Adafruit shield, these are the default.
#define TFT_DC 9
#define TFT_RST 8
#define TFT_CS 10

//#define USERIAL Serial
Stream *USERIAL = nullptr;
#define LED_PIN LED_BUILTIN

#define TIMING_PIN 2

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
//Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
Adafruit_ILI9341 tft = Adafruit_ILI9341(&SPI1, TFT_DC, TFT_CS, TFT_RST);
// If using the breakout, change pins as desired
//Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

typedef uint32_t (*pfnucr32)();

pfnucr32 getMicros;

void setup() {
  //  USERIAL->begin(9600);
  //  while (!Serial && millis() < 5000) {}
  pinMode(LED_PIN, OUTPUT);
  pinMode(TIMING_PIN, OUTPUT);
#if 0
  for (uint8_t i = 0; i < 5; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(250);
    digitalWrite(LED_PIN, LOW);
    delay(250);
  }
#endif
  if (HAL_GetCurrentCPUID() == CM7_CPUID) {
    Serial.begin(115200);
    while (!Serial && millis() < 5000) {};
    USERIAL = &Serial;
    USERIAL->println("ILI9341 Test! <CM7>");
    getMicros = micros;
  } else {
    // running on CM4 so slave device
    RPC.begin();
    USERIAL = &RPC;
    USERIAL->println("ILI9341 Test! <CM4>");
    delay(50);
    setupTimer();
    USERIAL->println("After setup timer");
    delay(50);
    USERIAL->println(getCurrentTimeInMicroseconds(), DEC);
    delay(50);
    getMicros = getCurrentTimeInMicroseconds;
  }

  // Lets try quick and dirty test of pin mapping.
  USERIAL->print("Builtin: "); USERIAL->print(LED_BUILTIN, DEC);
  USERIAL->print(" Red: "); USERIAL->print(LED_RED, DEC);
  USERIAL->print(" Green: "); USERIAL->print(LED_GREEN, DEC);
  USERIAL->print(" Blue: "); USERIAL->println(LED_BLUE, DEC);
  for (pin_size_t pin = 0; pin < PINS_COUNT; pin++) {
    PinName pinname = g_APinDescription[pin].name;
    int pinindex = PinNameToIndex(pinname);
    USERIAL->print(pin, DEC);
    USERIAL->print(" ");
    USERIAL->print(pinname, HEX);
    USERIAL->print(" P");
    uint8_t port_name = ((pinname >> 4) & 0xf) + 'A';
    USERIAL->write(port_name);
    USERIAL->print(pinname & 0xf, DEC);
    USERIAL->print(" ");
    USERIAL->println(pinindex, DEC);
  }



  tft.begin(40000000);
  USERIAL->println("After tft.begin");

  // read diagnostics (optional but can help debug problems)
  uint8_t x = tft.readcommand8(ILI9341_RDMODE);
  USERIAL->print("Display Power Mode: 0x");
  USERIAL->println(x, HEX);
  x = tft.readcommand8(ILI9341_RDMADCTL);
  USERIAL->print("MADCTL Mode: 0x");
  USERIAL->println(x, HEX);
  x = tft.readcommand8(ILI9341_RDPIXFMT);
  USERIAL->print("Pixel Format: 0x");
  USERIAL->println(x, HEX);
  x = tft.readcommand8(ILI9341_RDIMGFMT);
  USERIAL->print("Image Format: 0x");
  USERIAL->println(x, HEX);
  x = tft.readcommand8(ILI9341_RDSELFDIAG);
  USERIAL->print("Self Diagnostic: 0x");
  USERIAL->println(x, HEX);

  USERIAL->println((*getMicros)(), DEC);

  USERIAL->println(F("Benchmark                Time (microseconds)"));
  delay(10);
  USERIAL->print(F("Screen fill              "));
  USERIAL->println(testFillScreen());
  delay(200);

  USERIAL->print(F("Text                     "));
  USERIAL->println(testText());
  delay(200);

  USERIAL->print(F("Lines                    "));
  USERIAL->println(testLines(ILI9341_CYAN));
  delay(200);

  USERIAL->print(F("Horiz/Vert Lines         "));
  USERIAL->println(testFastLines(ILI9341_RED, ILI9341_BLUE));
  delay(200);

  USERIAL->print(F("Rectangles (outline)     "));
  USERIAL->println(testRects(ILI9341_GREEN));
  delay(200);

  USERIAL->print(F("Rectangles (filled)      "));
  USERIAL->println(testFilledRects(ILI9341_YELLOW, ILI9341_MAGENTA));
  delay(200);

  USERIAL->print(F("Circles (filled)         "));
  USERIAL->println(testFilledCircles(10, ILI9341_MAGENTA));
  delay(200);

  USERIAL->print(F("Circles (outline)        "));
  USERIAL->println(testCircles(10, ILI9341_WHITE));
  delay(200);

  USERIAL->print(F("Triangles (outline)      "));
  USERIAL->println(testTriangles());
  delay(200);

  USERIAL->print(F("Triangles (filled)       "));
  USERIAL->println(testFilledTriangles());
  delay(200);

  USERIAL->print(F("Rounded rects (outline)  "));
  USERIAL->println(testRoundRects());
  delay(200);

  USERIAL->print(F("Rounded rects (filled)   "));
  USERIAL->println(testFilledRoundRects());
  delay(200);

  USERIAL->println(F("Done!"));
}


void loop(void) {
  for (uint8_t rotation = 0; rotation < 4; rotation++) {
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    tft.setRotation(rotation);
    testText();
    delay(1000);
  }
}

unsigned long testFillScreen() {
  digitalWriteFast(TIMING_PIN, HIGH);
  unsigned long start = (*getMicros)();
  unsigned long startms = millis();
  tft.fillScreen(ILI9341_BLACK);  //1
  digitalWriteFast(TIMING_PIN, LOW);
  USERIAL->print((*getMicros)()-start, DEC);
  USERIAL->write(":");
  USERIAL->print(millis()-startms, DEC);
  yield();
  digitalWriteFast(TIMING_PIN, HIGH);
  start = (*getMicros)();
  startms = millis();
  tft.fillScreen(ILI9341_RED);  //2
  digitalWriteFast(TIMING_PIN, LOW);
  USERIAL->write(' '); USERIAL->print((*getMicros)()-start, DEC);
  USERIAL->write(":");
  USERIAL->print(millis()-startms, DEC);
  yield();
  start = (*getMicros)();
  startms = millis();
  digitalWriteFast(TIMING_PIN, HIGH);
  tft.fillScreen(ILI9341_GREEN); // 3
  digitalWriteFast(TIMING_PIN, LOW);
  USERIAL->write(' '); USERIAL->print((*getMicros)()-start, DEC);
  USERIAL->write(":");
  USERIAL->print(millis()-startms, DEC);
  yield();
  start = (*getMicros)();
  startms = millis();
  digitalWriteFast(TIMING_PIN, HIGH);
  tft.fillScreen(ILI9341_BLUE); //4
  digitalWriteFast(TIMING_PIN, LOW);
  USERIAL->write(' '); USERIAL->print((*getMicros)()-start, DEC);
  USERIAL->write(":");
  USERIAL->print(millis()-startms, DEC);
  yield();
  start = (*getMicros)();
  startms = millis();
  digitalWriteFast(TIMING_PIN, HIGH);
  tft.fillScreen(ILI9341_BLACK);  //5
  digitalWriteFast(TIMING_PIN, LOW);
  USERIAL->write(' '); USERIAL->print((*getMicros)()-start, DEC);
  USERIAL->write(":");
  USERIAL->print(millis()-startms, DEC);
  USERIAL->write(" ");
  yield();
  return (*getMicros)() - start;
}

unsigned long testText() {
  tft.fillScreen(ILI9341_BLACK);
  digitalWriteFast(TIMING_PIN, HIGH);
  unsigned long start = (*getMicros)();
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(1);
  tft.println("Hello World!");
  tft.setTextColor(ILI9341_YELLOW);
  tft.setTextSize(2);
  tft.println(1234.56);
  tft.setTextColor(ILI9341_RED);
  tft.setTextSize(3);
  tft.println(0xDEADBEEF, HEX);
  tft.println();
  tft.setTextColor(ILI9341_GREEN);
  tft.setTextSize(5);
  tft.println("Groop");
  tft.setTextSize(2);
  tft.println("I implore thee,");
  tft.setTextSize(1);
  tft.println("my foonting turlingdromes.");
  tft.println("And hooptiously drangle me");
  tft.println("with crinkly bindlewurdles,");
  tft.println("Or I will rend thee");
  tft.println("in the gobberwarts");
  tft.println("with my blurglecruncheon,");
  tft.println("see if I don't!");
  digitalWriteFast(TIMING_PIN, LOW);
  return (*getMicros)() - start;
}

unsigned long testLines(uint16_t color) {
  unsigned long start, t;
  int x1, y1, x2, y2,
    w = tft.width(),
    h = tft.height();

  tft.fillScreen(ILI9341_BLACK);
  yield();

  x1 = y1 = 0;
  y2 = h - 1;
  digitalWriteFast(TIMING_PIN, HIGH);
  start = (*getMicros)();
  for (x2 = 0; x2 < w; x2 += 6) tft.drawLine(x1, y1, x2, y2, color);
  x2 = w - 1;
  for (y2 = 0; y2 < h; y2 += 6) tft.drawLine(x1, y1, x2, y2, color);
  t = (*getMicros)() - start;  // fillScreen doesn't count against timing

  yield();
  tft.fillScreen(ILI9341_BLACK);
  yield();

  x1 = w - 1;
  y1 = 0;
  y2 = h - 1;
  start = (*getMicros)();
  for (x2 = 0; x2 < w; x2 += 6) tft.drawLine(x1, y1, x2, y2, color);
  x2 = 0;
  for (y2 = 0; y2 < h; y2 += 6) tft.drawLine(x1, y1, x2, y2, color);
  t += (*getMicros)() - start;

  yield();
  tft.fillScreen(ILI9341_BLACK);
  yield();

  x1 = 0;
  y1 = h - 1;
  y2 = 0;
  start = (*getMicros)();
  for (x2 = 0; x2 < w; x2 += 6) tft.drawLine(x1, y1, x2, y2, color);
  x2 = w - 1;
  for (y2 = 0; y2 < h; y2 += 6) tft.drawLine(x1, y1, x2, y2, color);
  t += (*getMicros)() - start;

  yield();
  tft.fillScreen(ILI9341_BLACK);
  yield();

  x1 = w - 1;
  y1 = h - 1;
  y2 = 0;
  start = (*getMicros)();
  for (x2 = 0; x2 < w; x2 += 6) tft.drawLine(x1, y1, x2, y2, color);
  x2 = 0;
  for (y2 = 0; y2 < h; y2 += 6) tft.drawLine(x1, y1, x2, y2, color);

  yield();
  digitalWriteFast(TIMING_PIN, LOW);
  return (*getMicros)() - start;
}

unsigned long testFastLines(uint16_t color1, uint16_t color2) {
  unsigned long start;
  int x, y, w = tft.width(), h = tft.height();

  tft.fillScreen(ILI9341_BLACK);
  digitalWriteFast(TIMING_PIN, HIGH);
  start = (*getMicros)();
  for (y = 0; y < h; y += 5) tft.drawFastHLine(0, y, w, color1);
  for (x = 0; x < w; x += 5) tft.drawFastVLine(x, 0, h, color2);

  digitalWriteFast(TIMING_PIN, LOW);
  return (*getMicros)() - start;
}

unsigned long testRects(uint16_t color) {
  unsigned long start;
  int n, i, i2,
    cx = tft.width() / 2,
    cy = tft.height() / 2;

  tft.fillScreen(ILI9341_BLACK);
  n = min(tft.width(), tft.height());
  digitalWriteFast(TIMING_PIN, HIGH);
  start = (*getMicros)();
  for (i = 2; i < n; i += 6) {
    i2 = i / 2;
    tft.drawRect(cx - i2, cy - i2, i, i, color);
  }

  digitalWriteFast(TIMING_PIN, LOW);
  return (*getMicros)() - start;
}

unsigned long testFilledRects(uint16_t color1, uint16_t color2) {
  unsigned long start, t = 0;
  int n, i, i2,
    cx = tft.width() / 2 - 1,
    cy = tft.height() / 2 - 1;

  tft.fillScreen(ILI9341_BLACK);
  n = min(tft.width(), tft.height());
  for (i = n; i > 0; i -= 6) {
    i2 = i / 2;
    digitalWriteFast(TIMING_PIN, HIGH);
    start = (*getMicros)();
    tft.fillRect(cx - i2, cy - i2, i, i, color1);
    t += (*getMicros)() - start;
    // Outlines are not included in timing results
    tft.drawRect(cx - i2, cy - i2, i, i, color2);
    yield();
  }

  digitalWriteFast(TIMING_PIN, LOW);
  return t;
}

unsigned long testFilledCircles(uint8_t radius, uint16_t color) {
  unsigned long start;
  int x, y, w = tft.width(), h = tft.height(), r2 = radius * 2;

  tft.fillScreen(ILI9341_BLACK);
  digitalWriteFast(TIMING_PIN, HIGH);
  start = (*getMicros)();
  for (x = radius; x < w; x += r2) {
    for (y = radius; y < h; y += r2) {
      tft.fillCircle(x, y, radius, color);
    }
  }

  digitalWriteFast(TIMING_PIN, LOW);
  return (*getMicros)() - start;
}

unsigned long testCircles(uint8_t radius, uint16_t color) {
  unsigned long start;
  int x, y, r2 = radius * 2,
            w = tft.width() + radius,
            h = tft.height() + radius;

  // Screen is not cleared for this one -- this is
  // intentional and does not affect the reported time.
  digitalWriteFast(TIMING_PIN, HIGH);
  start = (*getMicros)();
  for (x = 0; x < w; x += r2) {
    for (y = 0; y < h; y += r2) {
      tft.drawCircle(x, y, radius, color);
    }
  }

  digitalWriteFast(TIMING_PIN, LOW);
  return (*getMicros)() - start;
}

unsigned long testTriangles() {
  unsigned long start;
  int n, i, cx = tft.width() / 2 - 1,
            cy = tft.height() / 2 - 1;

  tft.fillScreen(ILI9341_BLACK);
  n = min(cx, cy);
  digitalWriteFast(TIMING_PIN, HIGH);
  start = (*getMicros)();
  for (i = 0; i < n; i += 5) {
    tft.drawTriangle(
      cx, cy - i,      // peak
      cx - i, cy + i,  // bottom left
      cx + i, cy + i,  // bottom right
      tft.color565(i, i, i));
  }

  digitalWriteFast(TIMING_PIN, LOW);
  return (*getMicros)() - start;
}

unsigned long testFilledTriangles() {
  unsigned long start, t = 0;
  int i, cx = tft.width() / 2 - 1,
         cy = tft.height() / 2 - 1;

  tft.fillScreen(ILI9341_BLACK);
  digitalWriteFast(TIMING_PIN, HIGH);
  start = (*getMicros)();
  for (i = min(cx, cy); i > 10; i -= 5) {
    start = (*getMicros)();
    tft.fillTriangle(cx, cy - i, cx - i, cy + i, cx + i, cy + i,
                     tft.color565(0, i * 10, i * 10));
    t += (*getMicros)() - start;
    tft.drawTriangle(cx, cy - i, cx - i, cy + i, cx + i, cy + i,
                     tft.color565(i * 10, i * 10, 0));
    yield();
  }

  digitalWriteFast(TIMING_PIN, LOW);
  return t;
}

unsigned long testRoundRects() {
  unsigned long start;
  int w, i, i2,
    cx = tft.width() / 2 - 1,
    cy = tft.height() / 2 - 1;

  tft.fillScreen(ILI9341_BLACK);
  w = min(tft.width(), tft.height());
  digitalWriteFast(TIMING_PIN, HIGH);
  start = (*getMicros)();
  for (i = 0; i < w; i += 6) {
    i2 = i / 2;
    tft.drawRoundRect(cx - i2, cy - i2, i, i, i / 8, tft.color565(i, 0, 0));
  }

  digitalWriteFast(TIMING_PIN, LOW);
  return (*getMicros)() - start;
}

unsigned long testFilledRoundRects() {
  unsigned long start;
  int i, i2,
    cx = tft.width() / 2 - 1,
    cy = tft.height() / 2 - 1;

  tft.fillScreen(ILI9341_BLACK);
  digitalWriteFast(TIMING_PIN, HIGH);
  start = (*getMicros)();
  for (i = min(tft.width(), tft.height()); i > 20; i -= 6) {
    i2 = i / 2;
    tft.fillRoundRect(cx - i2, cy - i2, i, i, i / 8, tft.color565(0, i, 0));
    yield();
  }

  digitalWriteFast(TIMING_PIN, LOW);
  return (*getMicros)() - start;
}

const uint32_t M4_CLOCK_SPEED = 240000000;
void setupTimer() {
  TIM2->CR1 = 0;
  TIM2->CNT = 0;
  TIM2->PSC = (M4_CLOCK_SPEED / 1000000) - 1;
  TIM2->ARR = 0xFFFFFFFF;
  TIM2->EGR = TIM_EGR_UG;
  TIM2->CR1 |= TIM_CR1_CEN;
}

uint32_t getCurrentTimeInMicroseconds() {
  return (uint32_t)TIM2->CNT;
}