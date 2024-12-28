#ifndef __SDRAM_H
#define __SDRAM_H
#include "Arduino.h"
#ifdef __cplusplus
  class SDRAMClass {
  public:
    SDRAMClass() {}
    int begin(uint32_t start_address = 0);
    void* malloc(size_t size);
    void free(void* ptr);
  //	bool test(bool fast = false, Stream& _serial = Serial);
  protected:
    bool _begin_called = false;
  };
extern SDRAMClass SDRAM;
#endif
#endif