/*---------------------------------------------------------------------------------------------
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file in
 * the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *--------------------------------------------------------------------------------------------*/

#ifndef __OD_SSD_PP_IF_H__
#define __OD_SSD_PP_IF_H__

#ifdef __cplusplus
 extern "C" {
#endif

#include "od_pp_output_if.h"


/* I/O structures for SSD detector type */
/* ------------------------------------ */
typedef struct ssd_pp_in_centroid
{
  void *pBoxes;
  void *pAnchors;
  void *pScores;
} od_ssd_pp_in_centroid_t;


/* Generic Static parameters */
/* ------------------------- */
typedef struct ssd_pp_static_param
{
  int32_t   nb_classes;
  int32_t   nb_detections;
  float32_t XY_scale;
  float32_t WH_scale;
  int32_t   max_boxes_limit;
  float32_t conf_threshold;
  float32_t iou_threshold;
  int32_t   nb_detect;
  void *scratchBuffer;
  float32_t boxe_scale;
  float32_t anchor_scale;
  float32_t score_scale;
  int8_t boxe_zero_point;
  int8_t anchor_zero_point;
  int8_t score_zero_point;
} od_ssd_pp_static_param_t;


/* Exported functions ------------------------------------------------------- */

/*!
 * @brief Resets object detection SSD post processing
 *
 * @param [IN] Input static parameters
 * @retval Error code
 */
int32_t od_ssd_pp_reset(od_ssd_pp_static_param_t *pInput_static_param);


/*!
 * @brief Object detector post processing : includes output detector remapping,
 *        nms and score filtering for SSD.
 *
 * @param [IN] Pointer on input structure pointing data
 *             Pointer on output structure pointing data
 *             pointer on static parameters
 * @retval Error code
 */
int32_t od_ssd_pp_process(od_ssd_pp_in_centroid_t *pInput,
                          od_pp_out_t *pOutput,
                          od_ssd_pp_static_param_t *pInput_static_param);

#ifdef __cplusplus
 }
#endif

#endif      /* __OD_SSD_PP_IF_H__  */
