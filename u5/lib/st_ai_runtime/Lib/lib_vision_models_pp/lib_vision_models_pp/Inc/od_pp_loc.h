/*---------------------------------------------------------------------------------------------
 * Copyright (c) 2022 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file in
 * the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *--------------------------------------------------------------------------------------------*/

#ifndef __OD_PP_LOC_H__
#define __OD_PP_LOC_H__


#ifdef __cplusplus
 extern "C" {
#endif

#include "arm_math.h"

/*-----------------------------     YOLO_V2      -----------------------------*/
/* Offsets to access YoloV2 input data */
#define AI_YOLOV2_PP_XCENTER      (0)
#define AI_YOLOV2_PP_YCENTER      (1)
#define AI_YOLOV2_PP_WIDTHREL     (2)
#define AI_YOLOV2_PP_HEIGHTREL    (3)
#define AI_YOLOV2_PP_OBJECTNESS   (4)
#define AI_YOLOV2_PP_CLASSPROB    (5)

/*-----------------------------     YOLO_V8      -----------------------------*/
/* Offsets to access YoloV8 input data */
#define AI_YOLOV8_PP_XCENTER      (0)
#define AI_YOLOV8_PP_YCENTER      (1)
#define AI_YOLOV8_PP_WIDTHREL     (2)
#define AI_YOLOV8_PP_HEIGHTREL    (3)
#define AI_YOLOV8_PP_CLASSPROB    (4)
#define AI_YOLOV8_PP_CLASSID      (5)
#define AI_YOLOV8_PP_BOX_STRIDE   (4)

/*-----------------------------     YOLO_V4      -----------------------------*/
/* Offsets to access YoloV4 input data */
#define AI_YOLOV4_PP_XMIN      (0)
#define AI_YOLOV4_PP_YMIN      (1)
#define AI_YOLOV4_PP_XMAX      (2)
#define AI_YOLOV4_PP_YMAX      (3)

/*-----------------------------     YOLO_V5      -----------------------------*/
/* Offsets to access YoloV5 input data */
#define AI_YOLOV5_PP_XCENTER      (0)
#define AI_YOLOV5_PP_YCENTER      (1)
#define AI_YOLOV5_PP_WIDTHREL     (2)
#define AI_YOLOV5_PP_HEIGHTREL    (3)
#define AI_YOLOV5_PP_CONFIDENCE   (4)
#define AI_YOLOV5_PP_CLASSPROB    (5)
#define AI_YOLOV5_PP_CLASSID      (5)
#define AI_YOLOV5_PP_BOX_STRIDE   (4)

/*-----------------------------       SSD        -----------------------------*/
/* Offsets to access SSD input data */
#define AI_SSD_PP_CENTROID_YCENTER          (0)
#define AI_SSD_PP_CENTROID_XCENTER          (1)
#define AI_SSD_PP_CENTROID_HEIGHTREL        (2)
#define AI_SSD_PP_CENTROID_WIDTHREL         (3)
#define AI_SSD_PP_BOX_STRIDE                (4)

/*-----------------------------       SSD  ST      -----------------------------*/
/* Offsets to access SSD ST input data */
#define AI_SSD_ST_PP_XMIN          (0)
#define AI_SSD_ST_PP_YMIN          (1)
#define AI_SSD_ST_PP_XMAX          (2)
#define AI_SSD_ST_PP_YMAX          (3)
#define AI_SSD_ST_PP_BOX_STRIDE    (4)

#define AI_SSD_ST_PP_CENTROID_YCENTER          (0)
#define AI_SSD_ST_PP_CENTROID_XCENTER          (1)
#define AI_SSD_ST_PP_CENTROID_HEIGHTREL        (2)
#define AI_SSD_ST_PP_CENTROID_WIDTHREL         (3)


/*-----------------------------     CENTER_NET      -----------------------------*/
/* Offsets to access CenterNet input data */
#define AI_CENTERNET_PP_CONFCENTER   (0)
#define AI_CENTERNET_PP_WIDTH        (1)
#define AI_CENTERNET_PP_HEIGHT       (2)
#define AI_CENTERNET_PP_XOFFSET      (3)
#define AI_CENTERNET_PP_YOFFSET      (4)
#define AI_CENTERNET_PP_CLASSPROB    (5)
#define AI_CENTERNET_PP_MAPSEG_NEXTOFFSET    (1)


/* Offsets to access face detect blaze face input data */
#define AI_FD_BLAZEFACE_PP_XCENTER      (0)
#define AI_FD_BLAZEFACE_PP_YCENTER      (1)
#define AI_FD_BLAZEFACE_PP_WIDTHREL     (2)
#define AI_FD_BLAZEFACE_PP_HEIGHTREL    (3)
#define AI_FD_BLAZEFACE_PP_KEYPOINTS    (4)

#ifdef __cplusplus
  }
#endif

#endif   /*  __OD_PP_LOC_H__  */
