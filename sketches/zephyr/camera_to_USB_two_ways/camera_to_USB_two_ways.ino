#include "camera.h"
#define USBSerial Serial

Camera cam;

#define FRAME_WIDTH 320
#define FRAME_HEIGHT 240
extern void maybe_send_image(uint8_t *buffer, uint32_t buffer_size, uint16_t frame_width, uint16_t frame_height);
extern void send_image(uint8_t *buffer, uint32_t buffer_size, uint16_t frame_width, uint16_t frame_height);
extern void send_raw_image(uint8_t *buffer, uint32_t buffer_size, uint16_t frame_width, uint16_t frame_height);

void fatal_error(const char *msg) {
  Serial.println(msg);
  pinMode(LED_BUILTIN, OUTPUT);
  while (1) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
  }
}

void setup(void) {
  Serial.begin(115200);
  while(!Serial && millis() < 4000) {}
  if (!cam.begin(320, 240, CAMERA_RGB565, false)) {
    fatal_error("Camera begin failed");
  }
  cam.setVerticalFlip(false);
  cam.setHorizontalMirror(false);

  pinMode(LED_BUILTIN, OUTPUT);
  show_all_gpio_regs();
}

void loop() {
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  FrameBuffer fb;
  if (cam.grabFrame(fb)) {
    //printk("grabFrame: (%p %u)\n", fb.getBuffer(), fb.getBufferSize());
    maybe_send_image(fb.getBuffer(), fb.getBufferSize(), FRAME_WIDTH, FRAME_HEIGHT);
    cam.releaseFrame(fb);
  }
}

inline uint16_t HTONS(uint16_t x) {
  return ((x >> 8) & 0x00FF) | ((x << 8) & 0xFF00);
}


void send_image(uint8_t *buffer, uint32_t buffer_size, uint16_t frame_width,
                uint16_t frame_height) {
  USBSerial.write(0xFF);
  USBSerial.write(0xAA);
  UNUSED(buffer_size);
  uint16_t *frameBuffer = (uint16_t *)buffer;

  // BUGBUG:: maybe combine with the save to SD card code
  unsigned char bmpFileHeader[14] = { 'B', 'M', 0, 0, 0, 0, 0,
                                      0, 0, 0, 54, 0, 0, 0 };
  unsigned char bmpInfoHeader[40] = { 40, 0, 0, 0, 0, 0, 0, 0,
                                      0, 0, 0, 0, 1, 0, 24, 0 };

  int rowSize = 4 * ((3 * frame_width + 3) / 4);  // how many bytes in the row (used to create padding)
  int fileSize = 54 + frame_height * rowSize;     // headers (54 bytes) + pixel data

  bmpFileHeader[2] = (unsigned char)(fileSize);
  bmpFileHeader[3] = (unsigned char)(fileSize >> 8);
  bmpFileHeader[4] = (unsigned char)(fileSize >> 16);
  bmpFileHeader[5] = (unsigned char)(fileSize >> 24);

  bmpInfoHeader[4] = (unsigned char)(frame_width);
  bmpInfoHeader[5] = (unsigned char)(frame_width >> 8);
  bmpInfoHeader[6] = (unsigned char)(frame_width >> 16);
  bmpInfoHeader[7] = (unsigned char)(frame_width >> 24);
  bmpInfoHeader[8] = (unsigned char)(frame_height);
  bmpInfoHeader[9] = (unsigned char)(frame_height >> 8);
  bmpInfoHeader[10] = (unsigned char)(frame_height >> 16);
  bmpInfoHeader[11] = (unsigned char)(frame_height >> 24);

  USBSerial.write(bmpFileHeader, sizeof(bmpFileHeader));  // write file header
  USBSerial.write(bmpInfoHeader, sizeof(bmpInfoHeader));  // " info header

  unsigned char bmpPad[rowSize - 3 * frame_width];
  for (int i = 0; i < (int)(sizeof(bmpPad)); i++) {  // fill with 0s
    bmpPad[i] = 0;
  }

#define PIX_PER_WRITE 20
  //#ifdef GRAY_IMAGE
  uint16_t *pfb = frameBuffer;
  uint8_t img[3];
  for (int y = frame_height - 1; y >= 0; y--) {  // iterate image array
    pfb = &frameBuffer[y * frame_width];
    for (int x = 0; x < frame_width; x++) {
      // r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3
      uint16_t pixel = HTONS(*pfb++);
      img[2] = (pixel >> 8) & 0xf8;  // r
      img[1] = (pixel >> 3) & 0xfc;  // g
      img[0] = (pixel << 3);         // b
      USBSerial.write(img, 3);
      k_sleep(K_USEC(8));
    }
    USBSerial.write(bmpPad,
                    (4 - (frame_width * 3) % 4) % 4);  // and padding as needed
  }

  USBSerial.write(0xBB);
  USBSerial.write(0xCC);

  USBSerial.println("ACK CMD CAM Capture Done. END");
  // camera.setHmirror(0);

  k_sleep(K_MSEC(50));
}

void send_raw_image(uint8_t *buffer, uint32_t buffer_size, uint16_t frame_width,
                    uint16_t frame_height) {
  uint8_t *pb = (uint8_t *)buffer;
  uint32_t cb = buffer_size;
  size_t cb_write = 64;

#ifndef RAW_TEST_MODE
  uint16_t *p = (uint16_t *)buffer;
  for (int i = 0; i < frame_width * frame_height; i++) {
    *p = HTONS(*p);
    p++;
  }
#endif

  while (cb) {
    if (cb < cb_write)
      cb_write = cb;
    size_t cb_written = USBSerial.write(pb, cb_write);
    if (cb_written != cb_write) {
      printk("Failed to write buffer");
      break;
    }
    cb -= cb_written;
    pb += cb_written;
    //printk(".");
  }
}

void maybe_send_image(uint8_t *buffer, uint32_t buffer_size, uint16_t frame_width,
                      uint16_t frame_height) {
  int ch;
  while ((ch = USBSerial.read()) != -1) {
    switch (ch) {
      case 0x10:
        {
          printk("Send JPEG: %u %u - Not supported\n", frame_width, frame_height);
          USBSerial.print("NAK CMD CAM start jpg single shoot. END");
          // send_jpeg();
          USBSerial.print("READY. END");
        }
        break;
      case 0x30:
        {
          printk("Send BMP: %u %u\n", frame_width, frame_height);
          USBSerial.print("ACK CMD CAM start single shoot ... ");
          send_image(buffer, buffer_size, frame_width, frame_height);
          USBSerial.print("READY. END");
        }
        break;

      case 0x1:
        send_raw_image(buffer, buffer_size, frame_width, frame_height);
        break;

      default:
        break;
    }
  }
}
void set_pin_moder(GPIO_TypeDef *port, uint8_t pin, uint8_t pin_mode) {
  // Set the MODER = could be done in fewer steps
  uint32_t moder = port->MODER;
  uint32_t mask = ~(0x3 << (pin * 2));
  moder = (moder & mask) | (pin_mode << (pin * 2));
  port->MODER = moder;
}

void set_gpio_pin_mode(GPIO_TypeDef *port, uint8_t pin, uint8_t af) {
  // Set the MODER = could be done in fewer steps
  uint32_t moder = port->MODER;
  uint32_t mask = ~(0x3 << (pin * 2));
  moder = (moder & mask) | (0x2 << (pin * 2));
  port->MODER = moder;

  if (pin < 8) {
    port->AFR[0] = port->AFR[0] & ~(0xf << (pin * 4)) | (af << (pin * 4));
  } else {
    pin -= 8;
    port->AFR[1] = port->AFR[1] & ~(0xf << (pin * 4)) | (af << (pin * 4));
  }
}

void print_gpio_regs(const char *name, GPIO_TypeDef *port) {
  //printk("GPIO%s(%p) %08X %08X %08x\n", name, port, port->MODER, port->AFR[0], port->AFR[1]);
  Serial.print("GPIO");
  Serial.print(name);
  Serial.print(" ");
  uint32_t moder = port->MODER;
  Serial.print(moder, HEX);
  Serial.print(" : ");
  for (uint8_t i = 0; i < 16; i++) {
    switch (moder & 0xC0000000) {
      case 0x00000000ul: Serial.print("I"); break;
      case 0x40000000ul: Serial.print("O"); break;
      case 0x80000000ul: Serial.print("F"); break;
      default: Serial.print("A"); break;
    }
    moder <<= 2;
  }
  Serial.print(" ");
  Serial.print(port->AFR[0], HEX);
  Serial.print(" ");
  Serial.print(port->AFR[1], HEX);
  Serial.print(" ");
  Serial.print(port->IDR, HEX);
  Serial.print(" ");
  Serial.print(port->ODR, HEX);
  Serial.print(" ");
  uint32_t pupdr = port->PUPDR;
  Serial.print(pupdr, HEX);
  Serial.print(" : ");
  for (uint8_t i = 0; i < 16; i++) {
    switch (pupdr & 0xC0000000) {
      case 0x00000000ul: Serial.print("-"); break;
      case 0x40000000ul: Serial.print("U"); break;
      case 0x80000000ul: Serial.print("D"); break;
      default: Serial.print("?"); break;
    }
    pupdr <<= 2;
  }
  Serial.println();
}

void show_all_gpio_regs() {
  print_gpio_regs("A", (GPIO_TypeDef *)GPIOA_BASE);
  print_gpio_regs("B", (GPIO_TypeDef *)GPIOB_BASE);
  print_gpio_regs("C", (GPIO_TypeDef *)GPIOC_BASE);
  print_gpio_regs("D", (GPIO_TypeDef *)GPIOD_BASE);
  print_gpio_regs("E", (GPIO_TypeDef *)GPIOE_BASE);
  print_gpio_regs("F", (GPIO_TypeDef *)GPIOF_BASE);
  print_gpio_regs("G", (GPIO_TypeDef *)GPIOG_BASE);
  print_gpio_regs("H", (GPIO_TypeDef *)GPIOH_BASE);
  print_gpio_regs("I", (GPIO_TypeDef *)GPIOI_BASE);
}

