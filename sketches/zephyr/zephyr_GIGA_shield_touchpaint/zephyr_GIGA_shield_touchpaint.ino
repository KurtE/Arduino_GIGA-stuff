/***************************************************
  This is our touchscreen painting example for the Adafruit ILI9341 Shield
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

//REDIRECT_STDOUT_TO(Serial)
#include "Arduino_GigaDisplay_GFX.h"
#include "GigaDisplayRGB.h"
#include "Arduino_GigaDisplayTouch.h"
#include "Arduino_GigaDisplay.h"

GigaDisplay_GFX display;
Arduino_GigaDisplayTouch touchDetector;

// Some of our displays appear to have a differnt orieintations of the touch sensor
// versus the display.
uint8_t g_touch_rotation = 0;


GigaDisplayRGB rgb;  //create rgb object

#define GC9A01A_CYAN 0x07FF
#define GC9A01A_RED 0xf800
#define GC9A01A_BLUE 0x001F
#define GC9A01A_GREEN 0x07E0
#define GC9A01A_MAGENTA 0xF81F
#define GC9A01A_WHITE 0xffff
#define GC9A01A_BLACK 0x0000
#define GC9A01A_YELLOW 0xFFE0

// Size of the color selection boxes and the paintbrush size
#define BOXSIZE 80
#define PENRADIUS 5
int oldcolor, currentcolor;
static const uint16_t paint_colors[] = { GC9A01A_RED, GC9A01A_YELLOW, GC9A01A_GREEN, GC9A01A_CYAN, GC9A01A_BLUE, GC9A01A_MAGENTA };
#define COUNT_PAINT_COLORS (sizeof(paint_colors) / sizeof(paint_colors[0]))
uint8_t current_color_index = 0;
void setup(void) {
  while (!Serial && millis() < 5000)
    ;  // used for leonardo debugging

  Serial.begin(9600);
  Serial.println(F("Touch Paint!"));

  rgb.begin();  //init the library

  display.begin();
  rgb.on(128, 0, 0);
  display.fillScreen(GC9A01A_RED);
  delay(500);
  rgb.on(0, 128, 0);  //turn on blue pixel
  display.fillScreen(GC9A01A_GREEN);
  delay(500);
  rgb.on(0, 0, 128);  //turn on blue pixel
  display.fillScreen(GC9A01A_BLUE);
  delay(500);
  rgb.on(128, 128, 128);
  display.fillScreen(GC9A01A_WHITE);
  delay(500);
  rgb.off();  //turn off all pixels
  display.fillScreen(GC9A01A_BLACK);
  delay(500);

  if (touchDetector.begin()) {
    Serial.println("Touch controller init - OK");
    Serial.print("Touch Orientation: ");
    Serial.println(g_touch_rotation);
    Serial.println("Can change by typing 0-3 in Serial monitor");
  } else {
    Serial.println("Touch controller init - FAILED");
    while (1) {
      rgb.on(128, 0, 0);
      delay(1000);
      rgb.off();
      delay(1000);
    }
  }

  display.fillScreen(GC9A01A_BLACK);

  display.startBuffering();
  display.fillScreen(GC9A01A_BLACK);
  for (uint8_t i = 0; i < COUNT_PAINT_COLORS; i++) {
    display.fillRect(BOXSIZE * i, 0, BOXSIZE, BOXSIZE, paint_colors[i]);
  }
  display.endBuffering();
  // select the current color 'red'
  //display.drawRect(0, 0, BOXSIZE, BOXSIZE, GC9A01A_WHITE);
  currentcolor = GC9A01A_RED;
  Serial.print("GFX Rotation: ");
  Serial.println(display.getRotation());
}

void convertRawTouchyByRotation(int xRaw, int yRaw, int &touch_x, int &touch_y) {
  switch (g_touch_rotation) {
    case 0:
      touch_y = xRaw;
      touch_x = display.width() - yRaw;
      break;
    case 1:
      Serial.print("@");
      touch_x = xRaw;  //display.width() - xRaw;
      touch_y = yRaw;  // display.height() - yRaw;
      break;
  }
}


void loop() {
  uint8_t contacts;
  int touch_x, touch_y;
  GDTpoint_t points[5];
#ifdef __MBED__
  contacts = touchDetector.getTouchPoints(points);
#else
  contacts = touchDetector.getTouchPoints(points, 50);
#endif

  if (contacts == 0) {
    rgb.off();
    delay(1);
    if (Serial.available()) {
      int ch = Serial.read();
      if ((ch >= '0') && (ch <= '3')) {
        g_touch_rotation = ch - '0';
        Serial.print("New touch rotation: ");
        Serial.println(g_touch_rotation);
        while (Serial.read() != -1) {}
      }
    }

    return;
  }
  //
  // Lets try setting the LED to current color RGB although maybe lesser brightness...
  rgb.on(((paint_colors[current_color_index] >> 8) & 0xf8) >> 4,
         ((paint_colors[current_color_index] >> 5) & 0xfc) >> 4,
         ((paint_colors[current_color_index] << 3) & 0xf8) >> 4);
  //rgb.on(0, 32, 0);

  // Retrieve a point
  Serial.print("X = ");
  Serial.print(points[0].x);
  Serial.print("\tY = ");
  Serial.println(points[0].y);

  // Lets map the the point to the screen rotation.
  convertRawTouchyByRotation(points[0].x, points[0].y, touch_x, touch_y);

  if (touch_y < BOXSIZE) {
    uint8_t new_color_index = touch_x / BOXSIZE;
    if ((new_color_index != current_color_index) && (new_color_index < COUNT_PAINT_COLORS)) {
      // highlight the new touch color
      display.drawRect(BOXSIZE * new_color_index, 0, BOXSIZE, BOXSIZE, GC9A01A_WHITE);
      // unhighlight the previous one.
      display.drawRect(BOXSIZE * current_color_index, 0, BOXSIZE, BOXSIZE, paint_colors[current_color_index]);
      current_color_index = new_color_index;
    }
  }

  if (((touch_y - PENRADIUS) > BOXSIZE) && ((touch_y + PENRADIUS) < display.height())) {
    display.fillCircle(touch_x, touch_y, PENRADIUS, paint_colors[current_color_index]);
  }
  for (uint8_t i = 1; i < contacts; i++) {
    convertRawTouchyByRotation(points[i].x, points[i].y, touch_x, touch_y);
    if (((touch_y - PENRADIUS) > BOXSIZE) && ((touch_y + PENRADIUS) < display.height())) {
      uint8_t color_index = current_color_index + i;
      if (color_index >= COUNT_PAINT_COLORS) color_index -= COUNT_PAINT_COLORS;
      display.fillCircle(touch_x, touch_y, PENRADIUS, paint_colors[color_index]);
    };
  }

  delay(1);
}
