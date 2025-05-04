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
 * @file Arduino_GigaDisplayTouch.cpp
 * @author Leonardo Cavagnis
 * @brief Source file for the Arduino Giga Display Touch library.
 */

 /* Includes -----------------------------------------------------------------*/
#include "Arduino_GigaDisplayTouch.h"

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/input/input.h>
#include <zephyr/drivers/display.h>
#include <zephyr/sys/util.h>


// experiment to try to capture touch screen events
extern "C" {
typedef struct {
	size_t x;
	size_t y;
	bool pressed;
} touch_point_t;

extern bool getVideoTouchEvent(touch_point_t *tp, k_timeout_t timeout);
}


/* Functions -----------------------------------------------------------------*/
Arduino_GigaDisplayTouch::Arduino_GigaDisplayTouch()
{ }

Arduino_GigaDisplayTouch::~Arduino_GigaDisplayTouch() 
{ }

bool Arduino_GigaDisplayTouch::begin() {    
    return true;
}


void Arduino_GigaDisplayTouch::end() 
{ }

uint8_t Arduino_GigaDisplayTouch::getTouchPoints(GDTpoint_t* points) {
    touch_point_t tp;
    if (!getVideoTouchEvent(&tp, K_NO_WAIT) || !tp.pressed) return 0;
    points[0].x = tp.x;
    points[0].y = tp.y;
    return 1;
}

#if 0  // from other code.
    uint8_t rawpoints[GT911_MAX_CONTACTS * GT911_CONTACT_SIZE];
    uint8_t contacts;
    uint8_t error;

    contacts    = 0;
    error       = _gt911ReadInputCoord(rawpoints, contacts);

    if (error) {
        return 0;
    }

    for (uint8_t i = 0; i < contacts; i++) {
        points[i].trackId  = rawpoints[1 + 8*i];	    
        points[i].x        = ((uint16_t)rawpoints[3 + 8*i] << 8) + rawpoints[2 + 8*i];
        points[i].y        = ((uint16_t)rawpoints[5 + 8*i] << 8) + rawpoints[4 + 8*i];
        points[i].area     = ((uint16_t)rawpoints[7 + 8*i] << 8) + rawpoints[6 + 8*i];
    }
 
    _gt911WriteOp(GT911_REG_GESTURE_START_POINT, 0); /* Reset buffer status to finish the reading */
    return contacts;
#endif

void Arduino_GigaDisplayTouch::onDetect(void (*handler)(uint8_t, GDTpoint_t*)) {
}


/**** END OF FILE ****/