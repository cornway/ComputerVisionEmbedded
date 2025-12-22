/**
 ******************************************************************************
 * @file    main.c
 * @author  GPM Application Team
 *
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

#include <zephyr/debug/cpu_load.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/video.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/__assert.h>
#include <zephyr/sys/util.h>

#include <font/lv_font.h>
#include <lvgl.h>

#include <stdarg.h>

#include "model.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

// #define DISPLAY_WIDTH DT_PROP(DT_CHOSEN(zephyr_display), width)
// #define DISPLAY_HEIGHT DT_PROP(DT_CHOSEN(zephyr_display), height)
#define DISPLAY_WIDTH 224
#define DISPLAY_HEIGHT 224
#define DISPLAY_BPP 2

#define NN_WIDTH CONFIG_NPU_WIDTH
#define NN_HEIGHT CONFIG_NPU_HEIGHT
#define NN_BPP 3
#define NPU_BUFFER_POOL_ALIGN 32

static struct k_thread nn_thread;
static K_THREAD_STACK_DEFINE(nn_thread_stack, 4096);

static int display_setup(const struct device *const display_dev) {
  int ret;

  ret = display_set_pixel_format(display_dev, PIXEL_FORMAT_RGB_565);
  if (ret) {
    LOG_INF("Failed: display_set_pixel_format ret = %d", ret);
  }

  ret = display_blanking_off(display_dev);
  __ASSERT_NO_MSG(ret == 0);

  return 0;
}

static void print_text(lv_layer_t *layer, int x, int y, lv_text_align_t align,
                       char *fmt, ...) {
  lv_draw_label_dsc_t tdsc;
  char text_buffer[128];
  lv_area_t coords;
  va_list ap;

  va_start(ap, fmt);
  vsnprintf(text_buffer, sizeof(text_buffer), fmt, ap);

  /* shadow */
  lv_draw_label_dsc_init(&tdsc);
  tdsc.color = lv_color_make(16, 16, 16);
  tdsc.font = &lv_font_montserrat_8;
  coords.x1 = x + 1;
  coords.y1 = y + 1;
  coords.x2 = DISPLAY_WIDTH;
  coords.y2 = DISPLAY_HEIGHT;
  tdsc.text = text_buffer;
  tdsc.text_local = 1;
  tdsc.align = align;
  lv_draw_label(layer, &tdsc, &coords);

  /* text */
  tdsc.color = lv_color_make(255, 255, 255);
  coords.x1 = x;
  coords.y1 = y;
  lv_draw_label(layer, &tdsc, &coords);
  va_end(ap);
}

static void decorate_canvas(lv_obj_t *canvas) {
  int latest_inference_time = model_get_latest_inference_time();
  const lv_font_t *font = &lv_font_montserrat_8;
  lv_draw_rect_dsc_t rdsc;
  struct dbox boxes[10];
  lv_area_t coords;
  lv_layer_t layer;
  struct dbox *b;
  uint32_t ver;
  int load;
  int nb;
  int i;

  ver = sys_kernel_version_get();
  nb = model_get_boxes(boxes, ARRAY_SIZE(boxes));

  lv_canvas_init_layer(canvas, &layer);
  /* Draw banner */
  print_text(&layer, 0, 0, LV_TEXT_ALIGN_LEFT, "People Detection demo");
  print_text(&layer, 0, font->line_height, LV_TEXT_ALIGN_LEFT,
             "Powered by zephyr OS %d.%d.%d", SYS_KERNEL_VER_MAJOR(ver),
             SYS_KERNEL_VER_MINOR(ver), SYS_KERNEL_VER_PATCHLEVEL(ver));

  /* Draw inference time */
  print_text(&layer, 0, font->line_height * 2, LV_TEXT_ALIGN_LEFT,
             "Inference %d ms", latest_inference_time);
  latest_inference_time = latest_inference_time ? latest_inference_time : 1;
  print_text(&layer, 0, font->line_height * 3, LV_TEXT_ALIGN_LEFT,
             "Fps       %d", 1000 / latest_inference_time);
#ifdef CONFIG_CPU_LOAD
  load = cpu_load_get(1);
  print_text(&layer, 0, font->line_height * 4, LV_TEXT_ALIGN_LEFT,
             "cpu load  %d.%d%%", load / 10, load % 10);
#else
  (void)load;
#endif

  /* Draw boxes */
  lv_draw_rect_dsc_init(&rdsc);
  rdsc.border_color = lv_palette_main(LV_PALETTE_RED);
  rdsc.border_width = 8;
  rdsc.radius = 10;
  rdsc.bg_opa = 0;
  for (i = 0; i < nb; i++) {
    b = &boxes[i];
    coords.x1 = b->x;
    coords.y1 = b->y;
    coords.x2 = b->x + b->w;
    coords.y2 = b->y + b->h;
    lv_draw_rect(&layer, &rdsc, &coords);
  }

  lv_canvas_finish_layer(canvas, &layer);
}

static int video_crop_setup(const struct device *const video_dev,
                            uint32_t sensor_width, uint32_t sensor_height) {
  const float ratioy = (float)sensor_height / DISPLAY_HEIGHT;
  const float ratiox = (float)sensor_width / DISPLAY_WIDTH;
  const float ratio = MIN(ratiox, ratioy);
  struct video_selection crop = {
      .type = VIDEO_BUF_TYPE_OUTPUT,
  };
  int ret;

  __ASSERT_NO_MSG(ratio >= 1);
  __ASSERT_NO_MSG(ratio < 64);

  crop.target = VIDEO_SEL_TGT_CROP;
  crop.rect.width = DISPLAY_WIDTH * ratio;
  crop.rect.height = DISPLAY_HEIGHT * ratio;
  crop.rect.left = (sensor_width - crop.rect.width + 1) / 2;
  crop.rect.top = (sensor_height - crop.rect.height + 1) / 2;

  ret = video_set_selection(video_dev, &crop);
  __ASSERT_NO_MSG(ret == 0);

  return 0;
}

static int video_resize_setup(const struct device *const video_dev,
                              uint32_t width, uint32_t height) {
  struct video_selection comp = {
      .type = VIDEO_BUF_TYPE_OUTPUT,
  };
  int ret;

  LOG_INF("video_resize_setup: %d x %d", width, height);

  comp.target = VIDEO_SEL_TGT_COMPOSE;
  comp.rect.left = 0;
  comp.rect.top = 0;
  comp.rect.width = width;
  comp.rect.height = height;

  ret = video_set_selection(video_dev, &comp);
  __ASSERT_NO_MSG(ret == 0);

  return 0;
}

static int video_alloc_and_queue_buffers(const struct device *const video_dev,
                                         size_t bsize, int align) {
  struct video_buffer *buffers[2];
  int ret;
  int i;

  for (i = 0; i < ARRAY_SIZE(buffers); i++) {
    buffers[i] = video_buffer_aligned_alloc(bsize, align, K_FOREVER);
    __ASSERT_NO_MSG(buffers[i]);
    buffers[i]->type = VIDEO_BUF_TYPE_OUTPUT;
    ret = video_enqueue(video_dev, buffers[i]);
    __ASSERT_NO_MSG(ret == 0);
    LOG_INF("buffer %p(%p) queue to %s pipe", (void *)buffers[i],
            (void *)buffers[i]->buffer, video_dev->name);
  }

  return 0;
}

static int video_main_setup(const struct device *const video_dev,
                            uint32_t sensor_width, uint32_t sensor_height) {
  struct video_format fmt;
  size_t bsize;
  int ret;

  /* setup crop, compose and output */
  ret = video_crop_setup(video_dev, sensor_width, sensor_height);
  __ASSERT_NO_MSG(ret == 0);
  ret = video_resize_setup(video_dev, DISPLAY_WIDTH, DISPLAY_HEIGHT);
  __ASSERT_NO_MSG(ret == 0);
  fmt.pixelformat = VIDEO_FOURCC_FROM_STR("RGBP");
  fmt.width = DISPLAY_WIDTH;
  fmt.height = DISPLAY_HEIGHT;
  fmt.pitch = fmt.width * DISPLAY_BPP;
  ret = video_set_format(video_dev, &fmt);
  __ASSERT_NO_MSG(ret == 0);

  /* alloc buffers and queue them */
  bsize = fmt.pitch * fmt.height;
  ret = video_alloc_and_queue_buffers(video_dev, bsize,
                                      CONFIG_VIDEO_BUFFER_POOL_ALIGN);
  __ASSERT_NO_MSG(ret == 0);

  return 0;
}

static int video_aux_setup(const struct device *const video_dev,
                           uint32_t sensor_width, uint32_t sensor_height) {
  struct video_format fmt;
  size_t bsize;
  int ret;

  /* setup crop, compose and output */
  ret = video_crop_setup(video_dev, sensor_width, sensor_height);
  __ASSERT_NO_MSG(ret == 0);
  ret = video_resize_setup(video_dev, NN_WIDTH, NN_HEIGHT);
  __ASSERT_NO_MSG(ret == 0);
  fmt.pixelformat = VIDEO_FOURCC_FROM_STR("RGB3");
  fmt.width = NN_WIDTH;
  fmt.height = NN_HEIGHT;
  fmt.pitch = fmt.width * NN_BPP;
  ret = video_set_format(video_dev, &fmt);
  __ASSERT_NO_MSG(ret == 0);

  /* alloc buffers and queue them */
  bsize = fmt.pitch * fmt.height;
  ret = video_alloc_and_queue_buffers(video_dev, bsize, NPU_BUFFER_POOL_ALIGN);
  __ASSERT_NO_MSG(ret == 0);

  return 0;
}

static int video_setup(const struct device *const main_dev,
                       const struct device *const aux_dev) {
  struct video_format fmt;
  uint32_t sensor_height;
  uint32_t sensor_width;
  int ret;

  /* get default format to get sensor width / height */
  ret = video_get_format(main_dev, &fmt);
  __ASSERT_NO_MSG(ret == 0);
  sensor_height = fmt.height;
  sensor_width = fmt.width;

  LOG_INF("Default format: %dx%d", sensor_width, sensor_height);

  ret = video_main_setup(main_dev, sensor_width, sensor_height);
  __ASSERT_NO_MSG(ret == 0);

  ret = video_aux_setup(aux_dev, sensor_width, sensor_height);
  __ASSERT_NO_MSG(ret == 0);

  return 0;
}

int main() {
  const struct device *const camera_aux_dev =
      DEVICE_DT_GET(DT_CHOSEN(zephyr_camera_aux));
  const struct device *const display_dev =
      DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
  const struct device *const video_dev =
      DEVICE_DT_GET(DT_CHOSEN(zephyr_camera));
  struct video_buffer *vbuf;
  lv_obj_t *canvas;
  k_tid_t nn_tid;
  int ret;

  __ASSERT_NO_MSG(device_is_ready(video_dev));
  __ASSERT_NO_MSG(device_is_ready(camera_aux_dev));
  __ASSERT_NO_MSG(device_is_ready(display_dev));

  /* move main thread priority to lowest one so we let others thread a chance to
   * run */
  k_thread_priority_set(k_current_get(), K_LOWEST_APPLICATION_THREAD_PRIO);

  /* create thread for nn process */
  nn_tid = k_thread_create(
      &nn_thread, nn_thread_stack, K_THREAD_STACK_SIZEOF(nn_thread_stack),
      model_thread_ep, (void *)camera_aux_dev, NULL, NULL, 0, 0, K_NO_WAIT);
  __ASSERT_NO_MSG(nn_tid);

  /* Configure display */
  ret = display_setup(display_dev);
  __ASSERT_NO_MSG(ret == 0);

  /* Configure video pipe */
  ret = video_setup(video_dev, camera_aux_dev);
  __ASSERT_NO_MSG(ret == 0);

  /* Start main pipe */
  LOG_INF("Starting main pipe");
  ret = video_stream_start(video_dev, VIDEO_BUF_TYPE_OUTPUT);
  __ASSERT_NO_MSG(ret == 0);

  LOG_INF("Starting aux pipe");
  ret = video_stream_start(camera_aux_dev, VIDEO_BUF_TYPE_OUTPUT);
  __ASSERT_NO_MSG(ret == 0);

  LOG_INF("STARTING");

  canvas = lv_canvas_create(lv_scr_act());
  while (1) {
    ret = video_dequeue(video_dev, &vbuf, K_FOREVER);
    __ASSERT_NO_MSG(ret == 0);

    lv_canvas_set_buffer(canvas, vbuf->buffer, DISPLAY_WIDTH, DISPLAY_HEIGHT,
                         LV_COLOR_FORMAT_RGB565);
    decorate_canvas(canvas);
    lv_timer_handler();

    ret = video_enqueue(video_dev, vbuf);
    __ASSERT_NO_MSG(ret == 0);
  }

  return 0;
}
