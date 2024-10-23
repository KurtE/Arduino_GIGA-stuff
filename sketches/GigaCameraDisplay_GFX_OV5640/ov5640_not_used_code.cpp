
#include "ov5640.h"
#include "ov5640_regs.h"

#define debug Serial
// #define NO_CLK_PIN

//#define DEBUG_CAMERA
// #define DEBUG_CAMERA_VERBOSE
// #define DEBUG_FLEXIO
// #define USE_DEBUG_PINS
#define DEBUG_CAMERA_REG
#define USE_VSYNC_PIN_INT

#if 0
size_t OV5640::readFrameGPIO_JPEG(void *buffer, size_t cb1, void *buffer2,
                                  size_t cb2) {
    uint16_t w = _width;
    uint16_t h = _height;
    uint32_t i_count = 0;

    debug.printf("$$readFrameGPIO_JPEG(%p, %u, %p, %u)\n", buffer, cb1, buffer2,
                 cb2);
    const uint32_t frame_size_bytes = w * h * _bytesPerPixel / 5;
    if ((cb1 + cb2) < frame_size_bytes) {
        if (_debug)
            debug.printf("Warning Buffers may be too small for JPEG %u < %u\n",
                         cb1 + cb2, frame_size_bytes);
    }
    DBGdigitalWriteFast(0, HIGH);
    uint8_t *b = (uint8_t *)buffer;
    uint32_t cb = (uint32_t)cb1;
    //  bool _grayscale;  // ????  member variable ?????????????
    int bytesPerRow = _width * _bytesPerPixel;

    // Falling edge indicates start of frame
    // pinMode(PCLK_PIN, INPUT); // make sure back to input pin...
    // lets add our own glitch filter.  Say it must be hig for at least 100us
    elapsedMicros emHigh;
    DBGdigitalWriteFast(0, LOW);
    do {
        while ((*_vsyncPort & _vsyncMask) == 0)
            ; // wait for HIGH
        emHigh = 0;
        while ((*_vsyncPort & _vsyncMask) != 0)
            ; // wait for LOW
    } while (emHigh < 1);

    // uint8_t *pu8 = (uint8_t *)b;
    uint8_t prev_char = 0;

    DBGdigitalWriteFast(0, HIGH);
    for (int i = 0; i < h; i++) {
        // rising edge indicates start of line
        while ((*_hrefPort & _hrefMask) == 0)
            ; // wait for HIGH
        while ((*_pclkPort & _pclkMask) != 0)
            ; // wait for LOW
        noInterrupts();

        for (int j = 0; j < bytesPerRow; j++) {
            // rising edges clock each data byte
            while ((*_pclkPort & _pclkMask) == 0)
                ; // wait for HIGH

            // uint32_t in = ((_frame_buffer_pointer)? GPIO1_DR : GPIO6_DR) >>
            // 18; // read all bits in parallel
            uint32_t in = (GPIO7_PSR >> 4); // read all bits in parallel

            // uint32_t in = mmBus;
            // bugbug what happens to the the data if grayscale?
            if (!(j & 1) || !_grayscale) {
                *b++ = in;
                cb--;
                if (cb == 0) {
                    if (buffer2) {
                        if (_debug)
                            debug.printf("\t$$ 2nd buffer: %u %u\n", i, j);
                        b = (uint8_t *)buffer2;
                        cb = (uint32_t)cb2;
                        buffer2 = nullptr;
                    } else {
                        if (_debug)
                            debug.printf("Error failed buffers too small\n");
                        interrupts();
                        DBGdigitalWriteFast(0, LOW);
                        return frame_size_bytes;
                    }
                }
            }

            if ((prev_char == 0xff) && ((uint8_t)in == 0xd9)) {
                interrupts();
                DBGdigitalWriteFast(0, LOW);
                return i_count + 1;
            }
            prev_char = (uint8_t)in;
            i_count = i_count + 1;

            while (((*_pclkPort & _pclkMask) != 0) &&
                   ((*_hrefPort & _hrefMask) != 0))
                ; // wait for LOW bail if _href is lost
        }

        while ((*_hrefPort & _hrefMask) != 0)
            ; // wait for LOW
        interrupts();
    }
    DBGdigitalWriteFast(0, LOW);

    return frame_size_bytes;
}

/*********************************************************************/

//======================================== DMA JUNK
//================================================================================
// experiment with DMA
//================================================================================
// Define our DMA structure.
DMAChannel OV5640::_dmachannel;
DMASetting OV5640::_dmasettings[10];
uint32_t OV5640::_dmaBuffer1[DMABUFFER_SIZE] __attribute__((used, aligned(32)));
uint32_t OV5640::_dmaBuffer2[DMABUFFER_SIZE] __attribute__((used, aligned(32)));
extern "C" void xbar_connect(unsigned int input,
                             unsigned int output); // in pwm.c

// OV2640 *OV5640::active_dma_camera = nullptr;

//===================================================================
// Start a DMA operation -
//===================================================================
#if 0 // def later
bool OV5640::startReadFrameDMA(bool(*callback)(void *frame_buffer), uint8_t *fb1, uint8_t *fb2) {return false;}
bool OV5640::stopReadFrameDMA() {return false;}

#else
bool OV5640::startReadFrameDMA(bool (*callback)(void *frame_buffer),
                               uint8_t *fb1, uint8_t *fb2) {
    // First see if we need to allocate frame buffers.
    if (fb1)
        _frame_buffer_1 = fb1;
    else if (_frame_buffer_1 == nullptr) {
        _frame_buffer_1 = (uint8_t *)malloc(_width * _height);
        if (_frame_buffer_1 == nullptr)
            return false;
    }
    if (fb2)
        _frame_buffer_2 = fb2;
    else if (_frame_buffer_2 == nullptr) {
        _frame_buffer_2 = (uint8_t *)malloc(_width * _height);
        if (_frame_buffer_2 == nullptr)
            return false; // BUGBUG should we 32 byte align?
    }
    // remember the call back if passed in
    _callback = callback;
    active_dma_camera = this;

    if (_debug)
        debug.printf("startReadFrameDMA called buffers %x %x\n",
                     (uint32_t)_frame_buffer_1, (uint32_t)_frame_buffer_2);

    // DebugDigitalToggle(OV7670_DEBUG_PIN_1);
    // lets figure out how many bytes we will tranfer per setting...
    //  _dmasettings[0].begin();
    _frame_row_buffer_pointer = _frame_buffer_pointer =
        (uint8_t *)_frame_buffer_1;

    // configure DMA channels
    _dmachannel.begin();
    _dmasettings[0].source(GPIO2_PSR); // setup source.
    _dmasettings[0].destinationBuffer(
        _dmaBuffer1, DMABUFFER_SIZE * 4); // 32 bits per logical byte
    _dmasettings[0].replaceSettingsOnCompletion(_dmasettings[1]);
    _dmasettings[0]
        .interruptAtCompletion(); // we will need an interrupt to process this.
    _dmasettings[0].TCD->CSR &=
        ~(DMA_TCD_CSR_DREQ); // Don't disable on this one
    // DebugDigitalToggle(OV7670_DEBUG_PIN_1);

    _dmasettings[1].source(GPIO2_PSR); // setup source.
    _dmasettings[1].destinationBuffer(
        _dmaBuffer2, DMABUFFER_SIZE * 4); // 32 bits per logical byte
    _dmasettings[1].replaceSettingsOnCompletion(_dmasettings[0]);
    _dmasettings[1]
        .interruptAtCompletion(); // we will need an interrupt to process this.
    _dmasettings[1].TCD->CSR &=
        ~(DMA_TCD_CSR_DREQ); // Don't disable on this one
    // DebugDigitalToggle(OV7670_DEBUG_PIN_1);

    GPIO2_GDIR = 0; // set all as input...
    GPIO2_DR = 0;   // see if I can clear it out...

    _dmachannel = _dmasettings[0]; // setup the first on...
    _dmachannel.attachInterrupt(dmaInterrupt);
    _dmachannel.triggerAtHardwareEvent(DMAMUX_SOURCE_XBAR1_0);
    // DebugDigitalToggle(OV7670_DEBUG_PIN_1);

    // Lets try to setup the DMA setup...
    // first see if we can convert the _pclk to be an XBAR Input pin...
    // OV7670_PLK   4
    // OV7670_PLK   8    //8       B1_00   FlexIO2:16  XBAR IO14

    _save_pclkPin_portConfigRegister = *(portConfigRegister(_pclkPin));
    *(portConfigRegister(_pclkPin)) = 1; // set to XBAR mode 14

    // route the timer outputs through XBAR to edge trigger DMA request
    CCM_CCGR2 |= CCM_CCGR2_XBAR1(CCM_CCGR_ON);
    xbar_connect(XBARA1_IN_IOMUX_XBAR_INOUT14, XBARA1_OUT_DMA_CH_MUX_REQ30);
    // DebugDigitalToggle(OV7670_DEBUG_PIN_1);

    // Tell XBAR to dDMA on Rising
    XBARA1_CTRL0 = XBARA_CTRL_STS0 | XBARA_CTRL_EDGE0(1) |
                   XBARA_CTRL_DEN0 /* | XBARA_CTRL_IEN0 */;

    IOMUXC_GPR_GPR6 &=
        ~(IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_14); // Make sure it is input mode
    IOMUXC_XBAR1_IN14_SELECT_INPUT =
        1; // Make sure this signal goes to this pin...

#if defined(ARDUINO_TEENSY_MICROMOD)
    // Need to switch the IO pins back to GPI1 from GPIO6
    _save_IOMUXC_GPR_GPR27 =
        IOMUXC_GPR_GPR27; // save away the configuration before we change...
    IOMUXC_GPR_GPR27 &= ~(0x0ff0u);

    // lets also un map the _hrefPin to GPIO1
    IOMUXC_GPR_GPR27 &= ~_hrefMask; //
#else
    // Need to switch the IO pins back to GPI1 from GPIO6
    _save_IOMUXC_GPR_GPR26 =
        IOMUXC_GPR_GPR26; // save away the configuration before we change...
    IOMUXC_GPR_GPR26 &= ~(0x0ff0u);

    // lets also un map the _hrefPin to GPIO1
    IOMUXC_GPR_GPR26 &= ~_hrefMask; //
#endif

    // Need to switch the IO pins back to GPI1 from GPIO6
    //_save_IOMUXC_GPR_GPR27 = IOMUXC_GPR_GPR27;  // save away the configuration
    // before we change... IOMUXC_GPR_GPR27 &= ~(0x0ff0u);

    // lets also un map the _hrefPin to GPIO1
    // IOMUXC_GPR_GPR27 &= ~_hrefMask; //

    // DebugDigitalToggle(OV7670_DEBUG_PIN_1);

    // Falling edge indicates start of frame
    //  while ((*_vsyncPort & _vsyncMask) == 0); // wait for HIGH
    //  while ((*_vsyncPort & _vsyncMask) != 0); // wait for LOW
    //  DebugDigitalWrite(OV7670_DEBUG_PIN_2, HIGH);

    // Debug stuff for now

    // We have the start of a frame, so lets start the dma.
#ifdef DEBUG_CAMERA
    dumpDMA_TCD(&_dmachannel, " CH: ");
    dumpDMA_TCD(&_dmasettings[0], " 0: ");
    dumpDMA_TCD(&_dmasettings[1], " 1: ");

    debug.printf("pclk pin: %d config:%lx control:%lx\n", _pclkPin,
                 *(portConfigRegister(_pclkPin)),
                 *(portControlRegister(_pclkPin)));
    debug.printf("IOMUXC_GPR_GPR26-29:%lx %lx %lx %lx\n", IOMUXC_GPR_GPR26,
                 IOMUXC_GPR_GPR27, IOMUXC_GPR_GPR28, IOMUXC_GPR_GPR29);
    debug.printf("GPIO1: %lx %lx, GPIO6: %lx %lx\n", GPIO1_DR, GPIO1_PSR,
                 GPIO6_DR, GPIO6_PSR);
    debug.printf("XBAR CTRL0:%x CTRL1:%x\n\n", XBARA1_CTRL0, XBARA1_CTRL1);
#endif
    _dma_state = DMASTATE_RUNNING;
    _dma_last_completed_frame = nullptr;
    _dma_frame_count = 0;

    // Now start an interrupt for start of frame.
    //  attachInterrupt(_vsyncPin, &frameStartInterrupt, RISING);

    // DebugDigitalToggle(OV7670_DEBUG_PIN_1);
    return true;
}

//===================================================================
// stopReadFrameDMA - stop doing the reading and then exit.
//===================================================================
bool OV5640::stopReadFrameDMA() {

// hopefully it start here (fingers crossed)
// for now will hang here to see if completes...
#ifdef OV7670_USE_DEBUG_PINS
// DebugDigitalWrite(OV7670_DEBUG_PIN_2, HIGH);
#endif
    elapsedMillis em = 0;
    // tell the background stuff DMA stuff to exit.
    // Note: for now let it end on on, later could disable the DMA directly.
    _dma_state = DMASTATE_STOP_REQUESTED;

    while ((em < 1000) && (_dma_state == DMASTATE_STOP_REQUESTED))
        ; // wait up to a second...
    if (_dma_state != DMA_STATE_STOPPED) {
        debug.println("*** stopReadFrameDMA DMA did not exit correctly...");
        debug.printf("  Bytes Left: %u frame buffer:%x Row:%u Col:%u\n",
                     _bytes_left_dma, (uint32_t)_frame_buffer_pointer,
                     _frame_row_index, _frame_col_index);
    }
#ifdef OV7670_USE_DEBUG_PINS
// DebugDigitalWrite(OV7670_DEBUG_PIN_2, LOW);
#endif
#ifdef DEBUG_CAMERA
    dumpDMA_TCD(&_dmachannel, nullptr);
    dumpDMA_TCD(&_dmasettings[0], nullptr);
    dumpDMA_TCD(&_dmasettings[1], nullptr);
    debug.println();
#endif
    // Lets restore some hardware pieces back to the way we found them.
#if defined(ARDUINO_TEENSY_MICROMOD)
    IOMUXC_GPR_GPR27 =
        _save_IOMUXC_GPR_GPR27; // Restore... away the configuration before we
                                // change...
#else
    IOMUXC_GPR_GPR26 =
        _save_IOMUXC_GPR_GPR26; // Restore... away the configuration before we
                                // change...
#endif
    *(portConfigRegister(_pclkPin)) = _save_pclkPin_portConfigRegister;

    return (em < 1000); // did we stop...
}

//===================================================================
// Our Frame Start interrupt.
//===================================================================
#if 0
void  OV5640::frameStartInterrupt() {
  active_dma_camera->processFrameStartInterrupt();  // lets get back to the main object...
}

void  OV5640::processFrameStartInterrupt() {
  _bytes_left_dma = (_width + _frame_ignore_cols) * _height; // for now assuming color 565 image...
  _dma_index = 0;
  _frame_col_index = 0;  // which column we are in a row
  _frame_row_index = 0;  // which row
  _save_lsb = 0xffff;
  // make sure our DMA is setup properly again. 
  _dmasettings[0].transferCount(DMABUFFER_SIZE);
  _dmasettings[0].TCD->CSR &= ~(DMA_TCD_CSR_DREQ); // Don't disable on this one
  _dmasettings[1].transferCount(DMABUFFER_SIZE);
  _dmasettings[1].TCD->CSR &= ~(DMA_TCD_CSR_DREQ); // Don't disable on this one
  _dmachannel = _dmasettings[0];  // setup the first on...
  _dmachannel.enable();
  
  detachInterrupt(_vsyncPin);
}
#endif

//===================================================================
// Our DMA interrupt.
//===================================================================
void OV5640::dmaInterrupt() {
    active_dma_camera
        ->processDMAInterrupt(); // lets get back to the main object...
}

// This version assumes only called when HREF...  as set pixclk to only fire
// when set.
void OV5640::processDMAInterrupt() {
    _dmachannel.clearInterrupt(); // tell system we processed it.
    asm("DSB");
#ifdef USE_DEBUG_PINS
// DebugDigitalWrite(OV7670_DEBUG_PIN_3, HIGH);
#endif

    if (_dma_state == DMA_STATE_STOPPED) {
        debug.println("OV5640::dmaInterrupt called when DMA_STATE_STOPPED");
        return; //
    }

    // lets guess which buffer completed.
    uint32_t *buffer;
    uint16_t buffer_size;
    _dma_index++;
    if (_dma_index & 1) {
        buffer = _dmaBuffer1;
        buffer_size = _dmasettings[0].TCD->CITER;

    } else {
        buffer = _dmaBuffer2;
        buffer_size = _dmasettings[1].TCD->CITER;
    }
    // lets try dumping a little data on 1st 2nd and last buffer.
#ifdef DEBUG_CAMERA_VERBOSE
    if ((_dma_index < 3) || (buffer_size < DMABUFFER_SIZE)) {
        debug.printf("D(%d, %d, %lu) %u : ", _dma_index, buffer_size,
                     _bytes_left_dma, pixformat);
        for (uint16_t i = 0; i < 8; i++) {
            uint16_t b = buffer[i] >> 4;
            debug.printf(" %lx(%02x)", buffer[i], b);
        }
        debug.print("...");
        for (uint16_t i = buffer_size - 8; i < buffer_size; i++) {
            uint16_t b = buffer[i] >> 4;
            debug.printf(" %lx(%02x)", buffer[i], b);
        }
        debug.println();
    }
#endif

    for (uint16_t buffer_index = 0; buffer_index < buffer_size;
         buffer_index++) {
        if (!_bytes_left_dma || (_frame_row_index >= _height))
            break;

        // only process if href high...
        uint16_t b = *buffer >> 4;
        *_frame_buffer_pointer++ = b;
        _frame_col_index++;
        if (_frame_col_index == _width) {
            // we just finished a row.
            _frame_row_index++;
            _frame_col_index = 0;
        }
        _bytes_left_dma--; // for now assuming color 565 image...
        buffer++;
    }

    if ((_frame_row_index == _height) ||
        (_bytes_left_dma == 0)) { // We finished a frame lets bail
        _dmachannel.disable();    // disable the DMA now...
#ifdef USE_DEBUG_PINS
// DebugDigitalWrite(OV7670_DEBUG_PIN_2, LOW);
#endif
#ifdef DEBUG_CAMERA_VERBOSE
        debug.println("EOF");
#endif
        _frame_row_index = 0;
        _dma_frame_count++;

        bool swap_buffers = true;

        // DebugDigitalToggle(OV7670_DEBUG_PIN_1);
        _dma_last_completed_frame = _frame_row_buffer_pointer;
        if (_callback)
            swap_buffers = (*_callback)(_dma_last_completed_frame);

        if (swap_buffers) {
            if (_frame_row_buffer_pointer != _frame_buffer_1)
                _frame_row_buffer_pointer = _frame_buffer_2;
            else
                _frame_row_buffer_pointer = _frame_buffer_2;
        }

        _frame_buffer_pointer = _frame_row_buffer_pointer;

        // DebugDigitalToggle(OV7670_DEBUG_PIN_1);

        if (_dma_state == DMASTATE_STOP_REQUESTED) {
#ifdef DEBUG_CAMERA
            debug.println("OV5640::dmaInterrupt - Stop requested");
#endif
            _dma_state = DMA_STATE_STOPPED;
        } else {
            // We need to start up our ISR for the next frame.
#if 1
            // bypass interrupt and just restart DMA...
            _bytes_left_dma = (_width + _frame_ignore_cols) *
                              _height; // for now assuming color 565 image...
            _dma_index = 0;
            _frame_col_index = 0; // which column we are in a row
            _frame_row_index = 0; // which row
            _save_lsb = 0xffff;
            // make sure our DMA is setup properly again.
            _dmasettings[0].transferCount(DMABUFFER_SIZE);
            _dmasettings[0].TCD->CSR &=
                ~(DMA_TCD_CSR_DREQ); // Don't disable on this one
            _dmasettings[1].transferCount(DMABUFFER_SIZE);
            _dmasettings[1].TCD->CSR &=
                ~(DMA_TCD_CSR_DREQ);       // Don't disable on this one
            _dmachannel = _dmasettings[0]; // setup the first on...
            _dmachannel.enable();

#else
            attachInterrupt(_vsyncPin, &frameStartInterrupt, RISING);
#endif
        }
    } else {

        if (_bytes_left_dma == (2 * DMABUFFER_SIZE)) {
            if (_dma_index & 1)
                _dmasettings[0].disableOnCompletion();
            else
                _dmasettings[1].disableOnCompletion();
        }
    }
#ifdef OV7670_USE_DEBUG_PINS
// DebugDigitalWrite(OV7670_DEBUG_PIN_3, LOW);
#endif
}

#endif // LATER
typedef struct {
    uint32_t frameTimeMicros;
    uint16_t vsyncStartCycleCount;
    uint16_t vsyncEndCycleCount;
    uint16_t hrefCount;
    uint32_t cycleCount;
    uint16_t pclkCounts[350]; // room to spare.
    uint32_t hrefStartTime[350];
    uint16_t pclkNoHrefCount;
} frameStatics_t;

frameStatics_t fstatOmni40;

void OV5640::captureFrameStatistics() {
    memset((void *)&fstatOmni40, 0, sizeof(fstatOmni40));

    // lets wait for the vsync to go high;
    while ((*_vsyncPort & _vsyncMask) != 0)
        ; // wait for HIGH
    // now lets wait for it to go low
    while ((*_vsyncPort & _vsyncMask) == 0)
        fstatOmni40.vsyncStartCycleCount++; // wait for LOW

    while ((*_hrefPort & _hrefMask) == 0)
        ; // wait for HIGH
    while ((*_pclkPort & _pclkMask) != 0)
        ; // wait for LOW

    uint32_t microsStart = micros();
    fstatOmni40.hrefStartTime[0] = microsStart;
    // now loop through until we get the next _vsynd
    // BUGBUG We know that HSYNC and PCLK on same GPIO VSYNC is not...
    uint32_t regs_prev = 0;
    // noInterrupts();
    while ((*_vsyncPort & _vsyncMask) != 0) {

        fstatOmni40.cycleCount++;
        uint32_t regs = (*_hrefPort & (_hrefMask | _pclkMask));
        if (regs != regs_prev) {
            if ((regs & _hrefMask) && ((regs_prev & _hrefMask) == 0)) {
                fstatOmni40.hrefCount++;
                fstatOmni40.hrefStartTime[fstatOmni40.hrefCount] = micros();
            }
            if ((regs & _pclkMask) && ((regs_prev & _pclkMask) == 0))
                fstatOmni40.pclkCounts[fstatOmni40.hrefCount]++;
            if ((regs & _pclkMask) && ((regs_prev & _hrefMask) == 0))
                fstatOmni40.pclkNoHrefCount++;
            regs_prev = regs;
        }
    }
    while ((*_vsyncPort & _vsyncMask) == 0)
        fstatOmni40.vsyncEndCycleCount++; // wait for LOW
    // interrupts();
    fstatOmni40.frameTimeMicros = micros() - microsStart;

    // Maybe return data. print now
    debug.printf("*** Frame Capture Data: elapsed Micros: %u loops: %u\n",
                 fstatOmni40.frameTimeMicros, fstatOmni40.cycleCount);
    debug.printf("   VSync Loops Start: %u end: %u\n",
                 fstatOmni40.vsyncStartCycleCount,
                 fstatOmni40.vsyncEndCycleCount);
    debug.printf("   href count: %u pclk ! href count: %u\n    ",
                 fstatOmni40.hrefCount, fstatOmni40.pclkNoHrefCount);
    for (uint16_t ii = 0; ii < fstatOmni40.hrefCount + 1; ii++) {
        debug.printf("%3u(%u) ", fstatOmni40.pclkCounts[ii],
                     (ii == 0) ? 0
                               : fstatOmni40.hrefStartTime[ii] -
                                     fstatOmni40.hrefStartTime[ii - 1]);
        if (!(ii & 0x0f))
            debug.print("\n    ");
    }
    debug.println();
}

void OV5640::end() {
    endXClk();
    pinMode(_xclkPin, INPUT);
    _i2c->end();
}
int OV5640::calc_pclk_freq(uint8_t sc_pll_ctrl_0, uint8_t sc_pll_ctrl_1,
                           uint8_t sc_pll_ctrl_2, uint8_t sc_pll_ctrl_3,
                           uint8_t sys_root_div) {
    uint32_t pclk_freq = _xclk_freq * 1000000;
    pclk_freq /= ((sc_pll_ctrl_3 & 0x10) != 0x00) ? 2 : 1;
    pclk_freq /= ((sc_pll_ctrl_0 & 0x0F) == 0x0A) ? 10 : 8;
    switch (sc_pll_ctrl_3 & 0x0F) {
    case 0:
        pclk_freq /= 1;
        break;
    case 1:
        pclk_freq /= 2;
        break;
    case 2:
        pclk_freq /= 3;
        break;
    case 3:
        pclk_freq /= 4;
        break;
    case 4:
        pclk_freq /= 6;
        break;
    case 5:
        pclk_freq /= 8;
        break;
    default:
        pclk_freq /= 3;
        break;
    }
    pclk_freq *= sc_pll_ctrl_2;
    sc_pll_ctrl_1 >>= 4;
    pclk_freq /= sc_pll_ctrl_1;
    switch (sys_root_div & 0x30) {
    case 0x00:
        pclk_freq /= 1;
        break;
    case 0x10:
        pclk_freq /= 2;
        break;
    case 0x20:
        pclk_freq /= 4;
        break;
    case 0x30:
        pclk_freq /= 8;
        break;
    default:
        pclk_freq /= 1;
        break;
    }
    return (int)pclk_freq;
}

#ifdef DEBUG_CAMERA

void print_pin_info(const char *pin_name, uint8_t pin) {
    debug_printf(&Serial, "\t%s(%d)", pin_name, pin);
    if (pin >= CORE_NUM_DIGITAL)
        Serial.println(" ** unused **");
    else {
        debug_printf(&Serial, ": %08x %08x\n", *(portControlRegister(pin)), *(portConfigRegister(pin)));
    }
}
#endif



bool OV5640::begin_omnivision(framesize_t framesize, pixformat_t format,
                              int fps, int camera_name, bool use_gpio) {

    _use_gpio = use_gpio;

    // WIP - Need set functions:
    if (_rst != 0xff) {
        if (_rst_init >= 0) {
            pinMode(_rst, OUTPUT);
            digitalWrite(_rst, _rst_init);
        } else if (_rst_init == -1)
            pinMode(_rst, INPUT);
        else if (_rst_init == -2)
            pinMode(_rst, INPUT_PULLUP);
        else if (_rst_init == -3)
            pinMode(_rst, INPUT_PULLDOWN);
        delay(5);
    }

    if (_pwdn != 0xff) {
        if (_pwdn_init >= 0) {
            pinMode(_pwdn, OUTPUT);
            digitalWrite(_pwdn, _pwdn_init);
        } else if (_pwdn_init == -1)
            pinMode(_pwdn, INPUT);
        else if (_pwdn_init == -2)
            pinMode(_pwdn, INPUT_PULLUP);
        else if (_pwdn_init == -3)
            pinMode(_pwdn, INPUT_PULLDOWN);
        delay(5);
    }

// BUGBUG::: see where frame is
#ifdef USE_DEBUG_PINS
    pinMode(49, OUTPUT);
#endif

    //_wire = &Wire;
    _i2c->begin();

    // Configure Camera Resolution
    _width = _resolutions[framesize][0];
    if (_width == 0) {
        if (_debug)
            _debug->println("Frame Size Invalid!!!");
        return false;
    }
    _height = _resolutions[framesize][1];
    _framesize = (uint8_t)framesize;

    // Configure pixel format
    _grayscale = false;
    switch (format) {
    case YUV422:
        _bytesPerPixel = 2;
        _format = 0;
        break;
    case PIXFORMAT_BAYER:
    case CAMERA_BAYER:
        _bytesPerPixel = 2;
        _format = 1;
        break;
    case RGB565:
        _bytesPerPixel = 2;
        _format = 2;
        break;
    case PIXFORMAT_GRAYSCALE:
    case CAMERA_GRAYSCALE:
        format = YUV422;    // We use YUV422 but discard U and V bytes
        _bytesPerPixel = 2; // 2 input bytes per pixel of which 1 is discarded
        _grayscale = true;
        _format = 4;
        break;
    case CAMERA_BAYER:
        _bytesPerPixel = 2;
        _format = 8;
        break;
    default:
        return false;
    }

    pinMode(_vsyncPin, INPUT /*INPUT_PULLDOWN*/);
    //  const struct digital_pin_bitband_and_config_table_struct *p;
    //  p = digital_pin_to_info_PGM + _vsyncPin;
    //  *(p->pad) = IOMUXC_PAD_DSE(7) | IOMUXC_PAD_HYS;  // See if I turn on
    //  HYS...
    pinMode(_hrefPin, INPUT);
    pinMode(_pclkPin, INPUT_PULLDOWN);
    pinMode(_xclkPin, OUTPUT);

#ifdef DEBUG_CAMERA
    debug_printf(_debug,"  VS=%d, HR=%d, PC=%d XC=%d\n", _vsyncPin, _hrefPin, _pclkPin,
                 _xclkPin);
    debug_printf(_debug, "  RST=%d(%d), PWDN=%d(%d)\n", _rst, _rst_init, _pwdn, _pwdn_init);

    for (int i = 0; i < 8; i++) {
        pinMode(_dPins[i], INPUT_PULLDOWN);
        debug_printf(_debug, "  _dpins(%d)=%d\n", i, _dPins[i]);
    }
#endif

    _vsyncPort = portInputRegister(digitalPinToPort(_vsyncPin));
    _vsyncMask = digitalPinToBitMask(_vsyncPin);
    _hrefPort = portInputRegister(digitalPinToPort(_hrefPin));
    _hrefMask = digitalPinToBitMask(_hrefPin);
    _pclkPort = portInputRegister(digitalPinToPort(_pclkPin));
    _pclkMask = digitalPinToBitMask(_pclkPin);

    /*
      if(camera_name == OV7670) {
          _xclk_freq = 14;  //was 16Mhz
      } else {
          if(fps <= 10){
           _xclk_freq = 14;
          } else {
          _xclk_freq = 16;
          }
      }
    */

    beginXClk();

    if (_rst != 0xFF) {
        pinMode(_rst, OUTPUT);
        digitalWriteFast(_rst, LOW); /* Reset */
        for (volatile uint32_t i = 0; i < 100000; i++) {
        }
        digitalWriteFast(_rst, HIGH); /* Normal mode. */
        for (volatile uint32_t i = 0; i < 100000; i++) {
        }
    }

    _i2c->begin();

    delay(500);
    Serial.println(getModelid(), HEX);
    if (getModelid() != 0x5640) {
        end();
        if (_debug)
            _debug->println("Camera detect failed");
        return false;
    }

#ifdef DEBUG_CAMERA
    debug_printf(_debug, "Calling ov5640_configure\n");
    debug_printf(_debug, "Cam Name: %d, Format: %d, Resolution: %d, Clock: %d\n",
                 camera_name, _format, _framesize, _xclk_freq);
    debug_printf(_debug, "Frame rate: %d\n", fps);
#endif

    // flexIO/DMA
    if (!_use_gpio) {
        hardware_configure();
        setVSyncISRPriority(102);
        setDMACompleteISRPriority(192);
    } else {
        setVSyncISRPriority(102);
        setDMACompleteISRPriority(192);
    }

    if (
        
    ) < 0) {
        if (_debug)
            _debug->println("Error: RESET failed");
        return false;
    }

    if (setPixformat(format) != 0) {
        if (_debug)
            _debug->println("Error: setPixformat failed");
        return false;
    }

    if (setFramesize(framesize) != 0) {
        if (_debug)
            _debug->println("Error: setFramesize failed");
        return false; // failed to set resolution
    }

    if (_useAF) {
        if (setAutoFocusMode() != 0) {
            if (_debug)
                _debug->println("Error: Failed to setAutoFocusMode");
            return false;
        }
    }

#ifdef DEBUG_CAMERA
    if (_debug) {
        // curious of pin setting between CSI and FlexIO
        Serial.println("\n*** Camera Pin Settings ***");
        print_pin_info("vsyncPin", _vsyncPin);
        print_pin_info("hrefPin", _hrefPin);
        print_pin_info("pclkPin", _pclkPin);
        print_pin_info("xclkPin", _xclkPin);
        print_pin_info("rst", _rst);
        print_pin_info("pwdn", _pwdn);
        print_pin_info("D0", _dPins[0]);
        print_pin_info("D1", _dPins[1]);
        print_pin_info("D2", _dPins[2]);
        print_pin_info("D3", _dPins[3]);
        print_pin_info("D4", _dPins[4]);
        print_pin_info("D5", _dPins[5]);
        print_pin_info("D6", _dPins[6]);
        print_pin_info("D7", _dPins[7]);
    }
#endif

    return true;
}

#endif
