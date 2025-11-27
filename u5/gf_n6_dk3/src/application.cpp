
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

#include <app/drivers/jpeg.h>

#include "config.hpp"
#include "gf/lvgl_utils.hpp"
#include "gf/utils.hpp"
#include "gf/video.hpp"
#include "application.h"

static const struct device *video_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_camera));
static const struct device *display_dev =
    DEVICE_DT_GET(DT_CHOSEN(zephyr_display));

#if defined(CONFIG_STM32_JPEG)
const struct device *jpeg_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_jpeg));
#endif

static uint8_t
    fullFrameBuffer[FULL_FRAME_WIDTH * FULL_FRAME_HEIGHT *
                    LV_COLOR_FORMAT_GET_SIZE(FULL_FRAME_COLOR_FORMAT)];
static uint8_t roiFrameBuffer[ROI_FRAME_WIDTH * ROI_FRAME_HEIGHT *
                              LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_L8)];
static uint8_t
    thumbnailFrameBuffer[THUMBNAIL_FRAME_WIDTH * THUMBNAIL_FRAME_HEIGHT *
                         LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_L8)];

static lv_obj_t *screen_canvas;
static lv_obj_t *dummy_canvas;

int init() {
  if (!device_is_ready(display_dev)) {
    LOG_ERR("Display device not ready, aborting test");
    return -ENODEV;
  }

  if (!device_is_ready(video_dev)) {
    LOG_ERR("Video device not ready, aborting test");
    return -ENODEV;
  }
#if defined(CONFIG_STM32_JPEG)
  if (!device_is_ready(jpeg_dev)) {
    printf("%s JPEG device not ready", jpeg_dev->name);
    return -ENODEV;
  }
#endif
  Video::setup();

  dummy_canvas = lv_canvas_create(NULL);
  screen_canvas = lv_canvas_create(lv_screen_active());
  lv_canvas_fill_bg(screen_canvas, lv_color_hex3(0xccc), LV_OPA_COVER);
  lv_obj_center(screen_canvas);

  display_blanking_off(display_dev);
  lv_task_handler();

  return 0;
}

int loop() {

  struct video_buffer vbuf {};
  struct video_buffer *vbuf_ptr = &vbuf;
  struct jpeg_out_prop jpeg_prop;

  vbuf_ptr->type = VIDEO_BUF_TYPE_OUTPUT;

  int err = video_dequeue(video_dev, &vbuf_ptr, K_FOREVER);
  if (err) {
    LOG_ERR("Unable to dequeue video buf");
    return 0;
  }

#if defined(CONFIG_GRINREFLEX_JPEG_VIDEO)
  // jpeg_hw_decode(jpeg_dev, (uint8_t *)vbuf_ptr->buffer, vbuf_ptr->bytesused,
  // jpeg_frame_buffer);

  jpeg_hw_poll(jpeg_dev, 0, &jpeg_prop);

  // printf("JPEG done w = %d, h = %d, color = %d chroma = %d\n",
  // jpeg_prop.width,
  //         jpeg_prop.height, jpeg_prop.color_space, jpeg_prop.chroma);

  jpeg_color_convert_helper(jpeg_dev, &jpeg_prop, vbuf_ptr->buffer,
                            fullFrameBuffer);

#else
  memcpy(fullFrameBuffer, vbuf_ptr->buffer, vbuf_ptr->bytesused);
#endif

  err = video_enqueue(video_dev, vbuf_ptr);
  if (err) {
    LOG_ERR("Unable to requeue video buf");
  }

  cropFullFrameToRoi(dummy_canvas, fullFrameBuffer, roiFrameBuffer,
                     lvgl::Size(FULL_FRAME_WIDTH, FULL_FRAME_HEIGHT),
                     lvgl::Size(ROI_FRAME_WIDTH, ROI_FRAME_HEIGHT),
                     FULL_FRAME_COLOR_FORMAT, LV_COLOR_FORMAT_L8,
                     FULL_FRAME_FLIP_Y);

  fitRoiFrameToThumbnail(
      screen_canvas, roiFrameBuffer, thumbnailFrameBuffer,
      lvgl::Size(ROI_FRAME_WIDTH, ROI_FRAME_HEIGHT),
      lvgl::Size(THUMBNAIL_FRAME_WIDTH, THUMBNAIL_FRAME_HEIGHT),
      LV_COLOR_FORMAT_L8, LV_COLOR_FORMAT_L8);
  lv_task_handler();

  k_msleep(50);

  return 0;
}