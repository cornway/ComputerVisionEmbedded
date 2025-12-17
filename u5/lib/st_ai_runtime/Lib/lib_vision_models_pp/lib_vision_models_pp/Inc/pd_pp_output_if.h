/**
  ******************************************************************************
  * @file    pd_p_output_if.h
  * @author  MDG Application Team
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#ifndef _PD_PP_OUTPUT_IF_
#define _PD_PP_OUTPUT_IF_

#ifdef __cplusplus
 extern "C" {
#endif

#include "arm_math.h"


/* Error return codes */
#define AI_PD_POSTPROCESS_ERROR_NO                    (0)
#define AI_PD_POSTPROCESS_ERROR                       (-1)


typedef struct {
  float32_t x;
  float32_t y;
} pd_pp_point_t;

typedef struct {
  float32_t prob;
  float32_t x_center;
  float32_t y_center;
  float32_t width;
  float32_t height;
  pd_pp_point_t *pKps; //[PD_KPS_NB];
} pd_pp_box_t;


typedef struct
{
  pd_pp_box_t *pOutData;
  uint32_t box_nb;
} pd_pp_out_t;

#ifdef __cplusplus
 }
#endif

#endif // _PD_PP_OUTPUT_IF_
