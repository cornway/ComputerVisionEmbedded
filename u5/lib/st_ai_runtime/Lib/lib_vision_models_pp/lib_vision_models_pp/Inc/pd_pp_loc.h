/**
  ******************************************************************************
  * @file    pd_pp_loc.h
  * @author  MDG Application Team
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file in
 * the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
******************************************************************************
*/

#ifndef _PD_PP_LOC_
#define _PD_PP_LOC_
#ifdef __cplusplus
 extern "C" {
#endif

#include "arm_math.h"

/*-----------------------------     CNN_pd      -----------------------------*/
/* Offsets to access CNN_pd input data */
#define AI_PD_MODEL_PP_XCENTER      (0)
#define AI_PD_MODEL_PP_YCENTER      (1)
#define AI_PD_MODEL_PP_WIDTHREL     (2)
#define AI_PD_MODEL_PP_HEIGHTREL    (3)
#define AI_PD_MODEL_PP_KEYPOINTS    (4)

#ifdef __cplusplus
  }
#endif

#endif // _PD_PP_LOC_

