#include "gf/utils.hpp"

lv_obj_t *allocLvROIRect(lv_obj_t *parent, uint32_t borderWidth, lv_color_t c) {
  lv_obj_t *rect = lv_obj_create(parent);
  lv_obj_set_pos(rect, 0, 0);
  lv_obj_set_size(rect, 1, 1);

  // Unfilled, just a thick border
  lv_obj_set_style_bg_opa(rect, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_border_width(rect, borderWidth, LV_PART_MAIN);
  lv_obj_set_style_border_opa(rect, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_border_color(rect, c, LV_PART_MAIN);
  lv_obj_set_style_radius(rect, 0, LV_PART_MAIN); // 0 = sharp corners
  lv_obj_set_style_pad_all(rect, 0, LV_PART_MAIN);
  lv_obj_set_style_outline_opa(rect, LV_OPA_TRANSP,
                               LV_PART_MAIN); // no focus outline
  lv_obj_set_style_shadow_opa(rect, LV_OPA_TRANSP, LV_PART_MAIN); // no shadow
  lv_obj_move_foreground(rect);
  lv_obj_add_flag(rect, LV_OBJ_FLAG_HIDDEN);

  return rect;
}

void cropFullFrameToRoi(lv_obj_t *parent, uint8_t *fullFb, uint8_t *roiFb,
                        lvgl::Size fullSz, lvgl::Size roiSz, uint32_t fullCf,
                        uint32_t roiCf, bool flip_y) {
  lvgl::Buffer src{};
  lvgl::Buffer dst{};

  uint32_t x = (fullSz.width - roiSz.width) / 2;
  // uint32_t y = (FULL_FRAME_HEIGHT - ROI_FRAME_HEIGHT) / 2;
  uint32_t y = flip_y ? 0 : fullSz.height - roiSz.height;

  uint32_t fullFbStride = fullSz.width * LV_COLOR_FORMAT_GET_SIZE(fullCf);
  uint32_t fullFbOffset = (y * fullFbStride) + x;
  uint32_t fullFbSize = roiSz.height * fullFbStride;

  src.buf = fullFb + fullFbOffset;
  src.width = roiSz.width;
  src.height = roiSz.height;
  src.stride = fullFbStride;
  src.cf = fullCf;
  src.size = fullFbSize - fullFbOffset;

  uint32_t roiFbStride = roiSz.width * LV_COLOR_FORMAT_GET_SIZE(roiCf);

  dst.buf = roiFb;
  dst.width = roiSz.width;
  dst.height = roiSz.height;
  dst.stride = roiFbStride;
  dst.cf = roiCf;
  dst.size = roiSz.height * roiFbStride;

  lvgl::fit(parent, src, dst, flip_y);
  // lv_task_handler();
}

void fitRoiFrameToThumbnail(lv_obj_t *parent, uint8_t *roiFb, uint8_t *tFb,
                            lvgl::Size roiSz, lvgl::Size tSz, uint32_t roiCf,
                            uint32_t tCf) {
  lvgl::Buffer src{};
  lvgl::Buffer dst{};

  uint32_t roiFbStride = roiSz.width * LV_COLOR_FORMAT_GET_SIZE(roiCf);

  src.buf = roiFb;
  src.width = roiSz.width;
  src.height = roiSz.height;
  src.stride = roiFbStride;
  src.cf = roiCf;
  src.size = roiSz.height * roiFbStride;

  uint32_t tnFbStride = tSz.width * LV_COLOR_FORMAT_GET_SIZE(tCf);

  dst.buf = tFb;
  dst.width = tSz.width;
  dst.height = tSz.height;
  dst.stride = tnFbStride;
  dst.cf = tCf;
  dst.size = tSz.height * tnFbStride;

  lvgl::fit(parent, src, dst);
  // lv_task_handler();
}

void displayFrame(lv_obj_t *parent, uint8_t *fb, lvgl::Size sz, uint32_t cf) {
  static lv_image_dsc_t img_dsc{};

  img_dsc.header.magic = LV_IMAGE_HEADER_MAGIC;
  img_dsc.header.w = sz.width;
  img_dsc.header.h = sz.height;
  img_dsc.header.stride = sz.width * LV_COLOR_FORMAT_GET_SIZE(cf);
  img_dsc.header.cf = cf; /* Set the color format */
  img_dsc.data_size = sz.width * sz.height * LV_COLOR_FORMAT_GET_SIZE(cf);
  img_dsc.data = fb;

  lv_img_set_src(parent, &img_dsc); // point the widget to our descriptor
}