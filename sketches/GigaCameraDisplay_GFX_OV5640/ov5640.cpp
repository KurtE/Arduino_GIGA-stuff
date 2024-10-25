/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for
 * details.
 *
 * OV5640 driver.
 *
 * Note: Some mods made to get JPEG working and other minor things, like
 * special effects, white balance, night mode, hue, sharpness.
 */

#include "ov5640.h"
#include "ov5640_regs.h"

//#define debug Serial
// #define NO_CLK_PIN

#define DEBUG_CAMERA
 #define DEBUG_CAMERA_VERBOSE
// #define DEBUG_FLEXIO
// #define USE_DEBUG_PINS
#define DEBUG_CAMERA_REG
#define USE_VSYNC_PIN_INT

// #define USE_DEBUG_PINS_TIMING

#ifdef USE_DEBUG_PINS_TIMING
#define DBGdigitalWriteFast digitalWriteFast
#define DBGdigitalToggleFast digitalToggleFast
#else
static inline void DBGdigitalWriteFast(uint8_t pin, uint8_t val)
    __attribute__((always_inline, unused));
static inline void DBGdigitalWriteFast(uint8_t pin, uint8_t val) {}
static inline void DBGdigitalToggleFast(uint8_t pin)
    __attribute__((always_inline, unused));
static inline void DBGdigitalToggleFast(uint8_t pin){}
#endif

// if not defined in the variant
#ifndef digitalPinToBitMask
#define digitalPinToBitMask(P) (1 << (digitalPinToPinName(P) % 64))
#endif

#ifndef portInputRegister
#define portInputRegister(P) ((P == 0) ? &NRF_P0->IN : &NRF_P1->IN)
#endif

/** ln(10) */
#ifndef LN10
#define LN10 2.30258509299404568402f
#endif /* !M_LN10 */

/* log_e 2 */
#ifndef LN2
#define LN2 0.69314718055994530942
#endif /*!M_LN2 */

#define LOG2_2(x) (((x) & 0x2ULL) ? (2) : 1) // NO ({ ... }) !
#define LOG2_4(x) \
    (((x) & 0xCULL) ? (2 + LOG2_2((x) >> 2)) : LOG2_2(x)) // NO ({ ... }) !
#define LOG2_8(x) \
    (((x) & 0xF0ULL) ? (4 + LOG2_4((x) >> 4)) : LOG2_4(x)) // NO ({ ... }) !
#define LOG2_16(x) \
    (((x) & 0xFF00ULL) ? (8 + LOG2_8((x) >> 8)) : LOG2_8(x)) // NO ({ ... }) !
#define LOG2_32(x)                                     \
    (((x) & 0xFFFF0000ULL) ? (16 + LOG2_16((x) >> 16)) \
                           : LOG2_16(x)) // NO ({ ... }) !
#define LOG2(x)                                                \
    (((x) & 0xFFFFFFFF00000000ULL) ? (32 + LOG2_32((x) >> 32)) \
                                   : LOG2_32(x)) // NO ({ ... }) !

#define BLANK_LINES 8
#define DUMMY_LINES 6

#define BLANK_COLUMNS 0
#define DUMMY_COLUMNS 8

#define SENSOR_WIDTH 2624
#define SENSOR_HEIGHT 1964

#define ACTIVE_SENSOR_WIDTH (SENSOR_WIDTH - BLANK_COLUMNS - (2 * DUMMY_COLUMNS))
#define ACTIVE_SENSOR_HEIGHT (SENSOR_HEIGHT - BLANK_LINES - (2 * DUMMY_LINES))

#define DUMMY_WIDTH_BUFFER 16
#define DUMMY_HEIGHT_BUFFER 8

#define HSYNC_TIME 252
#define VYSNC_TIME 24

static int16_t readout_x = 0;
static int16_t readout_y = 0;

static uint16_t readout_w = ACTIVE_SENSOR_WIDTH;
static uint16_t readout_h = ACTIVE_SENSOR_HEIGHT;

static uint16_t hts_target = 0;

/*
static const uint8_t af_firmware_command_regs[][3] = {
    {0x30, 0x22, 0x03}, {0x30, 0x23, 0x00}, {0x30, 0x24, 0x00}, {0x30, 0x25, 0x00}, {0x30, 0x26, 0x00}, {0x30, 0x27, 0x00}, {0x30, 0x28, 0x00}, {0x30, 0x29, 0x7f},
    {0x00, 0x00, 0x00}};
*/
#define NUM_BRIGHTNESS_LEVELS (9)

#define NUM_CONTRAST_LEVELS (7)
static const uint8_t contrast_regs[NUM_CONTRAST_LEVELS][1] = {
    {0x14}, /* -3 */
    {0x18}, /* -2 */
    {0x1C}, /* -1 */
    {0x00}, /* +0 */
    {0x10}, /* +1 */
    {0x18}, /* +2 */
    {0x1C}, /* +3 */
};

#define NUM_SATURATION_LEVELS (9)
static const uint8_t saturation_regs[NUM_SATURATION_LEVELS][2] = {
    {0x00, 0x00}, /* -4 */
    {0x10, 0x10}, /* -3 */
    {0x20, 0x20}, /* -2 */
    {0x30, 0x30}, /* -1 */
    {0x40, 0x40}, /* +0 */
    {0x50, 0x50}, /* +1 */
    {0x60, 0x60}, /* +2 */
    {0x70, 0x70}, /* +3 */
    {0x80, 0x80}, /* +4 */
};

#define NUM_WB_MODES (5)
static const uint8_t wb_modes_regs[NUM_WB_MODES][7] = {
    {0x00, 0x04, 0x00, 0x04, 0x00, 0x04, 0x00}, /* auto */
    {0x01, 0x06, 0x1c, 0x04, 0x00, 0x04, 0xf3}, /* sunny */
    {0x01, 0x06, 0x48, 0x04, 0x00, 0x04, 0xd3}, /* cloudy */
    {0x01, 0x05, 0x48, 0x04, 0x00, 0x07, 0xcf}, /* office */
    {0x01, 0x04, 0x10, 0x04, 0x00, 0x08, 0x50}, /* home */
};

#define NUM_SPECIAL_EFFECTS (9)
static const uint8_t special_effects_regs[NUM_SPECIAL_EFFECTS][4] = {
    {0x06, 0x40, 0x10, 0x08}, /* no effect */
    {0x40, 0x08, 0x40, 0x10}, /* negative */
    {0x1e, 0x80, 0x80, 0x08}, /* black and white */
    {0x1e, 0x80, 0xc0, 0x08}, /* reddish */
    {0x1e, 0x60, 0x60, 0x08}, /* greenish */
    {0x1e, 0xa0, 0x40, 0x08}, /* blue */
    {0x1e, 0x40, 0xa0, 0x08}, /* retro */
    {0x1e, 0xf0, 0xf0, 0x08}, /* Overexposure */
    {0x06, 0x40, 0x10, 0x09}  /* Solarize */

};

#define NUM_HUE_LEVELS (12)
static const uint8_t hue_regs[NUM_HUE_LEVELS][3] = {
    {0x80, 0x00, 0x32}, /* -180, -6*/
    {0x64, 0x40, 0x32}, /* -150, -5 */
    {0x40, 0x6F, 0x32}, /* -120, -4*/
    {0x00, 0x80, 0x02}, /* -90, -3*/
    {0x40, 0x6f, 0x02}, /* -60, -2*/
    {0x6f, 0x40, 0x02}, /* -30, -1 */
    {0x80, 0x00, 0x01}, /* 0, 0  */
    {0x6f, 0x40, 0x01}, /* +30, +1 */
    {0x40, 0x6F, 0x01}, /* +60, +2 */
    {0x00, 0x80, 0x31}, /* +90, +3 */
    {0x40, 0x6F, 0x31}, /* +120, +4 */
    {0x6f, 0x40, 0x31}, /* +150, +5 */
};

#define NUM_SHARPNESS_LEVELS (9)
static const uint8_t sharpness_regs[NUM_SHARPNESS_LEVELS][1] = {
    {0x00}, /* OFF */
    {0x02}, /* 1 */
    {0x04},
    {0x08},
    {0x0C},
    {0x10},
    {0x18},
    {0x20},
    {0x14}, /* 8 */
};

/********************************************************/

// const int OV5640_D[8] = {OV5640_D0, OV5640_D1, OV5640_D2, OV5640_D3,
//                          OV5640_D4, OV5640_D5, OV5640_D6, OV5640_D7};

OV5640::OV5640(WIRECLASS &i2c)
    : _i2c(&i2c) 
{
}

#include <stdarg.h>
void debug_printf(Stream *pstream, const char *format, ...) {
    if (pstream == nullptr) return;
    char buffer[256];
    va_list args;
    va_start(args, format);
    int count = vsnprintf(buffer, sizeof(buffer), format, args);
    pstream->write(buffer, count);
    va_end(args);
}

int OV5640::init()
{
    int ret = 0;
    int ret1 = 0;

    debug_printf(_debug, "\nOV5640::init called\n");
    _i2c->begin();
    _i2c->setClock(400000);

    readout_x = 0;
    readout_y = 0;

    readout_w = ACTIVE_SENSOR_WIDTH;
    readout_h = ACTIVE_SENSOR_HEIGHT;

    hts_target = 0;

    // Reset all registers
    ret |= cameraWriteRegister(SCCB_SYSTEM_CTRL_1, 0x11);
    ret |= cameraWriteRegister(SYSTEM_CTROL0, 0x82);

    // Delay 5 ms
    delay(5);

    // Write default registers
    for (int i = 0; default_regs[i][0]; i++) {
        int addr = (default_regs[i][0] << 8) | (default_regs[i][1] << 0);
        int data = default_regs[i][2];

        ret |= cameraWriteRegister(addr, data);
    }

    if (_useAF) {
        cameraWriteRegister(SYSTEM_RESET_00, 0x20);
        if (_debug) _debug->println(".... Loading firmware....");
        // Write firmware

        for (uint16_t i = 0; i < af_firmware_regs_size; i++) {
            ret |= cameraWriteRegister(MCU_FIRMWARE_BASE + i, af_firmware_regs[i]);
        }

        // cameraWriteFirmware();
        //  Configure command and status registers for AF
        cameraWriteRegister(OV5640_CMD_MAIN, 0x03);
        cameraWriteRegister(OV5640_CMD_ACK, 0x00);
        cameraWriteRegister(OV5640_CMD_PARA0, 0x00);
        cameraWriteRegister(OV5640_CMD_PARA1, 0x00);
        cameraWriteRegister(OV5640_CMD_PARA2, 0x00);
        cameraWriteRegister(OV5640_CMD_PARA3, 0x00);
        cameraWriteRegister(OV5640_CMD_PARA4, 0x00);
        cameraWriteRegister(OV5640_CMD_FW_STATUS, 0x7f);
        // release mcu reset
        ret |= cameraWriteRegister(SYSTEM_RESET_00, 0x00);

        ret1 |= checkAFCmdStatus(OV5640_CMD_FW_STATUS, AF_STATUS_S_IDLE);

        if (ret1 < 0) {
            if (_debug)
                _debug->println("AF config failed to start!!");
        }
        ret |= ret1;

        // Per Adafruit OV5640 need to set step size for motor
        // cameraWriteRegister(OV5640_CMD_PARA3, 0x00);
        // cameraWriteRegister(OV5640_CMD_PARA4, 180); // step size 0-255?
        // cameraWriteRegister(OV5640_CMD_ACK, 0x01);
        // cameraWriteRegister(OV5640_CMD_MAIN, AF_SET_VCM_STEP);
        // ret |= checkAFCmdStatus(OV5640_CMD_ACK, 0x00);
    }

//    // experiment set the second bit
//    uint8_t scb1;
//    cameraReadRegister(SCCB_SYSTEM_CTRL_1, scb1);
//    cameraWriteRegister(SCCB_SYSTEM_CTRL_1, scb1 | 0x2);

    delay(300);
    debug_printf(_debug, "\nOV5640::init exit\n");
    return ret;
}

int OV5640::reset() {
    return 0;
}

#ifdef DEBUG_CAMERA_REG
uint32_t touched_registers[400] = {0};


inline void set_camera_register_touched(uint16_t reg) {
    if (reg < 0x3000 ) return;
    reg -= 0x3000;
    touched_registers[reg >> 5] |= 1 << (reg & 0x1f);
}

inline bool was_camera_register_touched(uint16_t reg) {
    if (reg < 0x3000 ) return false;
    reg -= 0x3000;
    return (touched_registers[reg >> 5] &  (1 << (reg & 0x1f))) ? true : false;
}
#endif


// Read a single uint8_t from address and return it as a uint8_t
uint8_t OV5640::cameraReadRegister(uint16_t reg_addr, uint8_t &reg_data, bool debug_output) {
    _i2c->beginTransmission(0x3C);
    _i2c->write(reg_addr >> 8);
    _i2c->write(reg_addr);
    if (_i2c->endTransmission(false) != 0) {
        if (_debug)
            _debug->println("error reading OV5640, address");
        return 0;
    }
    if (_i2c->requestFrom(0x3C, 1) < 1) {
        Serial.println("error reading OV5640, data");
        return 0;
    }
    int ret = _i2c->read();
    #ifdef DEBUG_CAMERA_REG
    set_camera_register_touched(reg_addr);
    if (debug_output) {
        debug_printf(_debug, "CAM read reg: %04x = %02x", reg_addr, ret);
        for (uint16_t ii = 0; ii < CNT_REG_NAME_TABLE; ii++) {
            if (OV5640_reg_name_table[ii].reg == reg_addr) {
                debug_printf(_debug, "\t//%s", OV5640_reg_name_table[ii].reg_name);
                break;
            }
        }
        _debug->println();
    }
    #endif
    delay(1);
    if (ret < 0) {
        return 1;
    } else {
        uint8_t ret_data = ret;
        reg_data = ret_data;
    }
    return 0;
}

uint8_t OV5640::cameraWriteRegister(uint16_t reg, uint8_t data) {
#ifdef DEBUG_CAMERA_REG
    set_camera_register_touched(reg);
    debug_printf(_debug, "CAM set reg: %04x = %02x", reg, data);
    for (uint16_t ii = 0; ii < CNT_REG_NAME_TABLE; ii++) {
        if (OV5640_reg_name_table[ii].reg == reg) {
            debug_printf(_debug, "\t//%s", OV5640_reg_name_table[ii].reg_name);
            break;
        }
    }
    _debug->println();
#endif
    _i2c->beginTransmission(0x3C);
    _i2c->write(reg >> 8);
    _i2c->write(reg);
    _i2c->write(data);
    if (_i2c->endTransmission() != 0) {
        if (_debug)
            _debug->println("error writing to OV5640");
        return 1;
    }
    return 0;
}

uint8_t OV5640::cameraWriteFirmware() {
    uint32_t write_count = 0;
    uint16_t reg = 0;

    /* try this next */
    uint8_t cb_write = 128;
    uint16_t cb_left = sizeof(af_firmware_regs_size);
    const uint8_t *pb = af_firmware_regs;

    while (cb_left) {
        if (cb_left < cb_write)
            cb_left = cb_write;
        _i2c->beginTransmission(0x3C);
        reg = MCU_FIRMWARE_BASE + write_count * cb_write;
        _i2c->write(reg >> 8);
        _i2c->write(reg);
        _i2c->write(pb, cb_write);
        _i2c->endTransmission(cb_write == cb_left);
        pb += cb_write;
        cb_left -= cb_write;
        write_count += 1;
    }

    return 0;
}


// HTS (Horizontal Time) is the readout width plus the HSYNC_TIME time. However,
// if this value gets too low the OV5640 will crash. The minimum was determined
// empirically with testing... Additionally, when the image width gets too large
// we need to slow down the line transfer rate by increasing HTS so that
// DCMI_DMAConvCpltUser() can keep up with the data rate.
//
// WARNING! IF YOU CHANGE ANYTHING HERE RETEST WITH **ALL** RESOLUTIONS FOR THE
// AFFECTED MODE!
int OV5640::calculate_hts(uint16_t width) {
    uint16_t hts = hts_target;

    if ((_format == CAMERA_GRAYSCALE) || (_format == CAMERA_BAYER) || (_format == CAMERA_BAYER)) {
        if (width <= 1280) {
            hts = max((width * 2) + 8, hts_target);
        }
    } else {
        if (width > 640) {
            hts = max((width * 2) + 8, hts_target);
        }
    }

    if (width <= 640) {
        hts += 160; // Fix image quality at low resolutions.
    }
    return max(hts + HSYNC_TIME,
               (SENSOR_WIDTH + HSYNC_TIME) / 2); // Fix to prevent crashing.
}

// VTS (Vertical Time) is the readout height plus the VYSNC_TIME time. However,
// if this value gets too low the OV5640 will crash. The minimum was determined
// empirically with testing...
//
// WARNING! IF YOU CHANGE ANYTHING HERE RETEST WITH **ALL** RESOLUTIONS FOR THE
// AFFECTED MODE!
int OV5640::calculate_vts(uint16_t readout_height) {
    return max(readout_height + VYSNC_TIME,
               (SENSOR_HEIGHT + VYSNC_TIME) / 8); // Fix to prevent crashing.
}

uint16_t OV5640::getModelid() {
    uint8_t Data = 0;
    uint16_t MID = 0x0000;

    cameraReadRegister(0x300A, Data);
    MID = (Data << 8);

    cameraReadRegister(0x300B, Data);
    MID |= Data;
    return MID;
}


int OV5640::checkAFCmdStatus(uint16_t reg, uint8_t value) {
    uint8_t sensor = 0xFF;
    uint32_t timeout = millis();
    while (sensor != value) {
        cameraReadRegister(reg, sensor);
        // Serial.println(sensor, HEX);
        delay(1);
        if ((millis() - timeout) > 5000) {
            if (_debug) {
                _debug->print("AF command: 0x");
                _debug->print(value, HEX);
                _debug->println(" failed to start!!");
            }
            return -1;
        }
    }
    return 0;
}

int OV5640::setResolution(int32_t resolution)
{
    return setResolutionWithZoom(resolution, resolution, 0, 0);
}

int OV5640::setResolutionWithZoom(int32_t resolution, int32_t zoom_resolution, uint32_t zoom_x, uint32_t zoom_y)
{
    int ret = 0;

    // lets try to handle more than the default sets of resolutions.

    // out of range.
    if (resolution >= _resolutions_count) return -1;
    if (_resolutions[resolution][0] == 0) return -1;  // unused internal index.
    _framesize = resolution;

    // Will handle zoom later...
    if (resolution != zoom_resolution)
    {
        return -1;
    }

    ret |= setFramesize(_resolutions[resolution][0], _resolutions[resolution][1]);

    return ret;
}

int OV5640::setPixelFormat(int32_t pixformat) {
    debug_printf(_debug, "setPixelFormat(%x)\n", pixformat);
    // const uint8_t(*regs)[2];
    int ret = 0;
    uint8_t reg = 0;

    // Not a multiple of 8. The JPEG encoder on the OV5640 can't handle this.
    if ((_format == CAMERA_JPEG) &&
        ((_resolutions[_framesize][0] % 8) || (_resolutions[_framesize][1] % 8))) {
        if (_debug) {
            _debug->println("JPEG Framesize not divisible by 8........");

        }
        return 1;
    }

    switch (pixformat) {
    case CAMERA_GRAYSCALE:
        ret |= cameraWriteRegister(FORMAT_CONTROL, 0x10);
        ret |= cameraWriteRegister(FORMAT_CONTROL_MUX, 0x00);
        break;
    case CAMERA_RGB565:
        ret |= cameraWriteRegister(FORMAT_CONTROL, 0x6F);
        ret |= cameraWriteRegister(FORMAT_CONTROL_MUX, 0x01);
        break;
    //case YUV422:
    //    ret |= cameraWriteRegister(FORMAT_CONTROL, 0x30);
    //    ret |= cameraWriteRegister(FORMAT_CONTROL_MUX, 0x00);
    //    break;
    case CAMERA_BAYER:
        ret |= cameraWriteRegister(FORMAT_CONTROL, 0x00);
        ret |= cameraWriteRegister(FORMAT_CONTROL_MUX, 0x01);
        break;
    case CAMERA_JPEG: {
        ret |= cameraWriteRegister(FORMAT_CONTROL, 0x30);
        ret |= cameraWriteRegister(FORMAT_CONTROL_MUX, 0x00);
        ret |= cameraReadRegister(JPEG_MODE_SEL, reg);
        reg &= ~(0x07);
        uint8_t val = 0x03;
        val &= (0x07);
        val |= reg;
        ret |= cameraWriteRegister(JPEG_MODE_SEL, val);
        cameraWriteRegister(TIMING_TC_REG_20, 0x40);
        cameraWriteRegister(SC_PLL_CONTRL2, 0x69);
        cameraWriteRegister(SC_PLL_CONTRL1, 0x31);
    } break;
    default:
        return 1;
    }

    if (_debug) {
        _debug->print("Format: ");
        _debug->print(pixformat);
        _debug->print(", Framesize: ");
        _debug->println( _framesize);
    }

    ret |= cameraReadRegister(TIMING_TC_REG_21, reg);
    ret |= cameraWriteRegister(
        TIMING_TC_REG_21, (reg & 0xDF) | ((pixformat == CAMERA_JPEG) ? 0x26 : 0x00));

    ret |= cameraReadRegister(SYSTEM_RESET_02, reg);
    ret |= cameraWriteRegister(
        SYSTEM_RESET_02, (reg & 0xE3) | ((pixformat == CAMERA_JPEG) ? 0x00 : 0x1C));

    ret |= cameraReadRegister(CLOCK_ENABLE_02, reg);
    ret |= cameraWriteRegister(
        CLOCK_ENABLE_02, (reg & 0xD7) | ((pixformat == CAMERA_JPEG) ? 0x28 : 0x00));

    if (hts_target) {
        uint16_t sensor_hts = calculate_hts(_resolutions[_framesize][0]);

        ret |= cameraWriteRegister(TIMING_HTS_H, sensor_hts >> 8);
        ret |= cameraWriteRegister(TIMING_HTS_L, sensor_hts);
    }

    _format = int(pixformat);
    return ret;
}


uint8_t OV5640::setFramesize(int w, int h) {
    uint16_t sensor_w = 0;
    uint16_t sensor_h = 0;

    if ((w == 0) || (h == 0))
        return 1; // not valid

//    _width = w;
//    _height = h;

    debug_printf(_debug, "\nOV5640::setFramesize(%u, %u)/n", w, h);
    uint8_t reg = 0;
    int ret = 0;
    // uint16_t w = _resolutions[framesize][0];
    // uint16_t h = _resolutions[framesize][1];

    // Not a multiple of 8. The CAMERA_JPEG encoder on the OV5640 can't handle this.
    if ((_format == CAMERA_JPEG) && ((w % 8) || (h % 8))) {
        if (_debug)
            _debug->print("JPEG not multiple of 8: failed to set.......");
        return 1;
    }

    // Readout speed too fast. The DCMI_DMAConvCpltUser() line callback overhead
    // is too much to handle the line transfer speed. If we were to slow the
    // pixclk down these resolutions would work. As of right now, the image
    // shakes and scrolls with the current line transfer speed. Note that
    // there's an overhead to the DCMI_DMAConvCpltUser() function. It's not the
    // memory copy operation that's too slow. It's that there's too much
    // overhead in the DCMI_DMAConvCpltUser() method to even have time to start
    // the line transfer. If it were possible to slow the line readout speed of
    // the OV5640 this would enable these resolutions below. However, there's
    // nothing in the datasheet that when modified does this.
    // if (((_format == CAMERA_GRAYSCALE) || (_format == CAMERA_BAYER) || (_format == JPEG)))
    // {
    //    return 1;
    //}

    // Invalid resolution.

#if 0
    if ((w > ACTIVE_SENSOR_WIDTH) || (h > ACTIVE_SENSOR_HEIGHT)) {
        return 1;
    }
#endif

    // Step 0: Clamp readout settings.

    readout_w = max(readout_w, w);
    readout_h = max(readout_h, h);

    int readout_x_max = (ACTIVE_SENSOR_WIDTH - readout_w) / 2;
    int readout_y_max = (ACTIVE_SENSOR_HEIGHT - readout_h) / 2;
    readout_x = max(min(readout_x, readout_x_max), -readout_x_max);
    readout_y = max(min(readout_y, readout_y_max), -readout_y_max);

    // Step 1: Determine readout area and subsampling amount.

    uint16_t sensor_div = 0;

    if ((w > (readout_w / 2)) || (h > (readout_h / 2))) {
        sensor_div = 1;
    } else {
        sensor_div = 2;
    }

    // Step 2: Determine horizontal and vertical start and end points.

    sensor_w = readout_w +
               DUMMY_WIDTH_BUFFER; // camera hardware needs dummy pixels to sync
    sensor_h = readout_h +
               DUMMY_HEIGHT_BUFFER; // camera hardware needs dummy lines to sync

    uint16_t sensor_ws =
        max(min((((ACTIVE_SENSOR_WIDTH - sensor_w) / 4) + (readout_x / 2)) * 2,
                ACTIVE_SENSOR_WIDTH - sensor_w),
            -(DUMMY_WIDTH_BUFFER / 2)) +
        DUMMY_COLUMNS; // must be multiple of 2
    uint16_t sensor_we = sensor_ws + sensor_w - 1;

    uint16_t sensor_hs =
        max(min((((ACTIVE_SENSOR_HEIGHT - sensor_h) / 4) - (readout_y / 2)) * 2,
                ACTIVE_SENSOR_HEIGHT - sensor_h),
            -(DUMMY_HEIGHT_BUFFER / 2)) +
        DUMMY_LINES; // must be multiple of 2
    uint16_t sensor_he = sensor_hs + sensor_h - 1;

    // Step 3: Determine scaling window offset.

    float ratio = min((readout_w / sensor_div) / ((float)w),
                      (readout_h / sensor_div) / ((float)h));

    uint16_t w_mul = w * ratio;
    uint16_t h_mul = h * ratio;
    uint16_t x_off = ((sensor_w / sensor_div) - w_mul) / 2;
    uint16_t y_off = ((sensor_h / sensor_div) - h_mul) / 2;

    // Step 4: Compute total frame time.

    hts_target = sensor_w / sensor_div;

    uint16_t sensor_hts = calculate_hts(w);
    uint16_t sensor_vts = calculate_vts(sensor_h / sensor_div);

    uint16_t sensor_x_inc =
        (((sensor_div * 2) - 1) << 4) |
        (1 << 0); // odd[7:4]/even[3:0] pixel inc on the bayer pattern
    uint16_t sensor_y_inc =
        (((sensor_div * 2) - 1) << 4) |
        (1 << 0); // odd[7:4]/even[3:0] pixel inc on the bayer pattern

    if (_debug) {
        _debug->println("Setting FrameSize:");
        _debug->println("  Step 0:");
        debug_printf(_debug, "   readout_w: %d, readout_h: %d, readout_x_max: %d, "
                     "readout_x_max: %d\n",
                     readout_w, readout_h, readout_x_max, readout_y_max);
        debug_printf(_debug, "   readout_x: %d, readout_y: %d\n", readout_x, readout_y);
        _debug->println("  Step 1:");
        debug_printf(_debug, "   sensor_div: %d\n", sensor_div);
        _debug->println("  Step 2:");
        debug_printf(_debug, 
            "  sensor_w: %d, sensor_h: %d, sensor_ws: %d, sensor_we: %d\n",
            sensor_w, sensor_h, sensor_ws, sensor_we);
        debug_printf(_debug, "  sensor_hs: %d, sensor_hs: %d\n", sensor_hs, sensor_he);
        debug_printf(_debug, "  Step 3:");
        debug_printf(_debug, "  ratio: %f, w_mul: %d, h_mul: %d\n", ratio, w_mul,
                     h_mul);
        debug_printf(_debug, "  x_off: %d, y_off: %d\n", x_off, y_off);
        debug_printf(_debug, "  Step 4:");
        debug_printf(_debug, "  hts_target: %d, sensor_hts: %d, sensor_vts: %d: %d\n",
                     hts_target, sensor_hts, sensor_vts);
        debug_printf(_debug, "  sensor_x_inc: %d, sensor_x_inc:%d\n", sensor_x_inc,
                     sensor_x_inc);
    }

    // Step 5: Write regs.

    ret |= cameraWriteRegister(TIMING_HS_H, sensor_ws >> 8);
    ret |= cameraWriteRegister(TIMING_HS_L, sensor_ws);

    ret |= cameraWriteRegister(TIMING_VS_H, sensor_hs >> 8);
    ret |= cameraWriteRegister(TIMING_VS_L, sensor_hs);

    ret |= cameraWriteRegister(TIMING_HW_H, sensor_we >> 8);
    ret |= cameraWriteRegister(TIMING_HW_L, sensor_we);

    ret |= cameraWriteRegister(TIMING_VH_H, sensor_he >> 8);
    ret |= cameraWriteRegister(TIMING_VH_L, sensor_he);

    ret |= cameraWriteRegister(TIMING_DVPHO_H, w >> 8);
    ret |= cameraWriteRegister(TIMING_DVPHO_L, w);

    ret |= cameraWriteRegister(TIMING_DVPVO_H, h >> 8);
    ret |= cameraWriteRegister(TIMING_DVPVO_L, h);

    ret |= cameraWriteRegister(TIMING_HTS_H, sensor_hts >> 8);
    ret |= cameraWriteRegister(TIMING_HTS_L, sensor_hts);

    ret |= cameraWriteRegister(TIMING_VTS_H, sensor_vts >> 8);
    ret |= cameraWriteRegister(TIMING_VTS_L, sensor_vts);

    ret |= cameraWriteRegister(TIMING_HOFFSET_H, x_off >> 8);
    ret |= cameraWriteRegister(TIMING_HOFFSET_L, x_off);

    ret |= cameraWriteRegister(TIMING_VOFFSET_H, y_off >> 8);
    ret |= cameraWriteRegister(TIMING_VOFFSET_L, y_off);

    ret |= cameraWriteRegister(TIMING_X_INC, sensor_x_inc);
    ret |= cameraWriteRegister(TIMING_Y_INC, sensor_y_inc);

    ret |= cameraReadRegister(TIMING_TC_REG_20, reg);
    ret |= cameraWriteRegister(TIMING_TC_REG_20, (reg & 0xFE) | (sensor_div > 1));

    ret |= cameraReadRegister(TIMING_TC_REG_21, reg);
    ret |= cameraWriteRegister(TIMING_TC_REG_21, (reg & 0xFE) | (sensor_div > 1));

    ret |= cameraWriteRegister(VFIFO_HSIZE_H, w >> 8);
    ret |= cameraWriteRegister(VFIFO_HSIZE_L, w);

    ret |= cameraWriteRegister(VFIFO_VSIZE_H, h >> 8);
    ret |= cameraWriteRegister(VFIFO_VSIZE_L, h);

    return ret;
}

void OV5640::enableAutoFocus(bool useAF) {
    if (useAF == true) {
        _useAF = true;
    } else {
        _useAF = false;
    }
}

void OV5640::setContrast(int level) {
    int ret = 0;

    int new_level = level + (NUM_CONTRAST_LEVELS / 2);
    if (new_level < 0 || new_level >= NUM_CONTRAST_LEVELS) {
        new_level = 3;
    }

    ret |= cameraWriteRegister(0x3212, 0x03); // start group 3
    ret |= cameraWriteRegister(0x5586, (new_level + 5) << 2);
    ret |= cameraWriteRegister(0x5585, contrast_regs[new_level][0]);
    ret |= cameraWriteRegister(0x3212, 0x13); // end group 3
    ret |= cameraWriteRegister(0x3212, 0xa3); // launch group 3

    // return ret;
}

int OV5640::setBrightness(int level) {
    int ret = 0;

    int new_level = level + (NUM_BRIGHTNESS_LEVELS / 2);
    if (new_level < 0 || new_level >= NUM_BRIGHTNESS_LEVELS) {
        new_level = 5;
    }

    ret |= cameraWriteRegister(0x3212, 0x03); // start group 3
    ret |= cameraWriteRegister(0x5587, abs(new_level) << 4);
    ret |= cameraWriteRegister(0x5588, (new_level < 0) ? 0x09 : 0x01);
    ret |= cameraWriteRegister(0x3212, 0x13); // end group 3
    ret |= cameraWriteRegister(0x3212, 0xa3); // launch group 3

    return ret;
}

void OV5640::setSaturation(int level) {
    int ret = 0;

    int new_level = level + (NUM_SATURATION_LEVELS / 2);
    if (new_level < 0 || new_level >= NUM_SATURATION_LEVELS) {
        new_level = 5;
    }

    ret |= cameraWriteRegister(0x3212, 0x03); // start group 3
    ret |= cameraWriteRegister(0x5001, 0xFF);
    ret |= cameraWriteRegister(0x5583, saturation_regs[new_level][0]);
    ret |= cameraWriteRegister(0x5584, saturation_regs[new_level][1]);
    ret |= cameraWriteRegister(0x5580, 0x02);
    ret |= cameraWriteRegister(0x5588, 0x41);
    ret |= cameraWriteRegister(0x3212, 0x13); // end group 3
    ret |= cameraWriteRegister(0x3212, 0xa3); // launch group 3

    // return ret;
}

int OV5640::setGainceiling(gainceiling_t gainceiling) {
    uint8_t reg = 0;
    int ret = 0;

    int new_gainceiling = 16 << (gainceiling + 1);
    if (new_gainceiling >= 1024) {
        return 1;
    }

    ret |= cameraReadRegister(AEC_GAIN_CEILING_H, reg);
    ret |= cameraWriteRegister(AEC_GAIN_CEILING_H,
                               (reg & 0xFC) | (new_gainceiling >> 8));
    ret |= cameraWriteRegister(AEC_GAIN_CEILING_L, new_gainceiling);

    return ret;
}

int OV5640::setQuality(int qs) {
    uint8_t reg = 0;
    int ret = cameraReadRegister(JPEG_CTRL07, reg);
    ret |= cameraWriteRegister(JPEG_CTRL07, (reg & 0xC0) | (qs >> 2));

    return ret;
}

uint8_t OV5640::getQuality() {
    // int ret = 0;
    uint8_t reg = 0;

    /* Write QS register */
    cameraReadRegister(JPEG_CTRL07, reg);
    return reg;
}

int OV5640::setColorbar(int enable) {
    uint8_t reg = 0;
    int ret = cameraReadRegister(PRE_ISP_TEST, reg);
    return cameraWriteRegister(PRE_ISP_TEST,
                               (reg & 0x7F) | (enable ? 0x80 : 0x00)) |
           ret;
}


int OV5640::setAutoExposure(int enable, int exposure_us) {
    uint8_t reg = 0;
    uint8_t spc0 = 0;
    uint8_t spc1 = 0;
    uint8_t spc2 = 0;
    uint8_t spc3 = 0;
    uint8_t sysrootdiv = 0;
    uint8_t hts_h = 0;
    uint8_t hts_l = 0;
    uint8_t vts_h = 0;
    uint8_t vts_l;

    int ret = cameraReadRegister(AEC_PK_MANUAL, reg);
    ret |=
        cameraWriteRegister(AEC_PK_MANUAL, (reg & 0xFE) | ((enable == 0) << 0));

    if ((enable == 0) && (exposure_us >= 0)) {
        ret |= cameraReadRegister(SC_PLL_CONTRL0, spc0);
        ret |= cameraReadRegister(SC_PLL_CONTRL1, spc1);
        ret |= cameraReadRegister(SC_PLL_CONTRL2, spc2);
        ret |= cameraReadRegister(SC_PLL_CONTRL3, spc3);
        ret |= cameraReadRegister(SYSTEM_ROOT_DIVIDER, sysrootdiv);

        ret |= cameraReadRegister(TIMING_HTS_H, hts_h);
        ret |= cameraReadRegister(TIMING_HTS_L, hts_l);

        ret |= cameraReadRegister(TIMING_VTS_H, vts_h);
        ret |= cameraReadRegister(TIMING_VTS_L, vts_l);

        uint16_t hts = (hts_h << 8) | hts_l;
        uint16_t vts = (vts_h << 8) | vts_l;

        int pclk_freq = calc_pclk_freq(spc0, spc1, spc2, spc3, sysrootdiv);
        int clocks_per_us = pclk_freq / 1000000;
        int exposure =
            max(min((exposure_us * clocks_per_us) / hts, 0xFFFF), 0x0000);

        int new_vts = max(exposure, vts);

        ret |= cameraWriteRegister(AEC_PK_EXPOSURE_0, exposure >> 12);
        ret |= cameraWriteRegister(AEC_PK_EXPOSURE_1, exposure >> 4);
        ret |= cameraWriteRegister(AEC_PK_EXPOSURE_2, exposure << 4);

        ret |= cameraWriteRegister(TIMING_VTS_H, new_vts >> 8);
        ret |= cameraWriteRegister(TIMING_VTS_L, new_vts);
    }

    return ret;
}

int OV5640::getExposure_us(int *exposure_us) {
    uint8_t spc0 = 0;
    uint8_t spc1 = 0;
    uint8_t spc2 = 0;
    uint8_t spc3 = 0;
    uint8_t sysrootdiv = 0;
    uint8_t hts_h = 0;
    uint8_t hts_l = 0;
    uint8_t vts_h = 0;
    uint8_t vts_l;
    uint8_t aec_0 = 0;
    uint8_t aec_1 = 0;
    uint8_t aec_2 = 0;
    int ret = 0;

    ret |= cameraReadRegister(SC_PLL_CONTRL0, spc0);
    ret |= cameraReadRegister(SC_PLL_CONTRL1, spc1);
    ret |= cameraReadRegister(SC_PLL_CONTRL2, spc2);
    ret |= cameraReadRegister(SC_PLL_CONTRL3, spc3);
    ret |= cameraReadRegister(SYSTEM_ROOT_DIVIDER, sysrootdiv);

    ret |= cameraReadRegister(AEC_PK_EXPOSURE_0, aec_0);
    ret |= cameraReadRegister(AEC_PK_EXPOSURE_1, aec_1);
    ret |= cameraReadRegister(AEC_PK_EXPOSURE_2, aec_2);

    ret |= cameraReadRegister(TIMING_HTS_H, hts_h);
    ret |= cameraReadRegister(TIMING_HTS_L, hts_l);

    ret |= cameraReadRegister(TIMING_VTS_H, vts_h);
    ret |= cameraReadRegister(TIMING_VTS_L, vts_l);

    uint32_t aec = ((aec_0 << 16) | (aec_1 << 8) | aec_2) >> 4;
    uint16_t hts = (hts_h << 8) | hts_l;
    uint16_t vts = (vts_h << 8) | vts_l;

    aec = min(aec, vts);

    int pclk_freq = calc_pclk_freq(spc0, spc1, spc2, spc3, sysrootdiv);
    int clocks_per_us = pclk_freq / 1000000;
    *exposure_us = (aec * hts) / clocks_per_us;

    return ret;
}

int OV5640::setAutoGain(int enable, float gain_db, float gain_db_ceiling) {
    uint8_t reg = 0;
    int ret = cameraReadRegister(AEC_PK_MANUAL, reg);
    ret |=
        cameraWriteRegister(AEC_PK_MANUAL, (reg & 0xFD) | ((enable == 0) << 1));

    if ((enable == 0) && (!isnanf(gain_db)) && (!isinff(gain_db))) {
        int gain = max(
            min(fast_roundf(expf((gain_db / 20.0f) * M_LN10) * 16.0f), 1023),
            0);

        ret |= cameraReadRegister(AEC_PK_REAL_GAIN_H, reg);
        ret |=
            cameraWriteRegister(AEC_PK_REAL_GAIN_H, (reg & 0xFC) | (gain >> 8));
        ret |= cameraWriteRegister(AEC_PK_REAL_GAIN_L, gain);
    } else if ((enable != 0) && (!isnanf(gain_db_ceiling)) &&
               (!isinff(gain_db_ceiling))) {
        int gain_ceiling = max(
            min(fast_roundf(expf((gain_db_ceiling / 20.0f) * M_LN10) * 16.0f),
                1023),
            0);

        ret |= cameraReadRegister(AEC_GAIN_CEILING_H, reg);
        ret |= cameraWriteRegister(AEC_GAIN_CEILING_H,
                                   (reg & 0xFC) | (gain_ceiling >> 8));
        ret |= cameraWriteRegister(AEC_GAIN_CEILING_L, gain_ceiling);
    }

    return ret;
}

int OV5640::getGain_db(float *gain_db) {
    uint8_t gainh = 0;
    uint8_t gainl = 0;

    int ret = cameraReadRegister(AEC_PK_REAL_GAIN_H, gainh);
    ret |= cameraReadRegister(AEC_PK_REAL_GAIN_L, gainl);

    *gain_db = 20.0f * log10f((((gainh & 0x3) << 8) | gainl) / 16.0f);

    return ret;
}

int OV5640::setAutoWhitebal(int enable, float r_gain_db, float g_gain_db,
                            float b_gain_db) {
    uint8_t reg = 0;
    int ret = cameraReadRegister(AWB_MANUAL_CONTROL, reg);
    ret |=
        cameraWriteRegister(AWB_MANUAL_CONTROL, (reg & 0xFE) | (enable == 0));

    if ((enable == 0) && (!isnanf(r_gain_db)) && (!isnanf(g_gain_db)) &&
        (!isnanf(b_gain_db)) && (!isinff(r_gain_db)) && (!isinff(g_gain_db)) &&
        (!isinff(b_gain_db))) {

        int r_gain =
            max(min(fast_roundf(expf((r_gain_db / 20.0f) * M_LN10)), 4095), 0);
        int g_gain =
            max(min(fast_roundf(expf((g_gain_db / 20.0f) * M_LN10)), 4095), 0);
        int b_gain =
            max(min(fast_roundf(expf((b_gain_db / 20.0f) * M_LN10)), 4095), 0);

        ret |= cameraWriteRegister(AWB_R_GAIN_H, r_gain >> 8);
        ret |= cameraWriteRegister(AWB_R_GAIN_L, r_gain);
        ret |= cameraWriteRegister(AWB_G_GAIN_H, g_gain >> 8);
        ret |= cameraWriteRegister(AWB_G_GAIN_L, g_gain);
        ret |= cameraWriteRegister(AWB_B_GAIN_H, b_gain >> 8);
        ret |= cameraWriteRegister(AWB_B_GAIN_L, b_gain);
    }

    return ret;
}

int OV5640::getRGB_Gain_db(float *r_gain_db, float *g_gain_db,
                           float *b_gain_db) {
    uint8_t redh = 0;
    uint8_t redl = 0;
    uint8_t greenh = 0;
    uint8_t greenl = 0;
    uint8_t blueh = 0;
    uint8_t bluel = 0;

    int ret = cameraReadRegister(AWB_R_GAIN_H, redh);
    ret |= cameraReadRegister(AWB_R_GAIN_L, redl);
    ret |= cameraReadRegister(AWB_G_GAIN_H, greenh);
    ret |= cameraReadRegister(AWB_G_GAIN_L, greenl);
    ret |= cameraReadRegister(AWB_B_GAIN_H, blueh);
    ret |= cameraReadRegister(AWB_B_GAIN_L, bluel);

    *r_gain_db = 20.0f * log10f(((redh & 0xF) << 8) | redl);
    *g_gain_db = 20.0f * log10f(((greenh & 0xF) << 8) | greenl);
    *b_gain_db = 20.0f * log10f(((blueh & 0xF) << 8) | bluel);

    return ret;
}

int OV5640::setHorizontalMirror(bool enable) {
    uint8_t reg = 0;
    int ret = cameraReadRegister(TIMING_TC_REG_21, reg);
    if (enable) {
        ret |= cameraWriteRegister(TIMING_TC_REG_21, reg | 0x06);
    } else {
        ret |= cameraWriteRegister(TIMING_TC_REG_21, reg & 0xF9);
    }
    return ret;
}

int OV5640::setVerticalFlip(bool enable) {
    uint8_t reg = 0;
    int ret = cameraReadRegister(TIMING_TC_REG_20, reg);
    if (!enable) {
        ret |= cameraWriteRegister(TIMING_TC_REG_20, reg | 0x06);
    } else {
        ret |= cameraWriteRegister(TIMING_TC_REG_20, reg & 0xF9);
    }
    return ret;
}

void OV5640::setHue(int hue) {
    int ret = 0;
    int new_level = hue + (NUM_HUE_LEVELS / 2);
    if (new_level < 0 || new_level >= NUM_HUE_LEVELS) {
        new_level = 6;
    }

    ret |= cameraWriteRegister(ISP_CONTROL_01, 0xFF);
    ret |= cameraWriteRegister(SDE_CTRL0, 0x01);
    ret |= cameraWriteRegister(SDE_CTRL6, hue_regs[new_level][0]);
    ret |= cameraWriteRegister(SDE_CTRL5, hue_regs[new_level][1]);
    ret |= cameraWriteRegister(SDE_CTRL2, hue_regs[new_level][2]);
    ret |= cameraWriteRegister(SDE_CTRL8, 0xA3);

    if (ret < 0)
        debug_printf(_debug, "HUE NOT SET!!!");
}

/*  Functions specific to the OV5640 */
int OV5640::setWBmode(int mode) {
    int ret = 0;
    if (mode < 0 || mode > NUM_WB_MODES) {
        ret |= cameraWriteRegister(0x3212, 0x03); // start group 3
        ret |= cameraWriteRegister(0x3406, wb_modes_regs[0][0]);
        ret |= cameraWriteRegister(0x3400, wb_modes_regs[0][1]);
        ret |= cameraWriteRegister(0x3401, wb_modes_regs[0][2]);
        ret |= cameraWriteRegister(0x3402, wb_modes_regs[0][3]);
        ret |= cameraWriteRegister(0x3403, wb_modes_regs[0][4]);
        ret |= cameraWriteRegister(0x3404, wb_modes_regs[0][5]);
        ret |= cameraWriteRegister(0x3405, wb_modes_regs[0][6]);
        ret |= cameraWriteRegister(0x3212, 0x13); // end group 3
        ret |= cameraWriteRegister(0x3212, 0xa3); // lanuch group 3
        return 1;
    }

    ret |= cameraWriteRegister(0x3212, 0x03); // start group 3
    ret |= cameraWriteRegister(0x3406, wb_modes_regs[mode][0]);
    ret |= cameraWriteRegister(0x3400, wb_modes_regs[mode][1]);
    ret |= cameraWriteRegister(0x3401, wb_modes_regs[mode][2]);
    ret |= cameraWriteRegister(0x3402, wb_modes_regs[mode][3]);
    ret |= cameraWriteRegister(0x3403, wb_modes_regs[mode][4]);
    ret |= cameraWriteRegister(0x3404, wb_modes_regs[mode][5]);
    ret |= cameraWriteRegister(0x3405, wb_modes_regs[mode][6]);
    ret |= cameraWriteRegister(0x3212, 0x13); // end group 3
    ret |= cameraWriteRegister(0x3212, 0xa3); // lanuch group 3

    return ret;
}

int OV5640::setSpecialEffect(sde_t sde) {
    int ret = 0;
    int effect = 0;

    effect = sde;
    if (effect < 0 || effect > NUM_SPECIAL_EFFECTS) {
        return 1;
    }

    ret |= cameraWriteRegister(0x3212, 0x03); // start group 3
    ret |= cameraWriteRegister(0x5580, special_effects_regs[effect][0]);
    ret |=
        cameraWriteRegister(0x5583, special_effects_regs[effect][1]); // sat U
    ret |=
        cameraWriteRegister(0x5584, special_effects_regs[effect][2]); // sat V
    ret |= cameraWriteRegister(0x5003, special_effects_regs[effect][3]);
    ret |= cameraWriteRegister(0x3212, 0x13); // end group 3
    ret |= cameraWriteRegister(0x3212, 0xa3); // launch group 3

    return ret;
}

int OV5640::setAutoBlc(int enable, int *regs) {
    uint8_t reg = 0;
    int ret = cameraReadRegister(BLC_CTRL_00, reg);
    ret |= cameraWriteRegister(BLC_CTRL_00, (reg & 0xFE) | (enable != 0));

    if ((enable == 0) && (regs != NULL)) {
        for (uint32_t i = 0; i < 8; i++) {
            ret |= cameraWriteRegister(BLACK_LEVEL_00_H + i, regs[i]);
        }
    }

    return ret;
}

int OV5640::getBlcRegs(int *regs) {
    int ret = 0;

    for (uint32_t i = 0; i < 8; i++) {
        uint8_t reg = 0;
        ret |= cameraReadRegister(BLACK_LEVEL_00_H + i, reg);
        regs[i] = reg;
    }

    return ret;
}

int OV5640::setLensCorrection(int enable) {
    uint8_t reg;
    int ret = cameraReadRegister(ISP_CONTROL_00, reg);
    return cameraWriteRegister(ISP_CONTROL_00,
                               (reg & 0x7F) | (enable ? 0x80 : 0x00)) |
           ret;
}

int OV5640::setNightMode(int enable) {
    /* read HTS from register settings */
    uint8_t reg = 0;
    int ret = 0;
    if (enable) {
        ret = cameraReadRegister(AEC_CTRL_00, reg);
        if (ret)
            return ret;
        reg &= 0xfb;
        return cameraWriteRegister(AEC_CTRL_00, reg);
    } else {
        return cameraWriteRegister(AEC_CTRL_00, aecCtrl00_old);
    }
    return 0;
}

int OV5640::setSharpness(int level) {
    int ret = 0;

    if (level < 0 || level > NUM_SHARPNESS_LEVELS) {
        return 1;
    }

    ret |= cameraWriteRegister(CIP_CTRL, 0x65);
    ret |= cameraWriteRegister(CIP_SHARPENMT_OFFSET1, sharpness_regs[level][0]);

    return ret;
}

int OV5640::setAutoSharpness(int enable) {
    int ret = 0;

    if (enable) {
        ret |= cameraWriteRegister(CIP_CTRL, 0x25);
        ret |= cameraWriteRegister(CIP_SHARPENMT_THRESH1, 0x08);
        ret |= cameraWriteRegister(CIP_SHARPENMT_THRESH2, 0x30);
        ret |= cameraWriteRegister(CIP_SHARPENMT_OFFSET1, 0x10);
        ret |= cameraWriteRegister(CIP_SHARPENMT_OFFSET1, 0x00);
        ret |= cameraWriteRegister(CIP_SHARPENTH_THRESH1, 0x08);
        ret |= cameraWriteRegister(CIP_SHARPENTH_THRESH2, 0x30);
        ret |= cameraWriteRegister(CIP_SHARPENTH_OFFSET1, 0x04);
        ret |= cameraWriteRegister(CIP_SHARPENTH_OFFSET2, 0x06);
    } else {
        ret |= cameraWriteRegister(CIP_CTRL, 0x25);
        ret |= cameraWriteRegister(CIP_SHARPENMT_THRESH1, 0x08);
        ret |= cameraWriteRegister(CIP_SHARPENMT_THRESH2, 0x48);
        ret |= cameraWriteRegister(CIP_SHARPENMT_OFFSET1, 0x18);
        ret |= cameraWriteRegister(CIP_SHARPENMT_OFFSET1, 0x0E);
        ret |= cameraWriteRegister(CIP_SHARPENTH_THRESH1, 0x08);
        ret |= cameraWriteRegister(CIP_SHARPENTH_THRESH2, 0x48);
        ret |= cameraWriteRegister(CIP_SHARPENTH_OFFSET1, 0x04);
        ret |= cameraWriteRegister(CIP_SHARPENTH_OFFSET2, 0x06);
    }

    return ret;
}

/*******************************************************************/

#define FLEXIO_USE_DMA

uint8_t OV5640::setAutoFocusMode() {

    uint8_t ret = 0;

    ret = cameraWriteRegister(OV5640_CMD_MAIN, 0x01);
    ret |= cameraWriteRegister(OV5640_CMD_MAIN, 0x08);
    checkAFCmdStatus(OV5640_CMD_ACK, 0);

    ret |= cameraWriteRegister(OV5640_CMD_ACK, 0x01);
    ret |= cameraWriteRegister(OV5640_CMD_MAIN, AF_CONTINUE_AUTO_FOCUS);
    checkAFCmdStatus(OV5640_CMD_ACK, 0);

    return ret;
}


/*****************************************************************/

uint8_t OV5640::printRegs(void) {
    if (_debug == nullptr) return 0;
    uint8_t reg_value = 0;
#if 1

#define TOUCH_ARRAY_SIZE (sizeof(touched_registers) / sizeof(touched_registers[0]))    
    _debug->println("uint32_t touched_registers[] = {");
    for (uint16_t i = 0; i < TOUCH_ARRAY_SIZE; i +=8) {
        debug_printf(_debug, "\t0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x,\n",  touched_registers[i],
        touched_registers[i+1], touched_registers[i+2], touched_registers[i+3],
        touched_registers[i+4], touched_registers[i+5], touched_registers[i+6],
        touched_registers[i+7]);
    }
    _debug->println("};");

    for (uint16_t reg = 0x3000; reg < 0x6040; reg++) {
        if (was_camera_register_touched(reg)) {
            cameraReadRegister(reg, reg_value, false);
            // lets get as close as possible to format of data out of the python
            debug_printf(_debug, "0x%04X , %u ( 0x%x )", reg, reg_value, reg_value);
            // this as brute force as it gets...
            for (uint16_t ii = 0; ii < CNT_REG_NAME_TABLE; ii++) {
                if (OV5640_reg_name_table[ii].reg == reg) {
                    debug_printf(_debug, ", %s", OV5640_reg_name_table[ii].reg_name);
                    break;
                } 
            }
            _debug->println();
        }
    }
#else    
    _debug->println("\n*** Camera Registers ***");
    for (uint16_t ii = 0; ii < CNT_REG_NAME_TABLE; ii++) {
        cameraReadRegister(OV5640_reg_name_table[ii].reg, reg_value, false);
        //debug_printf(_debug, "%s(%x): %u(%x)\n", OV5640_reg_name_table[ii].reg_name,
        //             OV5640_reg_name_table[ii].reg, reg_value, reg_value);
        debug_printf(_debug, "\t0x%04X: %u(%x)\t//%s\n", 
                     OV5640_reg_name_table[ii].reg, reg_value, reg_value, OV5640_reg_name_table[ii].reg_name);
    }
    _debug->println();
#endif    
    return 0;
}