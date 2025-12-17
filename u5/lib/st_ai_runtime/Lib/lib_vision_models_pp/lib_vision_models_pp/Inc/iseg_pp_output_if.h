/*---------------------------------------------------------------------------------------------
#  * Copyright (c) 2023 STMicroelectronics.
#  * All rights reserved.
#  *
#  * This software is licensed under terms that can be found in the LICENSE file in
#  * the root directory of this software component.
#  * If no LICENSE file comes with this software, it is provided AS-IS.
#  *--------------------------------------------------------------------------------------------*/

#ifndef __ISEG_PP_OUTPUT_IF_H__
#define __ISEG_PP_OUTPUT_IF_H__


#ifdef __cplusplus
 extern "C" {
#endif

#include "arm_math.h"

/* Error return codes */
#define AI_ISEG_POSTPROCESS_ERROR_NO                    (0)
#define AI_ISEG_POSTPROCESS_ERROR_BAD_HW                (-1)


typedef struct
{
  float32_t x_center;
  float32_t y_center;
  float32_t width;
  float32_t height;
  float32_t conf;
  int32_t   class_index;
  uint8_t *pMask; // AI_ISEG_YOLOV8_PP_MASK_SIZE * AI_SEG_YOLOV8_PP_MASK_SIZE application definition
} iseg_pp_outBuffer_t;

typedef struct
{
  iseg_pp_outBuffer_t *pOutBuff;
  int32_t nb_detect;
} iseg_pp_out_t;

#ifdef __cplusplus
 }
#endif

#endif      /* __ISEG_PP_OUTPUT_IF_H__  */


