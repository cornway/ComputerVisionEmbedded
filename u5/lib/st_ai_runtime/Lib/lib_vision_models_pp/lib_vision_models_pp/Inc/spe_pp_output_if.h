/*---------------------------------------------------------------------------------------------
#  * Copyright (c) 2023 STMicroelectronics.
#  * All rights reserved.
#  *
#  * This software is licensed under terms that can be found in the LICENSE file in
#  * the root directory of this software component.
#  * If no LICENSE file comes with this software, it is provided AS-IS.
#  *--------------------------------------------------------------------------------------------*/

#ifndef __SPE_PP_OUTPUT_IF_H__
#define __SPE_PP_OUTPUT_IF_H__


#ifdef __cplusplus
 extern "C" {
#endif

#include "arm_math.h"

/* Error return codes */
#define AI_SPE_POSTPROCESS_ERROR_NO                    (0)
#define AI_SPE_POSTPROCESS_ERROR_BAD_HW                (-1)


typedef struct
{
	float32_t x_center;
	float32_t y_center;
	float32_t proba;
} spe_pp_outBuffer_t;

typedef struct
{
	spe_pp_outBuffer_t *pOutBuff;  /* pointer on AI_SPE_MOVENET_POSTPROC_NB_KEYPOINTS elements */
} spe_pp_out_t;


#ifdef __cplusplus
 }
#endif

#endif      /* __SPE_PP_OUTPUT_IF_H__  */


