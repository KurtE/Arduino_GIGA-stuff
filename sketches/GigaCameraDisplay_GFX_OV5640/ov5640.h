
#ifndef _OV5640_H_
#define _OV5640_H_

#include "ov5640_regs.h"
#include <Arduino.h>
// Teensy 4.1 default to CSI pisn
#include "arducam_dvp.h"
// #define OV5640_USE_DEBUG_PINS
#ifdef OV5640_USE_DEBUG_PINS
#define OV5640_DEBUG_PIN_1 14
#define OV5640_DEBUG_PIN_2 15
#define OV5640_DEBUG_PIN_3 3
#define DebugDigitalWrite(pin, val) digitalWriteFast(pin, val)
#define DebugDigitalToggle(pin) digitalToggleFast(pin)
#else
#define DebugDigitalWrite(pin, val)
#define DebugDigitalToggle(pin)
#endif


// dummy defines for camera class
#define HIMAX_MODE_STREAMING 0x01          // I2C triggered streaming enable
#define HIMAX_MODE_STREAMING_NFRAMES 0x03  // Output N frames

typedef enum {
    GAINCEILING_2X,
    GAINCEILING_4X,
    GAINCEILING_8X,
    GAINCEILING_16X,
    GAINCEILING_32X,  // greater that 32 for OV2640
    GAINCEILING_64X,
    GAINCEILING_128X,
} gainceiling_t;

typedef enum {
    NOEFFECT,
    NEGATIVE,
    BW,
    REDDISH,
    GREENISH,
    BLUEISH,
    RETRO,
    OVEREXPOSURE,
    SOLARIZE
} sde_t;


class OV5640 : public ImageSensor {
  public:
    OV5640(TwoWire &i2c = CameraWire);
    enum { CAMERA_JPEG = 42,  OV5640_I2C_ADDR=0x3C};  // temporarily here for now
    int init();
    int reset();
    int getID() {
        return OV5640_I2C_ADDR;
    };
    bool getMono() {
        return false;
    };
    uint32_t getClockFrequency() {
        return 12000000;
    };
    int setFrameRate(int32_t framerate) {
        return 0;
    }
    int setResolutionWithZoom(int32_t resolution, int32_t zoom_resolution, uint32_t zoom_x, uint32_t zoom_y);
    int setResolution(int32_t resolution);
    int setPixelFormat(int32_t pixformat);
    int enableMotionDetection(md_callback_t callback) {
        return 0;
    };
    int disableMotionDetection() {
        return 0;
    };
    int setMotionDetectionWindow(uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
        return 0;
    };
    int setMotionDetectionThreshold(uint32_t threshold) {
        return 0;
    };
    int motionDetected() {
        return 0;
    };
    int setVerticalFlip(bool flip_enable);
    int setHorizontalMirror(bool mirror_enable);
    uint8_t printRegs();
    void debug(Stream &stream) {
        _debug = &stream;
    }
    bool verboseDebug(bool fVerbose);

    uint16_t getModelid();


  private:
    int setWindow(uint16_t reg, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
    Stream *_debug;
    bool _verbose = false;
    TwoWire *_i2c;
    bool _grayscale = false;
    int _framesize = CAMERA_R320x240;
    uint8_t aecCtrl00_old = 0x78;
    //void *_OV5640;

    bool _useAF = false;
    int16_t _format;
    int _saturation;
    int _hue;


    uint8_t setAutoFocusMode();

    uint8_t cameraReadRegister(uint16_t reg_addr, uint8_t &reg_data, bool debug_output= true);
    uint8_t cameraWriteRegister(uint16_t reg, uint8_t data);
    uint8_t cameraWriteFirmware();
    uint8_t setFramesize(int w, int h);

    int calculate_vts(uint16_t readout_height);
    int calculate_hts(uint16_t width);
    int calc_pclk_freq(uint8_t sc_pll_ctrl_0, uint8_t sc_pll_ctrl_1,
                       uint8_t sc_pll_ctrl_2, uint8_t sc_pll_ctrl_3,
                       uint8_t sys_root_div);

    int checkAFCmdStatus(uint16_t reg, uint8_t value);


    //int regWrite(uint8_t dev_addr, uint16_t reg_addr, uint8_t reg_data, bool wide_addr = false);
    //uint8_t regRead(uint8_t dev_addr, uint16_t reg_addr, bool wide_addr = false);
    static const int _resolutions[][2];
    static const int32_t _resolutions_count;
    static const uint8_t default_regs[][3];

    static const uint8_t qqvga_regs[][2];
    static const uint8_t qvga_regs[][2];
    static const uint8_t rgb565_regs[][2];
    static const uint8_t af_firmware_regs[];
    static const uint16_t af_firmware_regs_size;

    typedef struct {
        const __FlashStringHelper *reg_name;
        uint16_t reg;
    } OV5640_TO_NAME_t;

    static const OV5640_TO_NAME_t OV5640_reg_name_table[];
    static const uint16_t CNT_REG_NAME_TABLE;

#ifdef TEENSY_CAMERA_STUFF
        /********************************************************************************************/
        //-------------------------------------------------------
        // normal Read mode
        // size_t readFrameGPIO(void* buffer, size_t cb1=(uint32_t)-1, void*
        // buffer2=nullptr, size_t cb2=0);
        size_t readFrameAF(void *buffer1, size_t cb1, void *buffer2 = nullptr,
                           size_t cb2 = 0);  // Autofocus

        size_t readFrameGPIO_JPEG(void *buffer, size_t cb1 = (uint32_t)-1,
                                  void *buffer2 = nullptr, size_t cb2 = 0);

        // Lets try a dma version.  Doing one DMA that is synchronous does not gain
        // anything So lets have a start, stop... Have it allocate 2 frame buffers
        // and it's own DMA buffers, with the option of setting your own buffers if
        // desired.
        bool startReadFrameDMA(bool (*callback)(void *frame_buffer) = nullptr,
                               uint8_t *fb1 = nullptr, uint8_t *fb2 = nullptr);
        void changeFrameBuffer(uint8_t * fbFrom, uint8_t * fbTo) {
            if (_frame_buffer_1 == fbFrom)
                _frame_buffer_1 = fbTo;
            else if (_frame_buffer_2 == fbFrom)
                _frame_buffer_2 = fbTo;
        }
        bool stopReadFrameDMA();
        inline uint32_t frameCount() {
            return _dma_frame_count;
        }
        inline void *frameBuffer() {
            return _dma_last_completed_frame;
        }
        void captureFrameStatistics();

        void setVSyncISRPriority(uint8_t priority) {
            NVIC_SET_PRIORITY(IRQ_GPIO6789, priority);
        }
        void setDMACompleteISRPriority(uint8_t priority) {
            NVIC_SET_PRIORITY(_dmachannel.channel & 0xf, priority);
        }

        /********************************************************************************************/

        // TBD Allow user to set all of the buffers...
        void readFrameDMA(void *buffer);

        void end();

        int bitsPerPixel() {
            return 16;
        }
        int bytesPerPixel() {
            return 2;
        }

        bool begin_omnivision(framesize_t resolution, pixformat_t format, int fps,
                              int camera_name, bool use_gpio);

        int reset();
        uint16_t getModelid();
        int setPixformat(pixformat_t pixformat);
        uint8_t setFramesize(framesize_t framesize);
        uint8_t setFramesize(int w, int h);
        bool setZoomWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
            return false;
        };
#endif

        void setContrast(int level);
        int setBrightness(int level);
        void setSaturation(int level);
        int setGainceiling(gainceiling_t gainceiling);
        int setQuality(int qs);
        uint8_t getQuality();
        int setColorbar(int enable);
        int setAutoGain(int enable, float gain_db, float gain_db_ceiling);
        void setGain(int gain){};
        int getGain_db(float *gain_db);
        void autoExposure(int enable){};
        int setAutoExposure(int enable, int exposure_us);
        int getExposure_us(int *exposure_us);
        int setAutoWhitebal(int enable, float r_gain_db, float g_gain_db,
                            float b_gain_db);
        int getRGB_Gain_db(float *r_gain_db, float *g_gain_db, float *b_gain_db);
        //int setHmirror(int enable);
        //int setVflip(int enable);
        void setHue(int hue);

        /**
     * Sets OV5640 Image Special Effects.
     *
     * Input: Enumerated
     * 0. NOEFFECT.
     * 1. NEGATIVE.
     * 2. BW.
     * 3. REDDISH.
     * 4. GREENISH.
     * 5. BLUEISH.
     * 6. RETRO.
     * 7. OVEREXPOSURE (5640 only).
     * 8. SOLARIZE (5640 only).
     * RETURNS:  Non-zero if it fails.
     */
        int setSpecialEffect(sde_t sde);

        /**
     * Sets Whitebalance mode for OV5640 camera Only.
     *
     * INPUT: integer.
     *   0 - Auto white balance.
     *   1 - Sunny.
     *   2 - Cloudy.
     *   3 - Office.
     *   4 - Home.
     *
     * RETURNS:   Non-zero if it fails.
     */
        int setWBmode(int mode);
        int setAutoBlc(int enable, int *regs);
        int getBlcRegs(int *regs);
        int setLensCorrection(int enable);
        int setNightMode(int enable);
        int setSharpness(int level);
        int setAutoSharpness(int enable);

#ifdef TEENSY_CAMERA_STUFF
        //    void showRegisters();
        //
        //    uint8_t readRegister(uint8_t reg) { return (uint8_t)-1; }
        //    bool writeRegister(uint8_t reg, uint8_t data) { return false; }

        /*********************************************************/
        void setExposure(int exposure) {}

        bool begin(framesize_t framesize = FRAMESIZE_QVGA, int framerate = 30,
                   bool use_gpio = false) {
            return 0;
        };
        uint8_t setMode(uint8_t Mode, uint8_t FrameCnt) {
            return 0;
        };  // covers and extra
        int setFramerate(int framerate) {
            return 0;
        };
        int get_vt_pix_clk(uint32_t * vt_pix_clk) {
            return 0;
        };
        int getCameraClock(uint32_t * vt_pix_clk) {
            return 0;
        };
        uint8_t cmdUpdate() {
            return 0;
        };
        uint8_t loadSettings(camera_reg_settings_t settings) {
            return 0;
        };
        uint8_t getAE(ae_cfg_t * psAECfg) {
            return 0;
        };
        uint8_t calAE(uint8_t CalFrames, uint8_t * Buffer, uint32_t ui32BufferLen,
                      ae_cfg_t * pAECfg) {
            return 0;
        };
        void readFrame4BitGPIO(void *buffer) {
            Serial.println("4 Bit mode not supported ..... !!!!!");
        }
        int16_t mode(void) {
            return 0;
        }
        void printRegisters(bool only_ones_set = true){};
        void autoGain(int enable, float gain_db, float gain_db_ceiling) {}
#endif
        // Autofocus
        void enableAutoFocus(bool useAF);



      private:

#ifdef TEENSY_CAMERA_STUFF
      private:
        Stream *_debug = nullptr;
        WIRECLASS *_i2c;
        int _xclk_freq = 12;

        bool _grayscale;
        int _framesize = FRAMESIZE_QVGA;
        uint8_t aecCtrl00_old = 0x78;

        void *_OV5640;

        bool _useAF = false;

        int _saturation;
        int _hue;

        // bool flexio_configure();

        // DMA STUFF
        enum { DMABUFFER_SIZE = 1296 };  // 640x480  so 640*2*2
        static DMAChannel _dmachannel;
        static DMASetting _dmasettings[10];  // For now lets have enough for two full
                                             // size buffers...
        static uint32_t _dmaBuffer1[DMABUFFER_SIZE];
        static uint32_t _dmaBuffer2[DMABUFFER_SIZE];

        bool (*_callback)(void *frame_buffer) = nullptr;
        uint32_t _dma_frame_count;
        uint8_t *_dma_last_completed_frame;

#if defined(ARDUINO_TEENSY_MICROMOD)
        uint32_t _save_IOMUXC_GPR_GPR27;
#else
        uint32_t _save_IOMUXC_GPR_GPR26;
#endif
        uint32_t _save_pclkPin_portConfigRegister;

        uint32_t _bytes_left_dma;
        uint16_t _save_lsb;
        uint16_t _frame_col_index;              // which column we are in a row
        uint16_t _frame_row_index;              // which row
        const uint16_t _frame_ignore_cols = 0;  // how many cols to ignore per row

        uint8_t *_frame_buffer_pointer;
        uint8_t *_frame_row_buffer_pointer;  // start of the row
        uint8_t _dma_index;
        volatile bool _dma_active;
        volatile uint32_t _vsync_high_time = 0;

        static void dmaInterrupt();
        void processDMAInterrupt();
#if 0
    static void frameStartInterrupt();
    void processFrameStartInterrupt();
#endif
#endif

        // OpenMV support functions extracted from imglib.h

        typedef union {
            uint32_t l;
            struct {
                uint32_t m : 20;
                uint32_t e : 11;
                uint32_t s : 1;
            };
        } exp_t;
        inline float fast_log2(float x) {
            union {
                float f;
                uint32_t i;
            } vx = { x };
            union {
                uint32_t i;
                float f;
            } mx = { (vx.i & 0x007FFFFF) | 0x3f000000 };
            float y = vx.i;
            y *= 1.1920928955078125e-7f;

            return y - 124.22551499f - 1.498030302f * mx.f - 1.72587999f / (0.3520887068f + mx.f);
        }
        inline float fast_log(float x) {
            return 0.69314718f * fast_log2(x);
        }
        inline int fast_floorf(float x) {
            int i;
            asm volatile("vcvt.S32.f32  %[r], %[x]\n"
                         : [r] "=t"(i)
                         : [x] "t"(x));
            return i;
        }
        inline int fast_ceilf(float x) {
            int i;
            x += 0.9999f;
            asm volatile("vcvt.S32.f32  %[r], %[x]\n"
                         : [r] "=t"(i)
                         : [x] "t"(x));
            return i;
        }
        inline int fast_roundf(float x) {
            int i;
            asm volatile("vcvtr.s32.f32  %[r], %[x]\n"
                         : [r] "=t"(i)
                         : [x] "t"(x));
            return i;
        }
        inline float fast_expf(float x) {
            exp_t e;
            e.l = (uint32_t)(1512775 * x + 1072632447);
            // IEEE binary32 format
            e.e = (e.e - 1023 + 127) & 0xFF;  // rebase

            // uint32_t packed = (e.s << 31) | (e.e << 23) | e.m <<3;
            // return *((float*)&packed);
            union {
                uint32_t ul;
                float f;
            } packed;
            packed.ul = (e.s << 31) | (e.e << 23) | e.m << 3;
            return packed.f;
        }
    };

#endif
