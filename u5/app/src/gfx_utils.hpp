
#pragma once

#include "stddef.h"
#include "stdint.h"

#include <lvgl.h>

namespace Gfx {

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
void fit(lv_obj_t *canvas, const GfxBuffer &src, const GfxBuffer &dst);

} // namespace Gfx
