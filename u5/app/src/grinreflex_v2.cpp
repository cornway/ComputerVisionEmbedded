
#include <iostream>
#include <vector>

#if defined(CONFIG_OPENCV_LIB)
#include "opencv2/opencv.hpp"
#include "opencv_utils.hpp"
#include "cascades.h"
#endif 

#include <lvgl.h>
#include <lvgl_input_device.h>
#include <stdio.h>
#include <string.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>

#include <stdio.h>

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(grinreflex_app);

#include "video.hpp"
#include "gfx_utils.hpp"

#include <app/drivers/jpeg.h>

#include "grinreflex.h"

static const struct device *video_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_camera));
static const struct device *display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
const struct device *jpeg_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_jpeg));

static lv_img_dsc_t video_img;
static lv_obj_t *vid_canvas;
static lv_obj_t *screen;

static uint8_t input_frame_buffer[CONFIG_GRINREFLEX_VIDEO_WIDTH * CONFIG_GRINREFLEX_VIDEO_HEIGHT * LV_COLOR_FORMAT_GET_SIZE(JPEG_COLOR_FORMAT)];

static uint8_t gray_frame_buffer[GRAY_FRAME_WIDTH * GRAY_FRAME_HEIGHT * LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_L8)];

#if defined(CONFIG_OPENCV_LIB)
static lv_obj_t *RedGreenDot;
static lv_obj_t *ROIRectFace;
static lv_obj_t *ROIRectSmile;
static cv::CascadeClassifier faceCascade;
static cv::CascadeClassifier smileCascade;
#endif /* defined(CONFIG_OPENCV_LIB) */

int init()
{
	if (!device_is_ready(display_dev)) {
		LOG_ERR("Display device not ready, aborting test");
		return -ENODEV;
	}

    if (!device_is_ready(video_dev)) {
		LOG_ERR("Video device not ready, aborting test");
		return -ENODEV;
	}

    if (!device_is_ready(jpeg_dev)) {
        printf("%s JPEG device not ready", jpeg_dev->name);
        return -ENODEV;
    }

#if defined(CONFIG_OPENCV_LIB)

    cv::setNumThreads(1);
    cv::setUseOptimized(false);


    if (false == loadCascade(faceCascade, cascade_face_data(), cascade_face_len())) {
        return -3;
    }

    if (false == loadCascade(smileCascade, cascade_smile_data(), cascade_smile_len())) {
        return -3;
    }

#endif /* defined(CONFIG_OPENCV_LIB) */

    Video::setup();

    vid_canvas = lv_canvas_create(lv_screen_active());
    lv_canvas_fill_bg(vid_canvas, lv_color_hex3(0xccc), LV_OPA_COVER);
    lv_obj_center(vid_canvas);

    display_blanking_off(display_dev);

    video_img.header.w = CONFIG_GRINREFLEX_VIDEO_WIDTH;
	video_img.header.h = CONFIG_GRINREFLEX_VIDEO_HEIGHT;
#if defined(CONFIG_GRINREFLEX_JPEG_VIDEO)
	video_img.data_size = CONFIG_GRINREFLEX_VIDEO_WIDTH * CONFIG_GRINREFLEX_VIDEO_HEIGHT * LV_COLOR_FORMAT_GET_SIZE(JPEG_COLOR_FORMAT);
	video_img.header.cf = JPEG_COLOR_FORMAT;
#else
	video_img.data_size = CONFIG_GRINREFLEX_VIDEO_WIDTH * CONFIG_GRINREFLEX_VIDEO_HEIGHT * LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_NATIVE);
	video_img.header.cf = LV_COLOR_FORMAT_NATIVE;
#endif
    screen = lv_img_create(lv_scr_act());

#if defined(CONFIG_OPENCV_LIB)
    RedGreenDot = lv_obj_create(lv_scr_act());
    lv_obj_set_size(RedGreenDot, 30, 30);                    // dot size
    lv_obj_set_style_radius(RedGreenDot, LV_RADIUS_CIRCLE, 0); // make it round
    lv_obj_set_style_border_width(RedGreenDot, 0, 0);
    lv_obj_set_style_bg_opa(RedGreenDot, LV_OPA_COVER, 0);
    lv_obj_clear_flag(RedGreenDot, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(RedGreenDot, LV_ALIGN_LEFT_MID, 4, 0);      // 4px from left
    lv_obj_move_foreground(RedGreenDot);                     // keep it on top

    ROIRectFace = lv_obj_create(lv_scr_act());
    lv_obj_set_pos(ROIRectFace, 0, 0);
    lv_obj_set_size(ROIRectFace, 1, 1);

    // Unfilled, just a thick border
    lv_obj_set_style_bg_opa(ROIRectFace, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(ROIRectFace, 4, LV_PART_MAIN);
    lv_obj_set_style_border_opa(ROIRectFace, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(ROIRectFace, lv_palette_main(LV_PALETTE_GREEN), LV_PART_MAIN);
    lv_obj_set_style_radius(ROIRectFace, 0, LV_PART_MAIN);      // 0 = sharp corners
    lv_obj_set_style_pad_all(ROIRectFace, 0, LV_PART_MAIN);
    lv_obj_set_style_outline_opa(ROIRectFace, LV_OPA_TRANSP, LV_PART_MAIN); // no focus outline
    lv_obj_set_style_shadow_opa(ROIRectFace, LV_OPA_TRANSP, LV_PART_MAIN);  // no shadow
    lv_obj_move_foreground(ROIRectFace);
    lv_obj_add_flag(ROIRectFace, LV_OBJ_FLAG_HIDDEN);

    ROIRectSmile = lv_obj_create(lv_scr_act());
    lv_obj_set_pos(ROIRectSmile, 0, 0);
    lv_obj_set_size(ROIRectSmile, 1, 1);

    // Unfilled, just a thick border
    lv_obj_set_style_bg_opa(ROIRectSmile, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(ROIRectSmile, 2, LV_PART_MAIN);
    lv_obj_set_style_border_opa(ROIRectSmile, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(ROIRectSmile, lv_palette_main(LV_PALETTE_GREEN), LV_PART_MAIN);
    lv_obj_set_style_radius(ROIRectSmile, 0, LV_PART_MAIN);      // 0 = sharp corners
    lv_obj_set_style_pad_all(ROIRectSmile, 0, LV_PART_MAIN);
    lv_obj_set_style_outline_opa(ROIRectSmile, LV_OPA_TRANSP, LV_PART_MAIN); // no focus outline
    lv_obj_set_style_shadow_opa(ROIRectSmile, LV_OPA_TRANSP, LV_PART_MAIN);  // no shadow
    lv_obj_move_foreground(ROIRectSmile);
    lv_obj_add_flag(ROIRectSmile, LV_OBJ_FLAG_HIDDEN);

#endif

    return 0;
}

int loop()
{

    struct video_buffer vbuf{};
    struct video_buffer *vbuf_ptr = &vbuf;
    struct jpeg_out_prop jpeg_prop;

    vbuf_ptr->type = VIDEO_BUF_TYPE_OUTPUT;
    
    int err = video_dequeue(video_dev, &vbuf_ptr, K_FOREVER);
    if (err) {
        LOG_ERR("Unable to dequeue video buf");
        return 0;
    }

#if defined(CONFIG_GRINREFLEX_JPEG_VIDEO)
    //jpeg_hw_decode(jpeg_dev, (uint8_t *)vbuf_ptr->buffer, vbuf_ptr->bytesused, jpeg_frame_buffer);

    jpeg_hw_poll(jpeg_dev, 0, &jpeg_prop);

    //printf("JPEG done w = %d, h = %d, color = %d chroma = %d\n", jpeg_prop.width,
    //        jpeg_prop.height, jpeg_prop.color_space, jpeg_prop.chroma);

    jpeg_color_convert_helper(jpeg_dev, &jpeg_prop, vbuf_ptr->buffer, input_frame_buffer);

#else
    video_img.data = (uint8_t *)vbuf_ptr->buffer;
    lv_img_set_src(screen, &video_img);
    lv_obj_align(screen, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_task_handler();
#endif

#if !defined(CONFIG_OPENCV_LIB)
    Gfx::GfxBuffer src{};
    Gfx::GfxBuffer dst{};

    src.buf = input_frame_buffer;
    src.width = CONFIG_GRINREFLEX_VIDEO_WIDTH;
    src.height = CONFIG_GRINREFLEX_VIDEO_HEIGHT;
    src.stride = CONFIG_GRINREFLEX_VIDEO_WIDTH * LV_COLOR_FORMAT_GET_SIZE(JPEG_COLOR_FORMAT);
    src.cf = JPEG_COLOR_FORMAT;
    src.size = CONFIG_GRINREFLEX_VIDEO_HEIGHT * CONFIG_GRINREFLEX_VIDEO_WIDTH * LV_COLOR_FORMAT_GET_SIZE(JPEG_COLOR_FORMAT);

    dst.buf = gray_frame_buffer;
    dst.width = GRAY_FRAME_WIDTH;
    dst.height = GRAY_FRAME_HEIGHT;
    dst.stride = GRAY_FRAME_WIDTH * LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_L8);
    dst.cf = LV_COLOR_FORMAT_L8;
    dst.size = sizeof(gray_frame_buffer);

    Gfx::fit(vid_canvas, src, dst);
    lv_task_handler();

    err = video_enqueue(video_dev, vbuf_ptr);
    if (err) {
        LOG_ERR("Unable to requeue video buf");
        return 0;
    }
#else /* !defined(CONFIG_OPENCV_LIB) */

    Gfx::GfxBuffer src{};
    Gfx::GfxBuffer dst{};

    src.buf = input_frame_buffer;
    src.width = CONFIG_GRINREFLEX_VIDEO_WIDTH;
    src.height = CONFIG_GRINREFLEX_VIDEO_HEIGHT;
    src.stride = CONFIG_GRINREFLEX_VIDEO_WIDTH * LV_COLOR_FORMAT_GET_SIZE(JPEG_COLOR_FORMAT);
    src.cf = JPEG_COLOR_FORMAT;
    src.size = sizeof(input_frame_buffer);

    dst.buf = gray_frame_buffer;
    dst.width = GRAY_FRAME_WIDTH;
    dst.height = GRAY_FRAME_HEIGHT;
    dst.stride = GRAY_FRAME_WIDTH * LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_L8);
    dst.cf = LV_COLOR_FORMAT_L8;
    dst.size = sizeof(gray_frame_buffer);

    Gfx::fit(vid_canvas, src, dst);
    lv_task_handler();

    err = video_enqueue(video_dev, vbuf_ptr);
    if (err) {
        LOG_ERR("Unable to requeue video buf");
        return 0;
    }

    cv::Mat matGraySmall(GRAY_FRAME_HEIGHT, GRAY_FRAME_WIDTH, CV_8UC1, gray_frame_buffer);

    if (JPEG_COLOR_FORMAT != LV_COLOR_FORMAT_L8) {
        LOG_ERR("Format is not supported");
        return -3;
    }

    cv::Mat matGrayFull(CONFIG_GRINREFLEX_VIDEO_HEIGHT, CONFIG_GRINREFLEX_VIDEO_WIDTH, CV_8UC1, input_frame_buffer);

    cv::Rect faceROIMax{};
    faceROIMax.width = GRAY_FRAME_WIDTH;
    faceROIMax.height = GRAY_FRAME_HEIGHT;

    std::vector<cv::Rect> objects = detectFaceAndSmile(
        faceCascade,
        smileCascade,
        matGraySmall,
        matGrayFull,
        faceROIMax
    );

    lv_color_t c = !objects.empty() ? lv_palette_main(LV_PALETTE_GREEN)
                                    : lv_palette_main(LV_PALETTE_RED);
    lv_obj_set_style_bg_color(RedGreenDot, c, 0);

    if (objects.size()) {
        lv_obj_remove_flag(ROIRectFace, LV_OBJ_FLAG_HIDDEN);

        auto object = objects[0];
        lv_obj_align_to(ROIRectFace, vid_canvas, LV_ALIGN_TOP_LEFT, object.x, object.y);
        lv_obj_set_size(ROIRectFace, object.width, object.height);

    } else {
        lv_obj_add_flag(ROIRectFace, LV_OBJ_FLAG_HIDDEN);
    }

    if (objects.size() > 1) {
        auto object = objects[1];
        lv_obj_align_to(ROIRectSmile, vid_canvas, LV_ALIGN_TOP_LEFT, object.x, object.y);
        lv_obj_set_size(ROIRectSmile, object.width, object.height);

        lv_obj_remove_flag(ROIRectSmile, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(ROIRectSmile, LV_OBJ_FLAG_HIDDEN);
    }

#endif /* !defined(CONFIG_OPENCV_LIB) */
    return 0;
}