/*
 * Copyright 2023 Arduino SA
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
 */

/**
 * @file Arduino_GigaDisplayTouch.h
 * @author Leonardo Cavagnis
 * @brief Header file for the Arduino Giga Display Touch library.
 *
 * This library allows to capture up to 5 concurrent touch points on Arduino Giga Display Shield.
 * Supported controller: Goodix GT911
 */

#ifndef __ARDUINO_GIGADISPLAYTOUCH_H
#define __ARDUINO_GIGADISPLAYTOUCH_H

/* Includes ------------------------------------------------------------------*/
#include <Arduino.h>
#include "Wire.h"
//#include "pinDefinitions.h"

/* Exported defines ----------------------------------------------------------*/
#define GT911_I2C_ADDR_BA_BB    (0x5D | 0x80)  // 0xBA/0xBB - 0x5D (7bit address)
#define GT911_I2C_ADDR_28_29    (0x14 | 0x80)  // 0x28/0x29 - 0x14 (7bit address)

#define GT911_CONTACT_SIZE      8
#define GT911_MAX_CONTACTS      5

/* Exported types ------------------------------------------------------------*/
typedef struct GDTpoint_s GDTpoint_t;

/* Exported enumeration ------------------------------------------------------*/

/* Exported struct -----------------------------------------------------------*/

/**
 * @brief Struct representing a touch point.
 */
struct GDTpoint_s {
  // 0x814F-0x8156, ... 0x8176 (5 points) 
  uint8_t trackId;
  uint16_t x;
  uint16_t y;
  uint16_t area;
  uint8_t reserved;
};


/* Class ----------------------------------------------------------------------*/

/**
 * @class Arduino_GigaDisplayTouch
 * @brief Class for Giga Display Touch controller driver.
 */
class Arduino_GigaDisplayTouch {
  public:  
      Arduino_GigaDisplayTouch();
      ~Arduino_GigaDisplayTouch();


      /**
       * @brief Initialize the touch controller.
       *
       * @return true If the touch controller is successfully initialized, false Otherwise
       */
      bool begin();

      /**
       * @brief De-initialize the touch controller.
       */
      void end();

      /**
       * @brief Check if a touch event is detected and get the touch points.
       * @param points The array containing the coordinates of the touch points.
       * @return uint8_t The number of detected touch points.
       */
      uint8_t getTouchPoints(GDTpoint_t* points, uint32_t timeout=0);

      /**
       * @brief Attach an interrupt handler function for touch detection callbacks.
       * @param handler The pointer to the user-defined handler function.
       */
      void onDetect(void (*handler)(uint8_t, GDTpoint_t*));
  private:

};

#endif /* __ARDUINO_GIGADISPLAYTOUCH_H */