#include "SDRAM.h"
#include <zephyr/kernel.h>

/*
extern "C" {
  Z_HEAP_DEFINE_IN_SECT(SDRAM1, sdram_pool, 2000000);

}
*/
#if 0
#if DT_INST_NODE_HAS_PROP(0, ext_sdram)
#if DT_SAME_NODE(DT_INST_PHANDLE(0, ext_sdram), DT_NODELABEL(sdram1))
#define BUFFER_POOL_SECTION __stm32_sdram1_section
#elif DT_SAME_NODE(DT_INST_PHANDLE(0, ext_sdram), DT_NODELABEL(sdram2))
#define BUFFER_POOL_SECTION __stm32_sdram2_section
#endif
#else
#define BUFFER_POOL_SECTION
#endif /* DT_SAME_NODE(DT_INST_PHANDLE(0, ext_sdram), DT_NODELABEL(sdram1)) */
#endif
#define CONFIG_SDRAM_BUFFER_MAX 400000
extern "C" {
static struct k_heap sdram_pool;
__stm32_sdram1_section
__aligned(32) static uint8_t sdram_pool_buf[CONFIG_SDRAM_BUFFER_MAX];
}

int SDRAMClass::begin(uint32_t start_address) {
  UNUSED(start_address);
  k_heap_init(&sdram_pool, sdram_pool_buf, CONFIG_SDRAM_BUFFER_MAX);
  _begin_called = true;
  return 1;

}

void* SDRAMClass::malloc(size_t size) {
  if(!_begin_called) begin();

	return k_heap_alloc(&sdram_pool, size, K_NO_WAIT);
}

void SDRAMClass::free(void* ptr) {
	k_heap_free(&sdram_pool, ptr);
}


SDRAMClass SDRAM;