#include <MemoryHexDump.h>

#include "Arduino.h"
#include "Arduino_H7_Video.h"
#include "lvgl.h"

Arduino_H7_Video Display(800, 480, GigaDisplayShield);

#define CANVAS_WIDTH 240
#define CANVAS_HEIGHT 240

void setup() {
  Serial.begin(115200);

  while (!Serial && millis() < 5000)
    ;

  Serial.print("LV_COLOR_DEPTH : ");
  Serial.println(LV_COLOR_DEPTH);

  Display.begin();

  static /*lv_color_t*/ uint8_t cbuf[LV_CANVAS_BUF_SIZE_TRUE_COLOR(CANVAS_WIDTH, CANVAS_HEIGHT)];

  Serial.print("sizeof lv_color_t:");
  Serial.println(sizeof(lv_color_t), DEC);
  lv_obj_set_style_bg_color(lv_scr_act(), lv_color_make(0, 0, 100), 0);

  lv_obj_t* canvas = lv_canvas_create(lv_scr_act());

  lv_canvas_set_buffer(canvas, cbuf, CANVAS_WIDTH, CANVAS_HEIGHT, LV_IMG_CF_TRUE_COLOR);
  lv_obj_align(canvas, LV_ALIGN_CENTER, 0, 0);
  lv_canvas_fill_bg(canvas, lv_color_make(100, 0, 0), LV_OPA_COVER);
  lv_color_t c = lv_color_make(0, 100, 0);

  for (int i = 0; i < CANVAS_WIDTH; i++) {
    lv_canvas_set_px_color(canvas, i, i, c);  //https://docs.lvgl.io/8.0/overview/color.html?highlight=color
    lv_canvas_set_px_color(canvas, i, (CANVAS_WIDTH - 1) - i, c);
  }

  lv_refr_now(lv_disp_get_default());
  Serial.print("CBUF Size: ");
  Serial.println(sizeof(cbuf), DEC);
  // dump a few rows...
  MemoryHexDump(Serial, cbuf, sizeof(cbuf) / (CANVAS_HEIGHT / 8), true);

  Serial.println("Image ready");
}

void loop() {
  lv_timer_handler();
}
