
#include <iostream>
#include <vector>

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

#if defined(CONFIG_OPENCV_LIB)
#include "opencv2/opencv.hpp"
#include "opencv_utils.hpp"
#endif

#include "grinreflex.h"

static const struct device *video_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_camera));
static const struct device *display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
const struct device *jpeg_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_jpeg));

static lv_img_dsc_t video_img;
static lv_obj_t *vid_canvas;
static lv_obj_t *screen;

static uint8_t rgb_frame_buffer[CONFIG_GRINREFLEX_VIDEO_WIDTH * CONFIG_GRINREFLEX_VIDEO_HEIGHT * LV_COLOR_FORMAT_GET_SIZE(JPEG_COLOR_FORMAT)];

static uint8_t gray_frame_buffer[GRAY_FRAME_WIDTH * GRAY_FRAME_HEIGHT * LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_L8)];


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

    jpeg_color_convert_helper(jpeg_dev, &jpeg_prop, vbuf_ptr->buffer, rgb_frame_buffer);

#else
    video_img.data = (uint8_t *)vbuf_ptr->buffer;
    lv_img_set_src(screen, &video_img);
    lv_obj_align(screen, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_task_handler();
#endif

    Gfx::GfxBuffer src{};
    Gfx::GfxBuffer dst{};

    src.buf = rgb_frame_buffer;
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

}