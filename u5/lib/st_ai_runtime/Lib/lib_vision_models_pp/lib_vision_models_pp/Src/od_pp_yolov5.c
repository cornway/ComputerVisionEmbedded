/*---------------------------------------------------------------------------------------------
#  * Copyright (c) 2023 STMicroelectronics.
#  * All rights reserved.
#  *
#  * This software is licensed under terms that can be found in the LICENSE file in
#  * the root directory of this software component.
#  * If no LICENSE file comes with this software, it is provided AS-IS.
#  *--------------------------------------------------------------------------------------------*/

#include "od_pp_loc.h"
#include "od_yolov5_pp_if.h"
#include "vision_models_pp.h"


/* Can't be removed if qsort is not re-written... */
static int32_t AI_YOLOV5_PP_SORT_CLASS;


int32_t yolov5_nms_comparator(const void *pa, const void *pb)
{
  od_pp_outBuffer_t a = *(od_pp_outBuffer_t *)pa;
  od_pp_outBuffer_t b = *(od_pp_outBuffer_t *)pb;

  float32_t diff = 0.0;
  float32_t a_weighted_conf = 0.0;
  float32_t b_weighted_conf = 0.0;

  if (a.class_index == AI_YOLOV5_PP_SORT_CLASS)
  {
    a_weighted_conf = a.conf;
  }
  else
  {
    a_weighted_conf = 0.0;
  }

  if (b.class_index == AI_YOLOV5_PP_SORT_CLASS)
  {
    b_weighted_conf = b.conf;
  }
  else
  {
    b_weighted_conf = 0.0;
  }

  diff = a_weighted_conf - b_weighted_conf;

  if (diff < 0) return 1;
  else if (diff > 0) return -1;
  return 0;
}


int32_t yolov5_pp_nmsFiltering_centroid(od_pp_out_t *pOutput,
                                        od_yolov5_pp_static_param_t *pInput_static_param)
{
  int32_t j, k, limit_counter, detections_per_class;

  for (k = 0; k < pInput_static_param->nb_classes; ++k)
  {
    limit_counter = 0;
    detections_per_class = 0;
    AI_YOLOV5_PP_SORT_CLASS = k;


    /* Counts the number of detections with class k */
    for (int32_t i = 0; i < pInput_static_param->nb_detect ; i ++)
    {
      if(pOutput->pOutBuff[i].class_index == k)
      {
        detections_per_class++;
      }
    }

    if (detections_per_class > 0)
    {
      /* Sorts detections based on class k */
      qsort(pOutput->pOutBuff,
            pInput_static_param->nb_detect,
            sizeof(od_pp_outBuffer_t),
            yolov5_nms_comparator);

      for (int32_t i = 0; i < detections_per_class ; i ++)
      {
        if (pOutput->pOutBuff[i].conf == 0) continue;
        float32_t *a = &(pOutput->pOutBuff[i].x_center);
        for (j = i + 1; j < detections_per_class; j ++)
        {
          if (pOutput->pOutBuff[j].conf == 0) continue;
          float32_t *b = &(pOutput->pOutBuff[j].x_center);
          if (vision_models_box_iou(a, b) > pInput_static_param->iou_threshold)
          {
            pOutput->pOutBuff[j].conf = 0;
          }
        }
      }

      /* Limits detections count */
      for (int32_t i = 0; i < detections_per_class; i++)
      {
        if ((limit_counter < pInput_static_param->max_boxes_limit) &&
            (pOutput->pOutBuff[i].conf != 0))
        {
          limit_counter++;
        }
        else
        {
          pOutput->pOutBuff[i].conf = 0;
        }
      }
    } // if detections_per_class
  } // for nb_classes
  return (AI_OD_POSTPROCESS_ERROR_NO);
}


int32_t yolov5_pp_scoreFiltering_centroid(od_pp_out_t *pOutput,
                                          od_yolov5_pp_static_param_t *pInput_static_param)
{
  int32_t det_count = 0;

  for (int32_t i = 0; i < pInput_static_param->nb_detect; i++)
  {
    if (pOutput->pOutBuff[i].conf >= pInput_static_param->conf_threshold)
    {
      pOutput->pOutBuff[det_count].x_center    = pOutput->pOutBuff[i].x_center;
      pOutput->pOutBuff[det_count].y_center    = pOutput->pOutBuff[i].y_center;
      pOutput->pOutBuff[det_count].width       = pOutput->pOutBuff[i].width;
      pOutput->pOutBuff[det_count].height      = pOutput->pOutBuff[i].height;
      pOutput->pOutBuff[det_count].conf        = pOutput->pOutBuff[i].conf;
      pOutput->pOutBuff[det_count].class_index = pOutput->pOutBuff[i].class_index;
      det_count++;
    }
  }
  pOutput->nb_detect = det_count;

  return (AI_OD_POSTPROCESS_ERROR_NO);
}


int32_t yolov5_pp_getNNBoxes_centroid(od_yolov5_pp_in_centroid_t *pInput,
                                      od_pp_out_t *pOutput,
                                      od_yolov5_pp_static_param_t *pInput_static_param)
{
  int32_t error   = AI_OD_POSTPROCESS_ERROR_NO;
  float32_t best_score = 0;
  float32_t confidence = 0;
  uint32_t class_index = 0;
  int32_t nb_classes = pInput_static_param->nb_classes;
  int32_t nb_total_boxes = pInput_static_param->nb_total_boxes;
  int32_t detection_len = pInput_static_param->nb_classes + AI_YOLOV5_PP_CLASSPROB;
  float32_t *pRaw_detections = (float32_t *)pInput->pRaw_detections;

  uint32_t nb_detect = pInput_static_param->nb_detect;

  for (int32_t i = 0; i < nb_total_boxes; i++)
  {
    confidence = pRaw_detections[i*detection_len + AI_YOLOV5_PP_CONFIDENCE];
    if (confidence >= pInput_static_param->conf_threshold)
    {
      vision_models_maxi_if32ou32(&pRaw_detections[i*detection_len + AI_YOLOV5_PP_CLASSPROB],
                                  nb_classes,
                                  &best_score,
                                  &class_index);
      if (best_score >= pInput_static_param->conf_threshold)
      {
        pOutput->pOutBuff[nb_detect].x_center    = pRaw_detections[i*detection_len + AI_YOLOV5_PP_XCENTER];
        pOutput->pOutBuff[nb_detect].y_center    = pRaw_detections[i*detection_len + AI_YOLOV5_PP_YCENTER];
        pOutput->pOutBuff[nb_detect].width       = pRaw_detections[i*detection_len + AI_YOLOV5_PP_WIDTHREL];
        pOutput->pOutBuff[nb_detect].height      = pRaw_detections[i*detection_len + AI_YOLOV5_PP_HEIGHTREL];
        pOutput->pOutBuff[nb_detect].conf        = best_score;
        pOutput->pOutBuff[nb_detect].class_index = class_index;
        nb_detect++;
      }
    }
  }

  pInput_static_param->nb_detect = nb_detect;

  return (error);
}


int32_t yolov5_pp_getNNBoxes_centroid_uint8(od_yolov5_pp_in_centroid_t *pInput,
                                            od_pp_out_t *pOutput,
                                            od_yolov5_pp_static_param_t *pInput_static_param)
{
  int32_t error   = AI_OD_POSTPROCESS_ERROR_NO;
  uint8_t best_score = 0;
  uint8_t confidence = 0;
  uint8_t class_index = 0;
  int32_t nb_classes = pInput_static_param->nb_classes;
  int32_t nb_total_boxes = pInput_static_param->nb_total_boxes;
  int32_t detection_len = pInput_static_param->nb_classes + AI_YOLOV5_PP_CLASSPROB;
  uint8_t *pRaw_detections = (uint8_t *)pInput->pRaw_detections;

  uint8_t zero_point = pInput_static_param->raw_output_zero_point;
  float32_t scale = pInput_static_param->raw_output_scale;

  uint32_t nb_detect = pInput_static_param->nb_detect;

  // scale must be strictly positive
  if (scale <= 0.0f) {
      return AI_OD_POSTPROCESS_ERROR;
  }

  uint8_t conf_threshold_u8 = MIN((uint32_t)(pInput_static_param->conf_threshold / scale + (int32_t)zero_point + 0.5f),255);
  for (int32_t i = 0; i < nb_total_boxes; i++)
  {
    confidence = pRaw_detections[ i*detection_len + AI_YOLOV5_PP_CONFIDENCE];
    if (confidence >= conf_threshold_u8)
    {
      vision_models_maxi_iu8ou8(&pRaw_detections[i*detection_len + AI_YOLOV5_PP_CLASSPROB],
                                nb_classes,
                                &best_score,
                                &class_index);

      if (best_score >= conf_threshold_u8)
      {
        pOutput->pOutBuff[nb_detect].x_center    = scale * (float32_t)((int32_t)pRaw_detections[i*detection_len + AI_YOLOV5_PP_XCENTER]    - (int32_t)zero_point);
        pOutput->pOutBuff[nb_detect].y_center    = scale * (float32_t)((int32_t)pRaw_detections[i*detection_len + AI_YOLOV5_PP_YCENTER]    - (int32_t)zero_point);
        pOutput->pOutBuff[nb_detect].width       = scale * (float32_t)((int32_t)pRaw_detections[i*detection_len + AI_YOLOV5_PP_WIDTHREL]   - (int32_t)zero_point);
        pOutput->pOutBuff[nb_detect].height      = scale * (float32_t)((int32_t)pRaw_detections[i*detection_len + AI_YOLOV5_PP_HEIGHTREL]  - (int32_t)zero_point);
        pOutput->pOutBuff[nb_detect].conf        = scale * (float32_t)((int32_t)best_score - (int32_t)zero_point); //best_score_f;
        pOutput->pOutBuff[nb_detect].class_index = class_index;
        nb_detect++;
      }

    }
  }
  pInput_static_param->nb_detect = nb_detect;;

  return (error);
}


/* ----------------------       Exported routines      ---------------------- */

int32_t od_yolov5_pp_reset(od_yolov5_pp_static_param_t *pInput_static_param)
{
    /* Initializations */
    pInput_static_param->nb_detect = 0;

    return (AI_OD_POSTPROCESS_ERROR_NO);
}


int32_t od_yolov5_pp_process(od_yolov5_pp_in_centroid_t *pInput,
                             od_pp_out_t *pOutput,
                             od_yolov5_pp_static_param_t *pInput_static_param)
{
    int32_t error   = AI_OD_POSTPROCESS_ERROR_NO;

    /* Call Get NN boxes first */
    error = yolov5_pp_getNNBoxes_centroid(pInput,
                                          pOutput,
                                          pInput_static_param);
    if (error != AI_OD_POSTPROCESS_ERROR_NO) return (error);

    /* Then NMS */
    error = yolov5_pp_nmsFiltering_centroid(pOutput,
                                            pInput_static_param);
    if (error != AI_OD_POSTPROCESS_ERROR_NO) return (error);

    /* And score re-filtering */
    error = yolov5_pp_scoreFiltering_centroid(pOutput,
                                              pInput_static_param);

    return (error);
}


int32_t od_yolov5_pp_process_uint8(od_yolov5_pp_in_centroid_t *pInput,
                                   od_pp_out_t *pOutput,
                                   od_yolov5_pp_static_param_t *pInput_static_param)
{
    int32_t error   = AI_OD_POSTPROCESS_ERROR_NO;

    /* Call Get NN boxes first */
    error = yolov5_pp_getNNBoxes_centroid_uint8(pInput,
                                               pOutput,
                                               pInput_static_param);
    if (error != AI_OD_POSTPROCESS_ERROR_NO) return (error);

    /* Then NMS */
    error = yolov5_pp_nmsFiltering_centroid(pOutput,
                                            pInput_static_param);
    if (error != AI_OD_POSTPROCESS_ERROR_NO) return (error);

    /* And score re-filtering */
    error = yolov5_pp_scoreFiltering_centroid(pOutput,
                                              pInput_static_param);

    return (error);
}

