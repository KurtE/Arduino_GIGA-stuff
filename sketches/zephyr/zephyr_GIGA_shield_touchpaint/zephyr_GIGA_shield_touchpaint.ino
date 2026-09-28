//=============================================================================
// Touch paint for The Arduino GIGA board with the GIGA display shield.
// This version is setup to run on ArduinoCore-zephyr Arduino Board type.
// 
// The sketch displays a color pallet on the screen, and allow you to paint
// by touching the screen and it will draw at that location with the currently
// selected color. As the GT911 touch controller allows up to 5 fingers touching, 
// this code, detects how many touches are detected and will paint the locations
// with the next colors up.
//
// You can change the orientation of the screen by typing 0-3 in the Serial
// monitor.
//
// I mainly use this sketch to help debug issues with the Display graphics and
// touch systems.  Currently we have detected that the Arduino Display Shield
// has shipped with at least 2 different register configurations.  The code here
// tries to detect which one is on your display and modify how the code converts
// the raw touch positions, into the corresponding Screen locations.
//
// The code was originally adapted from the Adafruit touchscreen sketch for
// the ILI9341 displays and the Adafruit_ILI9341 library
//=============================================================================

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

#define DUMP_GT911_REGISTERS
// uncomment if you wish to see th raw x, y and mapped for touch
//#define DEBUG_TOUCH  
#define TFT_ROTATION 0

#ifdef DUMP_GT911_REGISTERS
#include <MemoryHexDump.h>
#endif

//REDIRECT_STDOUT_TO(Serial)
#include "Arduino_GigaDisplay_GFX.h"
#include "GigaDisplayRGB.h"
#include "Arduino_GigaDisplayTouch.h"
#include "Arduino_GigaDisplay.h"

GigaDisplay_GFX display;
Arduino_GigaDisplayTouch touchDetector;

#define GT911_REG_CONFIG_VERSION 0x8047
#define GT911_REG_CONFIG_MODULE_SWITCH1 0x804D
#define GT911_REG_CONFIG_CHECKSUM 0x80FF
#define GT911_REG_80_RANGE_SIZE (0x8100 - 0x8047)

#define GT911_REG_CONFIG_PROCUCTID 0x8140
#define GT911_REG_PT5_SIZE_LAST 0x8175
#define GT911_REG_81_RANGE_SIZE (0x8176 - 0x8140)


// Some of our displays appear to have a differnt orieintations of the touch sensor
// versus the display.
uint8_t g_touch_rotation = 0;
bool g_touch_Alt_config = false;

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
  while (!Serial && millis() < 4000) {}
  Serial.println(F("Touch Paint!"));

  rgb.begin();  //init the library

  display.begin();
  display.setRotation(TFT_ROTATION);

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
#ifdef DUMP_GT911_REGISTERS
    Serial.println("typing a 'd' wil dump the GT911 registers");
#endif
  } else {
    Serial.println("Touch controller init - FAILED");
    while (1) {
      rgb.on(128, 0, 0);
      delay(1000);
      rgb.off();
      delay(1000);
    }
  }
  DrawScreen(TFT_ROTATION);

  //#ifdef DUMP_GT911_REGISTERS
  //  dump_gt911_registers();
  //#endif

  // lets see if we think the GT911 if off.
  uint8_t switch_1;
  ReadGT911Registers(GT911_REG_CONFIG_MODULE_SWITCH1, &switch_1, 1);
  g_touch_Alt_config = (switch_1 & 0x40) ? false : true;
  if (switch_1 & 0x40) Serial.println("GT911 Normal display(Sensor_Resersal(X2X))");
  else Serial.println("GT911 display sensor different");
}

void convertRawTouchByRotation(int xRaw, int yRaw, int &touch_x, int &touch_y) {
  if (g_touch_Alt_config) {
    switch (g_touch_rotation) {
      case 0:
        touch_x = xRaw;
        touch_y = yRaw;
        break;
      case 1:
        touch_x = yRaw;                     //display.width() - xRaw;
        touch_y = display.height() - xRaw;  // display.height() - yRaw;
        break;
      case 2:
        touch_x = display.width() - xRaw;
        touch_y = display.height() - yRaw;
        break;
      case 3:
        touch_x = display.width() - yRaw;
        touch_y = xRaw;
        break;
    }

  } else {
    switch (g_touch_rotation) {
      case 0:
        touch_y = xRaw;
        touch_x = display.width() - yRaw;
        break;
      case 1:
        touch_x = xRaw;  //display.width() - xRaw;
        touch_y = yRaw;  // display.height() - yRaw;
        break;
      case 2:
        touch_x = yRaw;
        touch_y = display.height() - xRaw;
        break;
      case 3:
        touch_x = display.width() - xRaw;
        touch_y = display.height() - yRaw;
        break;
    }
  }
}


void DrawScreen(uint8_t rotation) {
  display.setRotation(rotation);

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

  rgb.on(((paint_colors[current_color_index] >> 8) & 0xf8) >> 4,
         ((paint_colors[current_color_index] >> 5) & 0xfc) >> 4,
         ((paint_colors[current_color_index] << 3) & 0xf8) >> 4);
}

void loop() {
  uint8_t contacts;
  int touch_x, touch_y;
  GDTpoint_t points[5];
  contacts = touchDetector.getTouchPoints(points);

  if (contacts == 0) {
    rgb.off();
    delay(1);
    if (Serial.available()) {
      int ch = Serial.read();
      if ((ch >= '0') && (ch <= '3')) {
        g_touch_rotation = ch - '0';
        DrawScreen(g_touch_rotation);

        Serial.print("New touch rotation: ");
        Serial.println(g_touch_rotation);
#ifdef DUMP_GT911_REGISTERS
      } else if ((ch == 'd') || (ch == 'D')) {
        dump_gt911_registers();
#endif
      }
      while (Serial.read() != -1) {}
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
#ifdef DEBUG_TOUCH
  Serial.print("X = ");
  Serial.print(points[0].x);
  Serial.print("\tY = ");
  Serial.print(points[0].y);
#endif

  // Lets map the the point to the screen rotation.
  convertRawTouchByRotation(points[0].x, points[0].y, touch_x, touch_y);
#ifdef DEBUG_TOUCH
  Serial.print(" -> X = ");
  Serial.print(touch_x);
  Serial.print(" Y = ");
  Serial.println(touch_y);
#endif

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
    convertRawTouchByRotation(points[i].x, points[i].y, touch_x, touch_y);
    if (((touch_y - PENRADIUS) > BOXSIZE) && ((touch_y + PENRADIUS) < display.height())) {
      uint8_t color_index = current_color_index + i;
      if (color_index >= COUNT_PAINT_COLORS) color_index -= COUNT_PAINT_COLORS;
      display.fillCircle(touch_x, touch_y, PENRADIUS, paint_colors[color_index]);
    };
  }

  delay(1);
}

#ifdef DUMP_GT911_REGISTERS
void dump_gt911_registers() {
  uint8_t buffer[256];
  Serial.println("--------------------------------------------------------");
  ReadGT911Registers(GT911_REG_CONFIG_VERSION, buffer, GT911_REG_80_RANGE_SIZE);
  MemoryHexDump(Serial, buffer, GT911_REG_80_RANGE_SIZE, false, "GT911 Registers\n", -1, GT911_REG_CONFIG_VERSION);
  ReadGT911Registers(GT911_REG_CONFIG_PROCUCTID, buffer, GT911_REG_81_RANGE_SIZE);
  MemoryHexDump(Serial, buffer, GT911_REG_81_RANGE_SIZE, false, "GT911 Registers\n", -1, GT911_REG_CONFIG_PROCUCTID);
  Serial.println("--------------------------------------------------------");
}

#endif

uint8_t ReadGT911Registers(uint16_t reg, uint8_t *data, uint8_t len) {
  uint8_t status = 0;
#define _addr 0x5D
  Wire1.beginTransmission(_addr);
  Wire1.write(reg >> 8);   /* Register H */
  Wire1.write(reg & 0xFF); /* Register L */
  status = Wire1.endTransmission();

  if (status)
    return status;

  Wire1.requestFrom(_addr, len);
  uint8_t index = 0;
  /* Data [0..n] */
  while (Wire1.available()) {
    data[index++] = Wire1.read();
  }

  if (len == index)
    return 0;
  else
    return 4; /* Other error */
}
