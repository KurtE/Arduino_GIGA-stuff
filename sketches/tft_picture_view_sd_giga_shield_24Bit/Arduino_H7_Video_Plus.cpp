#include "Arduino_H7_Video_Plus.h"
#define COLOR_R(color) (uint8_t(color >> 16))
#define COLOR_G(color) (uint8_t(color >> 8))
#define COLOR_B(color) (uint8_t(color >> 0))

void Arduino_H7_Video_Plus::textFont(const Font& which) {
    _font = &which;
    ArduinoGraphics::textFont(which);
}

void Arduino_H7_Video_Plus::stroke(uint8_t r, uint8_t g, uint8_t b) {
    _stroke = true;
    _strokeR = r;
    _strokeG = g;
    _strokeB = b;
    ArduinoGraphics::stroke(r, g, b);
}

void Arduino_H7_Video_Plus::stroke(uint32_t color) {
    stroke(COLOR_R(color), COLOR_G(color), COLOR_B(color));
}

void Arduino_H7_Video_Plus::background(uint8_t r, uint8_t g, uint8_t b) {
    _backgroundR = r;
    _backgroundG = g;
    _backgroundB = b;
    ArduinoGraphics::background(r, g, b);
}

void Arduino_H7_Video_Plus::background(uint32_t color) {
    background(COLOR_R(color), COLOR_G(color), COLOR_B(color));
}




void Arduino_H7_Video_Plus::textSize(uint8_t xs, uint8_t ys) {
    _xTextScale = xs;
    _yTextScale = ys ? ys : xs;
}

void Arduino_H7_Video_Plus::text(const char* str, int x, int y) {
    if (!_font || !_stroke) {
        return;
    }
    if ((_xTextScale == 1) && (_yTextScale == 1)) {
        ArduinoGraphics::text(str, x, y);
        return;
    }

    while (*str) {
        uint8_t const c = (uint8_t)*str++;

        if (c == '\n') {
            y += _font->height * _yTextScale;
        } else if (c == '\r') {
            x = 0;
        } else if (c == 0xc2 || c == 0xc3) {
            // drop
        } else {
            const uint8_t* b = _font->data[c];

            if (b == NULL) {
                b = _font->data[0x20];
            }

            if (b) {
                scaledBitmap(b, x, y, _font->width, _font->height);
            }

            x += _font->width * _xTextScale;
        }
    }
}
void Arduino_H7_Video_Plus::scaledBitmap(const uint8_t* data, int x, int y, int w, int h) {
    if (!_stroke || !_xTextScale || !_yTextScale) {
        return;
    }

    if ((data == nullptr) || ((x + (w * _xTextScale) < 0)) || ((y + (h * _yTextScale) < 0)) || (x > width()) || (y > height())) {
        // offscreen
        return;
    }

    int xStart = x;
    for (int j = 0; j < h; j++) {
        uint8_t b = data[j];
        for (uint8_t ys = 0; ys < _yTextScale; ys++) {
            if (ys >= height()) return;  //
            x = xStart;                  // reset for each row
            for (int i = 0; i < w; i++) {
                if (b & (1 << (7 - i))) {
                    for (uint8_t xs = 0; xs < _xTextScale; xs++) set(x++, y, _strokeR, _strokeG, _strokeB);
                } else {
                    for (uint8_t xs = 0; xs < _xTextScale; xs++) set(x++, y, _backgroundR, _backgroundG, _backgroundB);
                }
                if (x >= width()) break;
            }
            y++;
        }
    }
}
