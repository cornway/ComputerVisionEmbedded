/*---------------------------------------------------------------------------------------------
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file in
 * the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *--------------------------------------------------------------------------------------------*/

#ifndef __OD_FD_BLAZEFACE_PP_IF_H__
#define __OD_FD_BLAZEFACE_PP_IF_H__

#ifdef __cplusplus
 extern "C" {
#endif

#include "od_pp_output_if.h"


/* I/O structures for SSD detector type */
/* ------------------------------------ */
typedef struct od_fd_blazeface_pp_in
{
  void *pRawDetections_0;
  void *pRawDetections_1;
  void *pScores_0;
  void *pScores_1;
} od_fd_blazeface_pp_in_t;


/* Generic Static parameters */
/* ------------------------- */
typedef struct od_fd_blazeface_pp_static_param
{
  int32_t   nb_classes;
  int32_t   nb_keypoints;
  int32_t   nb_detections_0;
  int32_t   nb_detections_1;
  uint32_t  in_size;
  int32_t   max_boxes_limit;
  float32_t conf_threshold;
  float32_t iou_threshold;
  int32_t   nb_detect;
  const void *pAnchors_0;
  const void *pAnchors_1;
  /* quantized parameters */
  float32_t boxe_0_scale;
  float32_t proba_0_scale;
  float32_t boxe_1_scale;
  float32_t proba_1_scale;
  uint8_t boxe_0_zero_point;
  uint8_t proba_0_zero_point;
  uint8_t boxe_1_zero_point;
  uint8_t proba_1_zero_point;
} od_fd_blazeface_pp_static_param_t;


/* Exported functions ------------------------------------------------------- */

/*!
 * @brief Resets face detection blazeface post processing
 *
 * @param [IN] Input static parameters
 * @retval Error code
 */
int32_t od_fd_blazeface_pp_reset(od_fd_blazeface_pp_static_param_t *pInput_static_param);


/*!
 * @brief Face detector post processing : includes output detector remapping,
 *        nms and score filtering for fd blazeface.
 *
 * @param [IN] Pointer on input structure pointing data
 *             Pointer on output structure pointing data
 *             pointer on static parameters
 * @retval Error code
 */
int32_t od_fd_blazeface_pp_process(od_fd_blazeface_pp_in_t *pInput,
                                   od_pp_out_t *pOutput,
                                   od_fd_blazeface_pp_static_param_t *pInput_static_param);

/*!
 * @brief Face detector post processing : includes output detector remapping,
 *        nms and score filtering for fd blazeface quantized unsigned inputs
 *
 * @param [IN] Pointer on input structure pointing data
 *             Pointer on output structure pointing data
 *             pointer on static parameters
 * @retval Error code
 */
int32_t od_fd_blazeface_pp_process_uint8(od_fd_blazeface_pp_in_t *pInput,
                                         od_pp_out_t *pOutput,
                                         od_fd_blazeface_pp_static_param_t *pInput_static_param);
/*!
 * @brief Face detector post processing : includes output detector remapping,
 *        nms and score filtering for fd blazeface quantized signed inputs
 *
 * @param [IN] Pointer on input structure pointing data
 *             Pointer on output structure pointing data
 *             pointer on static parameters
 * @retval Error code
 */
int32_t od_fd_blazeface_pp_process_int8(od_fd_blazeface_pp_in_t *pInput,
                                         od_pp_out_t *pOutput,
                                         od_fd_blazeface_pp_static_param_t *pInput_static_param);
#ifdef __cplusplus
 }
#endif

#endif      /* __OD_FD_BLAZEFACE_PP_IF_H__  */
