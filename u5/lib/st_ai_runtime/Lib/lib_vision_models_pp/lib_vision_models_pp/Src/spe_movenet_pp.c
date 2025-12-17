/*---------------------------------------------------------------------------------------------
#  * Copyright (c) 2023 STMicroelectronics.
#  * All rights reserved.
#  *
#  * This software is licensed under terms that can be found in the LICENSE file in
#  * the root directory of this software component.
#  * If no LICENSE file comes with this software, it is provided AS-IS.
#  *--------------------------------------------------------------------------------------------*/

#include "spe_movenet_pp_loc.h"
#include "spe_movenet_pp_if.h"
#include "vision_models_pp.h"


#include "stdio.h"

int32_t movenet_heatmap_max(spe_movenet_pp_in_t *pInput,
                            spe_pp_out_t *pOutput,
                            spe_movenet_pp_static_param_t *pInput_static_param)
{
  uint32_t i;
  float32_t x_center;
  float32_t y_center;
  float32_t proba;
  uint32_t index;
  uint32_t width = pInput_static_param->heatmap_width;
  uint32_t height = pInput_static_param->heatmap_height;
  float32_t *pInputData = (float32_t *)pInput->inBuff;

  for (i = 0; i < pInput_static_param->nb_keypoints; i+=4)
  {
    float32_t *pInputKp = &(pInputData[i]);
    float32_t proba_array[4];
    uint32_t index_array[4];
    vision_models_maxi_tr_p_if32ou32((float32_t *) pInputKp,
                                   (uint32_t) width * height,
                                   pInput_static_param->nb_keypoints,
                                   (float32_t *) &proba_array,
                                   (uint32_t *) &index_array,
                                   pInput_static_param->nb_keypoints-i);

    for (uint32_t _i = 0; _i < MIN(pInput_static_param->nb_keypoints-i,4); _i++) {
      index = index_array[_i];
      proba = proba_array[_i];
     /* This is not cartesian referential, to be aligned with Python code */
      y_center = ((index % height + 0.5f) / height);
      x_center = ((index / height + 0.5f) / width);

      /* Coordinates inversion for the application code that uses cartesian referential */
      pOutput->pOutBuff[i+_i].x_center = y_center;
      pOutput->pOutBuff[i+_i].y_center = x_center;
      pOutput->pOutBuff[i+_i].proba = proba;
    }
  }

  return (AI_SPE_POSTPROCESS_ERROR_NO);
}

int32_t movenet_heatmap_max_int8(spe_movenet_pp_in_t *pInput,
                                 spe_pp_out_t *pOutput,
                                 spe_movenet_pp_static_param_t *pInput_static_param)
{
  uint32_t i;
  float32_t x_center;
  float32_t y_center;
  int8_t proba;
  uint32_t index;
  uint32_t width = pInput_static_param->heatmap_width;
  uint32_t height = pInput_static_param->heatmap_height;
  int8_t *pInputData = (int8_t *)pInput->inBuff;
  int8_t zero_point = pInput_static_param->raw_zero_point;
  float32_t scale = pInput_static_param->raw_scale;

  for (i = 0; i < pInput_static_param->nb_keypoints; i+=16)
  {
    int8_t *pInputKp = &(pInputData[i]);
    int8_t proba_array[16];
    uint32_t index_array[16];
    vision_models_maxi_tr_p_is8ou32((int8_t *) pInputKp,
                                    (uint32_t) width * height,
                                    pInput_static_param->nb_keypoints,
                                    (int8_t *) proba_array,
                                    (uint32_t *) index_array,
                                    MIN(pInput_static_param->nb_keypoints-i,16));

    for (uint32_t _i = 0; _i <  MIN(pInput_static_param->nb_keypoints-i,16); _i++) {
      index = index_array[_i];
      proba = proba_array[_i];
      /* This is not cartesian referential, to be aligned with Python code */
      y_center = ((index % height + 0.5f) / height);
      x_center = ((index / height + 0.5f) / width);

      /* Coordinates inversion for the application code that uses cartesian referential */
      pOutput->pOutBuff[i+_i].x_center = y_center;
      pOutput->pOutBuff[i+_i].y_center = x_center;
      pOutput->pOutBuff[i+_i].proba = scale *(float32_t)((int32_t)proba - (int32_t)zero_point);
    }
  }

  return (AI_SPE_POSTPROCESS_ERROR_NO);
}


/* ----------------------       Exported routines      ---------------------- */

int32_t spe_movenet_pp_reset(spe_movenet_pp_static_param_t *pInput_static_param)
{
  /* Initializations */
  // pInput_static_param->... no initialization

	return (AI_SPE_POSTPROCESS_ERROR_NO);
}


int32_t spe_movenet_pp_process(spe_movenet_pp_in_t *pInput,
                                     spe_pp_out_t *pOutput,
                                     spe_movenet_pp_static_param_t *pInput_static_param)
{
    int32_t error   = AI_SPE_POSTPROCESS_ERROR_NO;

     /* Extracts max value and indexes of each heatmap */
    error = movenet_heatmap_max(pInput,
                                pOutput,
                                pInput_static_param);

    return (error);
}

int32_t spe_movenet_pp_process_int8(spe_movenet_pp_in_t *pInput,
                                     spe_pp_out_t *pOutput,
                                     spe_movenet_pp_static_param_t *pInput_static_param)
{
    int32_t error   = AI_SPE_POSTPROCESS_ERROR_NO;

     /* Extracts max value and indexes of each heatmap */
    error = movenet_heatmap_max_int8(pInput,
                                pOutput,
                                pInput_static_param);

    return (error);
}
