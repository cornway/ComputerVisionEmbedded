#include "grinreflex_config.hpp"
#include "gfx_utils.hpp"

#include "grinreflex_utils.hpp"

lv_obj_t *allocLvROIRect(lv_obj_t *parent, uint32_t borderWidth) {
    lv_obj_t *rect = lv_obj_create(parent);
    lv_obj_set_pos(rect, 0, 0);
    lv_obj_set_size(rect, 1, 1);

    // Unfilled, just a thick border
    lv_obj_set_style_bg_opa(rect, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(rect, borderWidth, LV_PART_MAIN);
    lv_obj_set_style_border_opa(rect, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(rect, lv_palette_main(LV_PALETTE_GREEN), LV_PART_MAIN);
    lv_obj_set_style_radius(rect, 0, LV_PART_MAIN);      // 0 = sharp corners
    lv_obj_set_style_pad_all(rect, 0, LV_PART_MAIN);
    lv_obj_set_style_outline_opa(rect, LV_OPA_TRANSP, LV_PART_MAIN); // no focus outline
    lv_obj_set_style_shadow_opa(rect, LV_OPA_TRANSP, LV_PART_MAIN);  // no shadow
    lv_obj_move_foreground(rect);
    lv_obj_add_flag(rect, LV_OBJ_FLAG_HIDDEN);

    return rect;
}

void cropFullFrameToRoi(lv_obj_t *parent, uint8_t *fullFb, uint8_t *roiFb) {
    Gfx::GfxBuffer src{};
    Gfx::GfxBuffer dst{};

    uint32_t x = (FULL_FRAME_WIDTH - ROI_FRAME_WIDTH) / 2;
    //uint32_t y = (FULL_FRAME_HEIGHT - ROI_FRAME_HEIGHT) / 2;
#if defined(FULL_FRAME_FLIP_Y)
    uint32_t y = 0;
#else
    uint32_t y = FULL_FRAME_HEIGHT - ROI_FRAME_HEIGHT;
#endif

    uint32_t fullFbStride = FULL_FRAME_WIDTH * LV_COLOR_FORMAT_GET_SIZE(FULL_FRAME_COLOR_FORMAT);
    uint32_t fullFbOffset = (y * fullFbStride) + x;
    uint32_t fullFbSize = ROI_FRAME_HEIGHT * fullFbStride;

    src.buf = fullFb + fullFbOffset;
    src.width = ROI_FRAME_WIDTH;
    src.height = ROI_FRAME_HEIGHT;
    src.stride = fullFbStride;
    src.cf = FULL_FRAME_COLOR_FORMAT;
    src.size = fullFbSize - fullFbOffset;

    uint32_t roiFbStride = ROI_FRAME_WIDTH * LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_L8);

    dst.buf = roiFb;
    dst.width = ROI_FRAME_WIDTH;
    dst.height = ROI_FRAME_HEIGHT;
    dst.stride = roiFbStride;
    dst.cf = LV_COLOR_FORMAT_L8;
    dst.size = ROI_FRAME_HEIGHT * roiFbStride;


#if defined(FULL_FRAME_FLIP_Y)
    Gfx::fit(parent, src, dst, true);
#else
    Gfx::fit(parent, src, dst);
#endif
    //lv_task_handler();
}

void fitRoiFrameToThumbnail(lv_obj_t *parent, uint8_t *roiFb, uint8_t *tFb) {
    Gfx::GfxBuffer src{};
    Gfx::GfxBuffer dst{};

    uint32_t roiFbStride = ROI_FRAME_WIDTH * LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_L8);

    src.buf = roiFb;
    src.width = ROI_FRAME_WIDTH;
    src.height = ROI_FRAME_HEIGHT;
    src.stride = roiFbStride;
    src.cf = LV_COLOR_FORMAT_L8;
    src.size = ROI_FRAME_HEIGHT * roiFbStride;

    uint32_t tnFbStride = THUMBNAIL_FRAME_WIDTH * LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_L8);

    dst.buf = tFb;
    dst.width = THUMBNAIL_FRAME_WIDTH;
    dst.height = THUMBNAIL_FRAME_HEIGHT;
    dst.stride = tnFbStride;
    dst.cf = LV_COLOR_FORMAT_L8;
    dst.size = THUMBNAIL_FRAME_HEIGHT * tnFbStride;

    Gfx::fit(parent, src, dst);
    //lv_task_handler();
}
