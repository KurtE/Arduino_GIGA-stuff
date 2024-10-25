#include <elapsedMillis.h>
#include <MemoryHexDump.h>
REDIRECT_STDOUT_TO(Serial)
#include "Arduino_GigaDisplay_GFX.h"
#define GC9A01A_CYAN 0x07FF
#define GC9A01A_RED 0xf800
#define GC9A01A_BLUE 0x001F
#define GC9A01A_GREEN 0x07E0
#define GC9A01A_MAGENTA 0xF81F
#define GC9A01A_WHITE 0xffff
#define GC9A01A_BLACK 0x0000
#define GC9A01A_YELLOW 0xFFE0
#define ALIGN_PTR(p, a) ((p & (a - 1)) ? (((uintptr_t)p + a) & ~(uintptr_t)(a - 1)) : p)

uint16_t color565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}


#include "arducam_dvp.h"
//#include "SDRAM.h"
// This example only works with Greyscale cameras (due to the palette + resize&rotate algo)
//#define ARDUCAM_CAMERA_HM01B0

//#include "OV7670/ov767x.h"
//#define ROTATE_CAMERA_IMAGE


//#define ARDUCAM_CAMERA_GC2145
//#define ARDUCAM_CAMERA_HM01B0
//#define ARDUCAM_CAMERA_HM0360
//#define ARDUCAM_CAMERA_OV767X
#define ARDUCAM_CAMERA_OV5640
#define CAMERA_WIDTH 640
#define CAMERA_HEIGHT 480


#ifdef ARDUCAM_CAMERA_HM01B0
#include "Himax_HM01B0/himax.h"
HM01B0 himax;
Camera cam(himax);
#define IMAGE_MODE CAMERA_GRAYSCALE
#define CANVAS_ROTATION 3

#elif defined(ARDUCAM_CAMERA_HM0360)
#include "Himax_HM0360/hm0360.h"
HM0360 himax;
Camera cam(himax);
#define IMAGE_MODE CAMERA_GRAYSCALE
#define CANVAS_ROTATION 3

#elif defined(ARDUCAM_CAMERA_OV767X)
#include "OV7670/ov767x.h"
// OV7670 ov767x;
OV7675 ov767x;
Camera cam(ov767x);
#define IMAGE_MODE CAMERA_RGB565
#ifdef ROTATE_CAMERA_IMAGE
#define CANVAS_ROTATION 0
#else
#define CANVAS_ROTATION 1
#endif

#elif defined(ARDUCAM_CAMERA_GC2145)
#include "GC2145/gc2145.h"
#define CANVAS_ROTATION 1
GC2145 galaxyCore;
Camera cam(galaxyCore);
#define IMAGE_MODE CAMERA_RGB565

#elif defined(ARDUCAM_CAMERA_OV5640)
#include "ov5640.h"
#define CANVAS_ROTATION 1
OV5640 ov5640;
Camera cam(ov5640);
#define IMAGE_MODE CAMERA_RGB565


#endif

// The buffer used to capture the frame
FrameBuffer fb;
uint16_t palette[256];

#ifdef ROTATE_CAMERA_IMAGE
uint16_t *rotate_buffer = nullptr;
#endif


// The buffer used to rotate and resize the frame
GigaDisplay_GFX display;

void blinkLED(uint32_t count = 0xFFFFFFFF) {
    pinMode(LED_BUILTIN, OUTPUT);
    while (count--) {
        digitalWrite(LED_BUILTIN, LOW);   // turn the LED on (HIGH is the voltage level)
        delay(50);                        // wait for a second
        digitalWrite(LED_BUILTIN, HIGH);  // turn the LED off by making the voltage LOW
        delay(50);                        // wait for a second
    }
}

void setup() {
    while (!Serial && millis() < 5000) {}
    Serial.begin(115200);

    // Init the cam QVGA, 30FPS
    SDRAM.begin();
    Serial.println("Before camera start");
    Serial.flush();
    cam.debug(Serial);
#if CAMERA_WIDTH == 640
    if (!cam.begin(CAMERA_R640x480, IMAGE_MODE, 15)) {
        blinkLED();
    }
#else
    if (!cam.begin(CAMERA_R320x240, IMAGE_MODE, 30)) {
        blinkLED();
    }
#endif

#if defined(ARDUCAM_CAMERA_HM0360) || defined(ARDUCAM_CAMERA_HM01B0)
    himax.printRegs();
#elif defined(ARDUCAM_CAMERA_OV767X)
    ov767x.printRegs();
#elif defined(ARDUCAM_CAMERA_OV5640)
    ov5640.printRegs();
#endif
    // lets try printing out DCMI as well
    Serial.println("\nDCMI registers: ");
    printf("\tCR:%08lX\n", DCMI->CR);
    printf("\tSR:%08lX\n", DCMI->SR);
    printf("\tRISR:%08lX\n", DCMI->RISR);
    printf("\tIER:%08lX\n", DCMI->IER);
    printf("\tMISR:%08lX\n", DCMI->MISR);
    printf("\tICR:%08lX\n", DCMI->ICR);
    printf("\tESCR:%08lX\n", DCMI->ESCR);
    printf("\tESUR:%08lX\n", DCMI->ESUR);
    printf("\tCWSTRTR:%08lX\n", DCMI->CWSTRTR);
    printf("\tCWSIZER:%08lX\n", DCMI->CWSIZER);
    printf("\tDR:%08lX\n", DCMI->DR);

    // Setup the palette to convert 8 bit greyscale to 32bit greyscale
    for (int i = 0; i < 256; i++) {
        palette[i] = color565(i, i, i);
    }

    Serial.println("Before setBuffer");
    Serial.flush();

#if defined(ARDUCAM_CAMERA_HM01B0) || defined(ARDUCAM_CAMERA_HM0360)
    uint8_t *fb_mem = (uint8_t *)SDRAM.malloc(CAMERA_WIDTH * CAMERA_HEIGHT + 32);
#else
    uint8_t *fb_mem = (uint8_t *)SDRAM.malloc(CAMERA_WIDTH * CAMERA_HEIGHT * 2 + 32);
#endif
#ifdef ROTATE_CAMERA_IMAGE
    rotate_buffer = (uint16_t *)SDRAM.malloc(CAMERA_WIDTH * CAMERA_HEIGHT);
    printf("Rotate buffer: %p\n", rotate_buffer);
#endif

    fb.setBuffer((uint8_t *)ALIGN_PTR((uintptr_t)fb_mem, 32));
    printf("Frame buffer: %p\n", fb.getBuffer());

    // clear the display (gives a nice black background)
    Serial.println("Before setRotation");
    Serial.flush();
    display.begin();
    display.setRotation(CANVAS_ROTATION);
    Serial.println("Before fillscreen");
    Serial.flush();
    elapsedMicros em;
    display.fillScreen(GC9A01A_BLUE);
    Serial.println(em, DEC);
    Serial.println("end setup");
    Serial.flush();
    pinMode(2, OUTPUT);
    pinMode(3, OUTPUT);
    pinMode(4, OUTPUT);
}

inline uint16_t HTONS(uint16_t x) {
    return ((x >> 8) & 0x00FF) | ((x << 8) & 0xFF00);
}
//#define HTONS(x) (((x >> 8) & 0x00FF) | ((x << 8) & 0xFF00))


void writeRect8BPP(GigaDisplay_GFX *pdisp, int16_t x, int16_t y, const uint8_t bitmap[],
                   int16_t w, int16_t h, const uint16_t *palette) {
    int display_width = 480;  // pdisp->WIDTH;
    //int display_height = 800;  //pdisp->HEIGHT;
    uint16_t *display_buffer = pdisp->getBuffer();

    pdisp->startWrite();
    // BUGBUG Assuming it will fit here and don't need to clip
#if CANVAS_ROTATION == 1
    // y = xIn ;  x = WIDTH - 1 - yIn
    for (int16_t j = 0; j < h; j++, y++) {
        uint16_t *p = display_buffer + (x * display_width) + display_width - 1 - y;
        for (int16_t i = 0; i < w; i++) {
            *p = palette[*bitmap++];
            p += display_width;
        }
    }
#elif CANVAS_ROTATION == 3
    // y = HEIGHT = 1 - xIn ;  x = yIn
    uint16_t *p_row = display_buffer + ((display_height - 1 - x) * display_width);
    for (int16_t j = 0; j < h; j++, y++) {
        uint16_t *p = p_row + y;
        for (int16_t i = 0; i < w; i++) {
            *p = palette[*bitmap++];
            p -= display_width;
        }
    }

#else
    for (int16_t j = 0; j < h; j++, y++) {
        for (int16_t i = 0; i < w; i++) {
            digitalWrite(4, HIGH);
            pdisp->writePixel(x + i, y, palette[bitmap[j * w + i]]);
            digitalWrite(4, LOW);
        }
    }
#endif
    digitalWrite(4, HIGH);
    pdisp->endWrite();
    digitalWrite(4, LOW);
}

#ifdef ROTATE_CAMERA_IMAGE
void rotate_rgb_camera_image(uint16_t *pixels, uint16_t *rotated_pixels) {
    for (int y = 0; y < CAMERA_HEIGHT; y++) {
        //uint16_t *prp = rotated_pixels + y - 1; // 0-> last column
        for (int x = 0; x < CAMERA_WIDTH; x++) {
            //*prp = HTONS(*pixels++);
            rotated_pixels[(CAMERA_WIDTH - 1 - x) * CAMERA_HEIGHT + y] = HTONS(*pixels++);
            //prp += CAMERA_HEIGHT;
        }
    }
}
#endif

uint32_t display_time_sum = 0;
uint8_t display_time_count = 0;

void loop() {

    // Grab frame and write to another framebuffer
    digitalWrite(2, HIGH);
    if (cam.grabFrame(fb, 3000) == 0) {
        digitalWrite(2, LOW);

        if (Serial.available()) {
            while (Serial.read() != -1) {}
            cam.printRegs();
            MemoryHexDump(Serial, fb.getBuffer(), 1024, true, "Start of Camera Buffer\n");
            Serial.println("*** Paused ***");
            while (Serial.read() == -1) {}
            while (Serial.read() != -1) {}
        }
        static bool print_once = true;
        if (print_once) {
            print_once = false;
            Serial.println("\nDCMI registers: ");
            printf("\tCR:%08lX\n", DCMI->CR);
            printf("\tSR:%08lX\n", DCMI->SR);
            printf("\tRISR:%08lX\n", DCMI->RISR);
            printf("\tIER:%08lX\n", DCMI->IER);
            printf("\tMISR:%08lX\n", DCMI->MISR);
            printf("\tICR:%08lX\n", DCMI->ICR);
            printf("\tESCR:%08lX\n", DCMI->ESCR);
            printf("\tESUR:%08lX\n", DCMI->ESUR);
            printf("\tCWSTRTR:%08lX\n", DCMI->CWSTRTR);
            printf("\tCWSIZER:%08lX\n", DCMI->CWSIZER);
            printf("\tDR:%08lX\n", DCMI->DR);
        }

        // We need to swap bytes.
        uint16_t *pixels = (uint16_t *)fb.getBuffer();
        digitalWrite(3, HIGH);
        elapsedMicros emDisplay;
#if defined(ARDUCAM_CAMERA_HM01B0) || defined(ARDUCAM_CAMERA_HM0360)
        writeRect8BPP(&display, (display.width() - CAMERA_WIDTH) / 2, (display.height() - CAMERA_HEIGHT) / 2, (uint8_t *)pixels, CAMERA_WIDTH, CAMERA_HEIGHT, palette);
#elif defined(ROTATE_CAMERA_IMAGE)
        rotate_rgb_camera_image(pixels, rotate_buffer);
        display.drawRGBBitmap((display.width() - CAMERA_HEIGHT) / 2, (display.height() - CAMERA_WIDTH) / 2, rotate_buffer, CAMERA_HEIGHT, CAMERA_WIDTH);
        static uint8_t debug_out_count = 4;
        if (debug_out_count) {
            printf("DRAWRGB (%d, %d, %p, %u, %u)\n", (display.width() - CAMERA_HEIGHT) / 2, (display.height() - CAMERA_WIDTH) / 2, rotate_buffer, CAMERA_HEIGHT, CAMERA_WIDTH);
            debug_out_count--;
        }
#else
        for (int i = 0; i < CAMERA_WIDTH * CAMERA_HEIGHT; i++) pixels[i] = HTONS(pixels[i]);
        display.drawRGBBitmap((display.width() - CAMERA_WIDTH) / 2, (display.height() - CAMERA_HEIGHT) / 2, pixels, CAMERA_WIDTH, CAMERA_HEIGHT);
#endif
        digitalWrite(3, LOW);
        display_time_sum += emDisplay;
        display_time_count++;
        if (display_time_count == 128) {
            Serial.print("Avg display Time: ");
            Serial.print(display_time_sum / display_time_count);
            Serial.print(" fps:");
            Serial.println(128000000.0 / float(display_time_sum), 2);
            display_time_sum = 0;
            display_time_count = 0;
        }

    } else {
        digitalWrite(2, LOW);
        blinkLED(20);
    }
}

#include <mbed_wait_api.h>
MBED_NORETURN void mbed_die () {
  gpio_t led_red, led_grn, led_blu;
         
  gpio_init_out(&led_red, LED_RED);
  gpio_init_out(&led_grn, LED_GREEN);
  gpio_init_out(&led_blu, LED_BLUE);

  gpio_write(&led_red, 1);
  gpio_write(&led_grn, 1);
  gpio_write(&led_blu, 1);

  for (int i = 0; i < 2; ++i) {
    for (int j = 0; j < 4; ++j) {
      gpio_write(&led_red, 0);
      wait_us(150000);
      gpio_write(&led_red, 1);
      wait_us(150000);
    }
    for (int j = 0; j < 4; ++j) {
      gpio_write(&led_red, 0);
      wait_us(400000);
      gpio_write(&led_red, 1);
      wait_us(400000);
    }
  }
  NVIC_SystemReset();
}
