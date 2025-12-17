/*---------------------------------------------------------------------------------------------
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file in
  * the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *--------------------------------------------------------------------------------------------*/

#ifndef __MPE_YOLOV8_PP_IF_H__
#define __MPE_YOLOV8_PP_IF_H__


#ifdef __cplusplus
 extern "C" {
#endif

#include "mpe_pp_output_if.h"

/*Pose structures*/
typedef struct
{
  int8_t x;
  int8_t y;
  int8_t conf;
} mpe_pp_keyPoints_s8_t;

typedef struct
{
  int8_t x_center;
  int8_t y_center;
  int8_t width;
  int8_t height;
  int8_t conf;
  uint8_t class_index;
  mpe_pp_keyPoints_s8_t *pKeyPoints;
} mpe_pp_scratchBuffer_s8_t;


/* I/O structures for YoloV8 MPE type */
/* --------------------------------------- */
typedef struct mpe_yolov8_pp_in_centroid
{
  void *pRaw_detections;
} mpe_yolov8_pp_in_centroid_t;


typedef struct mpe_yolov8_pp_static_param {
  int32_t  nb_classes;
  int32_t  nb_total_boxes;
  int32_t  max_boxes_limit;
  float32_t conf_threshold;
  float32_t iou_threshold;
  float32_t raw_output_scale;
  int8_t raw_output_zero_point;
  int32_t nb_detect;
  uint32_t nb_keypoints;
  mpe_pp_scratchBuffer_s8_t *pScratchBuffer;
} mpe_yolov8_pp_static_param_t;


/* Exported functions ------------------------------------------------------- */

/*!
 * @brief Resets multi-pose YoloV8 post processing
 *
 * @param [IN] Input static parameters
 * @retval Error code
 */
int32_t mpe_yolov8_pp_reset(mpe_yolov8_pp_static_param_t *pInput_static_param);


/*!
 * @brief Multi-pose post processing : includes output detector remapping,
 *        nms and score filtering for YoloV8.
 *
 * @param [IN] Pointer on input data
 *             Pointer on output data
 *             pointer on static parameters
 * @retval Error code
 */
int32_t mpe_yolov8_pp_process(mpe_yolov8_pp_in_centroid_t *pInput,
                              mpe_pp_out_t *pOutput,
                              mpe_yolov8_pp_static_param_t *pInput_static_param);

/*!
 * @brief Multi-pose post processing with int8 quantized input: includes output detector remapping,
 *        nms and score filtering for YoloV8.
 *
 * @param [IN] Pointer on input data
 *             Pointer on output data
 *             pointer on static parameters
 * @retval Error code
 */
int32_t mpe_yolov8_pp_process_int8(mpe_yolov8_pp_in_centroid_t *pInput,
                                   mpe_pp_out_t *pOutput,
                                   mpe_yolov8_pp_static_param_t *pInput_static_param);


#ifdef __cplusplus
  }
#endif

#endif      /* __MPE_YOLOV8_PP_IF_H__  */


