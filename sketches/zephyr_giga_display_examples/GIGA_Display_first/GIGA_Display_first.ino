#include "Arduino.h"
#include "Arduino_Giga_display.h"

Display display;

uint16_t color_table[] = {
  RGB565_RED, RGB565_MAGENTA, RGB565_YELLOW, RGB565_BLACK, RGB565_GREEN,
  RGB565_WHITE, RGB565_ORANGE, RGB565_GREENYELLOW, RGB565_PINK
};
uint8_t color_index = 0;

// Lets try adding a thread that blinks pins...
#define STACKSIZE 1024
#define PRIORITY 7
#define SLEEPTIME 10
#define LED_BLINK_COUNT 32
#define BLINK_PIN 2

K_THREAD_STACK_DEFINE(blink_pins_thread_stack, STACKSIZE);
static struct k_thread blink_pins_thread_data;

void blink_pins_thread(void *dummy1, void *dummy2, void *dummy3)
{
	ARG_UNUSED(dummy1);
	ARG_UNUSED(dummy2);
	ARG_UNUSED(dummy3);

	Serial.print("thread_a: thread started \n");

	while (1)
	{
    static uint8_t pin_state = 0;
    static uint8_t loop_count = 0;
    pin_state ^=1;
    digitalWrite(BLINK_PIN, pin_state);
    loop_count++;
    if ((loop_count & (LED_BLINK_COUNT - 1)) == 0) digitalWrite(LED_BUILTIN, (loop_count  & LED_BLINK_COUNT)? HIGH : LOW);
  	k_msleep(SLEEPTIME);
  }
}



void fillScreen(uint16_t color) {
  uint32_t sizeof_framebuffer2 = 2 * 160 * 160;
  uint8_t* frameBuffer2 =  (uint8_t*)  malloc(sizeof_framebuffer2);
  for (int x = 0; x < 480; x += 160) {
    for (int y = 0; y < 800; y += 160) {
      uint16_t *pb = (uint16_t *)frameBuffer2;
      uint32_t count = sizeof_framebuffer2 / 2;
      while(count--) *pb++ = color;
      display.write8(x, y, frameBuffer2);
    }
  }
  free(frameBuffer2);
}

void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  uint32_t sizeof_framebuffer2 = 2 * 160 * 160;
  uint8_t* frameBuffer2 =  (uint8_t*)  malloc(sizeof_framebuffer2);
 
  uint16_t *pb = (uint16_t *)frameBuffer2;
  uint32_t count = sizeof_framebuffer2 / 2;

  for (int x1 = 0; x1 < w; x1 += w) {
    for (int y1 = 0; y1 < w; y1 += h) {
      while(count--) *pb++ = color;
      display.write8(x+x1, y+y1, frameBuffer2);
    }
  }
  free(frameBuffer2);
}

void setup() {
  while (!Serial && millis() < 5000)
    ;
  Serial.begin(115200);

  if (!display.begin()) {
    Serial.println("Failed to start display");
  };

  Serial.println("Display configured!!");
  pinMode(BLINK_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  // Start a thread:
    k_thread_create(&blink_pins_thread_data, blink_pins_thread_stack,
      K_THREAD_STACK_SIZEOF(blink_pins_thread_stack),
      blink_pins_thread, NULL, NULL, NULL,
      PRIORITY, 0, K_FOREVER);
  k_thread_name_set(&blink_pins_thread_data, "blink pins");

	k_thread_start(&blink_pins_thread_data);



  void* FB =  display.getFrameBuffer();
  if (FB == nullptr){
    Serial.println("Memory not allocated successfully." );
    while(1){}
  }
  //Allocate memory to the framebuffer
  uint32_t sizeof_framebuffer = 2 * 160 * 160;
  void* ptrFB = malloc(sizeof_framebuffer);
  // Cast the void pointer to an int pointer to use it
  uint8_t* frameBuffer = static_cast<uint8_t*>(ptrFB);

  display.setFrameDesc(160, 160, 160, sizeof_framebuffer);
  display.startFrameBuffering();

  fillScreen(RGB565_DARKGREY);

  for (int x = 0; x < 480; x += 160) {
    for (int y = 0; y < 800; y += 160) {
      uint16_t *pb = (uint16_t *)frameBuffer;
      uint32_t count = sizeof_framebuffer / 2;
      while(count--) *pb++ = color_table[color_index];
      display.write8(x, y, frameBuffer);
      color_index++;
      if (color_index == (sizeof(color_table)/sizeof(color_table[0]))) color_index = 0;
    }
  }

  delay(2000);
  fillScreen(RGB565_DARKGREY);
  fillRect(160, 160, 160, 160, RGB565_RED);

  display.endFrameBuffering();
  free(ptrFB);

  delay(2000);
  display.startFrameBuffering();
  fillScreen(RGB565_BLACK);
  display.endFrameBuffering();
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(500);
}
