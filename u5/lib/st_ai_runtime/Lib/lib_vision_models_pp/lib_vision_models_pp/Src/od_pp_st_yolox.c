/*---------------------------------------------------------------------------------------------
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file in
 * the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *--------------------------------------------------------------------------------------------*/

#include "od_pp_loc.h"
#include "od_st_yolox_pp_if.h"
#include "vision_models_pp.h"


/* Can't be removed if qsort is not re-written... */
static int32_t AI_ST_YOLOX_PP_SORT_CLASS;


int32_t st_yolox_nms_comparator(const void *pa, const void *pb)
{
    od_pp_outBuffer_t a = *(od_pp_outBuffer_t *)pa;
    od_pp_outBuffer_t b = *(od_pp_outBuffer_t *)pb;

    float32_t diff = 0.0;
    float32_t a_weighted_conf = 0.0;
    float32_t b_weighted_conf = 0.0;

    if (a.class_index == AI_ST_YOLOX_PP_SORT_CLASS)
    {
        a_weighted_conf = a.conf;
    }
    else
    {
         a_weighted_conf = 0.0;
    }

    if (b.class_index == AI_ST_YOLOX_PP_SORT_CLASS)
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

int32_t st_yolox_pp_nmsFiltering_centroid(od_pp_out_t *pOutput,
                                          od_st_yolox_pp_static_param_t *pInput_static_param)
{
    int32_t j, k, limit_counter, detections_per_class;
    for (k = 0; k < pInput_static_param->nb_classes; ++k)
    {
        limit_counter = 0;
        detections_per_class = 0;
        AI_ST_YOLOX_PP_SORT_CLASS = k;


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
                  st_yolox_nms_comparator);

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
    return (AI_OD_POSTPROCESS_ERROR_NO);
}


int32_t st_yolox_pp_scoreFiltering_centroid(od_pp_out_t *pOutput,
                                            od_st_yolox_pp_static_param_t *pInput_static_param)
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
            det_count++;
        }
    }
    pOutput->nb_detect = det_count;

    return (AI_OD_POSTPROCESS_ERROR_NO);
}


int32_t st_yolox_pp_level_decode_and_store(float32_t *pInbuff,
                                           od_pp_out_t *pOutput,
                                           float32_t *pAnchors,
                                           int32_t grid_width,
                                           int32_t grid_height,
                                           od_st_yolox_pp_static_param_t *pInput_static_param)

{
    int32_t el_offset    = 0;
    float32_t best_score = 0;
    uint32_t class_index;
    int32_t anch_stride = (pInput_static_param->nb_classes + AI_YOLOV2_PP_CLASSPROB);
    float32_t grid_width_inv = 1.0f / grid_width;
    float32_t grid_height_inv = 1.0f / grid_height;
    int32_t det_count = pInput_static_param->nb_detect;
    od_pp_outBuffer_t *pOutBuff = (od_pp_outBuffer_t *)pOutput->pOutBuff;

    if ( 1 == pInput_static_param->nb_classes) {
      float32_t computedThreshold = -logf( 1 / pInput_static_param->conf_threshold - 1);
      for (int32_t row = 0; row < grid_width; ++row)
      {
        for (int32_t col = 0; col < grid_height; ++col)
        {
          for (int32_t anch = 0; anch < pInput_static_param->nb_anchors; ++anch)
          {
            if ( pInbuff[el_offset + AI_YOLOV2_PP_OBJECTNESS] >= computedThreshold) {

              /* read and activate objectness */
              float32_t prob = vision_models_sigmoid_f(pInbuff[el_offset + AI_YOLOV2_PP_OBJECTNESS]);

              /* activate array of classes pred */

              pOutBuff[det_count].conf = prob;
              pOutBuff[det_count].class_index = 0;

              pOutBuff[det_count].x_center   = (col + vision_models_sigmoid_f(pInbuff[el_offset + AI_YOLOV2_PP_XCENTER]))   * grid_width_inv;
              pOutBuff[det_count].y_center   = (row + vision_models_sigmoid_f(pInbuff[el_offset + AI_YOLOV2_PP_YCENTER]))   * grid_height_inv;
              pOutBuff[det_count].width      = (pAnchors[2 * anch + 0] * expf(pInbuff[el_offset + AI_YOLOV2_PP_WIDTHREL]))  * grid_width_inv;
              pOutBuff[det_count].height     = (pAnchors[2 * anch + 1] * expf(pInbuff[el_offset + AI_YOLOV2_PP_HEIGHTREL])) * grid_height_inv;

              det_count++;
            }

             el_offset += anch_stride;
          } // for anchh
        } // for col
      } // for row
    } // if nb_classes == 1
    else
    {
      for (int32_t row = 0; row < grid_width; ++row)
      {
          for (int32_t col = 0; col < grid_height; ++col)
          {
              for (int32_t anch = 0; anch < pInput_static_param->nb_anchors; ++anch)
              {
                  vision_models_maxi_p_if32ou32(&pInbuff[el_offset + AI_YOLOV2_PP_CLASSPROB],
                                              pInput_static_param->nb_classes,
                                              anch_stride,
                                              &best_score,
                                              &class_index,
                                              1);
                  /* read and activate objectness */
                  float32_t prob = vision_models_sigmoid_f(pInbuff[el_offset + AI_YOLOV2_PP_OBJECTNESS]);

                  /* activate array of classes pred */
                  /* in placce softmax */
                  float32_t sumf = 0.0;
                  for (int _i = 0; _i < pInput_static_param->nb_classes; _i++) {
                      sumf+= expf(pInbuff[el_offset + AI_YOLOV2_PP_CLASSPROB + _i]);
                  }
                  best_score = expf(best_score) / sumf;
                  best_score *= prob;

                  if (best_score >= pInput_static_param->conf_threshold)
                  {

                      pOutBuff[det_count].x_center    = (col + vision_models_sigmoid_f(pInbuff[el_offset + AI_YOLOV2_PP_XCENTER]))   * grid_width_inv;
                      pOutBuff[det_count].y_center    = (row + vision_models_sigmoid_f(pInbuff[el_offset + AI_YOLOV2_PP_YCENTER]))   * grid_height_inv;
                      pOutBuff[det_count].width       = (pAnchors[2 * anch + 0] * expf(pInbuff[el_offset + AI_YOLOV2_PP_WIDTHREL]))  * grid_width_inv;
                      pOutBuff[det_count].height      = (pAnchors[2 * anch + 1] * expf(pInbuff[el_offset + AI_YOLOV2_PP_HEIGHTREL])) * grid_height_inv;
                      pOutBuff[det_count].conf        = best_score;
                      pOutBuff[det_count].class_index = class_index;

                      det_count++;
                  }

                  el_offset += anch_stride;
              } // for anch
          } // for col
      } // for row
    } //  else (nb_classes != 1)
    pInput_static_param->nb_detect = det_count;

    return det_count;

}

int32_t st_yolox_pp_level_decode_and_store_is8(int8_t *pInbuff,
                                               od_pp_out_t *pOutput,
                                               float32_t *pAnchors,
                                               int32_t grid_width,
                                               int32_t grid_height,
                                               od_st_yolox_pp_static_param_t *pInput_static_param,
                                               float32_t raw_scale,
                                               int8_t raw_zp)

{
  int32_t el_offset    = 0;
  int32_t anch_stride = (pInput_static_param->nb_classes + AI_YOLOV2_PP_CLASSPROB);
  float32_t grid_width_inv = 1.0f / grid_width;
  float32_t grid_height_inv = 1.0f / grid_height;


  int32_t det_count = pInput_static_param->nb_detect;
  od_pp_outBuffer_t *pOutBuff = (od_pp_outBuffer_t *)pOutput->pOutBuff;

  if ( 1 == pInput_static_param->nb_classes) {

    float32_t computedThreshold = -logf( 1 / pInput_static_param->conf_threshold - 1);
    int8_t threshold_s8  = (int8_t)(computedThreshold / raw_scale + 0.5 + raw_zp);

    for (int32_t row = 0; row < grid_width; ++row)
    {
      for (int32_t col = 0; col < grid_height; ++col)
      {
        for (int32_t anch = 0; anch < pInput_static_param->nb_anchors; ++anch)
        {
          if ( pInbuff[el_offset + AI_YOLOV2_PP_OBJECTNESS] >= threshold_s8) {

            /* read and activate objectness */
            float32_t dequant;
            float32_t anchor;
            dequant = (float32_t)((int32_t)pInbuff[el_offset + AI_YOLOV2_PP_OBJECTNESS] - raw_zp) * raw_scale;
            float32_t prob = vision_models_sigmoid_f(dequant);
            pOutBuff[det_count].conf = prob;
            pOutBuff[det_count].class_index = 0;

            dequant = (float32_t)((int32_t)pInbuff[el_offset + AI_YOLOV2_PP_XCENTER] - raw_zp) * raw_scale;
            pOutBuff[det_count].x_center   = (col + vision_models_sigmoid_f(dequant)) * grid_width_inv;

            dequant = (float32_t)((int32_t)pInbuff[el_offset + AI_YOLOV2_PP_YCENTER] - raw_zp) * raw_scale;
            pOutBuff[det_count].y_center   = (row + vision_models_sigmoid_f(dequant)) * grid_height_inv;

            dequant = (float32_t)((int32_t)pInbuff[el_offset + AI_YOLOV2_PP_WIDTHREL] - raw_zp) * raw_scale;
            anchor                          = (float32_t)(pAnchors[2 * anch + 0]);
            pOutBuff[det_count].width      = (anchor * expf(dequant)) * grid_width_inv;

            dequant = (float32_t)((int32_t)pInbuff[el_offset + AI_YOLOV2_PP_HEIGHTREL] - raw_zp) * raw_scale;
            anchor                          = (float32_t)(pAnchors[2 * anch + 1]);
            pOutBuff[det_count].height     = (anchor * expf(dequant)) * grid_height_inv;

            det_count++;
          }

           el_offset += anch_stride;
        } // for anchh
      } // for col
    } // for row
  } // if nb_classes == 1
  else
  {
    int8_t best_score_s8;
    uint8_t class_index_u8;
    float32_t best_score;


    for (int32_t row = 0; row < grid_width; ++row)
    {
      for (int32_t col = 0; col < grid_height; ++col)
      {
        for (int32_t anch = 0; anch < pInput_static_param->nb_anchors; ++anch)
        {
          vision_models_maxi_p_is8ou8(&pInbuff[el_offset + AI_YOLOV2_PP_CLASSPROB],
                                    pInput_static_param->nb_classes,
                                    anch,
                                    &best_score_s8,
                                    &class_index_u8,
                                    1);
          /* read and activate objectness */
          float32_t dequant;
          dequant = (float32_t)((int32_t)pInbuff[el_offset + AI_YOLOV2_PP_OBJECTNESS] - raw_zp) * raw_scale;
          float32_t prob = vision_models_sigmoid_f(dequant);

          /* activate array of classes pred */
          /* in placce softmax */
          float32_t sumf = 0.0;
          for (int _i = 0; _i < pInput_static_param->nb_classes; _i++) {
              dequant = (float32_t)((int32_t)pInbuff[el_offset + AI_YOLOV2_PP_CLASSPROB + _i] - raw_zp) * raw_scale;
              sumf+= expf(dequant);
          }
          dequant = (float32_t)((int32_t)best_score_s8 - raw_zp) * raw_scale;
          best_score = expf(dequant) / sumf;
          best_score *= prob;

          if (best_score >= pInput_static_param->conf_threshold)
          {
            float32_t anchor;

            dequant                         = (float32_t)((int32_t)pInbuff[el_offset + AI_YOLOV2_PP_XCENTER] - raw_zp) * raw_scale;
            pOutBuff[det_count].x_center    = (col + vision_models_sigmoid_f(dequant)) * grid_width_inv;

            dequant                         = (float32_t)((int32_t)pInbuff[el_offset + AI_YOLOV2_PP_YCENTER] - raw_zp) * raw_scale;
            pOutBuff[det_count].y_center    = (row + vision_models_sigmoid_f(dequant)) * grid_height_inv;

            dequant                         = (float32_t)((int32_t)pInbuff[el_offset + AI_YOLOV2_PP_WIDTHREL] - raw_zp) * raw_scale;
            anchor                          = (float32_t)pAnchors[2 * anch + 0];
            pOutBuff[det_count].width       = (anchor * expf(dequant)) * grid_width_inv;

            dequant                         = (float32_t)((int32_t)pInbuff[el_offset + AI_YOLOV2_PP_HEIGHTREL] - raw_zp) * raw_scale;
            anchor                          = (float32_t)pAnchors[2 * anch + 1];
            pOutBuff[det_count].height      = (anchor * expf(dequant)) * grid_height_inv;

            pOutBuff[det_count].conf        = best_score;
            pOutBuff[det_count].class_index = class_index_u8;

            det_count++;
          }

          el_offset += anch_stride;
        } // for anch
      } // for col
    } // for row
  } //  else (nb_classes != 1)
  pInput_static_param->nb_detect = det_count;

  return det_count;

}


int32_t st_yolox_pp_getNNBoxes_centroid(od_st_yolox_pp_in_t *pInput,
                                        od_pp_out_t *pOut,
                                        od_st_yolox_pp_static_param_t *pInput_static_param)
{

    int32_t error   = AI_OD_POSTPROCESS_ERROR_NO;



    int32_t grid_width, grid_height;
    float32_t *pInbuff, *pAnchors;

    if (pOut->pOutBuff == NULL)
    {
      pOut->pOutBuff = (od_pp_outBuffer_t *)pInput->pRaw_detections_L;
    }

    //==============================================================================================================================================================

    //level L
    grid_width = pInput_static_param->grid_width_L;
    grid_height = pInput_static_param->grid_height_L;
    pInbuff = (float32_t *)pInput->pRaw_detections_L;
    pAnchors = (float32_t *)pInput_static_param->pAnchors_L;
    st_yolox_pp_level_decode_and_store(pInbuff, pOut, pAnchors, grid_width, grid_height, pInput_static_param);

    //==============================================================================================================================================================

    //level M
    grid_width = pInput_static_param->grid_width_M;
    grid_height = pInput_static_param->grid_height_M;
    pInbuff = (float32_t *)pInput->pRaw_detections_M;
    pAnchors = (float32_t *)pInput_static_param->pAnchors_M;

    st_yolox_pp_level_decode_and_store(pInbuff, pOut, pAnchors, grid_width, grid_height, pInput_static_param);
    //level S
    grid_width = pInput_static_param->grid_width_S;
    grid_height = pInput_static_param->grid_height_S;
    pInbuff = (float32_t *)pInput->pRaw_detections_S;
    pAnchors = (float32_t *)pInput_static_param->pAnchors_S;
    st_yolox_pp_level_decode_and_store(pInbuff, pOut, pAnchors, grid_width, grid_height, pInput_static_param);

    return (error);
}

int32_t st_yolox_pp_getNNBoxes_centroid_is8(od_st_yolox_pp_in_t *pInput,
                                            od_pp_out_t *pOut,
                                            od_st_yolox_pp_static_param_t *pInput_static_param)
{

    int32_t error   = AI_OD_POSTPROCESS_ERROR_NO;



    int32_t grid_width, grid_height;
    int8_t *pInbuff;
    float32_t *pAnchors;

  //==============================================================================================================================================================

    //level L
    float32_t scale = pInput_static_param->raw_l_scale;
    int8_t zp = pInput_static_param->raw_l_zero_point;
    grid_width = pInput_static_param->grid_width_L;
    grid_height = pInput_static_param->grid_height_L;
    pInbuff = (int8_t *)pInput->pRaw_detections_L;
    pAnchors = (float32_t *)pInput_static_param->pAnchors_L;
    st_yolox_pp_level_decode_and_store_is8(pInbuff, pOut, pAnchors, grid_width, grid_height,pInput_static_param, scale, zp);

    //==============================================================================================================================================================

    //level M
    scale = pInput_static_param->raw_m_scale;
    zp = pInput_static_param->raw_m_zero_point;
    grid_width = pInput_static_param->grid_width_M;
    grid_height = pInput_static_param->grid_height_M;
    pInbuff = (int8_t *)pInput->pRaw_detections_M;
    pAnchors = (float32_t *)pInput_static_param->pAnchors_M;


    st_yolox_pp_level_decode_and_store_is8(pInbuff, pOut, pAnchors, grid_width, grid_height,pInput_static_param, scale, zp);

    //level S
    scale = pInput_static_param->raw_s_scale;
    zp = pInput_static_param->raw_s_zero_point;
    grid_width = pInput_static_param->grid_width_S;
    grid_height = pInput_static_param->grid_height_S;
    pInbuff = (int8_t *)pInput->pRaw_detections_S;
    pAnchors = (float32_t *)pInput_static_param->pAnchors_S;

    st_yolox_pp_level_decode_and_store_is8(pInbuff, pOut, pAnchors, grid_width, grid_height,pInput_static_param, scale, zp);

    return (error);
}



/* ----------------------       Exported routines      ---------------------- */

int32_t od_st_yolox_pp_reset(od_st_yolox_pp_static_param_t *pInput_static_param)
{
    /* Initializations */
    pInput_static_param->nb_detect = 0;

	return (AI_OD_POSTPROCESS_ERROR_NO);
}


int32_t od_st_yolox_pp_process(od_st_yolox_pp_in_t *pInput,
                               od_pp_out_t *pOutput,
                               od_st_yolox_pp_static_param_t *pInput_static_param)
{
    int32_t error   = AI_OD_POSTPROCESS_ERROR_NO;

    /* Call Get NN boxes first */
    error = st_yolox_pp_getNNBoxes_centroid(pInput,
                                            pOutput,
                                            pInput_static_param);
    if (error != AI_OD_POSTPROCESS_ERROR_NO) return (error);

    /* Then NMS */
    error = st_yolox_pp_nmsFiltering_centroid(pOutput,
                                              pInput_static_param);
    if (error != AI_OD_POSTPROCESS_ERROR_NO) return (error);

    /* And score re-filtering */
    error = st_yolox_pp_scoreFiltering_centroid(pOutput,
                                                pInput_static_param);

    return (error);
}

int32_t od_st_yolox_pp_process_int8(od_st_yolox_pp_in_t *pInput,
                                    od_pp_out_t *pOutput,
                                    od_st_yolox_pp_static_param_t *pInput_static_param)
{
    int32_t error   = AI_OD_POSTPROCESS_ERROR_NO;

    /* Call Get NN boxes first */
    error = st_yolox_pp_getNNBoxes_centroid_is8(pInput,
                                            pOutput,
                                            pInput_static_param);
    if (error != AI_OD_POSTPROCESS_ERROR_NO) return (error);

    /* Then NMS */
    error = st_yolox_pp_nmsFiltering_centroid(pOutput,
                                              pInput_static_param);
    if (error != AI_OD_POSTPROCESS_ERROR_NO) return (error);

    /* And score re-filtering */
    error = st_yolox_pp_scoreFiltering_centroid(pOutput,
                                                pInput_static_param);

    return (error);
}

