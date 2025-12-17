/*---------------------------------------------------------------------------------------------
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file in
 * the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *--------------------------------------------------------------------------------------------*/

#include "od_pp_loc.h"
#include "od_ssd_pp_if.h"
#include "vision_models_pp.h"

/* Can't be removed if qsort is not re-written... */
static int32_t AI_SSD_PP_SORT_CLASS;

int32_t ssd_nms_comparator(const void *pa, const void *pb)
{
  od_pp_outBuffer_t *pa_s = (od_pp_outBuffer_t *)pa;
  od_pp_outBuffer_t *pb_s = (od_pp_outBuffer_t *)pb;
  float32_t a = (pa_s->class_index == AI_SSD_PP_SORT_CLASS) ? pa_s->conf : 0;
  float32_t b = (pb_s->class_index == AI_SSD_PP_SORT_CLASS) ? pb_s->conf : 0;
  float32_t diff = 0;

  diff = a - b;

  if (diff < 0) return 1;
  else if (diff > 0) return -1;
  return 0;
}

static int32_t SSD_quick_sort_partition(float32_t *pScores,
                                        float32_t *pBoxes,
                                        int32_t first,
                                        int32_t last,
                                        int32_t dir,
                                        int32_t ssd_sort_class,
                                        int32_t ssd_nb_classes)
{
    int32_t i, j, pivot_index;
    float32_t pivot;
    pivot_index = first;
    pivot = pScores[pivot_index * ssd_nb_classes + ssd_sort_class];
    i = first - 1;
    j = last + 1;

    while (i < j)
    {
        if (dir)
        {
            do
            {
                i++;
            } while ((pScores[i * ssd_nb_classes + ssd_sort_class] < pivot) &&
                     (i < last));
            do
            {
                j--;
            } while (pScores[j * ssd_nb_classes + ssd_sort_class] > pivot);
        }
        else
        {
            do
            {
                i++;
            } while ((pScores[i * ssd_nb_classes + ssd_sort_class] > pivot) &&
                     (i < last));
            do
            {
                j--;
            } while (pScores[j * ssd_nb_classes + ssd_sort_class] < pivot);
        }

        if (i < j)
        {
            for (int _i = 0; _i < AI_SSD_PP_BOX_STRIDE; _i++){
              float32_t _tmp = pBoxes[i * AI_SSD_PP_BOX_STRIDE + _i];
              pBoxes[i * AI_SSD_PP_BOX_STRIDE + _i] = pBoxes[j * AI_SSD_PP_BOX_STRIDE + _i];
              pBoxes[j * AI_SSD_PP_BOX_STRIDE + _i] = _tmp;
            }
            for (int _i = 0; _i < ssd_nb_classes; _i++){
              float32_t _tmp = pScores[i * ssd_nb_classes + _i];
              pScores[i * ssd_nb_classes + _i] = pScores[j * ssd_nb_classes + _i];
              pScores[j * ssd_nb_classes + _i] = _tmp;
            }
        }
    }
    return j;
}

static void SSD_quick_sort_core(float32_t *pScores,
                                float32_t *pBoxes,
                                int32_t first,
                                int32_t last,
                                int32_t dir,
                                int32_t ssd_sort_class,
                                int32_t ssd_nb_classes)
{
    /*
     dir 0  : descending
     dir 1  : ascending
    */
    if (first < last)
    {
        int32_t pivot;
        pivot = SSD_quick_sort_partition(pScores,
                                         pBoxes,
                                         first,
                                         last,
                                         dir,
                                         ssd_sort_class,
                                         ssd_nb_classes);
        SSD_quick_sort_core(pScores,
                            pBoxes,
                            first,
                            pivot,
                            dir,
                            ssd_sort_class,
                            ssd_nb_classes);
        SSD_quick_sort_core(pScores,
                            pBoxes,
                            pivot + 1,
                            last,
                            dir,
                            ssd_sort_class,
                            ssd_nb_classes);
    }
}



/**
 *
 * int32_t ssd_pp_getNNBoxes
 *
**/
int32_t ssd_pp_getNNBoxes(od_ssd_pp_in_centroid_t *pInput,
                          od_ssd_pp_static_param_t *pInput_static_param)
{
  float32_t *pScores  = (float32_t *)pInput->pScores;
  float32_t *pBoxes   = (float32_t *)pInput->pBoxes;
  float32_t *pAnchors = (float32_t *)pInput->pAnchors;
  uint32_t nb_detect = 0;

  for (int32_t i = 0; i < pInput_static_param->nb_detections; i+=4)
  {
    float32_t best_score[4];
    uint32_t class_index[4];
      vision_models_maxi_p_if32ou32(&(pScores[i * pInput_static_param->nb_classes]),
                     pInput_static_param->nb_classes,
                     pInput_static_param->nb_classes,
                     best_score,
                     class_index,
                     pInput_static_param->nb_detections-i);
      for (int _i = 0; _i < MIN(4, pInput_static_param->nb_detections-i); _i++) {

        if (best_score[_i] >= pInput_static_param->conf_threshold)
        {
            for (int32_t k = 0; k < pInput_static_param->nb_classes; ++k)
            {
                pScores[nb_detect * pInput_static_param->nb_classes + k] = pScores[(i + _i) * pInput_static_param->nb_classes + k];
            }
            pBoxes[nb_detect * AI_SSD_PP_BOX_STRIDE + AI_SSD_PP_CENTROID_XCENTER]   = pBoxes[(i + _i) * AI_SSD_PP_BOX_STRIDE + AI_SSD_PP_CENTROID_XCENTER] / pInput_static_param->XY_scale * pAnchors[(i + _i) * AI_SSD_PP_BOX_STRIDE + AI_SSD_PP_CENTROID_WIDTHREL] +
            pAnchors[(i + _i) * AI_SSD_PP_BOX_STRIDE + AI_SSD_PP_CENTROID_XCENTER];
            pBoxes[nb_detect * AI_SSD_PP_BOX_STRIDE + AI_SSD_PP_CENTROID_YCENTER]   = pBoxes[(i + _i) * AI_SSD_PP_BOX_STRIDE + AI_SSD_PP_CENTROID_YCENTER] / pInput_static_param->XY_scale * pAnchors[(i + _i) * AI_SSD_PP_BOX_STRIDE + AI_SSD_PP_CENTROID_HEIGHTREL] +
            pAnchors[(i + _i) * AI_SSD_PP_BOX_STRIDE + AI_SSD_PP_CENTROID_YCENTER];
            pBoxes[nb_detect * AI_SSD_PP_BOX_STRIDE + AI_SSD_PP_CENTROID_WIDTHREL]  = expf(pBoxes[(i + _i) * AI_SSD_PP_BOX_STRIDE + AI_SSD_PP_CENTROID_WIDTHREL] / pInput_static_param->WH_scale) * pAnchors[(i + _i) * AI_SSD_PP_BOX_STRIDE + AI_SSD_PP_CENTROID_WIDTHREL];
            pBoxes[nb_detect * AI_SSD_PP_BOX_STRIDE + AI_SSD_PP_CENTROID_HEIGHTREL] = expf(pBoxes[(i + _i) * AI_SSD_PP_BOX_STRIDE + AI_SSD_PP_CENTROID_HEIGHTREL] / pInput_static_param->WH_scale) * pAnchors[(i + _i) * AI_SSD_PP_BOX_STRIDE + AI_SSD_PP_CENTROID_HEIGHTREL];

            nb_detect++;
        }
      }
  }
  pInput_static_param->nb_detect = nb_detect;

  return (AI_OD_POSTPROCESS_ERROR_NO);
}

int32_t ssd_pp_getNNBoxes_scratchBuffer(od_ssd_pp_in_centroid_t *pInput,
                                        od_pp_outBuffer_t *pScratchBuffer,
                                     od_ssd_pp_static_param_t *pInput_static_param)
{
  float32_t *pScores = (float32_t *)pInput->pScores;
  float32_t *pBoxes = (float32_t *)pInput->pBoxes;
  float32_t *pAnchors = (float32_t *)pInput->pAnchors;
  uint32_t nb_detect = 0;

  float32_t inv_XY_scale = 1.0f / pInput_static_param->XY_scale;
  float32_t inv_WH_scale = 1.0f / pInput_static_param->WH_scale;

  for (int32_t i = 0; i < pInput_static_param->nb_detections; i+=4)
  {
    float32_t best_score[4];
    uint32_t class_index[4];
      vision_models_maxi_p_if32ou32(&(pScores[i * pInput_static_param->nb_classes]),
                     pInput_static_param->nb_classes,
                     pInput_static_param->nb_classes,
                     best_score,
                     class_index,
                     pInput_static_param->nb_detections-i);
      for (int _i = 0; _i < MIN(4, pInput_static_param->nb_detections-i); _i++) {

        if (best_score[_i] >= pInput_static_param->conf_threshold)
        {
          pScratchBuffer[nb_detect].class_index = class_index[_i];
          pScratchBuffer[nb_detect].conf = best_score[_i];
          pScratchBuffer[nb_detect].x_center = pBoxes[(i + _i) * AI_SSD_PP_BOX_STRIDE + AI_SSD_PP_CENTROID_XCENTER] *inv_XY_scale * pAnchors[(i + _i) * AI_SSD_PP_BOX_STRIDE + AI_SSD_PP_CENTROID_WIDTHREL] +
            pAnchors[(i + _i) * AI_SSD_PP_BOX_STRIDE + AI_SSD_PP_CENTROID_XCENTER];
          pScratchBuffer[nb_detect].y_center = pBoxes[(i + _i) * AI_SSD_PP_BOX_STRIDE + AI_SSD_PP_CENTROID_YCENTER] * inv_XY_scale * pAnchors[(i + _i) * AI_SSD_PP_BOX_STRIDE + AI_SSD_PP_CENTROID_HEIGHTREL] +
            pAnchors[(i + _i) * AI_SSD_PP_BOX_STRIDE + AI_SSD_PP_CENTROID_YCENTER];
          pScratchBuffer[nb_detect].width = expf(pBoxes[(i + _i) * AI_SSD_PP_BOX_STRIDE + AI_SSD_PP_CENTROID_WIDTHREL] * inv_WH_scale) * pAnchors[(i + _i) * AI_SSD_PP_BOX_STRIDE + AI_SSD_PP_CENTROID_WIDTHREL];
          pScratchBuffer[nb_detect].height =expf(pBoxes[(i + _i) * AI_SSD_PP_BOX_STRIDE + AI_SSD_PP_CENTROID_HEIGHTREL] *inv_WH_scale) * pAnchors[(i + _i) * AI_SSD_PP_BOX_STRIDE + AI_SSD_PP_CENTROID_HEIGHTREL];


          nb_detect++;
        }
        pInput_static_param->nb_detect = nb_detect;
      }
  }

  return (AI_OD_POSTPROCESS_ERROR_NO);
}


int32_t ssd_pp_getNNBoxes_int8_scratchBuffer(od_ssd_pp_in_centroid_t *pInput,
                                             od_pp_outBuffer_t *pScratchBuffer,
                                             od_ssd_pp_static_param_t *pInput_static_param)
{
  int8_t *pScores  = (int8_t *)pInput->pScores;
  int8_t *pBoxes   = (int8_t *)pInput->pBoxes;
  int8_t *pAnchors = (int8_t *)pInput->pAnchors;
  uint32_t nb_detect = 0;

  int8_t boxe_zp         = pInput_static_param->boxe_zero_point;
  float32_t boxe_scale   = pInput_static_param->boxe_scale;

  int8_t anchor_zp       = pInput_static_param->anchor_zero_point;
  float32_t anchor_scale = pInput_static_param->anchor_scale;

  int8_t score_zp        = pInput_static_param->score_zero_point;
  float32_t score_scale  = pInput_static_param->score_scale;

  float32_t inv_XY_scale = 1.0f / pInput_static_param->XY_scale;
  float32_t inv_WH_scale = 1.0f / pInput_static_param->WH_scale;

  int8_t conf_threshold_s8 = (int8_t)(pInput_static_param->conf_threshold / score_scale + 0.5f) + score_zp;

  for (int32_t i = 0; i < pInput_static_param->nb_detections; i+=16)
  {
    int8_t best_score[16];
    uint8_t class_index[16];
    vision_models_maxi_p_is8ou8(&(pScores[i * pInput_static_param->nb_classes]),
                                pInput_static_param->nb_classes,
                                pInput_static_param->nb_classes,
                                best_score,
                                class_index,
                                pInput_static_param->nb_detections-i);

    for (int _i = 0; _i < MIN(16, pInput_static_param->nb_detections-i); _i++)
    {
      if (best_score[_i] >= conf_threshold_s8)
      {
        float32_t value, anchor_center, anchor_rel_x, anchor_rel_y, score;
        score = (float32_t)((int32_t)best_score[_i] - score_zp) * score_scale;
        pScratchBuffer[nb_detect].class_index = class_index[_i];
        pScratchBuffer[nb_detect].conf = score;

        value         = (float32_t)((int32_t)  pBoxes[(i + _i) * AI_SSD_PP_BOX_STRIDE + AI_SSD_PP_CENTROID_XCENTER]   - boxe_zp )  * boxe_scale;
        anchor_rel_x  = (float32_t)((int32_t)pAnchors[(i + _i) * AI_SSD_PP_BOX_STRIDE + AI_SSD_PP_CENTROID_WIDTHREL]  - anchor_zp) * anchor_scale;
        anchor_center = (float32_t)((int32_t)pAnchors[(i + _i) * AI_SSD_PP_BOX_STRIDE + AI_SSD_PP_CENTROID_XCENTER]   - anchor_zp) * anchor_scale;
        pScratchBuffer[nb_detect].x_center = value * inv_XY_scale * anchor_rel_x + anchor_center;

        value         = (float32_t)((int32_t)  pBoxes[(i + _i) * AI_SSD_PP_BOX_STRIDE + AI_SSD_PP_CENTROID_YCENTER]   - boxe_zp )  * boxe_scale;
        anchor_rel_y  = (float32_t)((int32_t)pAnchors[(i + _i) * AI_SSD_PP_BOX_STRIDE + AI_SSD_PP_CENTROID_HEIGHTREL] - anchor_zp) * anchor_scale;
        anchor_center = (float32_t)((int32_t)pAnchors[(i + _i) * AI_SSD_PP_BOX_STRIDE + AI_SSD_PP_CENTROID_YCENTER]   - anchor_zp) * anchor_scale;
        pScratchBuffer[nb_detect].y_center = value * inv_XY_scale * anchor_rel_y + anchor_center;

        value         = (float32_t)((int32_t)  pBoxes[(i + _i) * AI_SSD_PP_BOX_STRIDE + AI_SSD_PP_CENTROID_WIDTHREL]  - boxe_zp )  * boxe_scale;
        pScratchBuffer[nb_detect].width  = expf(value * inv_WH_scale) * anchor_rel_x;

        value         = (float32_t)((int32_t)  pBoxes[(i + _i) * AI_SSD_PP_BOX_STRIDE + AI_SSD_PP_CENTROID_HEIGHTREL] - boxe_zp )  * boxe_scale;
        pScratchBuffer[nb_detect].height = expf(value * inv_WH_scale) * anchor_rel_y;

        nb_detect++;
      } // if > thrshold
    } // for _i (up to 16 elements)
  } // for i
  pInput_static_param->nb_detect = nb_detect;

  return (AI_OD_POSTPROCESS_ERROR_NO);
}



int32_t ssd_pp_nms_filtering(od_ssd_pp_in_centroid_t *pInput,
                             od_ssd_pp_static_param_t *pInput_static_param)
{
  int32_t i, j, k, limit_counter;
  float32_t *pScores = (float32_t *)pInput->pScores;
  float32_t *pBoxes = (float32_t *)pInput->pBoxes;

    for (k = 0; k < pInput_static_param->nb_classes; ++k)
    {
        limit_counter = 0;
        AI_SSD_PP_SORT_CLASS = k;

        SSD_quick_sort_core(pScores,
                            pBoxes,
                            0,
                            pInput_static_param->nb_detect - 1,
                            0,
                            AI_SSD_PP_SORT_CLASS,
                            pInput_static_param->nb_classes);

        for (i = 0; i < pInput_static_param->nb_detect; ++i)
        {
            if (pScores[i * pInput_static_param->nb_classes + k] == 0)
            {
                continue;
            }
            float32_t *pA = &(pBoxes[AI_SSD_PP_BOX_STRIDE * i + AI_SSD_PP_CENTROID_YCENTER]);
            for (j = i + 1; j < pInput_static_param->nb_detect; ++j)
            {
                float32_t *pB = &(pBoxes[AI_SSD_PP_BOX_STRIDE * j + AI_SSD_PP_CENTROID_YCENTER]);
                if (vision_models_box_iou(pA, pB) > pInput_static_param->iou_threshold)
                {
                    pScores[j * pInput_static_param->nb_classes + k] = 0;
                }
            }
        }

        for (int32_t it = 0; it < pInput_static_param->nb_detect; ++it)
        {
            if ((pScores[it * pInput_static_param->nb_classes + k] != 0) &&
                (limit_counter < pInput_static_param->max_boxes_limit))
            {
                limit_counter++;
            }
            else
            {
                pScores[it * pInput_static_param->nb_classes + k] = 0;
            }
        }
    }

    return (AI_OD_POSTPROCESS_ERROR_NO);
}

int32_t ssd_pp_nms_filtering_scratchBuffer(od_pp_outBuffer_t *pScratchBuffer,
                                          od_ssd_pp_static_param_t *pInput_static_param)
{
  int32_t i, j, k, limit_counter;

    for (k = 0; k < pInput_static_param->nb_classes; ++k)
    {
        limit_counter = 0;
        int32_t detections_per_class = 0;
        AI_SSD_PP_SORT_CLASS = k;

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

          qsort(pScratchBuffer,
              pInput_static_param->nb_detect,
              sizeof(od_pp_outBuffer_t),
              (_Cmpfun *)ssd_nms_comparator);
        for (i = 0; i < detections_per_class; i++)
        {
            if (pScratchBuffer[i].conf == 0)
            {
                continue;
            }
            float32_t *pA = &(pScratchBuffer[i].x_center);
            for (j = i + 1; j < detections_per_class; j++)
            {
                if (pScratchBuffer[j].conf == 0)
                {
                  continue;
                }
                float32_t *pB = &(pScratchBuffer[j].x_center);
                if (vision_models_box_iou(pA, pB) > pInput_static_param->iou_threshold)
                {
                    pScratchBuffer[j].conf = 0;
                }
            }
        }

        for (int32_t it = 0; it < detections_per_class; ++it)
        {
            if ((pScratchBuffer[it].conf != 0) &&
                (limit_counter < pInput_static_param->max_boxes_limit))
            {
                limit_counter++;
            }
            else
            {

                pScratchBuffer[it].conf = 0;
            }
        }
        } // if detections
    } // for k in classes

    return (AI_OD_POSTPROCESS_ERROR_NO);
}




int32_t ssd_pp_score_filtering(od_ssd_pp_in_centroid_t *pInput,
                               od_pp_out_t *pOutput,
                               od_ssd_pp_static_param_t *pInput_static_param)
{
  float32_t *pScores = (float32_t *)pInput->pScores;
  float32_t *pBoxes = (float32_t *)pInput->pBoxes;


    int32_t count = 0;
    for (int32_t i = 0; i < pInput_static_param->nb_detect; i+=4)
    {
      uint32_t class_index[4];
      float32_t best_score[4];
      vision_models_maxi_p_if32ou32(&(pScores[i * pInput_static_param->nb_classes]),
                                  pInput_static_param->nb_classes,
                                  pInput_static_param->nb_classes,
                                  best_score,
                                  class_index,
                                  pInput_static_param->nb_detect-i);

      for (int _i = 0; _i < MIN(4,pInput_static_param->nb_detect-i); _i++) {
        if (best_score[_i] >= pInput_static_param->conf_threshold)
        {
            pOutput->pOutBuff[count].class_index = class_index[_i];
            pOutput->pOutBuff[count].conf = best_score[_i];
            pOutput->pOutBuff[count].x_center = pBoxes[(i + _i) * AI_SSD_PP_BOX_STRIDE + AI_SSD_PP_CENTROID_XCENTER];
            pOutput->pOutBuff[count].y_center = pBoxes[(i + _i) * AI_SSD_PP_BOX_STRIDE + AI_SSD_PP_CENTROID_YCENTER];
            pOutput->pOutBuff[count].width    = pBoxes[(i + _i) * AI_SSD_PP_BOX_STRIDE + AI_SSD_PP_CENTROID_WIDTHREL];
            pOutput->pOutBuff[count].height   = pBoxes[(i + _i) * AI_SSD_PP_BOX_STRIDE + AI_SSD_PP_CENTROID_HEIGHTREL];

            count++;
        }
      }

    }
    pOutput->nb_detect = count;
  return (AI_OD_POSTPROCESS_ERROR_NO);
}

int32_t ssd_pp_score_filtering_scratchBuffer(od_pp_outBuffer_t *pScratch,
                               od_pp_out_t *pOutput,
                               od_ssd_pp_static_param_t *pInput_static_param)
{

  int32_t count = 0;
  od_pp_outBuffer_t *pOutBuff = pOutput->pOutBuff;
  for (int32_t i = 0; i < pInput_static_param->nb_detect; i++)
  {
    if (pOutBuff[i].conf)
    {
        pOutBuff[count].class_index = pScratch[i].class_index;
        pOutBuff[count].conf        = pScratch[i].conf;
        pOutBuff[count].x_center    = pScratch[i].x_center;
        pOutBuff[count].y_center    = pScratch[i].y_center;
        pOutBuff[count].width       = pScratch[i].width;
        pOutBuff[count].height      = pScratch[i].height;

        count++;
    }
  }

  pOutput->nb_detect = count;
  return (AI_OD_POSTPROCESS_ERROR_NO);
}

/* ----------------------       Exported routines      ---------------------- */

int32_t od_ssd_pp_reset(od_ssd_pp_static_param_t *pInput_static_param)
{
    /* Initializations */
    pInput_static_param->nb_detect = 0;

    return (AI_OD_POSTPROCESS_ERROR_NO);
}


int32_t od_ssd_pp_process(od_ssd_pp_in_centroid_t *pInput,
                          od_pp_out_t *pOutput,
                          od_ssd_pp_static_param_t *pInput_static_param)
{
  int32_t error = AI_OD_POSTPROCESS_ERROR_NO;

  od_pp_outBuffer_t *pScratchBuffer = pInput_static_param->scratchBuffer;
  /* if no scratch buffer is specified and space enough in score array, use it as scratch buffer */
  if (   (pScratchBuffer == NULL)
      && (pInput_static_param->nb_classes * sizeof(float32_t) >= sizeof(od_pp_outBuffer_t))) {
    pScratchBuffer = (od_pp_outBuffer_t *)pInput->pScores;
  }
  if ( (pOutput->pOutBuff == NULL))
  {
    if (pInput_static_param->nb_classes * sizeof(float32_t) >= sizeof(od_pp_outBuffer_t))
    {
      pOutput->pOutBuff = (od_pp_outBuffer_t *)pInput->pScores;
    }
    else
    {
      return AI_OD_POSTPROCESS_ERROR;
    }
  }

  if (pScratchBuffer)
  {
    /* Calls Get NN boxes first */
    error = ssd_pp_getNNBoxes_scratchBuffer(pInput,
                                            pScratchBuffer,
                                            pInput_static_param);
    if (error != AI_OD_POSTPROCESS_ERROR_NO) return (error);

    /* Then NMS */
    error = ssd_pp_nms_filtering_scratchBuffer(pScratchBuffer,
                                pInput_static_param);
    if (error != AI_OD_POSTPROCESS_ERROR_NO) return (error);

    /* And score re-filtering */
    error = ssd_pp_score_filtering_scratchBuffer(pScratchBuffer,
                                  pOutput,
                                  pInput_static_param);

  }
  else
  {

  /* Calls Get NN boxes first */
  error = ssd_pp_getNNBoxes(pInput,
                            pInput_static_param);
  if (error != AI_OD_POSTPROCESS_ERROR_NO) return (error);

  /* Then NMS */
  error = ssd_pp_nms_filtering(pInput,
                               pInput_static_param);
  if (error != AI_OD_POSTPROCESS_ERROR_NO) return (error);

  /* And score re-filtering */
  error = ssd_pp_score_filtering(pInput,
                                 pOutput,
                                 pInput_static_param);

  }

  return (error);
}
int32_t od_ssd_pp_process_int8(od_ssd_pp_in_centroid_t *pInput,
                               od_pp_out_t *pOutput,
                               od_ssd_pp_static_param_t *pInput_static_param)
{
  int32_t error = AI_OD_POSTPROCESS_ERROR_NO;

  od_pp_outBuffer_t *pScratchBuffer = pInput_static_param->scratchBuffer;
  /* if no scratch buffer is specified and space enough in score array, use it as scratch buffer */
  if (pScratchBuffer == NULL)
  {
    if (pInput_static_param->nb_classes * sizeof(int8_t) >= sizeof(od_pp_outBuffer_t))
    {
      pScratchBuffer = (od_pp_outBuffer_t *)pInput->pScores;
    }
    else
    {
      // no scratch buffer available
      return AI_OD_POSTPROCESS_ERROR;
    }
  }
  if ( (pOutput->pOutBuff == NULL))
  {
    if (pInput_static_param->nb_classes * sizeof(int8_t) >= sizeof(od_pp_outBuffer_t))
    {
      pOutput->pOutBuff = (od_pp_outBuffer_t *)pInput->pScores;
    }
    else
    {
      return AI_OD_POSTPROCESS_ERROR;
    }
  }

  /* Calls Get NN boxes first */
  error = ssd_pp_getNNBoxes_int8_scratchBuffer(pInput,
                                               pScratchBuffer,
                                               pInput_static_param);
  if (error != AI_OD_POSTPROCESS_ERROR_NO) return (error);

  /* Then NMS */
  error = ssd_pp_nms_filtering_scratchBuffer(pScratchBuffer,
                                             pInput_static_param);
  if (error != AI_OD_POSTPROCESS_ERROR_NO) return (error);

  /* And score re-filtering */
  error = ssd_pp_score_filtering_scratchBuffer(pScratchBuffer,
                                               pOutput,
                                               pInput_static_param);

  return (error);
}
