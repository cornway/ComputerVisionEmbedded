/*---------------------------------------------------------------------------------------------
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file in
 * the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *--------------------------------------------------------------------------------------------*/

#include "od_pp_loc.h"
#include "od_centernet_pp_if.h"
#include "vision_models_pp.h"


/* Trick to have this structure representation overlapped with real output representation */
typedef struct centernet_pp_tmp_outBuffer
{
  float32_t      top_left_x;
  float32_t      top_left_y;
  float32_t      bottom_right_x;
  float32_t      bottom_right_y;
  float32_t      conf;
  int32_t        class_index;
} centernet_pp_tmp_outBuffer_t;


//***************iou ********
static inline
float32_t centernet_overlap(float32_t l1, float32_t r1, float32_t l2, float32_t r2)
{
    float32_t left = l1 > l2 ? l1 : l2;
    float32_t right = r1 < r2 ? r1 : r2;
    return (right - left);
}

static inline
float32_t centernet_box_intersection(centernet_pp_tmp_outBuffer_t *a, centernet_pp_tmp_outBuffer_t *b)
{
    float32_t w = centernet_overlap(a->top_left_x, a->bottom_right_x, b->top_left_x, b->bottom_right_x);
    float32_t h = centernet_overlap(a->top_left_y, a->bottom_right_y, b->top_left_y, b->bottom_right_y);
    if (w < 0 || h < 0) return 0;
    float32_t area = w * h;
    return (area);
}

static inline
float32_t centernet_box_union(centernet_pp_tmp_outBuffer_t *a, centernet_pp_tmp_outBuffer_t *b)
{
    float32_t i = centernet_box_intersection(a, b);
    float32_t u = (a->bottom_right_x - a->top_left_x) * (a->bottom_right_y - a->top_left_y) +
                  (b->bottom_right_x - b->top_left_x) * (b->bottom_right_y - b->top_left_y) -
                  i;
    return (u);
}

static inline
float32_t centernet_box_iou(centernet_pp_tmp_outBuffer_t *a, centernet_pp_tmp_outBuffer_t *b)
{

    float32_t I = centernet_box_intersection(a, b);
    float32_t U = centernet_box_union(a, b);
    if (I == 0 || U == 0) {
        return 0;
    }
    return (I / U);
}


int32_t centernet_nms_comparator(const void *pa, const void *pb)
{
    od_pp_outBuffer_t *a = (od_pp_outBuffer_t *)pa;
    od_pp_outBuffer_t *b = (od_pp_outBuffer_t *)pb;
    float32_t diff = 0;
    diff = b->conf - a->conf;

    if (diff < 0) return 1;
    else if (diff > 0) return -1;
    return 0;
}


int32_t centernet_pp_nmsFiltering_centroid(centernet_pp_tmp_outBuffer_t  *pInput,
                                           od_pp_out_t  *pOutput,
                                           od_centernet_pp_static_param_t *pInput_static_param)
{
    int32_t i, j, k, limit_counter;
    int32_t error   = AI_VISION_MODELS_PP_ERROR_NO;
    int32_t det_count = 0;
    int32_t conf_stride = sizeof(od_pp_outBuffer_t);
    centernet_pp_tmp_outBuffer_t *pInbuff = (centernet_pp_tmp_outBuffer_t *)pInput;
    od_pp_outBuffer_t *pOutbuff = (od_pp_outBuffer_t *)pOutput->pOutBuff;

    /* First sorts by confidence scores */
    qsort((float32_t *)pInbuff,
           pInput_static_param->nb_detect,
           conf_stride,
           (_Cmpfun *)centernet_nms_comparator);

    /* Applies NMS per class */
    for (k = 0; k < pInput_static_param->nb_classifs; ++k)
    {
        limit_counter = 0;

        pInbuff = (centernet_pp_tmp_outBuffer_t *)pInput;
        for (i = 0; i < pInput_static_param->nb_detect; i++, pInbuff++)
        {
            if (pInbuff->class_index != k) continue;
            centernet_pp_tmp_outBuffer_t *pInbuff_2 = (pInbuff + 1);
            for (j = i + 1; j < pInput_static_param->nb_detect; j++, pInbuff_2++)
            {
                if (pInbuff_2->class_index != k) continue;
                if (centernet_box_iou(pInbuff, pInbuff_2) > pInput_static_param->iou_threshold)
                {
                    pInbuff->conf = 0;
                }
            }
        }

        pInbuff = (centernet_pp_tmp_outBuffer_t *)pInput;
        for (i = 0; i <= pInput_static_param->nb_detect; i++, pInbuff++)
        {
            if (pInbuff->class_index != k) continue;
            if ((limit_counter < pInput_static_param->max_boxes_limit) &&
                (pInbuff->conf != 0))
            {
                limit_counter++;
                det_count++;
                pOutbuff->x_center    = (pInbuff->top_left_x     + pInbuff->bottom_right_x) / 2.0f;
                pOutbuff->y_center    = (pInbuff->top_left_y     + pInbuff->bottom_right_y) / 2.0f;
                pOutbuff->width       = (pInbuff->bottom_right_x - pInbuff->top_left_x);
                pOutbuff->height      = (pInbuff->bottom_right_y - pInbuff->top_left_y);
                pOutbuff->conf        = pInbuff->conf;
                pOutbuff->class_index = pInbuff->class_index;
                pOutbuff++;
            }
            else
            {
                pInbuff->conf = 0;
            }
        }
    }

    pOutput->nb_detect = det_count;
    return (error);
}


int32_t centernet_pp_getNNBoxes_centroid(od_centernet_pp_in_t *pInput,
                                         od_centernet_pp_static_param_t *pInput_static_param)
{
    int32_t error   = AI_VISION_MODELS_PP_ERROR_NO;

    int32_t count_detect                  = 0;
    float32_t score_center                = 0;
    float32_t grid_width_inv              = 1.0f / pInput_static_param->grid_width;
    float32_t grid_height_inv             = 1.0f / pInput_static_param->grid_height;
    int32_t conf_stride_right             = (pInput_static_param->nb_classifs + AI_CENTERNET_PP_CLASSPROB + AI_CENTERNET_PP_MAPSEG_NEXTOFFSET);
    int32_t conf_stride_bottom            = conf_stride_right * pInput_static_param->grid_width;
    centernet_pp_tmp_outBuffer_t *pOutput = (centernet_pp_tmp_outBuffer_t *)(pInput->pRaw_detections);

    float32_t *pConf_11     = (float32_t *)pInput->pRaw_detections + AI_CENTERNET_PP_CONFCENTER;
    float32_t *pConf_12     = (float32_t *)pConf_11     + conf_stride_right;
    float32_t *pConf_13     = (float32_t *)pConf_12     + conf_stride_right;
    float32_t *pConf_21     = (float32_t *)pConf_11     + conf_stride_bottom;
	  float32_t *pConf_center = (float32_t *)pConf_21     + conf_stride_right;
    float32_t *pConf_23     = (float32_t *)pConf_center + conf_stride_right;
    float32_t *pConf_31     = (float32_t *)pConf_21     + conf_stride_bottom;
    float32_t *pConf_32     = (float32_t *)pConf_31     + conf_stride_right;
    float32_t *pConf_33     = (float32_t *)pConf_32     + conf_stride_right;

	/* Maps centers first */
	if (pInput_static_param->optim == AI_OD_CENTERNET_PP_OPTIM_ACCURACY)
	{
    /* Applies an inplace bilinear upsampling with ratio of 2 after virtual downsampling by 2 */
    for (int32_t col = 0; col < pInput_static_param->grid_height/2 - 1; ++col)
	  {
      for (int32_t row = 0; row < pInput_static_param->grid_width/2 - 1; ++row)
		  {
        *pConf_12 = *pConf_11 * 0.5f + *pConf_13 * 0.5f;
        *pConf_21 = *pConf_11 * 0.5f + *pConf_31 * 0.5f;
        *pConf_center = *pConf_11 * 0.25f + *pConf_13 * 0.25f + *pConf_31 * 0.25f + *pConf_33 * 0.25f;
        pConf_11 += conf_stride_right * 2;
        pConf_12 += conf_stride_right * 2;
        pConf_13 += conf_stride_right * 2;
        pConf_21 += conf_stride_right * 2;
        pConf_center += conf_stride_right * 2;
        pConf_31 += conf_stride_right * 2;
        pConf_33 += conf_stride_right * 2;
			}
			pConf_11 += conf_stride_bottom;
			pConf_12 += conf_stride_bottom;
			pConf_13 += conf_stride_bottom;
			pConf_21 += conf_stride_bottom;
			pConf_center += conf_stride_bottom;
			pConf_31 += conf_stride_bottom;
			pConf_33 += conf_stride_bottom;
		}
	}

    /* Searches center detection everywhere but on the external border */
    pConf_11 = (float32_t *)pInput->pRaw_detections + AI_CENTERNET_PP_CONFCENTER;
    pConf_12 = (float32_t *)pConf_11 + conf_stride_right;
    pConf_13 = (float32_t *)pConf_12 + conf_stride_right;
    pConf_21 = (float32_t *)pConf_11 + conf_stride_bottom;
	  pConf_center = (float32_t *)pConf_21 + conf_stride_right;
    pConf_23 = (float32_t *)pConf_center + conf_stride_right;
    pConf_31 = (float32_t *)pConf_21 + conf_stride_bottom;
    pConf_32 = (float32_t *)pConf_31 + conf_stride_right;
    pConf_33 = (float32_t *)pConf_32 + conf_stride_right;
    for (int32_t col = 1; col < pInput_static_param->grid_height - 1; ++col)
    {
        for (int32_t row = 1; row < pInput_static_param->grid_width - 1; ++row)
        {
            /* Get Peaks */
            score_center = *pConf_center;
            if ((score_center >= *pConf_11) &&
                (score_center >= *pConf_12) &&
                (score_center >= *pConf_13) &&
                (score_center >= *pConf_21) &&
                (score_center >= *pConf_23) &&
                (score_center >= *pConf_31) &&
                (score_center >= *pConf_32) &&
                (score_center >= *pConf_33) &&
                (score_center > pInput_static_param->conf_threshold))
            {
                /* A detection center is kept since higher than its 8 neighbors and the threshold */
                float32_t x_offset = pConf_center[AI_CENTERNET_PP_XOFFSET] * grid_width_inv;
                float32_t y_offset = pConf_center[AI_CENTERNET_PP_YOFFSET] * grid_height_inv;
                float32_t b_x = row * grid_width_inv + x_offset;
                float32_t b_y = col * grid_height_inv + y_offset;
                float32_t b_w = pConf_center[AI_CENTERNET_PP_WIDTH] * grid_width_inv;
                float32_t b_h = pConf_center[AI_CENTERNET_PP_HEIGHT] * grid_height_inv;
                float32_t x1 = b_x - b_w / 2.0f;
                float32_t y1 = b_y - b_h / 2.0f;
                float32_t x2 = b_x + b_w / 2.0f;
                float32_t y2 = b_y + b_h / 2.0f;
                if ((y2 > y1) &&
                    (x2 > x1))
                {
                    count_detect++;
                    pOutput->top_left_x = x1;
                    pOutput->top_left_y = y1;
                    pOutput->bottom_right_x = x2;
                    pOutput->bottom_right_y = y2;
                    pOutput->conf = score_center;
                    float32_t max_classifs = pConf_center[AI_CENTERNET_PP_CLASSPROB];
                    int32_t class_idx = 0;
                    for (int i = 1; i < pInput_static_param->nb_classifs; i++)
                    {
                        if (pConf_center[AI_CENTERNET_PP_CLASSPROB + i] > max_classifs)
                        {
                            max_classifs = pConf_center[AI_CENTERNET_PP_CLASSPROB + i];
                            class_idx = i;
                        }
                    }
                    pOutput->class_index = class_idx;
                    pOutput++;
                }
            }
            pConf_11 += conf_stride_right;
            pConf_12 += conf_stride_right;
            pConf_13 += conf_stride_right;
            pConf_21 += conf_stride_right;
            pConf_center += conf_stride_right;
            pConf_23 += conf_stride_right;
            pConf_31 += conf_stride_right;
            pConf_32 += conf_stride_right;
            pConf_33 += conf_stride_right;
        }

        /* Shifts twice to bypass feature map border */
        pConf_11 += (conf_stride_right * 2);
        pConf_12 += (conf_stride_right * 2);
        pConf_13 += (conf_stride_right * 2);
        pConf_21 += (conf_stride_right * 2);
        pConf_center += (conf_stride_right * 2);
        pConf_23 += (conf_stride_right * 2);
        pConf_31 += (conf_stride_right * 2);
        pConf_32 += (conf_stride_right * 2);
        pConf_33 += (conf_stride_right * 2);
    }

    pInput_static_param->nb_detect = count_detect;

    return (error);
}


int32_t centernet_pp_getNNBoxes_centroid_int8(od_centernet_pp_in_t *pInput,
                                         od_centernet_pp_static_param_t *pInput_static_param)
{
  int32_t error   = AI_VISION_MODELS_PP_ERROR_NO;

  int32_t count_detect                  = 0;
  int8_t score_center                = 0;
  float32_t grid_width_inv              = 1.0f / pInput_static_param->grid_width;
  float32_t grid_height_inv             = 1.0f / pInput_static_param->grid_height;
  int32_t conf_stride_right             = (pInput_static_param->nb_classifs + AI_CENTERNET_PP_CLASSPROB + AI_CENTERNET_PP_MAPSEG_NEXTOFFSET);
  int32_t conf_stride_bottom            = conf_stride_right * pInput_static_param->grid_width;
  centernet_pp_tmp_outBuffer_t *pOutput = (centernet_pp_tmp_outBuffer_t *)(pInput->pRaw_detections);

  int8_t *pConf_11     = (int8_t *)pInput->pRaw_detections + AI_CENTERNET_PP_CONFCENTER;
  int8_t *pConf_12     = (int8_t *)pConf_11     + conf_stride_right;
  int8_t *pConf_13     = (int8_t *)pConf_12     + conf_stride_right;
  int8_t *pConf_21     = (int8_t *)pConf_11     + conf_stride_bottom;
	int8_t *pConf_center = (int8_t *)pConf_21     + conf_stride_right;
  int8_t *pConf_23     = (int8_t *)pConf_center + conf_stride_right;
  int8_t *pConf_31     = (int8_t *)pConf_21     + conf_stride_bottom;
  int8_t *pConf_32     = (int8_t *)pConf_31     + conf_stride_right;
  int8_t *pConf_33     = (int8_t *)pConf_32     + conf_stride_right;
  float32_t raw_scale = pInput_static_param->raw_scale;
  int8_t raw_zp = pInput_static_param->raw_zp;

	/* Maps centers first */
	if (pInput_static_param->optim == AI_OD_CENTERNET_PP_OPTIM_ACCURACY)
	{
    /* Applies an inplace bilinear upsampling with ratio of 2 after virtual downsampling by 2 */
    for (int32_t col = 0; col < pInput_static_param->grid_height/2 - 1; ++col)
	  {
      for (int32_t row = 0; row < pInput_static_param->grid_width/2 - 1; ++row)
		  {
        *pConf_12 = (*pConf_11 + *pConf_13 + 1 ) / 2;
        *pConf_21 = (*pConf_11 + *pConf_31 + 1 ) / 2;
        *pConf_center = (*pConf_11 + *pConf_13 + *pConf_31 + *pConf_33 + 2 ) / 4;
        pConf_11 += conf_stride_right * 2;
        pConf_12 += conf_stride_right * 2;
        pConf_13 += conf_stride_right * 2;
        pConf_21 += conf_stride_right * 2;
        pConf_center += conf_stride_right * 2;
        pConf_31 += conf_stride_right * 2;
        pConf_33 += conf_stride_right * 2;
			}
			pConf_11 += conf_stride_bottom;
			pConf_12 += conf_stride_bottom;
			pConf_13 += conf_stride_bottom;
			pConf_21 += conf_stride_bottom;
			pConf_center += conf_stride_bottom;
			pConf_31 += conf_stride_bottom;
			pConf_33 += conf_stride_bottom;
		}
	}

  /* Searches center detection everywhere but on the external border */
  pConf_11     = (int8_t *)pInput->pRaw_detections + AI_CENTERNET_PP_CONFCENTER;
  pConf_12     = (int8_t *)pConf_11     + conf_stride_right;
  pConf_13     = (int8_t *)pConf_12     + conf_stride_right;
  pConf_21     = (int8_t *)pConf_11     + conf_stride_bottom;
	pConf_center = (int8_t *)pConf_21     + conf_stride_right;
  pConf_23     = (int8_t *)pConf_center + conf_stride_right;
  pConf_31     = (int8_t *)pConf_21     + conf_stride_bottom;
  pConf_32     = (int8_t *)pConf_31     + conf_stride_right;
  pConf_33     = (int8_t *)pConf_32     + conf_stride_right;
  const int8_t conf_threshold_s8 = (int8_t)(pInput_static_param->conf_threshold / raw_scale + 0.5f) + raw_zp;

  for (int32_t col = 1; col < pInput_static_param->grid_height - 1; ++col)
  {
      for (int32_t row = 1; row < pInput_static_param->grid_width - 1; ++row)
      {
          /* Get Peaks */
          score_center = *pConf_center;
          if ((score_center >= *pConf_11) &&
              (score_center >= *pConf_12) &&
              (score_center >= *pConf_13) &&
              (score_center >= *pConf_21) &&
              (score_center >= *pConf_23) &&
              (score_center >= *pConf_31) &&
              (score_center >= *pConf_32) &&
              (score_center >= *pConf_33) &&
              (score_center > conf_threshold_s8 /*pInput_static_param->conf_threshold*/))
          {
              /* A detection center is kept since higher than its 8 neighbors and the threshold */
              float32_t x_offset = (float32_t)((int32_t)pConf_center[AI_CENTERNET_PP_XOFFSET] - raw_zp ) * raw_scale * grid_width_inv;
              float32_t y_offset = (float32_t)((int32_t)pConf_center[AI_CENTERNET_PP_YOFFSET] - raw_zp ) * raw_scale * grid_height_inv;
              float32_t b_x = row * grid_width_inv + x_offset;
              float32_t b_y = col * grid_height_inv + y_offset;
              float32_t b_w = (float32_t)((int32_t)pConf_center[AI_CENTERNET_PP_WIDTH] - raw_zp ) * raw_scale * grid_width_inv;
              float32_t b_h = (float32_t)((int32_t)pConf_center[AI_CENTERNET_PP_HEIGHT] - raw_zp ) * raw_scale * grid_height_inv;
              float32_t x1 = b_x - b_w / 2.0f;
              float32_t y1 = b_y - b_h / 2.0f;
              float32_t x2 = b_x + b_w / 2.0f;
              float32_t y2 = b_y + b_h / 2.0f;
              if ((y2 > y1) &&
                  (x2 > x1))
              {
                  count_detect++;
                  pOutput->top_left_x = x1;
                  pOutput->top_left_y = y1;
                  pOutput->bottom_right_x = x2;
                  pOutput->bottom_right_y = y2;
                  pOutput->conf = (float32_t)((int32_t)score_center - raw_zp ) * raw_scale;
                  /*
                  float32_t max_classifs = (float32_t)((int32_t)pConf_center[AI_CENTERNET_PP_CLASSPROB] - raw_zp ) * raw_scale;
                  int32_t class_idx = 0;
                  for (int i = 1; i < pInput_static_param->nb_classifs; i++)
                  {
                      if (pConf_center[AI_CENTERNET_PP_CLASSPROB + i] > max_classifs)
                      {
                          max_classifs = pConf_center[AI_CENTERNET_PP_CLASSPROB + i];
                          class_idx = i;
                      }
                  }*/
                  int8_t max_classifs;
                  uint8_t class_idx;
                  vision_models_maxi_p_is8ou8(&pConf_center[AI_CENTERNET_PP_CLASSPROB],
                                              pInput_static_param->nb_classifs,
                                              AI_CENTERNET_PP_CLASSPROB + pInput_static_param->nb_classifs,
                                              &max_classifs,
                                              &class_idx,
                                              1);
                  pOutput->class_index = class_idx;
                  pOutput++;
              }
          }
          pConf_11 += conf_stride_right;
          pConf_12 += conf_stride_right;
          pConf_13 += conf_stride_right;
          pConf_21 += conf_stride_right;
          pConf_center += conf_stride_right;
          pConf_23 += conf_stride_right;
          pConf_31 += conf_stride_right;
          pConf_32 += conf_stride_right;
          pConf_33 += conf_stride_right;
      }

      /* Shifts twice to bypass feature map border */
      pConf_11 += (conf_stride_right * 2);
      pConf_12 += (conf_stride_right * 2);
      pConf_13 += (conf_stride_right * 2);
      pConf_21 += (conf_stride_right * 2);
      pConf_center += (conf_stride_right * 2);
      pConf_23 += (conf_stride_right * 2);
      pConf_31 += (conf_stride_right * 2);
      pConf_32 += (conf_stride_right * 2);
      pConf_33 += (conf_stride_right * 2);
  }

  pInput_static_param->nb_detect = count_detect;

  return (error);
}



/* ----------------------       Exported routines      ---------------------- */

int32_t od_centernet_pp_reset(od_centernet_pp_static_param_t *pInput_static_param)
{
    /* Initializations */
    pInput_static_param->nb_detect = 0;

    return (AI_VISION_MODELS_PP_ERROR_NO);
}


int32_t od_centernet_pp_process(od_centernet_pp_in_t *pInput,
                                od_pp_out_t *pOutput,
                                od_centernet_pp_static_param_t *pInput_static_param)
{
    int32_t error   = AI_VISION_MODELS_PP_ERROR_NO;

    /* Call Get NN boxes first */
    error = centernet_pp_getNNBoxes_centroid(pInput,
                                             pInput_static_param);
    if (error != AI_VISION_MODELS_PP_ERROR_NO) return (error);

    /* Then NMS */
    error = centernet_pp_nmsFiltering_centroid((centernet_pp_tmp_outBuffer_t *)(pInput->pRaw_detections),
                                               pOutput,
                                               pInput_static_param);
    if (error != AI_VISION_MODELS_PP_ERROR_NO) return (error);

    return (error);
}

int32_t od_centernet_pp_process_int8(od_centernet_pp_in_t *pInput,
                                od_pp_out_t *pOutput,
                                od_centernet_pp_static_param_t *pInput_static_param)
{
    int32_t error   = AI_VISION_MODELS_PP_ERROR_NO;

    /* Call Get NN boxes first */
    error = centernet_pp_getNNBoxes_centroid_int8(pInput,
                                             pInput_static_param);
    if (error != AI_VISION_MODELS_PP_ERROR_NO) return (error);

    /* Then NMS */
    error = centernet_pp_nmsFiltering_centroid((centernet_pp_tmp_outBuffer_t *)(pInput->pRaw_detections),
                                               pOutput,
                                               pInput_static_param);
    if (error != AI_VISION_MODELS_PP_ERROR_NO) return (error);

    return (error);
}

