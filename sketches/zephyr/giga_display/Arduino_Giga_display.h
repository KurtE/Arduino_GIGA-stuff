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
 */
#ifndef __GIGA_DISPLAY_H__
#define __GIGA_DISPLAY_H__

/** 
 * @enum DisplayPixelFormat
 * @brief Display pixel format enumeration.
 * 
 * The different formats use different numbers of bits per pixel:
 * - Grayscale (8-bit)
 * - RGB565 (16-bit)
 */
enum DisplayPixelFormat {
    DISPLAY_RGB565,    /**< RGB565 format (16-bit). */
    DISPLAY_RGB888    /**< RGB888 format (16-bit). */
};

// Color definitions
#define BLACK       0x0000      /*   0,   0,   0 */
#define NAVY        0x000F      /*   0,   0, 128 */
#define DARKGREEN   0x03E0      /*   0, 128,   0 */
#define DARKCYAN    0x03EF      /*   0, 128, 128 */
#define MAROON      0x7800      /* 128,   0,   0 */
#define PURPLE      0x780F      /* 128,   0, 128 */
#define OLIVE       0x7BE0      /* 128, 128,   0 */
#define LIGHTGREY   0xC618      /* 192, 192, 192 */
#define DARKGREY    0x7BEF      /* 128, 128, 128 */
#define BLUE        0x001F      /*   0,   0, 255 */
#define GREEN       0x07E0      /*   0, 255,   0 */
#define CYAN        0x07FF      /*   0, 255, 255 */
#define RED         0xF800      /* 255,   0,   0 */
#define MAGENTA     0xF81F      /* 255,   0, 255 */
#define YELLOW      0xFFE0      /* 255, 255,   0 */
#define WHITE       0xFFFF      /* 255, 255, 255 */
#define ORANGE      0xFD20      /* 255, 165,   0 */
#define GREENYELLOW 0xAFE5      /* 173, 255,  47 */
#define PINK        0xF81F

/**
 * @class Display
 * @brief The main class for controlling a camera.
 */
class Display {
    private:
      const struct device *gdev;
      struct display_buffer_descriptor *buf_desc;
      
    public:
        /**
         * @brief Construct a new Camera object.
         */
        Display();

        /**
         * @brief Initialize the camera.
         * 
         * @param width Frame width in pixels.
         * @param height Frame height in pixels.
         * @param pixformat Initial pixel format (default: CAMERA_RGB565).
         * @param rotation Intitial rotation of display
         * @return true if the camera is successfully initialized, otherwise false.
         */
        bool begin(DisplayPixelFormat pixformat = DISPLAY_RGB565, int rotation = 0);
        
        /**
         * @brief a frame.
         * 
         * @param fb Reference to a FrameBuffer object to store the frame data.
         * @param timeout Time in milliseconds to wait for a frame (default: 5000).
         * @return true if the frame is successfully captured, otherwise false.
         */
        //bool grabFrame(FrameBuffer &fb, uint32_t timeout = 5000);
        
        /**
         * @brief Display orientation.
         * 
         * @param 0-normal, 1-90 degrees, 2-180 degrees, 3-270 degrees
         * @return true on success, false on failure.
         */
        bool setRotation(int rotation);
        
        /**
         *
         *
         *
         *
         */
         int write8(const uint16_t x,
            const uint16_t y,
            const void *buf);
            
        void setFrameDesc(uint16_t w, uint16_t h, uint16_t pitch, uint32_t buf_size);
        void setFrameComplete(bool status);
        int setBlanking(bool on);
        
};

#endif // __GIGA_DISPLAY_H__