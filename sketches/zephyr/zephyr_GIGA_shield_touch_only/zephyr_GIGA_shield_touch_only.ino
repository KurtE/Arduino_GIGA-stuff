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

#include "GigaDisplayRGB.h"
#include "Arduino_GigaDisplayTouch.h"
Arduino_GigaDisplayTouch touchDetector;

GigaDisplayRGB rgb;  //create rgb object



// Size of the color selection boxes and the paintbrush size
void setup(void) {
  // while (!Serial);     // used for leonardo debugging

  Serial.begin(9600);
  Serial.println(F("Touch Paint!"));

  rgb.begin();  //init the library

  rgb.on(128, 0, 0);
  delay(500);
  rgb.on(0, 128, 0);  //turn on blue pixel
  delay(500);
  rgb.on(0, 0, 128);  //turn on blue pixel
  delay(500);
  rgb.on(128, 128, 128);
  delay(500);

  if (touchDetector.begin()) {
    Serial.println("Touch controller init - OK");
  } else {
    Serial.println("Touch controller init - FAILED");
    while (1) {
      rgb.on(128, 0, 0);
      delay(1000);
      rgb.off();
      delay(1000);
    }
  }

}


void loop() {
  uint8_t contacts;
  GDTpoint_t points[5];
  contacts = touchDetector.getTouchPoints(points, 50);

  if (contacts == 0) {
    rgb.off();
    return;
  }
  rgb.on(0, 32, 0);

  // Retrieve a point
  int touch_x = points[0].x;
  int touch_y = points[0].y;

  Serial.print("X = "); Serial.print(touch_x);
  Serial.print("\tY = "); Serial.println(touch_y);
  //printk("X = %d Y= %d\n", touch_x, touch_y);
 
  // Scale from ~0->4000 to display.width using the calibration #'s
  //  touch_x = map(touch_x, TS_MINX, TS_MAXX, 0, display.width());
  //  touch_y = map(touch_y, TS_MINY, TS_MAXY, 0, display.height());

  /*
  Serial.print("("); Serial.print(touch_x);
  Serial.print(", "); Serial.print(touch_y);
  Serial.println(")");
  */
}
