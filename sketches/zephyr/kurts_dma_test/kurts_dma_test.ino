/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Sample echo app for CDC ACM class
 *
 * Sample app for USB CDC ACM class driver. The received data is echoed back
 * to the serial port.
 */

//#include <sample_usbd.h>
// Turn on this option, to test to ee if camera errors out without screeen
//#define TIMED_WAIT_NO_TFT (1000/6)

// Try using fixed normal memory buffer for display
//#define ILI9341_USE_FIXED_BUFFER
// Hack try to use fixed buffer for camera

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/ring_buffer.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);
#include <zephyr/devicetree.h>
#include <zephyr/multi_heap/shared_multi_heap.h>
#include <zephyr/drivers/dma.h>
#include <core_cm7.h>

#define BUFFER_SIZE (256)
//#define BUFFER_SIZE (320*240*2)

#define USE_STATIC_BUFFERS
#ifdef USE_STATIC_BUFFERS
__aligned(32) uint8_t buffer0[BUFFER_SIZE];
__aligned(32) uint8_t buffer1[BUFFER_SIZE];
__aligned(32) uint8_t buffer2[BUFFER_SIZE];
uint8_t *sdram_buffers[3] = { buffer0, buffer1, buffer2 };
#else
uint8_t *sdram_buffers[3];
#endif
volatile bool dma_completed;

void dma_callback(const struct device *dma_dev, void *user_data, uint32_t channel, int status) {
  printk(" CB:%d", status);
  if (status == 0) {

    //        LOG_INF("DMA transfer complete on channel %d", channel);
  } else {
    LOG_ERR("DMA transfer failed with status %d on channel %d", status, channel);
  }
  dma_completed = true;
}


int configure_dma_transfer(const struct device *dma_dev, uint8_t *src, uint8_t *dest, size_t size, int channel) {
  int ret = 0;

  SCB_CleanDCache_by_Addr((uint32_t *)src, size);
  SCB_InvalidateDCache_by_Addr((uint32_t *)sdram_buffers[2], BUFFER_SIZE);

  static bool first_time = true;
  if (first_time) {
    first_time = false;
    //printk("configure_dma_transfer(%p, %p, %p, %u %u)\n", dma_dev, src, dest, size, channel);


    struct dma_block_config block_cfg = {
      .source_address = (uint32_t)src,
      .dest_address = (uint32_t)dest,
      .block_size = size,
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
      //.user_data = dest,
      .dma_callback = dma_callback,
    };
    //static bool fConfigured = false;
    //if (!fConfigured) {

    //printk(" CFG ");
    ret = dma_config(dma_dev, channel, &dma_cfg);
    if (ret != 0) {
      LOG_ERR("DMA configuration failed with error %d\n", ret);
      return ret;
    }

  } else {
    //printk(" RELOAD ");
    ret = dma_reload(dma_dev, channel, (uint32_t)src, (uint32_t)dest, size);
    if (ret) {
      LOG_ERR("DMA Reload failed: %d", ret);
    }
  }
  dma_completed = false;
  //printk(" Start ");
  ret = dma_start(dma_dev, channel);
  if (ret != 0) {
    LOG_ERR("Failed to start DMA transfer with error %d\n", ret);
  }

  //printk("DMA started");

  return ret;
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
  if (error_count) printk(" %u", error_count);
  return error_count;
}

#define DMA_NODE DT_NODELABEL(dma1)
void setup(void) {

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
           dmastat.free, dmastat.write_position, dmastat.read_position, dmastat.total_copied);
		if ((tx_dma_channel == 0) && (ret == 0) && (!dmastat.busy)) tx_dma_channel = channel;
  }


  dma_stop(dma_dev, tx_dma_channel);
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
    printk("loop: %u(%x) ", loop_count, loop_count & 0xff);
    memset(sdram_buffers[0], loop_count & 0xff, BUFFER_SIZE);
    memset(sdram_buffers[1], 0xff, BUFFER_SIZE);
    memset(sdram_buffers[2], 0xaa, BUFFER_SIZE);
    //printk(" Start DMA ");
    ret = configure_dma_transfer(dma_dev, sdram_buffers[0], sdram_buffers[2], BUFFER_SIZE, tx_dma_channel);
    if (ret != 0) {
      LOG_ERR("DMA configuration or transfer failed\n");
    }

    //printk(" MEMCPY ");
    memcpy(sdram_buffers[1], sdram_buffers[0], BUFFER_SIZE);

    CheckBuffers(" Memcpy:", sdram_buffers[0], sdram_buffers[1]);
    //printk(" wait ");
    while (!dma_completed) k_sleep(K_MSEC(1));

    __DSB();  // Data Synchronization Barrier
    SCB_InvalidateDCache_by_Addr((uint32_t *)sdram_buffers[2], BUFFER_SIZE);
    if (int ret = dma_stop(dma_dev, tx_dma_channel)) {
      LOG_ERR("dma_stop failed: %d", ret);
    }

    CheckBuffers(" DMA:", sdram_buffers[0], sdram_buffers[2]);
    printk("\n");
    k_sleep(K_MSEC(259));
  }
}

//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------


void loop() {
  // put your main code here, to run repeatedly:
}
