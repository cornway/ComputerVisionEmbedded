
#include <iostream>
#include <vector>

#if defined(CONFIG_OPENCV_LIB)
#include "cascades.h"
#include "gf/opencv_utils.hpp"
#include "opencv2/opencv.hpp"
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

#include "gf/lvgl_utils.hpp"
#include "gf/video.hpp"

#include <app/drivers/jpeg.h>

#include "config.hpp"
#include "gf/utils.hpp"
#include "grinreflex.h"

using namespace gf_cv;

static const struct device *video_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_camera));

static const struct device *display_dev =
    DEVICE_DT_GET(DT_CHOSEN(zephyr_display));

const struct device *jpeg_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_jpeg));

#if DT_NODE_EXISTS(DT_ALIAS(led0))
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
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

static lv_obj_t *ROIRectFace;
static lv_obj_t *ROIRectSmile;
static cv::CascadeClassifier faceCascade;
static cv::CascadeClassifier smileCascade;

int init() {
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

#if DT_NODE_EXISTS(DT_ALIAS(led0))
  if (!gpio_is_ready_dt(&led)) {
    return -ENODEV;
  }

  if (gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE) < 0) {
    return -ENODEV;
  }
#endif

  cv::setNumThreads(1);
  cv::setUseOptimized(false);

  if (false ==
      loadCascade(faceCascade, cascade_face_data(), cascade_face_len())) {
    return -3;
  }

  if (false ==
      loadCascade(smileCascade, cascade_smile_data(), cascade_smile_len())) {
    return -3;
  }

  Video::setup();

  dummy_canvas = lv_canvas_create(NULL);
  screen_canvas = lv_canvas_create(lv_screen_active());
  lv_canvas_fill_bg(screen_canvas, lv_color_hex3(0xccc), LV_OPA_COVER);
  lv_obj_center(screen_canvas);

  display_blanking_off(display_dev);

  ROIRectFace =
      allocLvROIRect(lv_scr_act(), 4, lv_palette_main(LV_PALETTE_GREEN));
  ROIRectSmile =
      allocLvROIRect(lv_scr_act(), 4, lv_palette_main(LV_PALETTE_RED));

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

  // jpeg_hw_decode(jpeg_dev, (uint8_t *)vbuf_ptr->buffer, vbuf_ptr->bytesused,
  // jpeg_frame_buffer);

  jpeg_hw_poll(jpeg_dev, 0, &jpeg_prop);

  // printf("JPEG done w = %d, h = %d, color = %d chroma = %d\n",
  // jpeg_prop.width,
  //         jpeg_prop.height, jpeg_prop.color_space, jpeg_prop.chroma);

  jpeg_color_convert_helper(jpeg_dev, &jpeg_prop, vbuf_ptr->buffer,
                            fullFrameBuffer);

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

  // Return buffer back, we don't need it anymore, thank you
  err = video_enqueue(video_dev, vbuf_ptr);
  if (err) {
    LOG_ERR("Unable to requeue video buf");
    return 0;
  }

  cv::Mat matGraySmall(THUMBNAIL_FRAME_HEIGHT, THUMBNAIL_FRAME_WIDTH, CV_8UC1,
                       thumbnailFrameBuffer);
  cv::Mat matGrayFull(ROI_FRAME_HEIGHT, ROI_FRAME_WIDTH, CV_8UC1,
                      roiFrameBuffer);

  // This is the best configuration I could find, 64 x 30 works even better
  // But that may miss smile area
  auto smileCascadeSize = smileCascade.getOriginalWindowSize();
  cv::Rect faceROIMax(0, 0, smileCascadeSize.width * 2,
                      smileCascadeSize.height * 2);

  auto sigmaX = Range<float>{0.85, 0.85, 0.01};
  BlurStage blurStage = BlurStage(sigmaX);

  auto _gamma = Range<float>{0.30, 0.50, 0.05};
  GammaStage gammaStage = GammaStage(_gamma);

  auto d = Range<int>{3, 3, 1};
  auto sigmaColor = Range<float>{20.0, 20.0, 10.0};
  auto sigmaSpace = Range<float>{50.0, 50.0, 1.0};
  BilateralStage bilateralStage = BilateralStage(d, sigmaColor, sigmaSpace);

  auto clipLimit = Range<float>{5.0, 5.0, 1.0};
  auto tileSize = Range<int>{4, 4, 1};
  ClaheStage claheStage = ClaheStage(clipLimit, tileSize);

  claheStage.setNextStage(&bilateralStage)
      .setNextStage(&gammaStage)
      .setNextStage(&blurStage)
      .setNextStage(nullptr);

  cv::equalizeHist(matGraySmall, matGraySmall);
  auto rects = detectFaceAndSmile(faceCascade, smileCascade, matGraySmall,
                                  matGrayFull, faceROIMax, claheStage);

  cv::Rect face, smile;
  if (rects.size()) {
    lv_obj_remove_flag(ROIRectFace, LV_OBJ_FLAG_HIDDEN);

    face = rects[0];
    lv_obj_align_to(ROIRectFace, screen_canvas, LV_ALIGN_TOP_LEFT, face.x,
                    face.y);
    lv_obj_set_size(ROIRectFace, face.width, face.height);

  } else {
    lv_obj_add_flag(ROIRectFace, LV_OBJ_FLAG_HIDDEN);
  }

  if (rects.size() > 1) {
    smile = rects[1];
    if ((smile & face).area() != 0) {
      lv_obj_align_to(ROIRectSmile, screen_canvas, LV_ALIGN_TOP_LEFT, smile.x,
                      smile.y);
      lv_obj_set_size(ROIRectSmile, smile.width, smile.height);

      lv_obj_remove_flag(ROIRectSmile, LV_OBJ_FLAG_HIDDEN);
#if DT_NODE_EXISTS(DT_ALIAS(led0))
      gpio_pin_toggle_dt(&led);
#endif
    }
  } else {
    lv_obj_add_flag(ROIRectSmile, LV_OBJ_FLAG_HIDDEN);
  }

  return 0;
}