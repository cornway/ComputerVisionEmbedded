

#pragma once

#include <lvgl.h>

lv_obj_t *allocLvROIRect(lv_obj_t *parent, uint32_t borderWidth);

void cropFullFrameToRoi(lv_obj_t *parent, uint8_t *fullFb, uint8_t *roiFb);
void fitRoiFrameToThumbnail(lv_obj_t *parent, uint8_t *roiFb, uint8_t *tFb);
