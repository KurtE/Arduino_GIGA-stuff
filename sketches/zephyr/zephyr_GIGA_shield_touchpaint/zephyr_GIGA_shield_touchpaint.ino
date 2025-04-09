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
//#include "Arduino_GigaDisplayTouch.h"
#include "Arduino_GigaDisplay.h"

GigaDisplay_GFX display;
// /Arduino_GigaDisplayTouch touchDetector;

GigaDisplayRGB rgb;  //create rgb object

#define GC9A01A_CYAN 0x07FF
#define GC9A01A_RED 0xf800
#define GC9A01A_BLUE 0x001F
#define GC9A01A_GREEN 0x07E0
#define GC9A01A_MAGENTA 0xF81F
#define GC9A01A_WHITE 0xffff
#define GC9A01A_BLACK 0x0000
#define GC9A01A_YELLOW 0xFFE0
// The buffer used to rotate and resize the frame


// Size of the color selection boxes and the paintbrush size
#define BOXSIZE 80
#define PENRADIUS 5
int oldcolor, currentcolor;

void setup(void) {
  // while (!Serial);     // used for leonardo debugging

  Serial.begin(9600);
  Serial.println(F("Touch Paint!"));

  rgb.begin();  //init the library

  display.begin();
  rgb.on(255, 0, 0);
  display.fillScreen(GC9A01A_RED);
  delay(500);
  rgb.on(0, 255, 0);  //turn on blue pixel
  display.fillScreen(GC9A01A_GREEN);
  delay(500);
  rgb.on(0, 0, 255);  //turn on blue pixel
  display.fillScreen(GC9A01A_BLUE);
  delay(500);
  rgb.on(255, 255, 255);
  display.fillScreen(GC9A01A_WHITE);
  delay(500);
  rgb.off();  //turn off all pixels
  display.fillScreen(GC9A01A_BLACK);
  delay(500);

#if 0
  if (touchDetector.begin()) {
    Serial.print("Touch controller init - OK");
  } else {
    Serial.print("Touch controller init - FAILED");
    while (1) {
      rgb.on(255, 0, 0);
      delay(1000);
      rgb.off();
      delay(1000);
    }
  }
#endif

  display.fillScreen(GC9A01A_BLACK);

  // make the color selection boxes
  display.startBuffering();
  display.fillRect(0, 0, BOXSIZE, BOXSIZE, GC9A01A_RED);
  display.fillRect(BOXSIZE, 0, BOXSIZE, BOXSIZE, GC9A01A_YELLOW);
  display.fillRect(BOXSIZE * 2, 0, BOXSIZE, BOXSIZE, GC9A01A_GREEN);
  display.fillRect(BOXSIZE * 3, 0, BOXSIZE, BOXSIZE, GC9A01A_CYAN);
  display.fillRect(BOXSIZE * 4, 0, BOXSIZE, BOXSIZE, GC9A01A_BLUE);
  display.fillRect(BOXSIZE * 5, 0, BOXSIZE, BOXSIZE, GC9A01A_MAGENTA);
  display.endBuffering();
  // select the current color 'red'
  display.drawRect(0, 0, BOXSIZE, BOXSIZE, GC9A01A_WHITE);
  currentcolor = GC9A01A_RED;
}


void loop() {
#if 0
  uint8_t contacts;
  GDTpoint_t points[5];
  contacts = touchDetector.getTouchPoints(points);

  if (contacts == 0) {
    rgb.off();
    return;
  }
  rgb.on(0, 32, 0);

  // Retrieve a point
  int touch_x = points[0].x;
  int touch_y = points[0].y;

  /*
  Serial.print("X = "); Serial.print(touch_x);
  Serial.print("\tY = "); Serial.print(touch_y);
  Serial.print("\tPressure = "); Serial.println(p.z);  
 */

  // Scale from ~0->4000 to display.width using the calibration #'s
  //  touch_x = map(touch_x, TS_MINX, TS_MAXX, 0, display.width());
  //  touch_y = map(touch_y, TS_MINY, TS_MAXY, 0, display.height());

  /*
  Serial.print("("); Serial.print(touch_x);
  Serial.print(", "); Serial.print(touch_y);
  Serial.println(")");
  */

  if (touch_y < BOXSIZE) {
    oldcolor = currentcolor;

    if (touch_x < BOXSIZE) {
      currentcolor = GC9A01A_RED;
      display.drawRect(0, 0, BOXSIZE, BOXSIZE, GC9A01A_WHITE);
    } else if (touch_x < BOXSIZE * 2) {
      currentcolor = GC9A01A_YELLOW;
      display.drawRect(BOXSIZE, 0, BOXSIZE, BOXSIZE, GC9A01A_WHITE);
    } else if (touch_x < BOXSIZE * 3) {
      currentcolor = GC9A01A_GREEN;
      display.drawRect(BOXSIZE * 2, 0, BOXSIZE, BOXSIZE, GC9A01A_WHITE);
    } else if (touch_x < BOXSIZE * 4) {
      currentcolor = GC9A01A_CYAN;
      display.drawRect(BOXSIZE * 3, 0, BOXSIZE, BOXSIZE, GC9A01A_WHITE);
    } else if (touch_x < BOXSIZE * 5) {
      currentcolor = GC9A01A_BLUE;
      display.drawRect(BOXSIZE * 4, 0, BOXSIZE, BOXSIZE, GC9A01A_WHITE);
    } else if (touch_x < BOXSIZE * 6) {
      currentcolor = GC9A01A_MAGENTA;
      display.drawRect(BOXSIZE * 5, 0, BOXSIZE, BOXSIZE, GC9A01A_WHITE);
    }

    if (oldcolor != currentcolor) {
      if (oldcolor == GC9A01A_RED)
        display.fillRect(0, 0, BOXSIZE, BOXSIZE, GC9A01A_RED);
      if (oldcolor == GC9A01A_YELLOW)
        display.fillRect(BOXSIZE, 0, BOXSIZE, BOXSIZE, GC9A01A_YELLOW);
      if (oldcolor == GC9A01A_GREEN)
        display.fillRect(BOXSIZE * 2, 0, BOXSIZE, BOXSIZE, GC9A01A_GREEN);
      if (oldcolor == GC9A01A_CYAN)
        display.fillRect(BOXSIZE * 3, 0, BOXSIZE, BOXSIZE, GC9A01A_CYAN);
      if (oldcolor == GC9A01A_BLUE)
        display.fillRect(BOXSIZE * 4, 0, BOXSIZE, BOXSIZE, GC9A01A_BLUE);
      if (oldcolor == GC9A01A_MAGENTA)
        display.fillRect(BOXSIZE * 5, 0, BOXSIZE, BOXSIZE, GC9A01A_MAGENTA);
    }
  }
  if (((touch_y - PENRADIUS) > BOXSIZE) && ((touch_y + PENRADIUS) < display.height())) {
    display.fillCircle(touch_x, touch_y, PENRADIUS, currentcolor);
  }
#endif  
}
