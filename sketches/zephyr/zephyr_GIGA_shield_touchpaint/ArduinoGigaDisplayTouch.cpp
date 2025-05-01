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

//LOG_MODULE_REGISTER(sample, LOG_LEVEL_INF);

#if !DT_NODE_EXISTS(DT_CHOSEN(zephyr_touch))
#error "Unsupported board: zephyr,touch is not assigned"
#endif

static const struct device *const touch_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_touch));


static struct {
	size_t x;
	size_t y;
	bool pressed;
} touch_point, touch_point_drawn;

static void touch_event_callback(struct input_event *evt, void *user_data)
{
    printk("touch_event_callback(%p %p): %p %u %u %u %d\n", evt, user_data,
            evt->dev, evt->sync, evt->type, evt->code, evt->value);
#if 0
	if (evt->code == INPUT_ABS_X) {
		touch_point.x = evt->value;
	}
	if (evt->code == INPUT_ABS_Y) {
		touch_point.y = evt->value;
	}
	if (evt->code == INPUT_BTN_TOUCH) {
		touch_point.pressed = evt->value;
	}
	if (evt->sync) {
		k_sem_give(&sync);
	}
#endif
}

INPUT_CALLBACK_DEFINE(touch_dev, touch_event_callback, NULL);



/* Functions -----------------------------------------------------------------*/
Arduino_GigaDisplayTouch::Arduino_GigaDisplayTouch()
{ }

Arduino_GigaDisplayTouch::~Arduino_GigaDisplayTouch() 
{ }

bool Arduino_GigaDisplayTouch::begin() {
    printk("Touch::begin dev:%p %s\n", touch_dev, touch_dev->name);
	if (!device_is_ready(touch_dev)) {
		Serial.print("Touch Device ");
        Serial.print(touch_dev->name);
        Serial.println(" not found. Aborting.");
		return false;
	}
    return true;
}


void Arduino_GigaDisplayTouch::end() 
{ }

uint8_t Arduino_GigaDisplayTouch::getTouchPoints(GDTpoint_t* points) {
#if 0
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
#else
    return 0;
#endif
}

void Arduino_GigaDisplayTouch::onDetect(void (*handler)(uint8_t, GDTpoint_t*)) {
#if 0
    _gt911TouchHandler = handler;
    t.start(callback(&queue, &events::EventQueue::dispatch_forever));
    _irqInt.rise(queue.event(mbed::callback(this, &Arduino_GigaDisplayTouch::_gt911onIrq)));
#endif    
}


/**** END OF FILE ****/