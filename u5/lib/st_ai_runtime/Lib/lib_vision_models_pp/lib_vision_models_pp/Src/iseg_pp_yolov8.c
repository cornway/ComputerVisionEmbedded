/*---------------------------------------------------------------------------------------------
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file in
 * the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *--------------------------------------------------------------------------------------------*/

#include "iseg_yolov8_pp_if.h"
#include "vision_models_pp.h"
#include "iseg_pp_loc.h"

/* Can't be removed if qsort is not re-written... */
static int32_t AI_YOLOV8_SEG_PP_SORT_CLASS;

static int32_t iseg_yolov8_nms_comparator_s8(const void *pa, const void *pb)
{
    iseg_yolov8_pp_scratchBuffer_s8_t a = *(iseg_yolov8_pp_scratchBuffer_s8_t *)pa;
    iseg_yolov8_pp_scratchBuffer_s8_t b = *(iseg_yolov8_pp_scratchBuffer_s8_t *)pb;

    int16_t diff = 0;
    int8_t a_weighted_conf = -128;
    int8_t b_weighted_conf = -128;

    if (a.class_index == AI_YOLOV8_SEG_PP_SORT_CLASS)
    {
        a_weighted_conf = a.conf;
    }

    if (b.class_index == AI_YOLOV8_SEG_PP_SORT_CLASS)
    {
        b_weighted_conf = b.conf;
    }

    diff = (int16_t)a_weighted_conf - (int16_t)b_weighted_conf;

    if (diff < 0) return 1;
    else if (diff > 0) return -1;
    return 0;
}

static
int32_t iseg_yolov8_pp_nmsFiltering_centroid_is8os8(iseg_yolov8_pp_static_param_t *pInput_static_param)
{
  int32_t j, k, limit_counter, detections_per_class;//, limit_type;
  iseg_yolov8_pp_scratchBuffer_s8_t *pOutBuff_s8 = pInput_static_param->pTmpBuff;

    for (k = 0; k < pInput_static_param->nb_classes; ++k)
    {
        limit_counter = 0;
        detections_per_class = 0;
        AI_YOLOV8_SEG_PP_SORT_CLASS = k;


        /* Counts the number of detections with class k */
        for (int32_t i = 0; i < pInput_static_param->nb_detect ; i ++)
        {
            if(pOutBuff_s8[i].class_index == k)
            {
                detections_per_class++;
            }
        }

        if (detections_per_class > 0)
        {
            /* Sorts detections based on class k */
            qsort(pOutBuff_s8,
                  pInput_static_param->nb_detect,
                  sizeof(iseg_yolov8_pp_scratchBuffer_s8_t),
                  iseg_yolov8_nms_comparator_s8);

            for (int32_t i = 0; i < detections_per_class ; i ++)
            {
                if (pOutBuff_s8[i].conf == -128) continue;
                int8_t *a = &pOutBuff_s8[i].x_center;
                for (j = i + 1; j < detections_per_class; j ++)
                {
                    if (pOutBuff_s8[j].conf == -128) continue;
                    int8_t * b = &pOutBuff_s8[j].x_center;
                    float32_t f_iou = vision_models_box_iou_is8(a, b, pInput_static_param->raw_output_zero_point);
                    if (f_iou > pInput_static_param->iou_threshold)
                    {
                        pOutBuff_s8[j].conf = -128;
                    }
                }
            }

            /* Limits detections count */
            for (int32_t i = 0; i < detections_per_class; i++)
            {
                if ((limit_counter < pInput_static_param->max_boxes_limit) &&
                    (pOutBuff_s8[i].conf != -128))
                {
                    limit_counter++;
                }
                else
                {
                    pOutBuff_s8[i].conf = -128;
                }
            }
        }
    }

    return (AI_ISEG_POSTPROCESS_ERROR_NO);
}
static
int32_t iseg_yolov8_pp_scoreFiltering_centroid_is8(iseg_yolov8_pp_in_centroid_t *pInput,
                                                   iseg_pp_out_t *pOutput,
                                                   iseg_yolov8_pp_static_param_t *pInput_static_param)
{
  int32_t det_count = 0;
  iseg_yolov8_pp_scratchBuffer_s8_t *pOutBuff_s8 = pInput_static_param->pTmpBuff;
  pOutput->nb_detect = MIN(pInput_static_param->nb_detect, pInput_static_param->max_boxes_limit);

  // get masks
  int8_t mask_zp       = pInput_static_param->mask_raw_output_zero_point;
  float32_t mask_scale = pInput_static_param->mask_raw_output_scale;
  int8_t raw_zp        = pInput_static_param->raw_output_zero_point;
  float32_t raw_scale  = pInput_static_param->raw_output_scale;
  int8_t threshold_s8  = (int8_t)(pInput_static_param->conf_threshold / raw_scale + 0.5f + raw_zp);

  float32_t threshold_check = 0.5f / (mask_scale * raw_scale);
  int32_t threshold_check_s32 = (int32_t)(threshold_check+0.5f);

#ifdef ARM_MATH_MVEF
  uint32x4_t offset = vidupq_n_u32(0,1);
  offset *= (uint32_t)pInput_static_param->nb_masks;
#endif

  for (int32_t d = 0; d < pInput_static_param->nb_detect; d++)
  {
    if (pOutBuff_s8[d].conf >= threshold_s8 && det_count < pInput_static_param->max_boxes_limit)
    {
      int32_t *detection_mask = (int32_t *)pInput_static_param->pMask;
      uint8_t *binary_mask    = pOutput->pOutBuff[det_count].pMask;
      int8_t  *Raw_masks      = pInput->pRaw_masks;//(64x64x32)

      pOutput->pOutBuff[det_count].x_center    = ((int32_t)pOutBuff_s8[d].x_center - raw_zp) * raw_scale;
      pOutput->pOutBuff[det_count].y_center    = ((int32_t)pOutBuff_s8[d].y_center - raw_zp) * raw_scale;
      pOutput->pOutBuff[det_count].width       = ((int32_t)pOutBuff_s8[d].width    - raw_zp) * raw_scale;
      pOutput->pOutBuff[det_count].height      = ((int32_t)pOutBuff_s8[d].height   - raw_zp) * raw_scale;
      pOutput->pOutBuff[det_count].conf        = ((int32_t)pOutBuff_s8[d].conf     - raw_zp) * raw_scale;
      pOutput->pOutBuff[det_count].class_index =  (int32_t)pOutBuff_s8[d].class_index;

      for (int32_t k = 0; k < pInput_static_param->nb_masks ; k++)
      {
        detection_mask[k] = ((int32_t)pOutBuff_s8[d].pMask[k] - raw_zp);
      }

      // Perform matrix multiplication
      for (int32_t i = 0; i < pInput_static_param->size_masks; i++)
      {
#ifdef ARM_MATH_MVEF
          int32_t iter_loop = pInput_static_param->size_masks >> 2;
          while(iter_loop--)
          {
            // Read for 4 ouputs
              int32x4_t sum_product_s32x4 = vdupq_n_s32(0);

              for (int32_t k = 0; k < pInput_static_param->nb_masks ; k++)
              {
                // Load 4 int8_t in int32x4_t register
                int32x4_t rawMask = vldrbq_gather_offset_s32(Raw_masks, offset);
                sum_product_s32x4 += detection_mask[k] * (rawMask - (int32_t)mask_zp);

                Raw_masks++;
              }
              // Compare and store 4 results
              mve_pred16_t p0 = vcmpgeq_n_s32(sum_product_s32x4, threshold_check_s32);
              uint32x4_t outBinary = vpselq_u32(vdupq_n_u32(1), vdupq_n_u32(0), p0);
              vstrbq_u32(binary_mask, outBinary);

              binary_mask+=4;
              Raw_masks+=3*pInput_static_param->nb_masks;
          }
          // Remaining
          iter_loop = pInput_static_param->size_masks & 3;
          while(iter_loop--)
          {
              int32_t sum_product = 0;
              for (int32_t k = 0; k < pInput_static_param->nb_masks; k++)
              {
                sum_product += detection_mask[k] * ((int32_t)(*Raw_masks) - (int32_t)mask_zp);

                Raw_masks++;
              }
              *binary_mask++ = (sum_product >= threshold_check_s32)?1:0;
          }
#else
          for (int32_t j = 0; j < pInput_static_param->size_masks; j++)
          {
              int32_t sum_product = 0;
              for (int32_t k = 0; k < pInput_static_param->nb_masks ; k++)
              {
                sum_product += detection_mask[k] * ((int32_t)(*Raw_masks) - (int32_t)mask_zp);

                Raw_masks++;
              }
              *binary_mask++ = (sum_product >= threshold_check_s32)?1:0;
          }
#endif
      }
      det_count++;
    }
  }

  pOutput->nb_detect = det_count;

  return (AI_ISEG_POSTPROCESS_ERROR_NO);
}

static
int32_t iseg_yolov8_pp_getNNBoxes_centroid_is8os8(iseg_yolov8_pp_in_centroid_t *pInput,
                                                  iseg_yolov8_pp_static_param_t *pInput_static_param)
{
  int32_t error                                    = AI_ISEG_POSTPROCESS_ERROR_NO;
  iseg_yolov8_pp_scratchBuffer_s8_t *pOutBuff_s8 = pInput_static_param->pTmpBuff;
  int32_t nb_classes      = pInput_static_param->nb_classes;
  int32_t nb_total_boxes  = pInput_static_param->nb_total_boxes;
  int8_t *pRaw_detections = (int8_t *)pInput->pRaw_detections;
  int8_t zero_point       = pInput_static_param->raw_output_zero_point;
  float32_t scale         = pInput_static_param->raw_output_scale;
  int8_t threshold_s8     = (int8_t)(pInput_static_param->conf_threshold / scale + 0.5f + zero_point);

  pInput_static_param->nb_detect = 0;
  int32_t loop_cnt = nb_total_boxes;

  while (loop_cnt > 0)
  {
    int8_t best_score_array[16];
    uint8_t class_index_array[16];

    const uint8_t parallel = MIN(loop_cnt, 16);

    vision_models_maxi_tr_p_is8ou8(&pRaw_detections[AI_YOLOV8_PP_CLASSPROB * nb_total_boxes],
                                   nb_classes,
                                   nb_total_boxes,
                                   best_score_array,
                                   class_index_array,
                                   parallel);

    for (int _i = 0; _i < parallel; _i++)
    {
      if (best_score_array[_i] >= threshold_s8)
      {
        pOutBuff_s8->x_center    = pRaw_detections[AI_YOLOV8_PP_XCENTER   * nb_total_boxes];
        pOutBuff_s8->y_center    = pRaw_detections[AI_YOLOV8_PP_YCENTER   * nb_total_boxes];
        pOutBuff_s8->width       = pRaw_detections[AI_YOLOV8_PP_WIDTHREL  * nb_total_boxes];
        pOutBuff_s8->height      = pRaw_detections[AI_YOLOV8_PP_HEIGHTREL * nb_total_boxes];
        pOutBuff_s8->conf        = best_score_array[_i];
        pOutBuff_s8->class_index = class_index_array[_i];
        for (int32_t j = 0; j < pInput_static_param->nb_masks; j++)
        {
          pOutBuff_s8->pMask[j] =  pRaw_detections[(AI_YOLOV8_PP_CLASSPROB + nb_classes + j) * nb_total_boxes];
        }
        pInput_static_param->nb_detect++;
        pOutBuff_s8++;
      }
      pRaw_detections++;
    }
    loop_cnt -= parallel;

  }

  return (error);
}

/* ----------------------       Exported routines      ---------------------- */

int32_t iseg_yolov8_pp_reset(iseg_yolov8_pp_static_param_t *pInput_static_param)
{
    /* Initializations */
    pInput_static_param->nb_detect = 0;

    return (AI_ISEG_POSTPROCESS_ERROR_NO);
}

int32_t iseg_yolov8_pp_process_int8(iseg_yolov8_pp_in_centroid_t *pInput,
                                    iseg_pp_out_t *pOutput,
                                    iseg_yolov8_pp_static_param_t *pInput_static_param)
{
    int32_t error   = AI_ISEG_POSTPROCESS_ERROR_NO;

    /* Call Get NN boxes first */
    error = iseg_yolov8_pp_getNNBoxes_centroid_is8os8(pInput,
                                                      pInput_static_param);


    if (error != AI_ISEG_POSTPROCESS_ERROR_NO) return (error);

    /* Then NMS */
    error = iseg_yolov8_pp_nmsFiltering_centroid_is8os8(pInput_static_param);


    if (error != AI_ISEG_POSTPROCESS_ERROR_NO) return (error);

    /* And score re-filtering */
   error = iseg_yolov8_pp_scoreFiltering_centroid_is8(pInput,
                                                      pOutput,
                                                      pInput_static_param);
    return (error);
}


