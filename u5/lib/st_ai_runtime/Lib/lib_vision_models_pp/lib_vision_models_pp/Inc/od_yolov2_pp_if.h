/*---------------------------------------------------------------------------------------------
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file in
 * the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *--------------------------------------------------------------------------------------------*/

#ifndef __OD_YOLOV2_PP_IF_H__
#define __OD_YOLOV2_PP_IF_H__


#ifdef __cplusplus
 extern "C" {
#endif

#include "od_pp_output_if.h"


/* I/O structures for YoloV2 detector type */
/* --------------------------------------- */
typedef struct yolov2_pp_in
{
  void* pRaw_detections;
} od_yolov2_pp_in_t;



/* Generic Static parameters */
/* ------------------------- */

typedef struct od_yolov2_pp_static_param {
  int32_t  nb_classes;
  int32_t  nb_anchors;
  int32_t  grid_width;
  int32_t  grid_height;
  int32_t  nb_input_boxes;
  int32_t  max_boxes_limit;
  float32_t	conf_threshold;
  float32_t	iou_threshold;
  const float32_t	*pAnchors;
  int32_t nb_detect;
  float32_t raw_scale;
  int8_t raw_zero_point;
  void *pScratchBuffer;
} od_yolov2_pp_static_param_t;



/* Exported functions ------------------------------------------------------- */

/*!
 * @brief Resets object detection YoloV2 post processing
 *
 * @param [IN] Input static parameters
 * @retval Error code
 */
int32_t od_yolov2_pp_reset(od_yolov2_pp_static_param_t *pInput_static_param);


/*!
 * @brief Object detector post processing : includes output detector remapping,
 *        nms and score filtering for YoloV2 (float32_t input data).
 *
 * @param [IN] Pointer on input data
 *             Pointer on output data
 *             pointer on static parameters
 * @retval Error code
 */
int32_t od_yolov2_pp_process(od_yolov2_pp_in_t *pInput,
                             od_pp_out_t *pOutput,
                             od_yolov2_pp_static_param_t *pInput_static_param);


/*!
 * @brief Object detector post processing : includes output detector remapping,
 *        nms and score filtering for YoloV2 (int8_t input data).
 *
 * @param [IN] Pointer on input data
 *             Pointer on output data
 *             pointer on static parameters
 * @retval Error code
 */
int32_t od_yolov2_pp_process_int8(od_yolov2_pp_in_t *pInput,
                                  od_pp_out_t *pOutput,
                                  od_yolov2_pp_static_param_t *pInput_static_param);


#ifdef __cplusplus
 }
#endif

#endif      /* __OD_YOLOV2_PP_IF_H__  */


