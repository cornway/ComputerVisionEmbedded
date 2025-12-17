/*---------------------------------------------------------------------------------------------
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 *in the root directory of this software component. If no LICENSE file comes
 *with this software, it is provided AS-IS.
 *--------------------------------------------------------------------------------------------*/

#include "od_pp_loc.h"
#include "od_yolov2_pp_if.h"
#include "vision_models_pp.h"

/* Can't be removed if qsort is not re-written... */
static int32_t AI_YOLOV2_PP_SORT_CLASS;

int32_t yolov2_nms_comparator(const void *pa, const void *pb) {
  od_pp_outBuffer_t *pa_s = (od_pp_outBuffer_t *)pa;
  od_pp_outBuffer_t *pb_s = (od_pp_outBuffer_t *)pb;
  float32_t a = (pa_s->class_index == AI_YOLOV2_PP_SORT_CLASS) ? pa_s->conf : 0;
  float32_t b = (pb_s->class_index == AI_YOLOV2_PP_SORT_CLASS) ? pb_s->conf : 0;
  float32_t diff = 0;

  diff = a - b;

  if (diff < 0)
    return 1;
  else if (diff > 0)
    return -1;
  return 0;
}

int32_t yolov2_pp_nmsFiltering_centroid(
    od_pp_outBuffer_t *pScratchBuffer,
    od_yolov2_pp_static_param_t *pInput_static_param) {
  int32_t i, j, k, limit_counter;

  for (k = 0; k < pInput_static_param->nb_classes; ++k) {
    limit_counter = 0;
    AI_YOLOV2_PP_SORT_CLASS = k;

    qsort(pScratchBuffer, pInput_static_param->nb_detect,
          sizeof(od_pp_outBuffer_t), (_Cmpfun *)yolov2_nms_comparator);

    for (i = 0; i < (pInput_static_param->nb_detect); i++) {
      if (pScratchBuffer[i].conf == 0)
        continue;
      float32_t *a = &(pScratchBuffer[i].x_center);
      for (j = i + 1; j < (pInput_static_param->nb_detect); j++) {
        if (pScratchBuffer[j].conf == 0)
          continue;
        float32_t *b = &(pScratchBuffer[j].x_center);
        if (vision_models_box_iou(a, b) > pInput_static_param->iou_threshold) {
          pScratchBuffer[j].conf = 0;
        }
      }
    }

    for (int32_t y = 0; y <= pInput_static_param->nb_detect; y++) {
      if ((limit_counter < pInput_static_param->max_boxes_limit) &&
          (pScratchBuffer[y].conf != 0)) {
        limit_counter++;
      } else {
        pScratchBuffer[y].conf = 0;
      }
    }
  }

  return (AI_OD_POSTPROCESS_ERROR_NO);
}

int32_t yolov2_pp_scoreFiltering_centroid(
    od_pp_outBuffer_t *pScratchBuff, od_pp_out_t *pOutput,
    od_yolov2_pp_static_param_t *pInput_static_param) {
  int32_t det_count = 0;

  for (int32_t i = 0; i < (pInput_static_param->nb_detect); i++) {
    if (pScratchBuff[i].conf != 0) {
      memcpy(&pOutput->pOutBuff[det_count], &pScratchBuff[i],
             sizeof(od_pp_outBuffer_t));
      det_count++;
    }
  }
  pOutput->nb_detect = det_count;

  return (AI_OD_POSTPROCESS_ERROR_NO);
}

int32_t yolov2_pp_getNNBoxes_centroid(
    od_yolov2_pp_in_t *pInput, od_pp_outBuffer_t *pOutbuff,
    od_yolov2_pp_static_param_t *pInput_static_param) {
  int32_t error = AI_OD_POSTPROCESS_ERROR_NO;
  int32_t count_detect = 0;
  float32_t best_score = 0;
  uint32_t class_index;
  int32_t anch_stride =
      pInput_static_param->nb_classes + AI_YOLOV2_PP_CLASSPROB;

  float32_t grid_width_inv = 1.0f / pInput_static_param->grid_width;
  float32_t grid_height_inv = 1.0f / pInput_static_param->grid_height;
  int32_t el_offset = 0;
  float32_t *pInbuff = (float32_t *)pInput->pRaw_detections;

  if (1 == pInput_static_param->nb_classes) {
    float32_t computedThreshold =
        -logf(1 / pInput_static_param->conf_threshold - 1);
    for (int32_t row = 0; row < pInput_static_param->grid_width; ++row) {
      for (int32_t col = 0; col < pInput_static_param->grid_height; ++col) {
        for (int32_t anch = 0; anch < pInput_static_param->nb_anchors; ++anch) {

          if (pInbuff[el_offset + AI_YOLOV2_PP_OBJECTNESS] >=
              computedThreshold) {
            float32_t prob = vision_models_sigmoid_f(
                pInbuff[el_offset + AI_YOLOV2_PP_OBJECTNESS]);
            pOutbuff[count_detect].conf = prob;
            pOutbuff[count_detect].class_index = 0;

            pOutbuff[count_detect].x_center =
                (col + vision_models_sigmoid_f(
                           pInbuff[el_offset + AI_YOLOV2_PP_XCENTER])) *
                grid_width_inv;
            pOutbuff[count_detect].y_center =
                (row + vision_models_sigmoid_f(
                           pInbuff[el_offset + AI_YOLOV2_PP_YCENTER])) *
                grid_height_inv;
            pOutbuff[count_detect].width =
                (pInput_static_param->pAnchors[2 * anch] *
                 expf(pInbuff[el_offset + AI_YOLOV2_PP_WIDTHREL])) *
                grid_width_inv;
            pOutbuff[count_detect].height =
                (pInput_static_param->pAnchors[2 * anch + 1] *
                 expf(pInbuff[el_offset + AI_YOLOV2_PP_HEIGHTREL])) *
                grid_height_inv;

            count_detect++;
          }

          el_offset += anch_stride;
        }
      }
    }
  } else {
    for (int32_t row = 0; row < pInput_static_param->grid_width; ++row) {
      for (int32_t col = 0; col < pInput_static_param->grid_height; ++col) {
        for (int32_t anch = 0; anch < pInput_static_param->nb_anchors; ++anch) {
          vision_models_maxi_p_if32ou32(
              &pInbuff[el_offset + AI_YOLOV2_PP_CLASSPROB],
              pInput_static_param->nb_classes, anch_stride, &best_score,
              &class_index, 1);
          /* read and activate objectness */
          float32_t prob = vision_models_sigmoid_f(
              pInbuff[el_offset + AI_YOLOV2_PP_OBJECTNESS]);

          /* activate array of classes pred */
          float32_t sumf = 0.0;
          for (int _i = 0; _i < pInput_static_param->nb_classes; _i++) {
            sumf += expf(pInbuff[el_offset + AI_YOLOV2_PP_CLASSPROB + _i]);
          }
          best_score = expf(best_score) / sumf;
          best_score *= prob;

          if (best_score >= pInput_static_param->conf_threshold) {

            pOutbuff[count_detect].x_center =
                (col + vision_models_sigmoid_f(
                           pInbuff[el_offset + AI_YOLOV2_PP_XCENTER])) *
                grid_width_inv;
            pOutbuff[count_detect].y_center =
                (row + vision_models_sigmoid_f(
                           pInbuff[el_offset + AI_YOLOV2_PP_YCENTER])) *
                grid_height_inv;
            pOutbuff[count_detect].width =
                (pInput_static_param->pAnchors[2 * anch + 0] *
                 expf(pInbuff[el_offset + AI_YOLOV2_PP_WIDTHREL])) *
                grid_width_inv;
            pOutbuff[count_detect].height =
                (pInput_static_param->pAnchors[2 * anch + 1] *
                 expf(pInbuff[el_offset + AI_YOLOV2_PP_HEIGHTREL])) *
                grid_height_inv;
            pOutbuff[count_detect].conf = best_score;
            pOutbuff[count_detect].class_index = class_index;

            count_detect++;
          }

          el_offset += anch_stride;
        }
      }
    }
  }
  pInput_static_param->nb_detect = count_detect;
  return (error);
}

int32_t yolov2_pp_getNNBoxes_centroid_int8(
    od_yolov2_pp_in_t *pInput, od_pp_outBuffer_t *pOutBuff,
    od_yolov2_pp_static_param_t *pInput_static_param) {
  int32_t error = AI_OD_POSTPROCESS_ERROR_NO;
  int32_t count_detect = 0;
  int32_t anch_stride =
      pInput_static_param->nb_classes + AI_YOLOV2_PP_CLASSPROB;

  float32_t grid_width_inv = 1.0f / pInput_static_param->grid_width;
  float32_t grid_height_inv = 1.0f / pInput_static_param->grid_height;

  int8_t raw_zp = pInput_static_param->raw_zero_point;
  float32_t raw_scale = pInput_static_param->raw_scale;

  int32_t el_offset = 0;
  int8_t *pInbuff = (int8_t *)pInput->pRaw_detections;
  if (1 == pInput_static_param->nb_classes) {
    float32_t computedThreshold =
        -logf(1 / pInput_static_param->conf_threshold - 1);
    int8_t threshold_s8 =
        (int8_t)(computedThreshold / raw_scale + 0.5 + raw_zp);
    for (int32_t row = 0; row < pInput_static_param->grid_width; ++row) {
      for (int32_t col = 0; col < pInput_static_param->grid_height; ++col) {
        for (int32_t anch = 0; anch < pInput_static_param->nb_anchors; ++anch) {

          if (pInbuff[el_offset + AI_YOLOV2_PP_OBJECTNESS] >= threshold_s8) {
            /* read and activate objectness */
            float32_t dequant;
            float32_t anchor;
            dequant =
                (float32_t)((int32_t)
                                pInbuff[el_offset + AI_YOLOV2_PP_OBJECTNESS] -
                            raw_zp) *
                raw_scale;
            float32_t prob = vision_models_sigmoid_f(dequant);
            pOutBuff[count_detect].conf = prob;
            pOutBuff[count_detect].class_index = 0;

            dequant =
                (float32_t)((int32_t)pInbuff[el_offset + AI_YOLOV2_PP_XCENTER] -
                            raw_zp) *
                raw_scale;
            pOutBuff[count_detect].x_center =
                (col + vision_models_sigmoid_f(dequant)) * grid_width_inv;

            dequant =
                (float32_t)((int32_t)pInbuff[el_offset + AI_YOLOV2_PP_YCENTER] -
                            raw_zp) *
                raw_scale;
            pOutBuff[count_detect].y_center =
                (row + vision_models_sigmoid_f(dequant)) * grid_height_inv;

            dequant =
                (float32_t)((int32_t)
                                pInbuff[el_offset + AI_YOLOV2_PP_WIDTHREL] -
                            raw_zp) *
                raw_scale;
            anchor = (float32_t)pInput_static_param->pAnchors[2 * anch + 0];
            pOutBuff[count_detect].width =
                (anchor * expf(dequant)) * grid_width_inv;

            dequant =
                (float32_t)((int32_t)
                                pInbuff[el_offset + AI_YOLOV2_PP_HEIGHTREL] -
                            raw_zp) *
                raw_scale;
            anchor = (float32_t)pInput_static_param->pAnchors[2 * anch + 1];
            pOutBuff[count_detect].height =
                (anchor * expf(dequant)) * grid_height_inv;

            count_detect++;
          }
          el_offset += anch_stride;
        }
      }
    }
  } else {
    int8_t best_score_s8 = 0;
    uint8_t class_index_u8;

    for (int32_t row = 0; row < pInput_static_param->grid_width; ++row) {
      for (int32_t col = 0; col < pInput_static_param->grid_height; ++col) {
        for (int32_t anch = 0; anch < pInput_static_param->nb_anchors; ++anch) {
          vision_models_maxi_p_is8ou8(
              &pInbuff[el_offset + AI_YOLOV2_PP_CLASSPROB],
              pInput_static_param->nb_classes, anch, &best_score_s8,
              &class_index_u8, 1);
          /* read and activate objectness */
          float32_t dequant;
          float32_t best_score;

          dequant =
              (float32_t)((int32_t)
                              pInbuff[el_offset + AI_YOLOV2_PP_OBJECTNESS] -
                          raw_zp) *
              raw_scale;
          float32_t prob = vision_models_sigmoid_f(dequant);

          /* activate array of classes pred */
          /* in placce softmax */
          float32_t sumf = 0.0;
          for (int _i = 0; _i < pInput_static_param->nb_classes; _i++) {
            dequant =
                (float32_t)((int32_t)pInbuff[el_offset +
                                             AI_YOLOV2_PP_CLASSPROB + _i] -
                            raw_zp) *
                raw_scale;
            sumf += expf(dequant);
          }
          dequant = (float32_t)((int32_t)best_score_s8 - raw_zp) * raw_scale;
          best_score = expf(dequant) / sumf;
          best_score *= prob;

          if (best_score >= pInput_static_param->conf_threshold) {
            float32_t anchor;

            dequant =
                (float32_t)((int32_t)pInbuff[el_offset + AI_YOLOV2_PP_XCENTER] -
                            raw_zp) *
                raw_scale;
            pOutBuff[count_detect].x_center =
                (col + vision_models_sigmoid_f(dequant)) * grid_width_inv;

            dequant =
                (float32_t)((int32_t)pInbuff[el_offset + AI_YOLOV2_PP_YCENTER] -
                            raw_zp) *
                raw_scale;
            pOutBuff[count_detect].y_center =
                (row + vision_models_sigmoid_f(dequant)) * grid_height_inv;

            dequant =
                (float32_t)((int32_t)
                                pInbuff[el_offset + AI_YOLOV2_PP_WIDTHREL] -
                            raw_zp) *
                raw_scale;
            anchor = (float32_t)pInput_static_param->pAnchors[2 * anch + 0];
            pOutBuff[count_detect].width =
                (anchor * expf(dequant)) * grid_width_inv;

            dequant =
                (float32_t)((int32_t)
                                pInbuff[el_offset + AI_YOLOV2_PP_HEIGHTREL] -
                            raw_zp) *
                raw_scale;
            anchor = (float32_t)pInput_static_param->pAnchors[2 * anch + 1];
            pOutBuff[count_detect].height =
                (anchor * expf(dequant)) * grid_height_inv;

            pOutBuff[count_detect].conf = best_score;
            pOutBuff[count_detect].class_index = class_index_u8;

            count_detect++;
          }

          el_offset += anch_stride;
        }
      }
    }
  }
  pInput_static_param->nb_detect = count_detect;
  return (error);
}

/* ----------------------       Exported routines      ---------------------- */

int32_t od_yolov2_pp_reset(od_yolov2_pp_static_param_t *pInput_static_param) {
  /* Initializations */
  pInput_static_param->nb_detect = 0;

  return (AI_OD_POSTPROCESS_ERROR_NO);
}

int32_t od_yolov2_pp_process(od_yolov2_pp_in_t *pInput, od_pp_out_t *pOutput,
                             od_yolov2_pp_static_param_t *pInput_static_param) {
  int32_t error = AI_OD_POSTPROCESS_ERROR_NO;
  od_pp_outBuffer_t *pScratchBuff = pInput_static_param->pScratchBuffer;

  if (NULL == pScratchBuff) {
    if (((AI_YOLOV2_PP_CLASSPROB + pInput_static_param->nb_classes) *
         sizeof(float32_t)) >= sizeof(od_pp_outBuffer_t)) {
      pScratchBuff = pInput->pRaw_detections;
    }
    if (NULL == pScratchBuff) {
      return AI_OD_POSTPROCESS_ERROR;
    }
  }

  /* Call Get NN boxes first */
  error =
      yolov2_pp_getNNBoxes_centroid(pInput, pScratchBuff, pInput_static_param);
  if (error != AI_OD_POSTPROCESS_ERROR_NO)
    return (error);

  /* Then NMS */
  error = yolov2_pp_nmsFiltering_centroid(pScratchBuff, pInput_static_param);
  if (error != AI_OD_POSTPROCESS_ERROR_NO)
    return (error);

  /* And score re-filtering */
  error = yolov2_pp_scoreFiltering_centroid(pScratchBuff, pOutput,
                                            pInput_static_param);

  return (error);
}

int32_t
od_yolov2_pp_process_int8(od_yolov2_pp_in_t *pInput, od_pp_out_t *pOutput,
                          od_yolov2_pp_static_param_t *pInput_static_param) {
  int32_t error = AI_OD_POSTPROCESS_ERROR_NO;
  od_pp_outBuffer_t *pScratchBuff = pInput_static_param->pScratchBuffer;

  if (NULL == pScratchBuff) {
    if (((AI_YOLOV2_PP_CLASSPROB + pInput_static_param->nb_classes) *
         sizeof(int8_t)) > sizeof(od_pp_outBuffer_t)) {
      pScratchBuff = pInput->pRaw_detections;
    }
    if (NULL == pScratchBuff) {
      return AI_OD_POSTPROCESS_ERROR;
    }
  }

  /* Call Get NN boxes first */
  error = yolov2_pp_getNNBoxes_centroid_int8(pInput, pScratchBuff,
                                             pInput_static_param);
  if (error != AI_OD_POSTPROCESS_ERROR_NO)
    return (error);

  /* Then NMS */
  error = yolov2_pp_nmsFiltering_centroid(pScratchBuff, pInput_static_param);
  if (error != AI_OD_POSTPROCESS_ERROR_NO)
    return (error);

  /* And score re-filtering */
  error = yolov2_pp_scoreFiltering_centroid(pScratchBuff, pOutput,
                                            pInput_static_param);

  return (error);
}
