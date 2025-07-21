
#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/ring_buffer.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);
#include <zephyr/devicetree.h>
#include <zephyr/multi_heap/shared_multi_heap.h>
#include <zephyr/drivers/dma.h>
#include <zephyr/timing/timing.h>

//#define BUFFER_SIZE (64 * 1024 - 1)
//#define BUFFER_SIZE (128 * 1024)
#define MAX_DMA_BLOCK_SIZE (65536 - 32)
#define BUFFER_SIZE (320*240*2)

//#define USE_STATIC_BUFFERS
#ifdef USE_STATIC_BUFFERS
__aligned(32) uint8_t buffer0[BUFFER_SIZE];
__aligned(32) uint8_t buffer1[BUFFER_SIZE];
__aligned(32) uint8_t buffer2[BUFFER_SIZE];
uint8_t *sdram_buffers[3] = { buffer0, buffer1, buffer2 };
#else
uint8_t *sdram_buffers[3];
#endif
volatile bool dma_completed;

typedef struct {
  uint8_t *tx_buffer;
  uint8_t *rx_buffer;
  size_t cb_left;
  uint32_t channel;
} user_transfer_data_t;

user_transfer_data_t user_transfer_data;


void dma_callback(const struct device *dma_dev, void *user_data, uint32_t channel, int status) {
  int ret;
  printk("DCB (%p %p %u %d)\n", dma_dev, user_data, channel, status);
  if (status == 0) {
    user_transfer_data_t *puser_transfer_data = (user_transfer_data_t *)user_data;
    if (puser_transfer_data->cb_left == 0) {
      printk("DMA tramsfer complete on channel %d\n", channel);
      dma_completed = true;
      return;
    }
    // we still have data left...
    size_t chunk_size = MIN(MAX_DMA_BLOCK_SIZE, puser_transfer_data->cb_left);
    //SCB_CleanDCache_by_Addr((uint32_t *)puser_transfer_data->tx_buffer, chunk_size);
    //SCB_InvalidateDCache_by_Addr((uint32_t *)puser_transfer_data->rx_buffer, chunk_size);

    printk("DMA CB: reload %p %p %u\n", puser_transfer_data->tx_buffer, puser_transfer_data->rx_buffer, chunk_size);
    //dma_stop(dma_dev, puser_transfer_data->channel);  // Reset before each chunk
#if 1
    ret = dma_reload(dma_dev, puser_transfer_data->channel, (uint32_t)puser_transfer_data->tx_buffer, (uint32_t)puser_transfer_data->rx_buffer, chunk_size);
    if (ret) {
      LOG_ERR("DMA reload failed: %d", ret);
    }
#else
    struct dma_block_config block_cfg = {
      .source_address = (uint32_t)puser_transfer_data->tx_buffer,
      .dest_address = (uint32_t)puser_transfer_data->rx_buffer,
      .block_size = chunk_size,
    };

    struct dma_config dma_cfg = {
      .dma_slot = 0,
      .channel_direction = MEMORY_TO_MEMORY,
      .complete_callback_en = true,
      .source_data_size = 1,
      .dest_data_size = 1,
      .source_burst_length = 1,
      .dest_burst_length = 1,
      .block_count = 1,
      .head_block = &block_cfg,
      .user_data = user_data,
      .dma_callback = dma_callback,
    };
    ret = dma_config(dma_dev, puser_transfer_data->channel, &dma_cfg);
    if (ret) {
      LOG_ERR("DMA config failed: %d", ret);
      dma_completed = true;
      return;
    }
#endif
    puser_transfer_data->tx_buffer += chunk_size;
    puser_transfer_data->rx_buffer += chunk_size;
    puser_transfer_data->cb_left -= chunk_size;

    printk(" start ");
    ret = dma_start(dma_dev, puser_transfer_data->channel);
    if (ret) {
      LOG_ERR("DMA start failed: %d", ret);
      dma_completed = true;
    }

  } else {
    LOG_ERR("DMA chunk failed with status %d on channel %d", status, channel);
    dma_completed = true;
  }
}

int configure_dma_transfer(const struct device *dma_dev, uint8_t *src, uint8_t *dest, size_t size, int channel) {
  dma_completed = false;

  size_t chunk_size = MIN(MAX_DMA_BLOCK_SIZE, size);

  SCB_CleanDCache_by_Addr((uint32_t *)src, size);
  SCB_InvalidateDCache_by_Addr((uint32_t *)dest, size);

  struct dma_block_config block_cfg = {
    .source_address = (uint32_t)(src),
    .dest_address = (uint32_t)(dest),
    .block_size = chunk_size,
  };

  user_transfer_data.tx_buffer = src + chunk_size;
  user_transfer_data.rx_buffer = dest + chunk_size;
  user_transfer_data.cb_left = size - chunk_size;
  user_transfer_data.channel = channel;

  struct dma_config dma_cfg = {
    .dma_slot = 0,
    .channel_direction = MEMORY_TO_MEMORY,
    .complete_callback_en = true,
    .source_data_size = 1,
    .dest_data_size = 1,
    .source_burst_length = 1,
    .dest_burst_length = 1,
    .block_count = 1,
    .head_block = &block_cfg,
    .user_data = &user_transfer_data,
    .dma_callback = dma_callback,
  };

  dma_stop(dma_dev, channel);  // Reset before each chunk
  int ret = dma_config(dma_dev, channel, &dma_cfg);
  if (ret) {
    LOG_ERR("DMA config failed: %d", ret);
    return ret;
  }

  printk("DMAStart");
  ret = dma_start(dma_dev, channel);
  if (ret) {
    LOG_ERR("DMA start failed: %d", ret);
    return ret;
  }

  return 0;
}


int CheckBuffers(const char *sz, uint8_t *source, uint8_t *dest) {
  int error_count = 0;

  for (int i = 0; i < BUFFER_SIZE; i++) {
    if (source[i] != dest[i]) {
      error_count++;
      if (error_count == 1) printk("%s ", sz);
      if (error_count < 5) printk(" (%i %x %x)", i, source[i], dest[i]);
    }
  }
  if (error_count) {
    printk(" %u\n", error_count);
  } else {
    printk("No errors\n");
  }
  return error_count;
}

#define DMA_NODE DT_NODELABEL(dma1)
void setup(void) {
  while (!Serial && millis() < 5000)
    ;

  Serial.begin(115200);

  const struct device *dma_dev = DEVICE_DT_GET(DMA_NODE);
  //struct dma_status dma_status = {0};
  int ret = 0;
  int tx_dma_channel = 0;
  // Start up the Serial
  //SerialX.begin();
  k_sleep(K_MSEC(250));
  printk("SDRAM test...\n");

  for (uint8_t channel = 1; channel <= 7; channel++) {
    dma_status dmastat;
    memset(&dmastat, 0, sizeof(dmastat));
    ret = dma_get_status(dma_dev, channel, &dmastat);
    printk("ch:%u stat:%d - %u %u %u %u %u %u %u\n", channel, ret,
           dmastat.busy, dmastat.dir, dmastat.pending_length,
           dmastat.free, dmastat.write_position, dmastat.read_position, (uint32_t)dmastat.total_copied);
    if ((tx_dma_channel == 0) && (ret == 0) && (!dmastat.busy)) tx_dma_channel = channel;
  }


  //tx_dma_channel = dma_request_channel(dma_dev, 0);
  printk("DMA Channel allocated %d\n", tx_dma_channel);

  for (uint8_t i = 0; i < (sizeof(sdram_buffers) / sizeof(sdram_buffers[0])); i++) {
#ifndef USE_STATIC_BUFFERS
    sdram_buffers[i] = (uint8_t *)shared_multi_heap_aligned_alloc(SMH_REG_ATTR_EXTERNAL, 32, BUFFER_SIZE);
//sdram_buffers[i] = (uint8_t *)k_malloc(BUFFER_SIZE);
#endif
    printk("Buffer: %u ADDR:%p\n", i, sdram_buffers[i]);
  }

  for (uint32_t loop_count = 0;; loop_count++) {
    printk("loop: %u(%x)\n ", loop_count, loop_count & 0xff);
    memset(sdram_buffers[0], loop_count & 0xff, BUFFER_SIZE);
    memset(sdram_buffers[1], 0xff, BUFFER_SIZE);
    memset(sdram_buffers[2], 0xaa, BUFFER_SIZE);

    printk(" call conf ");
    ret = configure_dma_transfer(dma_dev, sdram_buffers[0], sdram_buffers[2], BUFFER_SIZE, tx_dma_channel);
    if (ret != 0) {
      LOG_ERR("DMA configuration or transfer failed\n");
    }
    printk(" memcpy ");
    memcpy(sdram_buffers[1], sdram_buffers[0], BUFFER_SIZE);
    printk("Check BUffers via memcpy\n");
    CheckBuffers(" Memcpy:", sdram_buffers[0], sdram_buffers[1]);

    // DMA completion is already handled per chunk inside configure_dma_transfer()
    int wait_count = 0;
    while (!dma_completed && wait_count++ < 1000) {
      k_sleep(K_USEC(100));
    }
    if (!dma_completed) {
      LOG_ERR("DMA transfer timeout");
    }

    __DSB();  // Data Synchronization Barrier
    SCB_InvalidateDCache_by_Addr((uint32_t *)sdram_buffers[2], BUFFER_SIZE);
    if (int ret = dma_stop(dma_dev, tx_dma_channel)) {
      LOG_ERR("dma_stop failed: %d", ret);
    }
    printk("Check BUffers via DMA\n");
    CheckBuffers(" DMA:", sdram_buffers[0], sdram_buffers[2]);
    printk("\n");
    k_sleep(K_MSEC(250));
  }
}


void loop() {
  // put your main code here, to run repeatedly:
}
