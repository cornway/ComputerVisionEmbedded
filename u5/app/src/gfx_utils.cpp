

#include <lvgl.h>

#include "gfx_utils.hpp"

namespace Gfx {

void fit(const GfxBuffer &src, const GfxBuffer &dst) {

  lv_image_dsc_t src_img = {
      .header =
          {
              .magic = LV_IMAGE_HEADER_MAGIC,
              .cf = src.cf,
              .w = src.width,
              .h = src.height,
              .stride = src.stride,
          },
      .data_size = src.size,
      .data = src.buf,
  };

  // TODO: don't use screen ?
  lv_obj_t *canvas = lv_canvas_create(lv_screen_active());

  lv_canvas_set_buffer(canvas, dst.buf, dst.width, dst.height,
                       static_cast<lv_color_format_t>(dst.cf));
  // lv_canvas_fill_bg(canvas, lv_color_hex3(0xccc), LV_OPA_COVER);
  lv_obj_center(canvas);

  lv_layer_t layer;
  lv_canvas_init_layer(canvas, &layer);
  lv_draw_image_dsc_t img_dsc;
  lv_draw_image_dsc_init(&img_dsc);

  img_dsc.src = &src_img;

  img_dsc.scale_x = (LV_SCALE_NONE * (int32_t)(dst.width)) / src.width;
  img_dsc.scale_y = (LV_SCALE_NONE * (int32_t)(dst.height)) / src.height;

  lv_area_t coords_img = {0, 0, dst.width - 1, dst.height - 1};

  lv_draw_image(&layer, &img_dsc, &coords_img);

  lv_canvas_finish_layer(canvas, &layer);
}

} // namespace Gfx