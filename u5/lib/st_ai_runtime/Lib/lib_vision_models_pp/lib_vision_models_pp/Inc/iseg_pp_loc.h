/*---------------------------------------------------------------------------------------------
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file in
 * the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *--------------------------------------------------------------------------------------------*/

#ifndef __SEG_PP_LOC_H__
#define __SEG_PP_LOC_H__


#ifdef __cplusplus
 extern "C" {
#endif

#include "arm_math.h"

/*-----------------------------     YOLO_V8      -----------------------------*/
/* Offsets to access YoloV8 input data */
#define AI_YOLOV8_PP_XCENTER      (0)
#define AI_YOLOV8_PP_YCENTER      (1)
#define AI_YOLOV8_PP_WIDTHREL     (2)
#define AI_YOLOV8_PP_HEIGHTREL    (3)
#define AI_YOLOV8_PP_CLASSPROB    (4)
#define AI_YOLOV8_PP_CLASSID      (5)
#define AI_YOLOV8_PP_BOX_STRIDE   (4)

#ifdef __cplusplus
  }
#endif

#endif   /*  __SEG_PP_LOC_H__  */
