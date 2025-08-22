

#define DT_DRV_COMPAT st_jpeg_hw

#include <stdlib.h>
#include <zephyr/device.h>

#include <zephyr/devicetree.h>
#include <zephyr/drivers/dma.h>
#include <zephyr/drivers/dma/dma_stm32.h>
#include <zephyr/irq.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/atomic.h>

#include <app/drivers/jpeg.h>
#include <stm32_ll_dma.h>
#include <stm32u5xx_hal.h>

#include "jpeg_utils.h"

LOG_MODULE_REGISTER(jpeg_hw, CONFIG_STM32_JPEG_LOG_LEVEL);

struct stream {
  const struct device *dma_dev;
  uint32_t dma_channel;
  struct dma_config dma_cfg;
};

struct jpeg_in_prop {
  uint32_t src_addr;
  uint32_t dst_addr;
  uint32_t src_size;
  uint32_t frame_index;
};

struct jpeg_hw_data {
  struct stream tx;
  struct stream rx;
  DMA_HandleTypeDef hdma_tx, hdma_rx;
  JPEG_HandleTypeDef hjpeg;

  struct jpeg_in_prop in_prop;
  struct jpeg_out_prop out_prop;

  atomic_t data_ready;
};

struct jpeg_hw_config {};

#define CHUNK_SIZE_IN ((uint32_t)(128))
#define CHUNK_SIZE_OUT ((uint32_t)(512))

void HAL_JPEG_InfoReadyCallback(JPEG_HandleTypeDef *hjpeg,
                                JPEG_ConfTypeDef *pInfo) {

  struct jpeg_hw_data *data = CONTAINER_OF(hjpeg, struct jpeg_hw_data, hjpeg);

  data->out_prop.color_space = pInfo->ColorSpace;
  data->out_prop.chroma = pInfo->ChromaSubsampling;
  data->out_prop.width = pInfo->ImageHeight;
  data->out_prop.height = pInfo->ImageWidth;
  data->out_prop.quality = pInfo->ImageQuality;

  //LOG_INF("ColorSpace = %u, ChromaSubsampling = %u", pInfo->ColorSpace,
  //        pInfo->ChromaSubsampling);

  // LOG_INF("HAL_JPEG_InfoReadyCallback");
}

void HAL_JPEG_GetDataCallback(JPEG_HandleTypeDef *hjpeg,
                              uint32_t NbDecodedData) {
  uint32_t inDataLength;
  struct jpeg_hw_data *data = CONTAINER_OF(hjpeg, struct jpeg_hw_data, hjpeg);

  data->in_prop.frame_index += NbDecodedData;

  if (data->in_prop.frame_index < data->in_prop.src_size) {
    data->in_prop.src_addr += NbDecodedData;

    if ((data->in_prop.src_size - data->in_prop.frame_index) >= CHUNK_SIZE_IN) {
      inDataLength = CHUNK_SIZE_IN;
    } else {
      inDataLength = data->in_prop.src_size - data->in_prop.frame_index;
    }
  } else {
    inDataLength = 0;
  }
  // LOG_INF("HAL_JPEG_GetDataCallback : inDataLength = %d", inDataLength);

  HAL_JPEG_ConfigInputBuffer(hjpeg, (uint8_t *)data->in_prop.src_addr,
                             inDataLength);
}

void HAL_JPEG_DataReadyCallback(JPEG_HandleTypeDef *hjpeg, uint8_t *pDataOut,
                                uint32_t OutDataLength) {
  /* Update JPEG encoder output buffer address*/

  struct jpeg_hw_data *data = CONTAINER_OF(hjpeg, struct jpeg_hw_data, hjpeg);

  data->in_prop.dst_addr += OutDataLength;

  HAL_JPEG_ConfigOutputBuffer(hjpeg, (uint8_t *)data->in_prop.dst_addr,
                              CHUNK_SIZE_OUT);

  // LOG_INF("HAL_JPEG_DataReadyCallback");
}

/**
 * @brief  JPEG Error callback
 * @param hjpeg: JPEG handle pointer
 * @retval None
 */
void HAL_JPEG_ErrorCallback(JPEG_HandleTypeDef *hjpeg) {
  LOG_ERR("HAL_JPEG_ErrorCallback");
}

void HAL_JPEG_DecodeCpltCallback(JPEG_HandleTypeDef *hjpeg) {
  struct jpeg_hw_data *data = CONTAINER_OF(hjpeg, struct jpeg_hw_data, hjpeg);

  atomic_set(&data->data_ready, 1);
  // LOG_INF("HAL_JPEG_DecodeCpltCallback");
}

static void dma_callback(const struct device *dma_dev, void *arg,
                         uint32_t channel, int status) {
  DMA_HandleTypeDef *hdma = arg;

  ARG_UNUSED(dma_dev);

  if (status < 0) {
    LOG_ERR("DMA callback error with channel %d, status = %d", channel, status);
  }
  HAL_DMA_IRQHandler(hdma);
}

static void jpeg_hw_irq_handler(const void *parameter) {
  const struct device *dev = parameter;
  struct jpeg_hw_data *dev_data = dev->data;

  HAL_JPEG_IRQHandler(&dev_data->hjpeg);
}

static int jpeg_hw_dma_init(DMA_HandleTypeDef *hdma, struct stream *stream) {

  stream->dma_cfg.user_data = hdma;
  stream->dma_cfg.linked_channel = STM32_DMA_HAL_OVERRIDE;

  if (!device_is_ready(stream->dma_dev)) {
    LOG_ERR("%s DMA device not ready", stream->dma_dev->name);
    return -ENODEV;
  }

  int ret = dma_config(stream->dma_dev, stream->dma_channel, &stream->dma_cfg);

  if (ret != 0) {
    LOG_ERR("Failed to configure DMA channel %d",
            stream->dma_channel + STM32_DMA_STREAM_OFFSET);
    return ret;
  }

  DMA_Channel_TypeDef *dma_channel =
      (DMA_Channel_TypeDef *)LL_DMA_GET_CHANNEL_INSTANCE(GPDMA1,
                                                         stream->dma_channel);

  // TX channel -> PERIPHERAL_TO_MEMORY
  if ((enum dma_channel_direction)PERIPHERAL_TO_MEMORY ==
      stream->dma_cfg.channel_direction) {

    hdma->Instance = dma_channel;
    hdma->Init.Request = stream->dma_cfg.dma_slot;
    hdma->Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
    hdma->Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma->Init.SrcInc = DMA_SINC_FIXED;
    hdma->Init.DestInc = DMA_DINC_INCREMENTED;
  } else {
    // RX channel -> MEMORY_TO_PERIPHERAL

    /* GPDMA1_REQUEST_JPEG_RX Init */
    hdma->Instance = (DMA_Channel_TypeDef *)LL_DMA_GET_CHANNEL_INSTANCE(
        GPDMA1, stream->dma_channel);
    hdma->Init.Request = stream->dma_cfg.dma_slot;
    hdma->Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
    hdma->Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma->Init.SrcInc = DMA_SINC_INCREMENTED;
    hdma->Init.DestInc = DMA_DINC_FIXED;
  }

  hdma->Init.SrcDataWidth = DMA_SRC_DATAWIDTH_WORD;
  hdma->Init.DestDataWidth = DMA_DEST_DATAWIDTH_WORD;
  hdma->Init.Priority = DMA_LOW_PRIORITY_HIGH_WEIGHT;
  hdma->Init.SrcBurstLength = 8;
  hdma->Init.DestBurstLength = 8;
  hdma->Init.TransferAllocatedPort =
      DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT1;
  hdma->Init.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
  hdma->Init.Mode = DMA_NORMAL;

  if (HAL_DMA_Init(hdma) != HAL_OK) {
    LOG_ERR("HAL_DMA_Init Failed");
    return -EINVAL;
  }

  if (HAL_DMA_ConfigChannelAttributes(hdma, DMA_CHANNEL_NPRIV) != HAL_OK) {
    LOG_ERR("HAL_DMA_ConfigChannelAttributes Failed");
    return -EINVAL;
  }

  return ret;
}

static int jpeg_hw_init(const struct device *dev) {

  struct jpeg_hw_data *dev_data = dev->data;

  dev_data->hjpeg.Instance = JPEG;

  __HAL_RCC_JPEG_CLK_ENABLE();

  if (HAL_JPEG_Init(&dev_data->hjpeg) != HAL_OK) {
    abort();
  }
  int ret = 0;

  ret = jpeg_hw_dma_init(&dev_data->hdma_tx, &dev_data->tx);
  if (ret) {
    return ret;
  }

  ret = jpeg_hw_dma_init(&dev_data->hdma_rx, &dev_data->rx);
  if (ret) {
    return ret;
  }

  __HAL_LINKDMA(&dev_data->hjpeg, hdmaout, dev_data->hdma_tx);
  __HAL_LINKDMA(&dev_data->hjpeg, hdmain, dev_data->hdma_rx);

  IRQ_CONNECT(JPEG_IRQn, 0, jpeg_hw_irq_handler, DEVICE_DT_INST_GET(0), 0);

  HAL_NVIC_SetPriority(JPEG_IRQn, 8, 0);
  HAL_NVIC_EnableIRQ(JPEG_IRQn);

  LOG_INF("STM32 JPEG HW OK\n");

  JPEG_InitColorTables();

  return 0;
}

static int _jpeg_hw_decode(const struct device *dev, const uint8_t *src,
                           size_t src_size, uint8_t *dst) {

  struct jpeg_hw_data *dev_data = dev->data;

  if (src_size & 0x3) {
    LOG_ERR("JPEG HW decode : src_size must be multiple of 4");
    return -EINVAL;
  }

  memset(&dev_data->out_prop, 0, sizeof(dev_data->out_prop));
  atomic_set(&dev_data->data_ready, 0);

  dev_data->in_prop.src_addr = (uint32_t)src;
  dev_data->in_prop.dst_addr = (uint32_t)dst;
  dev_data->in_prop.frame_index = 0;
  dev_data->in_prop.src_size = src_size;
  // Input_frameSize = ((FrameSize+3)/4)*4;

  /* Start JPEG decoding with DMA method */
  int ret = HAL_JPEG_Decode_DMA(
      &dev_data->hjpeg, (uint8_t *)dev_data->in_prop.src_addr, CHUNK_SIZE_IN,
      (uint8_t *)dev_data->in_prop.dst_addr, CHUNK_SIZE_OUT);

  //LOG_INF("JPEG_Decode_DMA : src = %x dst = %x size = %d ret = %d",
  //        dev_data->in_prop.src_addr, dev_data->in_prop.dst_addr,
  //        dev_data->in_prop.src_size, ret);

  return ret == HAL_OK ? 0 : -EINVAL;
}

static int _jpeg_hw_poll(const struct device *dev, uint32_t timeout_ms,
                         struct jpeg_out_prop *prop) {
  uint32_t retry_count = 0;
  int ret = 0;
  struct jpeg_hw_data *dev_data = dev->data;
  while (1) {

    if (atomic_get(&dev_data->data_ready)) {
      break;
    }

    if (timeout_ms && retry_count >= timeout_ms) {
      ret = -EBUSY;
      break;
    }
    retry_count++;

    k_sleep(K_MSEC(1));
  }

  if (0 == ret) {
    atomic_set(&dev_data->data_ready, 0);
    *prop = dev_data->out_prop;
  }

  return ret;
}

static inline uint32_t _jpeg_get_in_size(JPEG_ConfTypeDef *pInfo,
                                         uint32_t NrMCUs) {

  switch (pInfo->ColorSpace) {
  case JPEG_GRAYSCALE_COLORSPACE: {
    return NrMCUs * GRAY_444_BLOCK_SIZE;
  }
  case JPEG_CMYK_COLORSPACE: {
    return NrMCUs * CMYK_444_BLOCK_SIZE;
  }
  case JPEG_YCBCR_COLORSPACE: {

    switch (pInfo->ChromaSubsampling) {
    case JPEG_444_SUBSAMPLING:
      return NrMCUs * YCBCR_444_BLOCK_SIZE;
    case JPEG_420_SUBSAMPLING:
      return NrMCUs * YCBCR_420_BLOCK_SIZE;
    case JPEG_422_SUBSAMPLING:
      return NrMCUs * YCBCR_422_BLOCK_SIZE;
    default: {
      LOG_ERR("Unknown ChromaSubsampling");
      return 0;
    }
    }
  }
  default: {
    LOG_ERR("Unknown ColorSpace");
    return 0;
  }
  }
  return 0;
}

static int _jpeg_color_convert_helper(const struct device *dev,
                                      struct jpeg_out_prop *prop,
                                      const uint8_t *src, uint8_t *dst) {

  JPEG_ConfTypeDef pInfo;

  pInfo.ColorSpace = prop->color_space;
  pInfo.ChromaSubsampling = prop->chroma;
  pInfo.ImageHeight = prop->width;
  pInfo.ImageWidth = prop->height;
  pInfo.ImageQuality = prop->quality;

  //LOG_INF("ColorSpace = %u, ChromaSubsampling = %u", pInfo.ColorSpace,
  //        pInfo.ChromaSubsampling);

  JPEG_YCbCrToRGB_Convert_Function convert_function;
  uint32_t ImageNbMCUs = 0;

  HAL_StatusTypeDef ret =
      JPEG_GetDecodeColorConvertFunc(&pInfo, &convert_function, &ImageNbMCUs);
  if (ret != HAL_OK || convert_function == NULL) {
    LOG_ERR("Failed JPEG_GetDecodeColorConvertFunc ret = %d", ret);
    return -EINVAL;
  }

  uint32_t ConvertedDataCount = 0;
  uint32_t inSize = _jpeg_get_in_size(&pInfo, ImageNbMCUs);

  //LOG_INF("ImageNbMCUs = %d size = %d", ImageNbMCUs, inSize);

  if (0 == inSize) {
    return -EINVAL;
  }

  uint32_t NrBlocks =
      convert_function(src, dst, 0, inSize, &ConvertedDataCount);

  //LOG_INF("Decoded image : NrBlocks = %d, size = %d", NrBlocks,
  //        ConvertedDataCount);

  return NrBlocks ? NrBlocks : -EINVAL;
}

static DEVICE_API(jpeg_hw, jpeg_hw_driver_api) = {
    .decode = _jpeg_hw_decode,
    .poll = _jpeg_hw_poll,
    .cc_helper = _jpeg_color_convert_helper,
};

#define JPEG_DMA_CHANNEL_INIT(stream, index, dir, src_dev, dest_dev)           \
  .stream = {                                                                  \
      .dma_dev = DEVICE_DT_GET(STM32_DMA_CTLR(index, dir)),                    \
      .dma_channel = DT_INST_DMAS_CELL_BY_NAME(index, dir, channel),           \
      .dma_cfg =                                                               \
          {                                                                    \
              .dma_slot = STM32_DMA_SLOT(index, dir, slot),                    \
              .channel_direction = src_dev##_TO_##dest_dev,                    \
              .dma_callback = dma_callback,                                    \
          },                                                                   \
  }

#define JPEG_HW_DEFINE(index)                                                  \
                                                                               \
  static struct jpeg_hw_data data##index = {                                   \
      JPEG_DMA_CHANNEL_INIT(tx, index, tx, PERIPHERAL, MEMORY),                \
      JPEG_DMA_CHANNEL_INIT(rx, index, rx, MEMORY, PERIPHERAL)};               \
                                                                               \
  static const struct jpeg_hw_config config##index;                            \
                                                                               \
  DEVICE_DT_INST_DEFINE(index, jpeg_hw_init, NULL, &data##index,               \
                        &config##index, POST_KERNEL,                           \
                        CONFIG_STM32_JPEG_INIT_PRIORITY, &jpeg_hw_driver_api);

DT_INST_FOREACH_STATUS_OKAY(JPEG_HW_DEFINE)