

#pragma once

#include <gf/lvgl_utils.hpp>

lv_obj_t *allocLvROIRect(lv_obj_t *parent, uint32_t borderWidth, lv_color_t c);

void cropFullFrameToRoi(lv_obj_t *parent, uint8_t *fullFb, uint8_t *roiFb,
                        lvgl::Size fullSz, lvgl::Size roiSz, uint32_t fullCf,
                        uint32_t roiCf, bool flip_y = false);
void fitRoiFrameToThumbnail(lv_obj_t *parent, uint8_t *roiFb, uint8_t *tFb,
                            lvgl::Size roiSz, lvgl::Size tSz, uint32_t roiCf,
                            uint32_t tCf);
