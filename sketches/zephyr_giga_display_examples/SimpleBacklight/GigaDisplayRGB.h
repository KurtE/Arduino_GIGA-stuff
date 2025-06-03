#ifndef GigaDisplayRGB_h
#define GigaDisplayRGB_h

#include "Wire.h"

class GigaDisplayRGB {
    public:
        GigaDisplayRGB();
        void begin();
        void on(uint8_t,uint8_t,uint8_t);
        void off();
    private:
        void writeByte(uint8_t,uint8_t);
};

#include <Arduino.h>

class GigaDisplayBacklight {
    public:
        GigaDisplayBacklight() {}
        void begin(int target_percent = 100) {
            // BUGBUG:: We should get the current device and then from it get the parameter...
            //const struct device *gdev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
            // for now try pin 74
            pinMode(74, OUTPUT);
            k_timer_init(&timer, _timerHandlerDispatch, NULL);
            k_timer_user_data_set(&timer, this);
            k_timer_start(&timer, K_MSEC(2), K_MSEC(2));
        }
        void set(int target_percent) {
            intensity = target_percent;
        }
        void off() {
            set(0);
        }

    protected:
	    void _timerHandler() {
            static int counter = 0;
            if (counter >= intensity) {
                digitalWrite(74, LOW);
            } else {
                digitalWrite(74, HIGH);
            }
            counter += 10;
            if (counter == 100) {
                counter = 0;
            }

        }
	    static void _timerHandlerDispatch(struct k_timer *timer) {
            GigaDisplayBacklight* dev = (GigaDisplayBacklight*)k_timer_user_data_get(timer);
            dev->_timerHandler();
        }

    private:
    	struct k_timer timer;
        int intensity;

};

#endif
