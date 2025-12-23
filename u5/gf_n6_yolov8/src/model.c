/**
 ******************************************************************************
 * @file    model.c
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

#include "model.h"

#include <zephyr/cache.h>
#include <zephyr/drivers/video.h>
#include <zephyr/logging/log.h>
#include <zephyr/mem_mgmt/mem_attr_heap.h>

#include "ll_aton_rt_user_api.h"
#include "npu_cache.h"
#include "od_pp_loc.h"
#include "od_yolov8_pp_if.h"

// #define DISPLAY_WIDTH DT_PROP(DT_CHOSEN(zephyr_display), width)
// #define DISPLAY_HEIGHT DT_PROP(DT_CHOSEN(zephyr_display), height)

#define DISPLAY_WIDTH 224
#define DISPLAY_HEIGHT 224

/* post process conf */
#define AI_OD_YOLOV8_PP_CONF_THRESHOLD (0.5)
#define AI_OD_YOLOV8_PP_IOU_THRESHOLD (0.3)
#define AI_OD_YOLOV8_PP_MAX_BOXES_LIMIT (10)
#define AI_OD_YOLOV8_PP_NB_CLASSES (1)
#define AI_OD_YOLOV8_PP_TOTAL_BOXES (1029)

LOG_MODULE_REGISTER(model, LOG_LEVEL_INF);

LL_ATON_DECLARE_NAMED_NN_INSTANCE_AND_INTERFACE(yolov8_obj_det);

/* latest detections and associate mutex */
static od_pp_outBuffer_t pp_detections_buffer[AI_OD_YOLOV8_PP_TOTAL_BOXES];
static od_pp_outBuffer_t detections[AI_OD_YOLOV8_PP_MAX_BOXES_LIMIT];
static int detections_nb = -1;
static struct k_mutex boxes_lock;

static int latest_inference_time = 0;

static void Run_Inference(NN_Instance_TypeDef *network_instance) {
  LL_ATON_RT_RetValues_t ll_aton_rt_ret;

  do {
    /* Execute first/next step of Cube.AI/ATON runtime */
    ll_aton_rt_ret = LL_ATON_RT_RunEpochBlock(network_instance);
    /* Wait for next event */
    if (ll_aton_rt_ret == LL_ATON_RT_WFE)
      LL_ATON_OSAL_WFE();
  } while (ll_aton_rt_ret != LL_ATON_RT_DONE);

  LL_ATON_RT_Reset_Network(network_instance);
}

static void NPUCache_config() {
  npu_cache_init();
  npu_cache_enable();
}

static void model_npu_init() {
  NPUCache_config();

  LOG_INF("Npu subsystem ready");
}

static int clamp_point(int *x, int *y) {
  int xi = *x;
  int yi = *y;

  if (*x < 0)
    *x = 0;
  if (*y < 0)
    *y = 0;
  if (*x >= DISPLAY_WIDTH)
    *x = DISPLAY_WIDTH - 1;
  if (*y >= DISPLAY_HEIGHT)
    *y = DISPLAY_HEIGHT - 1;

  return (xi != *x) || (yi != *y);
}

static void convert_length(float32_t wi, float32_t hi, int *wo, int *ho) {
  *wo = (int)(DISPLAY_WIDTH * wi);
  *ho = (int)(DISPLAY_HEIGHT * hi);
}

static void convert_point(float32_t xi, float32_t yi, int *xo, int *yo) {
  *xo = (int)(DISPLAY_WIDTH * xi);
  *yo = (int)(DISPLAY_HEIGHT * yi);
}

static void model_detection_to_box(od_pp_outBuffer_t *d, struct dbox *b) {
  int xc, yc;
  int x0, y0;
  int x1, y1;
  int w, h;

  convert_point(d->x_center, d->y_center, &xc, &yc);
  convert_length(d->width, d->height, &w, &h);
  x0 = xc - (w + 1) / 2;
  y0 = yc - (h + 1) / 2;
  x1 = xc + (w + 1) / 2;
  y1 = yc + (h + 1) / 2;
  clamp_point(&x0, &y0);
  clamp_point(&x1, &y1);
  w = x1 - x0 + 1;
  h = y1 - y0 + 1;

  b->x = x0;
  b->y = y0;
  b->w = w;
  b->h = h;
}

void model_thread_ep(void *arg1, void *arg2, void *arg3) {
  const LL_Buffer_InfoTypeDef *nn_out_info =
      LL_ATON_Output_Buffers_Info_yolov8_obj_det();
  const LL_Buffer_InfoTypeDef *nn_in_info =
      LL_ATON_Input_Buffers_Info_yolov8_obj_det();
  const struct device *const camera_aux_dev = arg1;
  od_yolov8_pp_static_param_t pp_params;
  struct video_buffer *vbuf;
  od_yolov8_pp_in_centroid_t pp_input;
  od_pp_out_t pp_output;
  uint32_t nn_out_len;
  uint32_t nn_in_len;
  uint8_t *nn_out;
  uint32_t tick;
  int ret;
  int i;

  ret = k_mutex_init(&boxes_lock);
  __ASSERT_NO_MSG(ret == 0);
  detections_nb = 0;

  model_npu_init();

  /* Initialize Cube.AI/ATON ... */
  LL_ATON_RT_RuntimeInit();
  /* ... and model instance */
  LL_ATON_RT_Init_Network(&NN_Instance_yolov8_obj_det);

  nn_in_len = LL_Buffer_len(nn_in_info);
  nn_out_len = LL_Buffer_len(nn_out_info);
  nn_out = LL_Buffer_addr_start(nn_out_info);
  __ASSERT_NO_MSG(nn_out);

  /* pp init */
  pp_params.nb_classes = AI_OD_YOLOV8_PP_NB_CLASSES;
  pp_params.nb_total_boxes = AI_OD_YOLOV8_PP_TOTAL_BOXES;
  pp_params.max_boxes_limit = AI_OD_YOLOV8_PP_MAX_BOXES_LIMIT;
  pp_params.conf_threshold = AI_OD_YOLOV8_PP_CONF_THRESHOLD;
  pp_params.iou_threshold = AI_OD_YOLOV8_PP_IOU_THRESHOLD;
  pp_params.pScratchBuff = NULL;

  ret = od_yolov8_pp_reset(&pp_params);
  __ASSERT_NO_MSG(ret == 0);

  while (1) {
    ret = video_dequeue(camera_aux_dev, &vbuf, K_FOREVER);
    __ASSERT_NO_MSG(ret == 0);

    /* run inference */
    __ASSERT_NO_MSG(vbuf->bytesused == nn_in_len);
    /* setup input buffer. No need cache ops since full hw path DCMIPP -> NPU */
    ret = LL_ATON_Set_User_Input_Buffer_yolov8_obj_det(0, vbuf->buffer, nn_in_len);
    __ASSERT_NO_MSG(ret == LL_ATON_User_IO_NOERROR);
    /* setup output. Invalidate ouput so post-process access latest inference
     * result */
    ret = sys_cache_data_invd_range(nn_out, nn_out_len);
    __ASSERT_NO_MSG(ret == 0);
    /* let's go */
    tick = HAL_GetTick();
    Run_Inference(&NN_Instance_yolov8_obj_det);
    tick = HAL_GetTick() - tick;
    latest_inference_time = tick;

    ret = video_enqueue(camera_aux_dev, vbuf);
    __ASSERT_NO_MSG(ret == 0);

    /* post process */
    pp_input.pRaw_detections = (float32_t *)nn_out;
    pp_output.pOutBuff = pp_detections_buffer;
    ret = od_yolov8_pp_process(&pp_input, &pp_output, &pp_params);
    __ASSERT_NO_MSG(ret == 0);

    /* boxes state */
    ret = k_mutex_lock(&boxes_lock, K_FOREVER);
    __ASSERT_NO_MSG(ret == 0);

    detections_nb = pp_output.nb_detect;
    for (i = 0; i < pp_output.nb_detect; i++)
      detections[i] = pp_output.pOutBuff[i];

    ret = k_mutex_unlock(&boxes_lock);
    __ASSERT_NO_MSG(ret == 0);
  }
}

int model_get_boxes(struct dbox *boxes, int boxes_nb) {
  int nb = 0;
  int ret;
  int i;

  /* not yet init */
  if (detections_nb < 0)
    return 0;

  ret = k_mutex_lock(&boxes_lock, K_FOREVER);
  __ASSERT_NO_MSG(ret == 0);

  nb = MIN(boxes_nb, detections_nb);
  for (i = 0; i < nb; i++)
    model_detection_to_box(&detections[i], &boxes[i]);

  ret = k_mutex_unlock(&boxes_lock);
  __ASSERT_NO_MSG(ret == 0);

  return nb;
}

int model_get_latest_inference_time() { return latest_inference_time; }
