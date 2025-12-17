# /*---------------------------------------------------------------------------------------------
#  * Copyright (c) 2022 STMicroelectronics.
#  * All rights reserved.
#  *
#  * This software is licensed under terms that can be found in the LICENSE file in
#  * the root directory of this software component.
#  * If no LICENSE file comes with this software, it is provided AS-IS.
#  *--------------------------------------------------------------------------------------------*/

#ifndef __SPE_MOVENET_PP_LOC_H__
#define __SPE_MOVENET_PP_LOC_H__


#ifdef __cplusplus
 extern "C" {
#endif

#include "arm_math.h"


/*-----------------------------     SPE Movenet     -----------------------------*/
/* Offsets to access SPE Movenet input data */
#define AI_SPE_MOVENET_XCENTER      (0)
#define AI_SPE_MOVENET_YCENTER      (1)
#define AI_SPE_MOVENET_PROBA        (2)


#ifdef __cplusplus
 }
#endif



#endif   /*  __SPE_MOVENET_PP_LOC_H__  */
