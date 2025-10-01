
#pragma once

#include "stddef.h"
#include "stdint.h"

#include <lvgl.h>

namespace Gfx {

struct GfxRect {
  uint32_t x, y;
  uint32_t width, height;
};

struct GfxBuffer {
  uint8_t *buf;
  uint32_t width, height, stride;
  size_t size;
  uint32_t cf;
};

/**
 * @brief Fit source buffer onto destination
 *
 */
void fit(lv_obj_t *canvas, const GfxBuffer &src, const GfxBuffer &dst, const int32_t x = 0, const int32_t y = 0);

} // namespace Gfx
