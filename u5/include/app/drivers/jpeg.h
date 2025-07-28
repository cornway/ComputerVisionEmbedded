
#ifndef APP_DRIVERS_STM32_JPEG_HW_H_
#define APP_DRIVERS_STM32_JPEG_HW_H_

#include <zephyr/device.h>
#include <zephyr/toolchain.h>

typedef enum {
  JpegColorSpaceGray = 0,
  JpegColorSpaceYCBCR = 1,
  JpegColorSpaceCMYK = 2,
  JpegColorSpaceUnknown = 3,
} JpegColorSpace_E;

typedef enum {
  // 0-> 4:4:4 , 1-> 4:2:2, 2 -> 4:1:1, 3 -> 4:2:0
  JpegChromaSS_444 = 0,
  JpegChromaSS_422 = 1,
  JpegChromaSS_411 = 2,
  JpegChromaSS_420 = 3,
} JpegChromaSubSampling_E;

struct jpeg_out_prop {
  JpegColorSpace_E color_space;
  JpegChromaSubSampling_E chroma;
  uint32_t width;
  uint32_t height;
  uint32_t quality;
};

__subsystem struct jpeg_hw_driver_api {
  int (*decode)(const struct device *dev, const uint8_t *src, size_t src_size,
                uint8_t *dst);
  int (*poll)(const struct device *dev, uint32_t timeout_ms,
              struct jpeg_out_prop *prop);
  int (*cc_helper)(const struct device *dev, struct jpeg_out_prop *prop,
                   const uint8_t *src, uint8_t *dst);
};

static inline int jpeg_hw_decode(const struct device *dev, const uint8_t *src,
                                 size_t src_size, uint8_t *dst) {
  __ASSERT_NO_MSG(DEVICE_API_IS(jpeg_hw, dev));

  return DEVICE_API_GET(jpeg_hw, dev)->decode(dev, src, src_size, dst);
}

static inline int jpeg_hw_poll(const struct device *dev, uint32_t timeout_ms,
                               struct jpeg_out_prop *prop) {
  __ASSERT_NO_MSG(DEVICE_API_IS(jpeg_hw, dev));

  return DEVICE_API_GET(jpeg_hw, dev)->poll(dev, timeout_ms, prop);
}

static inline int jpeg_color_convert_helper(const struct device *dev,
                                            struct jpeg_out_prop *prop,
                                            const uint8_t *src, uint8_t *dst) {
  __ASSERT_NO_MSG(DEVICE_API_IS(jpeg_hw, dev));

  return DEVICE_API_GET(jpeg_hw, dev)->cc_helper(dev, prop, src, dst);
}

static inline int jpeg_hw_decode_blocking(const struct device *dev,
                                          const uint8_t *src, size_t src_size,
                                          uint8_t *dst,
                                          struct jpeg_out_prop *prop) {
  int ret = jpeg_hw_decode(dev, src, src_size, dst);
  if (ret) {
    return ret;
  }
  ret = jpeg_hw_poll(dev, 0, prop);
}

#endif /*APP_DRIVERS_STM32_JPEG_HW_H_*/
