/*---------------------------------------------------------------------------------------------
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file in
 * the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *--------------------------------------------------------------------------------------------*/

#include "od_pp_loc.h"
#include "od_yolov4_pp_if.h"
#include "vision_models_pp.h"


/* Can't be removed if qsort is not re-written... */
static int32_t AI_YOLOV4_PP_SORT_CLASS;

typedef struct
{
	int8_t  x_center;
	int8_t  y_center;
	int8_t  width;
	int8_t  height;
	int8_t  conf;
	uint8_t class_index;
} od_yolov4_pp_scratch_s8_t;



int32_t yolov4_nms_comparator(const void *pa, const void *pb)
{
    od_pp_outBuffer_t a = *(od_pp_outBuffer_t *)pa;
    od_pp_outBuffer_t b = *(od_pp_outBuffer_t *)pb;

    float32_t diff = 0.0;
    float32_t a_weighted_conf = 0.0;
    float32_t b_weighted_conf = 0.0;

    if (a.class_index == AI_YOLOV4_PP_SORT_CLASS)
    {
        a_weighted_conf = a.conf;
    }
    else
    {
         a_weighted_conf = 0.0;
    }

    if (b.class_index == AI_YOLOV4_PP_SORT_CLASS)
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

int32_t yolov4_nms_comparator_is8(const void *pa, const void *pb)
{
    od_yolov4_pp_scratch_s8_t a = *(od_yolov4_pp_scratch_s8_t *)pa;
    od_yolov4_pp_scratch_s8_t b = *(od_yolov4_pp_scratch_s8_t *)pb;

    int16_t diff = 0;
    int8_t a_weighted_conf = INT8_MIN;
    int8_t b_weighted_conf = INT8_MIN;

    if (a.class_index == AI_YOLOV4_PP_SORT_CLASS)
    {
        a_weighted_conf = a.conf;
    }
    else
    {
         a_weighted_conf = INT8_MIN;
    }

    if (b.class_index == AI_YOLOV4_PP_SORT_CLASS)
    {
        b_weighted_conf = b.conf;
    }
    else
    {
         b_weighted_conf = INT8_MIN;
    }

    diff = a_weighted_conf - b_weighted_conf;

    if (diff < 0) return 1;
    else if (diff > 0) return -1;
    return 0;
}


int32_t yolov4_pp_nmsFiltering_centroid(od_pp_out_t *pOutput,
                                        od_yolov4_pp_static_param_t *pInput_static_param)
{
    int32_t j, k, limit_counter, detections_per_class;

    for (k = 0; k < pInput_static_param->nb_classes; ++k)
    {
        limit_counter = 0;
        detections_per_class = 0;
        AI_YOLOV4_PP_SORT_CLASS = k;


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
                  yolov4_nms_comparator);

            for (int32_t i = 0; i < detections_per_class ; i ++)
            {
                if (pOutput->pOutBuff[i].conf == 0) continue;
                float32_t *a = &(pOutput->pOutBuff[i].x_center);
                for (j = i + 1; j < detections_per_class; j ++)
                {
                    if (pOutput->pOutBuff[j].conf == 0) continue;

                    float32_t *b = &(pOutput->pOutBuff[j].x_center);
                    float32_t iou = vision_models_box_iou(a, b);

                    if (iou > pInput_static_param->iou_threshold)
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
        }
    }
    return (AI_OD_POSTPROCESS_ERROR_NO);
}

int32_t yolov4_pp_nmsFiltering_centroid_is8(od_yolov4_pp_scratch_s8_t *ptrScratch,
                                            od_yolov4_pp_static_param_t *pInput_static_param)
{
  int32_t j, k, limit_counter, detections_per_class;

  for (k = 0; k < pInput_static_param->nb_classes; ++k)
  {
    limit_counter = 0;
    detections_per_class = 0;
    AI_YOLOV4_PP_SORT_CLASS = k;


    /* Counts the number of detections with class k */
    for (int32_t i = 0; i < pInput_static_param->nb_detect ; i ++)
    {
      if(ptrScratch[i].class_index == k)
      {
          detections_per_class++;
      }
    }

    if (detections_per_class > 0)
    {
      /* Sorts detections based on class k */
      qsort(ptrScratch,
            pInput_static_param->nb_detect,
            sizeof(od_yolov4_pp_scratch_s8_t),
            yolov4_nms_comparator_is8);

      for (int32_t i = 0; i < detections_per_class ; i ++)
      {
        if (ptrScratch[i].conf == INT8_MIN) continue;
        int8_t *a = &(ptrScratch[i].x_center);
        for (j = i + 1; j < detections_per_class; j ++)
        {
          if (ptrScratch[j].conf == INT8_MIN) continue;
          int8_t *b = &(ptrScratch[j].x_center);
          float32_t iou = vision_models_box_iou_is8(a, b, pInput_static_param->boxe_zero_point);

          if ( iou > pInput_static_param->iou_threshold)
          {
              ptrScratch[j].conf = INT8_MIN;
          }
        }
      }

        /* Limits detections count */
        for (int32_t i = 0; i < detections_per_class; i++)
        {
          if ((limit_counter < pInput_static_param->max_boxes_limit) &&
              (ptrScratch[i].conf != INT8_MIN))
          {
            limit_counter++;
          }
          else
          {

            ptrScratch[i].conf = INT8_MIN;
          }
        }
    }
  }
  return (AI_OD_POSTPROCESS_ERROR_NO);
}

int32_t yolov4_pp_scoreFiltering_centroid(od_pp_out_t *pOutput,
                                          od_yolov4_pp_static_param_t *pInput_static_param)
{
  od_pp_outBuffer_t *pOutBuff = pOutput->pOutBuff;
  int32_t det_count = 0;

  for (int32_t i = 0; i < pInput_static_param->nb_detect; i++)
  {
    if (pOutput->pOutBuff[i].conf >= pInput_static_param->conf_threshold)
    {
      pOutBuff[det_count].x_center    = pOutBuff[i].x_center;
      pOutBuff[det_count].y_center    = pOutBuff[i].y_center;
      pOutBuff[det_count].width       = pOutBuff[i].width;
      pOutBuff[det_count].height      = pOutBuff[i].height;
      pOutBuff[det_count].conf        = pOutBuff[i].conf;
      pOutBuff[det_count].class_index = pOutBuff[i].class_index;
      det_count++;
    }
  }
  pOutput->nb_detect = det_count;

  return (AI_OD_POSTPROCESS_ERROR_NO);
}

int32_t yolov4_pp_scoreFiltering_centroid_is8(od_yolov4_pp_scratch_s8_t *ptrScratch,
                                              od_pp_out_t *pOutput,
                                              od_yolov4_pp_static_param_t *pInput_static_param)
{
    const int8_t proba_zp = pInput_static_param->proba_zero_point;
    const float32_t proba_scale = pInput_static_param->proba_scale;
    const int8_t boxe_zp = pInput_static_param->boxe_zero_point;
    const float32_t boxe_scale = pInput_static_param->boxe_scale;
    int8_t conf_threshold_s8 = (int8_t)(pInput_static_param->conf_threshold / proba_scale + 0.5f) + proba_zp;

    od_pp_outBuffer_t *pOutBuff = pOutput->pOutBuff;
    int32_t det_count           = 0;
    for (int32_t i = 0; i < pInput_static_param->nb_detect; i++)
    {
        if (ptrScratch[i].conf >= conf_threshold_s8)
        {
            pOutBuff[det_count].x_center    = boxe_scale * (float32_t)((int32_t)ptrScratch[i].x_center - (int32_t)boxe_zp);
            pOutBuff[det_count].y_center    = boxe_scale * (float32_t)((int32_t)ptrScratch[i].y_center - (int32_t)boxe_zp);
            pOutBuff[det_count].width       = boxe_scale * (float32_t)((int32_t)ptrScratch[i].width    - (int32_t)boxe_zp);
            pOutBuff[det_count].height      = boxe_scale * (float32_t)((int32_t)ptrScratch[i].height   - (int32_t)boxe_zp);
            pOutBuff[det_count].conf        = proba_scale * (float32_t)((int32_t)ptrScratch[i].conf     - (int32_t)proba_zp);
            pOutBuff[det_count].class_index = (int32_t)ptrScratch[i].class_index;
            det_count++;
        }
    }
    pOutput->nb_detect = det_count;

    return (AI_OD_POSTPROCESS_ERROR_NO);
}

int32_t yolov4_pp_getNNBoxes_centroid(od_yolov4_pp_in_centroid_t *pInput,
                                      od_pp_out_t *pOutput,
                                      od_yolov4_pp_static_param_t *pInput_static_param)
{
    int32_t error                = AI_OD_POSTPROCESS_ERROR_NO;
    const int32_t nb_classes     = pInput_static_param->nb_classes;
    const int32_t nb_total_boxes = pInput_static_param->nb_total_boxes;
    float32_t *pRaw_boxes        = (float32_t *)pInput->pRaw_boxes;
    float32_t *pRaw_probas       = (float32_t *)pInput->pRaw_probas;
    int32_t remaining_boxes      = nb_total_boxes;

    int32_t nb_detect = 0;
    od_pp_outBuffer_t *pOutBuff = pOutput->pOutBuff;
    for (int32_t i = 0; i < nb_total_boxes; i+=4)
    {
        float32_t best_score_array[4];
        uint32_t class_index_array[4];

        vision_models_maxi_tr_p_if32ou32(pRaw_probas + i,
                                         nb_classes,
                                         nb_total_boxes,
                                         best_score_array,
                                         class_index_array,
                                         remaining_boxes);
        for (int _i = 0; _i < ((remaining_boxes>4)?4:remaining_boxes); _i++) {

            if (best_score_array[_i] >= pInput_static_param->conf_threshold)
            {
              float32_t x_min = pRaw_boxes[i + _i + AI_YOLOV4_PP_XMIN   * nb_total_boxes];
              float32_t x_max = pRaw_boxes[i + _i + AI_YOLOV4_PP_XMAX   * nb_total_boxes];
              float32_t y_min = pRaw_boxes[i + _i + AI_YOLOV4_PP_YMIN   * nb_total_boxes];
              float32_t y_max = pRaw_boxes[i + _i + AI_YOLOV4_PP_YMAX   * nb_total_boxes];

              pOutBuff[nb_detect].x_center    = (x_max + x_min) /2;
              pOutBuff[nb_detect].y_center    = (y_max + y_min) /2;
              pOutBuff[nb_detect].width       = x_max - x_min;
              pOutBuff[nb_detect].height      = y_max - y_min;
              pOutBuff[nb_detect].conf        = best_score_array[_i];
              pOutBuff[nb_detect].class_index = class_index_array[_i];
              nb_detect++;
            }
        }
        remaining_boxes-=4;
    }
    pInput_static_param->nb_detect = nb_detect;

    return (error);
}

int32_t yolov4_pp_getNNBoxes_centroid_int8(od_yolov4_pp_in_centroid_t *pInput,
                                           od_pp_out_t *pOutput,
                                           od_yolov4_pp_static_param_t *pInput_static_param)
{
    int32_t error          = AI_OD_POSTPROCESS_ERROR_NO;
    int32_t nb_classes     = pInput_static_param->nb_classes;
    int32_t nb_total_boxes = pInput_static_param->nb_total_boxes;
    int8_t *pRaw_boxes     = (int8_t *)pInput->pRaw_boxes;
    int8_t *pRaw_probas    = (int8_t *)pInput->pRaw_probas;
    const int8_t proba_zp = pInput_static_param->proba_zero_point;
    const float32_t proba_scale = pInput_static_param->proba_scale;
    const int8_t boxe_zp = pInput_static_param->boxe_zero_point;
    const float32_t boxe_scale = pInput_static_param->boxe_scale;

    int32_t remaining_boxes  = nb_total_boxes;
    int8_t conf_threshold_s8 = (int8_t)(pInput_static_param->conf_threshold / proba_scale) + proba_zp;

    od_pp_outBuffer_t *pOutBuff = pOutput->pOutBuff;
    int32_t nb_detect = 0;
    if (nb_classes < 256) {
        int8_t best_score_array[16];
        uint8_t class_index_array[16];
        for (int32_t i = 0; i < nb_total_boxes; i+=16)
        {
            vision_models_maxi_tr_p_is8ou8(pRaw_probas + i,
                                           nb_classes,
                                           nb_total_boxes,
                                           best_score_array,
                                           class_index_array,
                                           remaining_boxes);
            for (int _i = 0; _i < ((remaining_boxes>16)?16:remaining_boxes); _i++) {

                if ( best_score_array[_i] >= conf_threshold_s8) {
                  float32_t best_score_f = proba_scale * (float32_t)(best_score_array[_i] - proba_zp);

                  float32_t x_min = boxe_scale * (float32_t)((int32_t)pRaw_boxes[i + _i + AI_YOLOV4_PP_XMIN * nb_total_boxes] - (int32_t)boxe_zp);
                  float32_t x_max = boxe_scale * (float32_t)((int32_t)pRaw_boxes[i + _i + AI_YOLOV4_PP_XMAX * nb_total_boxes] - (int32_t)boxe_zp);
                  float32_t y_min = boxe_scale * (float32_t)((int32_t)pRaw_boxes[i + _i + AI_YOLOV4_PP_YMIN * nb_total_boxes] - (int32_t)boxe_zp);
                  float32_t y_max = boxe_scale * (float32_t)((int32_t)pRaw_boxes[i + _i + AI_YOLOV4_PP_YMAX * nb_total_boxes] - (int32_t)boxe_zp);

                  pOutBuff[nb_detect].x_center    = (x_max + x_min) /2;
                  pOutBuff[nb_detect].y_center    = (y_max + y_min) /2;;
                  pOutBuff[nb_detect].width       = x_max - x_min;
                  pOutBuff[nb_detect].height      = y_max - y_min;

                  pOutBuff[nb_detect].conf        = best_score_f;
                  pOutBuff[nb_detect].class_index = class_index_array[_i];
                  nb_detect++;
                }
            }
            remaining_boxes-=16;
        }
    } else {
        int8_t best_score_array[16];
        uint16_t class_index_array[16];
        for (int32_t i = 0; i < nb_total_boxes; i+=16)
        {
            vision_models_maxi_tr_p_is8ou16(pRaw_probas + i,
                                            nb_classes,
                                            nb_total_boxes,
                                            best_score_array,
                                            class_index_array,
                                            remaining_boxes);

            for (int _i = 0; _i < ((remaining_boxes>16)?16:remaining_boxes); _i++) {
                if ( best_score_array[_i] >= conf_threshold_s8)
                {
                  float32_t  best_score_f = proba_scale * (float32_t)(best_score_array[_i] - proba_zp);

                  float32_t x_min = boxe_scale * (float32_t)((int32_t)pRaw_boxes[i + _i + AI_YOLOV4_PP_XMIN   * nb_total_boxes] - (int32_t)boxe_zp);
                  float32_t x_max = boxe_scale * (float32_t)((int32_t)pRaw_boxes[i + _i + AI_YOLOV4_PP_XMAX   * nb_total_boxes] - (int32_t)boxe_zp);
                  float32_t y_min = boxe_scale * (float32_t)((int32_t)pRaw_boxes[i + _i + AI_YOLOV4_PP_YMIN   * nb_total_boxes] - (int32_t)boxe_zp);
                  float32_t y_max = boxe_scale * (float32_t)((int32_t)pRaw_boxes[i + _i + AI_YOLOV4_PP_YMAX   * nb_total_boxes] - (int32_t)boxe_zp);

                  pOutBuff[nb_detect].x_center    = (x_max + x_min) /2;
                  pOutBuff[nb_detect].y_center    = (y_max + y_min) /2;
                  pOutBuff[nb_detect].width       = x_max - x_min;
                  pOutBuff[nb_detect].height      = y_max - y_min;

                  pOutBuff[nb_detect].conf        = best_score_f;
                  pOutBuff[nb_detect].class_index = class_index_array[_i];

                  nb_detect++;
                }
            }

           remaining_boxes-=16;
        }

    }
    pInput_static_param->nb_detect = nb_detect;

    return (error);
}

int32_t yolov4_pp_getNNBoxes_centroid_is8os8(od_yolov4_pp_in_centroid_t *pInput,
                                             od_yolov4_pp_scratch_s8_t *ptrScratch,
                                             od_yolov4_pp_static_param_t *pInput_static_param)
{
    int32_t error   = AI_OD_POSTPROCESS_ERROR_NO;
    const int32_t nb_classes = pInput_static_param->nb_classes;
    int32_t nb_total_boxes = pInput_static_param->nb_total_boxes;
    int8_t *pRaw_boxes = (int8_t *)pInput->pRaw_boxes;
    int8_t *pRaw_probas = (int8_t *)pInput->pRaw_probas;
    const int8_t proba_zp = pInput_static_param->proba_zero_point;
    const float32_t proba_scale = pInput_static_param->proba_scale;
    const int8_t boxe_zp = pInput_static_param->boxe_zero_point;

    pInput_static_param->nb_detect =0;
    int32_t remaining_boxes = nb_total_boxes;
    const int8_t conf_threshold_s8 = (int8_t)(pInput_static_param->conf_threshold / proba_scale + 0.5f) + proba_zp;
    int32_t nb_detect = 0;

    int8_t best_score_array[16];
    uint8_t class_index_array[16];

    for (int32_t i = 0; i < nb_total_boxes; i+=16)
    {
        vision_models_maxi_tr_p_is8ou8(pRaw_probas + i,
                                       nb_classes,
                                       nb_total_boxes,
                                       best_score_array,
                                       class_index_array,
                                      remaining_boxes);
        for (int _i = 0; _i < ((remaining_boxes>16)?16:remaining_boxes); _i++) {

            if ( best_score_array[_i] >= conf_threshold_s8) {
              int8_t x_min = pRaw_boxes[i + _i + AI_YOLOV4_PP_XMIN * nb_total_boxes];
              int8_t x_max = pRaw_boxes[i + _i + AI_YOLOV4_PP_XMAX * nb_total_boxes];
              int8_t y_min = pRaw_boxes[i + _i + AI_YOLOV4_PP_YMIN * nb_total_boxes];
              int8_t y_max = pRaw_boxes[i + _i + AI_YOLOV4_PP_YMAX * nb_total_boxes];

              ptrScratch[nb_detect].x_center    = (x_max + x_min) / 2;
              ptrScratch[nb_detect].y_center    = (y_max + y_min) / 2;
              ptrScratch[nb_detect].width       = x_max - x_min + boxe_zp;
              ptrScratch[nb_detect].height      = y_max - y_min + boxe_zp;

              ptrScratch[nb_detect].conf        = best_score_array[_i];
              ptrScratch[nb_detect].class_index = class_index_array[_i];
              nb_detect++;
            }
        }
        remaining_boxes-=16;
    }
    pInput_static_param->nb_detect = nb_detect;
    return (error);
}

/* ----------------------       Exported routines      ---------------------- */

int32_t od_yolov4_pp_reset(od_yolov4_pp_static_param_t *pInput_static_param)
{
    /* Initializations */
    pInput_static_param->nb_detect = 0;

    return (AI_OD_POSTPROCESS_ERROR_NO);
}


int32_t od_yolov4_pp_process(od_yolov4_pp_in_centroid_t *pInput,
                             od_pp_out_t *pOutput,
                             od_yolov4_pp_static_param_t *pInput_static_param)
{
    int32_t error   = AI_OD_POSTPROCESS_ERROR_NO;

    /* Call Get NN boxes first */
    error = yolov4_pp_getNNBoxes_centroid(pInput,
                                          pOutput,
                                          pInput_static_param);
    if (error != AI_OD_POSTPROCESS_ERROR_NO) return (error);

    /* Then NMS */
    error = yolov4_pp_nmsFiltering_centroid(pOutput,
                                            pInput_static_param);
    if (error != AI_OD_POSTPROCESS_ERROR_NO) return (error);

    /* And score re-filtering */
    error = yolov4_pp_scoreFiltering_centroid(pOutput,
                                              pInput_static_param);

    return (error);
}


int32_t od_yolov4_pp_process_int8(od_yolov4_pp_in_centroid_t *pInput,
                                  od_pp_out_t *pOutput,
                                  od_yolov4_pp_static_param_t *pInput_static_param)
{
  int32_t error = AI_OD_POSTPROCESS_ERROR_NO;
  od_yolov4_pp_scratch_s8_t *ptrScratch = pInput_static_param->pScratchBuff;

  if ((NULL == ptrScratch) && (pInput_static_param->nb_classes < UINT8_MAX) ){
    if ((pInput_static_param->nb_classes * sizeof(int8_t)) > sizeof(od_yolov4_pp_scratch_s8_t)) {
      ptrScratch = (od_yolov4_pp_scratch_s8_t *)pInput->pRaw_probas;
    }
  }

  if (ptrScratch) {
    /* Call Get NN boxes first */
    error = yolov4_pp_getNNBoxes_centroid_is8os8(pInput,
                                                 ptrScratch,
                                                 pInput_static_param);
    if (error != AI_OD_POSTPROCESS_ERROR_NO) return (error);

    /* Then NMS */
    error = yolov4_pp_nmsFiltering_centroid_is8(ptrScratch,
                                                pInput_static_param);
    if (error != AI_OD_POSTPROCESS_ERROR_NO) return (error);

    /* And score re-filtering */
    error = yolov4_pp_scoreFiltering_centroid_is8(ptrScratch,
                                                  pOutput,
                                                  pInput_static_param);
  } else {
    /* Call Get NN boxes first */
    error = yolov4_pp_getNNBoxes_centroid_int8(pInput,
                                               pOutput,
                                               pInput_static_param);
    if (error != AI_OD_POSTPROCESS_ERROR_NO) return (error);

    /* Then NMS */
    error = yolov4_pp_nmsFiltering_centroid(pOutput,
                                            pInput_static_param);
    if (error != AI_OD_POSTPROCESS_ERROR_NO) return (error);

    /* And score re-filtering */
    error = yolov4_pp_scoreFiltering_centroid(pOutput,
                                              pInput_static_param);
  }
  return (error);
}

