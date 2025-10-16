
#pragma once

#include "stddef.h"
#include "stdint.h"

#include <lvgl.h>

namespace lvgl {

struct Size {
  Size (uint32_t w, uint32_t h) : width(w), height(h) {}
  uint32_t width, height;
};

struct Buffer {
  uint8_t *buf;
  uint32_t width, height, stride;
  size_t size;
  uint32_t cf;
};

/**
 * @brief Fit source buffer onto destination
 *
 */
void fit(lv_obj_t *canvas, const Buffer &src, const Buffer &dst, bool flip_y = false);

} // namespace Gfx
