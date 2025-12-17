/*---------------------------------------------------------------------------------------------
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file in
 * the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *--------------------------------------------------------------------------------------------*/

#ifndef __SSEG_PP_OUTPUT_IF__
#define __SSEG_PP_OUTPUT_IF__


#ifdef __cplusplus
 extern "C" {
#endif

#include "arm_math.h"


/* Error return codes */
#define AI_SSEG_POSTPROCESS_ERROR_NO                    (0)
#define AI_SSEG_POSTPROCESS_ERROR_BAD_HW                (-1)
#define AI_SSEG_POSTPROCESS_ERROR                       (-2)


typedef struct
{
  uint8_t *pOutBuff;
} sseg_pp_out_t;

#ifdef __cplusplus
  }
#endif

#endif      /* __SSEG_PP_OUTPUT_IF__  */


