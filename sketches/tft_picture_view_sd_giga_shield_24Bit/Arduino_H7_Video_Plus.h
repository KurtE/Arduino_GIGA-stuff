#pragma once

// Quick and dirty version to see if I can extend the text output like Arduino to duplicate
// the bitmap to make it bigger.

#include "Arduino_H7_Video.h"
#include "ArduinoGraphics.h"

class Arduino_H7_Video_Plus : public Arduino_H7_Video {
  public:
    Arduino_H7_Video_Plus(int width = 800, int height = 480, H7DisplayShield& shield = GigaDisplayShield)
        : Arduino_H7_Video(width, height, shield) {}
    ~Arduino_H7_Video_Plus() {}

    //
    virtual void text(const char* str, int x = 0, int y = 0);
    virtual void textFont(const Font& which);

    void textSize(uint8_t xs = 1, uint8_t ys = 0);
    void stroke(uint8_t r, uint8_t g, uint8_t b);
    void stroke(uint32_t color);
    void background(uint8_t r, uint8_t g, uint8_t b);
    void background(uint32_t color);
  
    void scaledBitmap(const uint8_t* data, int x, int y, int width, int height);

  protected:
    uint8_t _xTextScale = 1;
    uint8_t _yTextScale = 1;
    const Font* _font;
    uint8_t _strokeR, _strokeG, _strokeB;
    uint8_t _backgroundR, _backgroundG, _backgroundB;
    bool _stroke = false;
};
