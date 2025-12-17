/*---------------------------------------------------------------------------------------------
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file in
 * the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *--------------------------------------------------------------------------------------------*/

#include "od_pp_loc.h"
#include "mpe_yolov8_pp_if.h"
#include "vision_models_pp.h"


/* Can't be removed if qsort is not re-written... */
static int32_t AI_YOLOV8_POSE_PP_SORT_CLASS;


int32_t mpe_yolov8_nms_comparator(const void *pa, const void *pb)
{
    mpe_pp_outBuffer_t a = *(mpe_pp_outBuffer_t *)pa;
    mpe_pp_outBuffer_t b = *(mpe_pp_outBuffer_t *)pb;

    float32_t diff = 0.0;
    float32_t a_weighted_conf = 0.0;
    float32_t b_weighted_conf = 0.0;

    if (a.class_index == AI_YOLOV8_POSE_PP_SORT_CLASS)
    {
        a_weighted_conf = a.conf;
    }
    else
    {
         a_weighted_conf = 0.0;
    }

    if (b.class_index == AI_YOLOV8_POSE_PP_SORT_CLASS)
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

int32_t mpe_yolov8_nms_comparator_is8(const void *pa, const void *pb)
{
    mpe_pp_scratchBuffer_s8_t a = *(mpe_pp_scratchBuffer_s8_t *)pa;
    mpe_pp_scratchBuffer_s8_t b = *(mpe_pp_scratchBuffer_s8_t *)pb;

    int32_t diff = 0;
    int8_t a_weighted_conf = -128;
    int8_t b_weighted_conf = -128;

    if (a.class_index == AI_YOLOV8_POSE_PP_SORT_CLASS)
    {
        a_weighted_conf = a.conf;
    }
    else
    {
         a_weighted_conf = -128;
    }

    if (b.class_index == AI_YOLOV8_POSE_PP_SORT_CLASS)
    {
        b_weighted_conf = b.conf;
    }
    else
    {
         b_weighted_conf = -128;
    }

    diff = a_weighted_conf - b_weighted_conf;

    if (diff < 0) return 1;
    else if (diff > 0) return -1;
    return 0;
}


int32_t mpe_yolo_pp_nmsFiltering_centroid(mpe_pp_out_t *pOutput,
                                          mpe_yolov8_pp_static_param_t *pInput_static_param)
{
    int32_t j, k, limit_counter, detections_per_class;

    for (k = 0; k < pInput_static_param->nb_classes; ++k)
    {
        limit_counter = 0;
        detections_per_class = 0;
        AI_YOLOV8_POSE_PP_SORT_CLASS = k;


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
                  sizeof(mpe_pp_outBuffer_t),
                  mpe_yolov8_nms_comparator);

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
        }
    }
    return (AI_VISION_MODELS_PP_ERROR_NO);
}

int32_t mpe_yolo_pp_nmsFiltering_centroid_is8(mpe_pp_scratchBuffer_s8_t *pScratchBuffer,
                                              mpe_yolov8_pp_static_param_t *pInput_static_param)
{
    int32_t j, k, limit_counter, detections_per_class;

    for (k = 0; k < pInput_static_param->nb_classes; ++k)
    {
        limit_counter = 0;
        detections_per_class = 0;
        AI_YOLOV8_POSE_PP_SORT_CLASS = k;


        /* Counts the number of detections with class k */
        for (int32_t i = 0; i < pInput_static_param->nb_detect ; i ++)
        {
            if(pScratchBuffer[i].class_index == k)
            {
                detections_per_class++;
            }
        }

        if (detections_per_class > 0)
        {
          /* Sorts detections based on class k */
          qsort(pScratchBuffer,
                  pInput_static_param->nb_detect,
                  sizeof(mpe_pp_scratchBuffer_s8_t),
                  mpe_yolov8_nms_comparator_is8);

            for (int32_t i = 0; i < detections_per_class ; i ++)
            {

                if (pScratchBuffer[i].conf == -128) continue;
                int8_t *a = &(pScratchBuffer[i].x_center);
                for (j = i + 1; j < detections_per_class; j ++)
                {
                    int8_t *b = &(pScratchBuffer[j].x_center);
                    if (vision_models_box_iou_is8(a, b, pInput_static_param->raw_output_zero_point) > pInput_static_param->iou_threshold)
                    {
                        pScratchBuffer[j].conf = -128;
                    }
                }
            }

            /* Limits detections count */
            for (int32_t i = 0; i < detections_per_class; i++)
            {
                if ((limit_counter < pInput_static_param->max_boxes_limit) &&
                    (pScratchBuffer[i].conf != -128))
                {
                    limit_counter++;
                }
                else
                {
                    pScratchBuffer[i].conf = -128;
                }
            }
        }
    }
    return (AI_VISION_MODELS_PP_ERROR_NO);
}


int32_t mpe_yolo_pp_scoreFiltering_centroid(mpe_pp_out_t *pOutput,
                                          mpe_yolov8_pp_static_param_t *pInput_static_param)
{
    int32_t det_count = 0;

    for (int32_t i = 0; i < pInput_static_param->nb_detect; i++)
    {
        if (pOutput->pOutBuff[i].conf >= pInput_static_param->conf_threshold)
        {
            pOutput->pOutBuff[det_count].x_center = pOutput->pOutBuff[i].x_center;
            pOutput->pOutBuff[det_count].y_center = pOutput->pOutBuff[i].y_center;
            pOutput->pOutBuff[det_count].width = pOutput->pOutBuff[i].width;
            pOutput->pOutBuff[det_count].height = pOutput->pOutBuff[i].height;
            pOutput->pOutBuff[det_count].conf = pOutput->pOutBuff[i].conf;
            pOutput->pOutBuff[det_count].class_index = pOutput->pOutBuff[i].class_index;
            for (uint32_t j = 0; j < pInput_static_param->nb_keypoints; j++)
            {
                pOutput->pOutBuff[det_count].pKeyPoints[j].x = pOutput->pOutBuff[i].pKeyPoints[j].x;
                pOutput->pOutBuff[det_count].pKeyPoints[j].y = pOutput->pOutBuff[i].pKeyPoints[j].y;
                pOutput->pOutBuff[det_count].pKeyPoints[j].conf = pOutput->pOutBuff[i].pKeyPoints[j].conf;
            }
            det_count++;
        }
    }
    pOutput->nb_detect = det_count;

    return (AI_VISION_MODELS_PP_ERROR_NO);
}

int32_t mpe_yolo_pp_scoreFiltering_centroid_is8(mpe_pp_scratchBuffer_s8_t *pScratchBuffer,
                                                mpe_pp_out_t *pOutput,
                                                mpe_yolov8_pp_static_param_t *pInput_static_param)
{
    int32_t det_count = 0;
    int8_t zero_point = pInput_static_param->raw_output_zero_point;
    float32_t scale = pInput_static_param->raw_output_scale;
    int8_t conf_threshold_s8 = (int8_t)(pInput_static_param->conf_threshold / scale + 0.5f) + zero_point;

    for (int32_t i = 0; i < pInput_static_param->nb_detect; i++)
    {
        if (pScratchBuffer[i].conf >= conf_threshold_s8)
        {
            pOutput->pOutBuff[det_count].x_center     = scale * (float32_t)((int32_t)pScratchBuffer[i].x_center - (int32_t)zero_point);
            pOutput->pOutBuff[det_count].y_center     = scale * (float32_t)((int32_t)pScratchBuffer[i].y_center - (int32_t)zero_point);
            pOutput->pOutBuff[det_count].width        = scale * (float32_t)((int32_t)pScratchBuffer[i].width    - (int32_t)zero_point);
            pOutput->pOutBuff[det_count].height       = scale * (float32_t)((int32_t)pScratchBuffer[i].height   - (int32_t)zero_point);
            pOutput->pOutBuff[det_count].conf         = scale * (float32_t)((int32_t)pScratchBuffer[i].conf     - (int32_t)zero_point);
            pOutput->pOutBuff[det_count].class_index = pScratchBuffer[i].class_index;
            for (uint32_t j = 0; j < pInput_static_param->nb_keypoints; j++)
            {
                pOutput->pOutBuff[det_count].pKeyPoints[j].x    = scale * (float32_t)((int32_t)pScratchBuffer[i].pKeyPoints[j].x    - (int32_t)zero_point);
                pOutput->pOutBuff[det_count].pKeyPoints[j].y    = scale * (float32_t)((int32_t)pScratchBuffer[i].pKeyPoints[j].y    - (int32_t)zero_point);
                pOutput->pOutBuff[det_count].pKeyPoints[j].conf = scale * (float32_t)((int32_t)pScratchBuffer[i].pKeyPoints[j].conf - (int32_t)zero_point);
            }
            det_count++;
        }
    }
    pOutput->nb_detect = det_count;

    return (AI_VISION_MODELS_PP_ERROR_NO);
}

int32_t mpe_yolo_pp_getNNBoxes_centroid(mpe_yolov8_pp_in_centroid_t *pInput,
                                      mpe_pp_out_t *pOutput,
                                      mpe_yolov8_pp_static_param_t *pInput_static_param)
{
    int32_t error   = AI_VISION_MODELS_PP_ERROR_NO;
    int32_t nb_classes = pInput_static_param->nb_classes;
    int32_t nb_total_boxes = pInput_static_param->nb_total_boxes;
    float32_t *pRaw_detections = (float32_t *)pInput->pRaw_detections;
    int32_t remaining_boxes = nb_total_boxes;

    int32_t nb_detect = 0;
    for (int32_t i = 0; i < nb_total_boxes; i+=4)
    {
        float32_t best_score_array[4];
        uint32_t class_index_array[4];

        vision_models_maxi_tr_p_if32ou32(&pRaw_detections[i + AI_YOLOV8_PP_CLASSPROB * nb_total_boxes],
                                         nb_classes,
                                         nb_total_boxes,
                                         best_score_array,
                                         class_index_array,
                                         remaining_boxes);
        for (int _i = 0; _i < ((remaining_boxes>4)?4:remaining_boxes); _i++) {


            if (best_score_array[_i] >= pInput_static_param->conf_threshold)
            {
                pOutput->pOutBuff[nb_detect].x_center    = pRaw_detections[i + _i + AI_YOLOV8_PP_XCENTER   * nb_total_boxes];
                pOutput->pOutBuff[nb_detect].y_center    = pRaw_detections[i + _i + AI_YOLOV8_PP_YCENTER   * nb_total_boxes];
                pOutput->pOutBuff[nb_detect].width       = pRaw_detections[i + _i + AI_YOLOV8_PP_WIDTHREL  * nb_total_boxes];
                pOutput->pOutBuff[nb_detect].height      = pRaw_detections[i + _i + AI_YOLOV8_PP_HEIGHTREL * nb_total_boxes];
                pOutput->pOutBuff[nb_detect].conf        = best_score_array[_i];
                pOutput->pOutBuff[nb_detect].class_index = class_index_array[_i];
                for (uint32_t j = 0; j <  pInput_static_param->nb_keypoints; j++)
                {
                    pOutput->pOutBuff[nb_detect].pKeyPoints[j].x    = pRaw_detections[i + _i + (AI_YOLOV8_PP_CLASSPROB + nb_classes + 3 * j + 0) * nb_total_boxes];
                    pOutput->pOutBuff[nb_detect].pKeyPoints[j].y    = pRaw_detections[i + _i + (AI_YOLOV8_PP_CLASSPROB + nb_classes + 3 * j + 1) * nb_total_boxes];
                    pOutput->pOutBuff[nb_detect].pKeyPoints[j].conf = pRaw_detections[i + _i + (AI_YOLOV8_PP_CLASSPROB + nb_classes + 3 * j + 2) * nb_total_boxes];
                }
                nb_detect++;
            }
        }
        remaining_boxes-=4;
    }
    pInput_static_param->nb_detect = nb_detect;

    return (error);
}

int32_t mpe_yolo_pp_getNNBoxes_centroid_int8(mpe_yolov8_pp_in_centroid_t *pInput,
                                             mpe_pp_out_t *pOutput,
                                             mpe_yolov8_pp_static_param_t *pInput_static_param)
{
    int32_t error           = AI_VISION_MODELS_PP_ERROR_NO;
    int32_t nb_classes      = pInput_static_param->nb_classes;
    int32_t nb_total_boxes  = pInput_static_param->nb_total_boxes;
    int8_t *pRaw_detections = pInput->pRaw_detections;
    int32_t remaining_boxes = nb_total_boxes;
    int8_t zero_point = pInput_static_param->raw_output_zero_point;
    float32_t scale = pInput_static_param->raw_output_scale;
    int8_t conf_threshold_s8 = (int8_t)(pInput_static_param->conf_threshold / scale) + zero_point;

    int32_t nb_detect = 0;
    for (int32_t i = 0; i < nb_total_boxes; i+=16)
    {
        int8_t best_score_array[16];
        uint8_t class_index_array[16];

        vision_models_maxi_tr_p_is8ou8(&pRaw_detections[i + AI_YOLOV8_PP_CLASSPROB * nb_total_boxes],
                                       nb_classes,
                                       nb_total_boxes,
                                       best_score_array,
                                       class_index_array,
                                       remaining_boxes);
        for (int _i = 0; _i < ((remaining_boxes>16)?16:remaining_boxes); _i++) {
            if (best_score_array[_i] >= conf_threshold_s8)
            {
                 pOutput->pOutBuff[nb_detect].x_center    = scale * (float32_t)((int32_t)pRaw_detections[i + _i + AI_YOLOV8_PP_XCENTER * nb_total_boxes] - (int32_t)zero_point);
                 pOutput->pOutBuff[nb_detect].y_center    = scale * (float32_t)((int32_t)pRaw_detections[i + _i + AI_YOLOV8_PP_YCENTER * nb_total_boxes] - (int32_t)zero_point);
                 pOutput->pOutBuff[nb_detect].width       = scale * (float32_t)((int32_t)pRaw_detections[i + _i + AI_YOLOV8_PP_WIDTHREL * nb_total_boxes] - (int32_t)zero_point);
                 pOutput->pOutBuff[nb_detect].height      = scale * (float32_t)((int32_t)pRaw_detections[i + _i + AI_YOLOV8_PP_HEIGHTREL * nb_total_boxes] - (int32_t)zero_point);
                 pOutput->pOutBuff[nb_detect].conf        = scale * (float32_t)((int32_t)best_score_array[_i] - (int32_t)zero_point);
                 pOutput->pOutBuff[nb_detect].class_index = class_index_array[_i];
                for (uint32_t j = 0; j <  pInput_static_param->nb_keypoints; j++)
                {
                     pOutput->pOutBuff[nb_detect].pKeyPoints[j].x    = scale * (float32_t)((int32_t)pRaw_detections[i + _i + (AI_YOLOV8_PP_CLASSPROB + nb_classes + 3 * j + 0) * nb_total_boxes] - (int32_t)zero_point);
                     pOutput->pOutBuff[nb_detect].pKeyPoints[j].y    = scale * (float32_t)((int32_t)pRaw_detections[i + _i + (AI_YOLOV8_PP_CLASSPROB + nb_classes + 3 * j + 1) * nb_total_boxes] - (int32_t)zero_point);
                     pOutput->pOutBuff[nb_detect].pKeyPoints[j].conf = scale * (float32_t)((int32_t)pRaw_detections[i + _i + (AI_YOLOV8_PP_CLASSPROB + nb_classes + 3 * j + 2) * nb_total_boxes] - (int32_t)zero_point);
                }
                nb_detect++;
            }
        }
        remaining_boxes-=16;
    }
    pInput_static_param->nb_detect = nb_detect;

    return (error);
}


int32_t mpe_yolo_pp_getNNBoxes_centroid_is8os8(mpe_yolov8_pp_in_centroid_t *pInput,
                                             mpe_pp_scratchBuffer_s8_t *pScratchBuffer,
                                             mpe_yolov8_pp_static_param_t *pInput_static_param)
{
    int32_t error           = AI_VISION_MODELS_PP_ERROR_NO;
    int32_t nb_classes      = pInput_static_param->nb_classes;
    int32_t nb_total_boxes  = pInput_static_param->nb_total_boxes;
    int8_t *pRaw_detections = (int8_t *)pInput->pRaw_detections;
    int32_t remaining_boxes = nb_total_boxes;
    int8_t zero_point = pInput_static_param->raw_output_zero_point;
    float32_t scale = pInput_static_param->raw_output_scale;
    int8_t conf_threshold_s8 = (int8_t)((int32_t)((float32_t)pInput_static_param->conf_threshold / scale) + zero_point);

    int32_t nb_detect = 0;
    if (nb_classes < 256)
    {
      for (int32_t i = 0; i < nb_total_boxes; i+=16)
      {
          int8_t best_score_array[16];
          uint8_t class_index_array[16];

          vision_models_maxi_tr_p_is8ou8(&pRaw_detections[i + AI_YOLOV8_PP_CLASSPROB * nb_total_boxes],
                                         nb_classes,
                                         nb_total_boxes,
                                         best_score_array,
                                         class_index_array,
                                         remaining_boxes);
          for (int _i = 0; _i < MIN(remaining_boxes,16); _i++) {

              if (best_score_array[_i] >= conf_threshold_s8)
              {
                  pScratchBuffer[nb_detect].x_center    = pRaw_detections[i + _i + AI_YOLOV8_PP_XCENTER   * nb_total_boxes];
                  pScratchBuffer[nb_detect].y_center    = pRaw_detections[i + _i + AI_YOLOV8_PP_YCENTER   * nb_total_boxes];
                  pScratchBuffer[nb_detect].width       = pRaw_detections[i + _i + AI_YOLOV8_PP_WIDTHREL  * nb_total_boxes];
                  pScratchBuffer[nb_detect].height      = pRaw_detections[i + _i + AI_YOLOV8_PP_HEIGHTREL * nb_total_boxes];
                  pScratchBuffer[nb_detect].conf        = best_score_array[_i];
                  pScratchBuffer[nb_detect].class_index = class_index_array[_i];
                  for (uint32_t j = 0; j <  pInput_static_param->nb_keypoints; j++)
                  {
                      pScratchBuffer[nb_detect].pKeyPoints[j].x    = pRaw_detections[i + _i + (AI_YOLOV8_PP_CLASSPROB + nb_classes + 3 * j + 0) * nb_total_boxes];
                      pScratchBuffer[nb_detect].pKeyPoints[j].y    = pRaw_detections[i + _i + (AI_YOLOV8_PP_CLASSPROB + nb_classes + 3 * j + 1) * nb_total_boxes];
                      pScratchBuffer[nb_detect].pKeyPoints[j].conf = pRaw_detections[i + _i + (AI_YOLOV8_PP_CLASSPROB + nb_classes + 3 * j + 2) * nb_total_boxes];
                  }
                  nb_detect++;
              }
          }
          remaining_boxes-=16;
      }
    }
    else
    {
      for (int32_t i = 0; i < nb_total_boxes; i+=16)
      {
          int8_t best_score_array[16];
          uint16_t class_index_array[16];

          vision_models_maxi_tr_p_is8ou16(&pRaw_detections[i + AI_YOLOV8_PP_CLASSPROB * nb_total_boxes],
                                          nb_classes,
                                          nb_total_boxes,
                                          best_score_array,
                                          class_index_array,
                                          remaining_boxes);
          for (int _i = 0; _i < MIN(remaining_boxes,16); _i++) {

              if (best_score_array[_i] >= conf_threshold_s8)
              {
                  pScratchBuffer[nb_detect].x_center    = pRaw_detections[i + _i + AI_YOLOV8_PP_XCENTER   * nb_total_boxes];
                  pScratchBuffer[nb_detect].y_center    = pRaw_detections[i + _i + AI_YOLOV8_PP_YCENTER   * nb_total_boxes];
                  pScratchBuffer[nb_detect].width       = pRaw_detections[i + _i + AI_YOLOV8_PP_WIDTHREL  * nb_total_boxes];
                  pScratchBuffer[nb_detect].height      = pRaw_detections[i + _i + AI_YOLOV8_PP_HEIGHTREL * nb_total_boxes];
                  pScratchBuffer[nb_detect].conf        = best_score_array[_i];
                  pScratchBuffer[nb_detect].class_index = class_index_array[_i];
                  for (uint32_t j = 0; j <  pInput_static_param->nb_keypoints; j++)
                  {
                      pScratchBuffer[nb_detect].pKeyPoints[j].x    = pRaw_detections[i + _i + (AI_YOLOV8_PP_CLASSID + 3 * j + 0) * nb_total_boxes];
                      pScratchBuffer[nb_detect].pKeyPoints[j].y    = pRaw_detections[i + _i + (AI_YOLOV8_PP_CLASSID + 3 * j + 1) * nb_total_boxes];
                      pScratchBuffer[nb_detect].pKeyPoints[j].conf = pRaw_detections[i + _i + (AI_YOLOV8_PP_CLASSID + 3 * j + 2) * nb_total_boxes];
                  }
                  nb_detect++;
              }
          }
          remaining_boxes-=16;
      }
    }
    pInput_static_param->nb_detect = nb_detect;

    return (error);
}

/* ----------------------       Exported routines      ---------------------- */

int32_t mpe_yolov8_pp_reset(mpe_yolov8_pp_static_param_t *pInput_static_param)
{
    /* Initializations */
    pInput_static_param->nb_detect = 0;

    return (AI_VISION_MODELS_PP_ERROR_NO);
}


int32_t mpe_yolov8_pp_process(mpe_yolov8_pp_in_centroid_t *pInput,
                              mpe_pp_out_t *pOutput,
                              mpe_yolov8_pp_static_param_t *pInput_static_param)
{
    int32_t error   = AI_VISION_MODELS_PP_ERROR_NO;

    /* Call Get NN boxes first */
    error = mpe_yolo_pp_getNNBoxes_centroid(pInput,
                                          pOutput,
                                          pInput_static_param);
    if (error != AI_VISION_MODELS_PP_ERROR_NO) return (error);

    /* Then NMS */
    error = mpe_yolo_pp_nmsFiltering_centroid(pOutput,
                                            pInput_static_param);
    if (error != AI_VISION_MODELS_PP_ERROR_NO) return (error);

    /* And score re-filtering */
    error = mpe_yolo_pp_scoreFiltering_centroid(pOutput,
                                              pInput_static_param);

    return (error);
}

int32_t mpe_yolov8_pp_process_int8(mpe_yolov8_pp_in_centroid_t *pInput,
                                    mpe_pp_out_t *pOutput,
                                    mpe_yolov8_pp_static_param_t *pInput_static_param)
{
    int32_t error   = AI_VISION_MODELS_PP_ERROR_NO;

    mpe_pp_scratchBuffer_s8_t *ptrScratch = pInput_static_param->pScratchBuffer;

    if (ptrScratch) {
        /* Call Get NN boxes first */
      error = mpe_yolo_pp_getNNBoxes_centroid_is8os8(pInput,
                                                   ptrScratch,
                                                   pInput_static_param);
      if (error != AI_VISION_MODELS_PP_ERROR_NO) return (error);

      /* Then NMS */
      error = mpe_yolo_pp_nmsFiltering_centroid_is8(ptrScratch,
                                                pInput_static_param);
      if (error != AI_VISION_MODELS_PP_ERROR_NO) return (error);

      /* And score re-filtering */
      error = mpe_yolo_pp_scoreFiltering_centroid_is8(ptrScratch,
                                                      pOutput,
                                                pInput_static_param);
    }
    else
    {
      /* Call Get NN boxes first */
      error = mpe_yolo_pp_getNNBoxes_centroid_int8(pInput,
                                                    pOutput,
                                                    pInput_static_param);
      if (error != AI_VISION_MODELS_PP_ERROR_NO) return (error);

      /* Then NMS */
      error = mpe_yolo_pp_nmsFiltering_centroid(pOutput,
                                                pInput_static_param);
      if (error != AI_VISION_MODELS_PP_ERROR_NO) return (error);

      /* And score re-filtering */
      error = mpe_yolo_pp_scoreFiltering_centroid(pOutput,
                                                pInput_static_param);
    }

    return (error);
}


