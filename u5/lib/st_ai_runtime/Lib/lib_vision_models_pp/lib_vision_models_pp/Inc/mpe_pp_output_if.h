/*---------------------------------------------------------------------------------------------
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file in
 * the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *--------------------------------------------------------------------------------------------*/

#ifndef __MPE_PP_INTERFACE_IF_H__
#define __MPE_PP_INTERFACE_IF_H__

/* Error return codes */
#define AI_MPE_PP_ERROR_NO     (0)
#define AI_MPE_PP_ERROR_BAD_HW (-1)
#define AI_MPE_PP_ERROR        (-2)

#ifdef __cplusplus
 extern "C" {
#endif

#include "arm_math.h"


/*Pose structures*/
typedef struct
{
  float32_t x;
  float32_t y;
  float32_t conf;
} mpe_pp_keyPoints_t;

typedef struct
{
  float32_t x_center;
  float32_t y_center;
  float32_t width;
  float32_t height;
  float32_t conf;
  int32_t   class_index;
  mpe_pp_keyPoints_t *pKeyPoints;
} mpe_pp_outBuffer_t;

typedef struct
{
  mpe_pp_outBuffer_t *pOutBuff;
  int32_t nb_detect;
} mpe_pp_out_t;



#ifdef __cplusplus
  }
#endif

#endif      /* __MPE_PP_INTERFACE_IF_H__  */


