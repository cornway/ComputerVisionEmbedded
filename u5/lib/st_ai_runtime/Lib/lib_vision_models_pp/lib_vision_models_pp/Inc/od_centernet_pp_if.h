/*---------------------------------------------------------------------------------------------
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file in
 * the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *--------------------------------------------------------------------------------------------*/

#ifndef __OD_CENTERNET_PP_IF_H__
#define __OD_CENTERNET_PP_IF_H__


#ifdef __cplusplus
 extern "C" {
#endif

#include "arm_math.h"
#include "od_pp_output_if.h"



/* I/O structures for CenterNet detector type */
/* ------------------------------------------ */

typedef struct centernet_pp_in
{
  void* pRaw_detections;
} od_centernet_pp_in_t;


/* Generic Static parameters */
/* ------------------------- */
typedef enum centernet_pp_optim {
  AI_OD_CENTERNET_PP_OPTIM_NORMAL     = 0,
  AI_OD_CENTERNET_PP_OPTIM_ACCURACY,
  AI_OD_CENTERNET_PP_OPTIM_SPEED
} od_centernet_pp_optim_e;


typedef struct centernet_pp_static_param {
  int32_t  nb_classifs;
  int32_t  grid_width;
  int32_t  grid_height;
  int32_t  max_boxes_limit;
  float32_t	conf_threshold;
  float32_t	iou_threshold;
  od_centernet_pp_optim_e optim;
  int32_t nb_detect;
  float32_t raw_scale;
  int8_t raw_zp;
} od_centernet_pp_static_param_t;



/* Exported functions ------------------------------------------------------- */

/*!
 * @brief Resets object detection CenterNet post processing
 *
 * @param [IN] Input static parameters
 * @retval Error code
 */
int32_t od_centernet_pp_reset(od_centernet_pp_static_param_t *pInput_static_param);


/*!
 * @brief Object detector post processing : includes output detector remapping,
 *        nms and score filtering for CenterNet.
 *
 * @param [IN] Pointer on input data
 *             Pointer on output data
 *             pointer on static parameters
 * @retval Error code
 */
int32_t od_centernet_pp_process(od_centernet_pp_in_t *pInput,
                                od_pp_out_t *pOutput,
                                od_centernet_pp_static_param_t *pInput_static_param);

/*!
 * @brief Object detector post processing : includes output detector remapping,
 *        nms and score filtering for CenterNet int8 input
 *
 * @param [IN] Pointer on input data structure
 *             Pointer on output structure
 *             pointer on static parameters
 * @retval Error code
 */
int32_t od_centernet_pp_process_int8(od_centernet_pp_in_t *pInput,
                                     od_pp_out_t *pOutput,
                                     od_centernet_pp_static_param_t *pInput_static_param);

#ifdef __cplusplus
  }
#endif

#endif      /* __OD_CENTERNET_PP_IF_H__  */


