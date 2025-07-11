/*
 * Copyright 2025 Arduino SA
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Camera driver.
 */
#include "Arduino.h"
#include "Arduino_Giga_Display.h"

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>

Display::Display() : gdev(NULL){
}

bool Display::begin(DisplayPixelFormat pixformat, int rotation) {
  
  int ret = 0;

  #if DT_HAS_CHOSEN(zephyr_display)
  this->gdev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
  #endif

  if (!this->gdev || !device_is_ready(this->gdev)) {
    Serial.println("\t<err> Zephy Display Not Ready!...");
      return false;
  }

  // Get capabilities
  struct display_capabilities capabilities = {0};
  display_get_capabilities(this->gdev, &capabilities);

  uint8_t format_spec = 0;
  
  switch (pixformat) {
      case DISPLAY_RGB565:
          Serial.println("Set RGB565");
          if (capabilities.current_pixel_format != PIXEL_FORMAT_RGB_565) {
            ret = display_set_pixel_format(this->gdev, PIXEL_FORMAT_RGB_565);
          }
          format_spec = 1;
          break;
      case DISPLAY_RGB888:
          Serial.println("Set RGB888");
          if (capabilities.current_pixel_format != PIXEL_FORMAT_RGB_888) {
            ret = display_set_pixel_format(this->gdev, PIXEL_FORMAT_RGB_888);
          }
          format_spec = 1;
          break;
      default:
          break;
  }

  if(format_spec == 0) {
    Serial.println("\t<err> The specified format is not supported");
    return false;
  }
   
  if (ret) {
		Serial.println("\t<err> Unable to set display format");
		return false;
	}
  

  display_orientation orientation;
  switch(rotation){
    case 0:
      orientation = DISPLAY_ORIENTATION_NORMAL;
      break;
    case 1:
      orientation = DISPLAY_ORIENTATION_ROTATED_90;
      break;
    case 2:
      orientation = DISPLAY_ORIENTATION_ROTATED_180;
      break;      
    case 3:
      orientation = DISPLAY_ORIENTATION_ROTATED_270;
      break;
    default:
      orientation = DISPLAY_ORIENTATION_NORMAL;
      break;   
  }

  Serial.print("Orientation: "); Serial.println(orientation);
  ret = display_set_orientation(this->gdev, orientation);
  Serial.println(ret);
  if(ret) {
    Serial.println("\t<err> Failed to set display rotation");
    //return false;
  }

  display_get_capabilities(this->gdev, &capabilities);

	printk("- Capabilities:\n");
	printk("  x_resolution = %u, y_resolution = %u\n, supported_pixel_formats = %u\n"
	       "  current_pixel_format = %u, current_orientation = %u\n",
	       capabilities.x_resolution, capabilities.y_resolution,
	       capabilities.supported_pixel_formats, capabilities.current_pixel_format,
	       capabilities.current_orientation);

  return true;
}

int Display::write8(const uint16_t x,
            const uint16_t y,
            const void *buf) {
              
  return display_write(this->gdev, x, y, this->buf_desc, buf);
  
}

void Display::setFrameDesc(uint16_t w, uint16_t h, uint16_t pitch, uint32_t buf_size) {
	this->buf_desc->buf_size = buf_size;
	this->buf_desc->pitch = w;  /** Number of pixels between consecutive rows in the data buffer */
	this->buf_desc->width = w;  /** Data buffer row width in pixels */
	this->buf_desc->height = h;	/** Data buffer row height in pixels */
    
}

void Display::setFrameComplete(bool status) {

	this->buf_desc->frame_incomplete = status;

}

int Display::setBlanking(bool on) {
  int ret = 0;
  if(on) {
    ret = display_blanking_on(this->gdev);
  } else {
    ret = display_blanking_off(this->gdev);
  }

  return ret;
}
