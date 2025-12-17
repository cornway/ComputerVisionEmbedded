/*---------------------------------------------------------------------------------------------
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file in
 * the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *--------------------------------------------------------------------------------------------*/

#include "sseg_deeplabv3_pp_if.h"
#include "vision_models_pp.h"

#if 0 // Not used
int32_t sseg_deeplabv3_pp_argmax_to_colormap(sseg_deeplabv3_pp_in_t *pInput,
                                    sseg_pp_out_t *pOutput,
                                    sseg_deeplabv3_pp_static_param_t *pInput_static_param)
{
  int32_t error   = AI_SSEG_POSTPROCESS_ERROR_NO;
  uint32_t nb_classes = pInput_static_param->nb_classes;
  uint8_t *out = pOutput->pOutBuff;


  switch (pInput->type) {
  case AI_SSEG_DATA_FLOAT:
    {
      float32_t *pSrc = (float32_t *)pInput->pRawData;
      uint32_t _index_a[4];
      float32_t _maxim_a[4];
        for (size_t i = 0; i < (pInput_static_param->width * pInput_static_param->height) >> 2; i++)
        {
            vision_models_maxi_p_if32ou32(pSrc, nb_classes, nb_classes, _maxim_a, _index_a, 4);
            pSrc+=4*nb_classes;
            for (int p = 0; p < 4; p++) {
                memcpy(out, &pInput_static_param->color_map[_index_a[p]], 3 * sizeof(uint8_t));
                out+=3 * sizeof(uint8_t);
            }
        }
        int remaing = (pInput_static_param->width * pInput_static_param->height) & 3;
        while(remaing-- > 0) {
          uint32_t index;
          float32_t maxim;
          vision_models_maxi_if32ou32(pSrc, nb_classes, &maxim, &index);
          memcpy(out, &pInput_static_param->color_map[index], 3 * sizeof(uint8_t));
          out+=3 * sizeof(uint8_t);
        }
    }
    break;
  case AI_SSEG_DATA_UINT8:
    {
      uint8_t *pSrc = (uint8_t *)pInput->pRawData;
      if (nb_classes < 256) {
        uint8_t _index_a[16];
        uint8_t _maxim_a[16];
        for (size_t i = 0; i < (pInput_static_param->width * pInput_static_param->height) >> 4; i++)
        {
            vision_models_maxi_p_iu8ou8(pSrc, nb_classes, nb_classes, _maxim_a, _index_a, 4);
            pSrc+=16*nb_classes;
            for (int p = 0; p < 16; p++) {
                memcpy(out, &pInput_static_param->color_map[_index_a[p]], 3 * sizeof(uint8_t));
                out+=3 * sizeof(uint8_t);
            }
        }
        int remaing = (pInput_static_param->width * pInput_static_param->height) & 15;
        while(remaing-- > 0) {
          uint8_t index;
          uint8_t maxim;
          vision_models_maxi_iu8ou8(pSrc, nb_classes, &maxim, &index);
          memcpy(out, &pInput_static_param->color_map[index], 3 * sizeof(uint8_t));
          out+=3 * sizeof(uint8_t);
        }
      } else {
        uint16_t _index_a[8];
        uint8_t _maxim_a[8];
        for (size_t i = 0; i < (pInput_static_param->width * pInput_static_param->height) >> 3; i++)
        {
            vision_models_maxi_p_iu8ou16(pSrc, nb_classes, nb_classes, _maxim_a, _index_a, 4);
            pSrc+=8*nb_classes;
            for (int p = 0; p < 8; p++) {
                memcpy(out, &pInput_static_param->color_map[_index_a[p]], 3 * sizeof(uint8_t));
                out+=3 * sizeof(uint8_t);
            }
        }
        int remaing = (pInput_static_param->width * pInput_static_param->height) & 7;
        while(remaing-- > 0) {
          uint16_t index;
          uint8_t maxim;
          vision_models_maxi_iu8ou16(pSrc, nb_classes, &maxim, &index);
          memcpy(out, &pInput_static_param->color_map[index], 3 * sizeof(uint8_t));
          out+=3 * sizeof(uint8_t);
        }
      }
    }
    break;
  default:
    error = AI_SSEG_POSTPROCESS_ERROR;
  }
  return error;
}
#endif

int32_t sseg_deeplabv3_pp_argmax(sseg_deeplabv3_pp_in_t *pInput,
                                 sseg_pp_out_t          *pOutput,
                                 sseg_deeplabv3_pp_static_param_t *pInput_static_param)
{
  int32_t error   = AI_SSEG_POSTPROCESS_ERROR_NO;
  uint32_t nb_classes = pInput_static_param->nb_classes;

  float32_t *pSrc = (float32_t *)pInput->pRawData;
  int32_t loop = pInput_static_param->width * pInput_static_param->height;
  float32_t _maxim_a[4];
  if (nb_classes < 256) {
    uint8_t *out = (uint8_t *)pOutput->pOutBuff;
    while(loop > 0)
    {
        vision_models_maxi_p_if32ou8(pSrc, nb_classes, nb_classes, _maxim_a, out, loop);
        pSrc+= 4*nb_classes;
        out += 4;
        loop-=4;
    }
  } else {
    uint16_t *out = (uint16_t *)pOutput->pOutBuff;
    while(loop > 0)
    {
        vision_models_maxi_p_if32ou16(pSrc, nb_classes, nb_classes, _maxim_a, (uint16_t *)out, loop);
        pSrc+= 4*nb_classes;
        out += 4;
        loop-=4;
    }
  }
  return error;
}

int32_t sseg_deeplabv3_pp_argmax_uint8(sseg_deeplabv3_pp_in_t *pInput,
                                       sseg_pp_out_t          *pOutput,
                                       sseg_deeplabv3_pp_static_param_t *pInput_static_param)
{
  int32_t error   = AI_SSEG_POSTPROCESS_ERROR_NO;
  uint32_t nb_classes = pInput_static_param->nb_classes;

  uint8_t *pSrc = (uint8_t *)pInput->pRawData;
  int32_t loop = pInput_static_param->width * pInput_static_param->height;
  uint8_t _maxim_a[16];
  if (nb_classes < UCHAR_MAX) {
    uint8_t *out = (uint8_t *)pOutput->pOutBuff;
    while(loop > 0)
    {
        vision_models_maxi_p_iu8ou8(pSrc, nb_classes, nb_classes, _maxim_a, out, loop);
        pSrc+=16*nb_classes;
        out +=16;
        loop-=16;
    }
  } else {
    uint16_t *out = (uint16_t *)pOutput->pOutBuff;
    while(loop > 0)
    {
        vision_models_maxi_p_iu8ou16(pSrc, nb_classes, nb_classes, _maxim_a, out, loop);
       pSrc+=8*nb_classes;
        out +=8;
        loop-=8;
    }
  }
  return error;
}

int32_t sseg_deeplabv3_pp_argmax_int8(sseg_deeplabv3_pp_in_t *pInput,
                                      sseg_pp_out_t          *pOutput,
                                      sseg_deeplabv3_pp_static_param_t *pInput_static_param)
{
  int32_t error   = AI_SSEG_POSTPROCESS_ERROR_NO;
  uint32_t nb_classes = pInput_static_param->nb_classes;

  int8_t *pSrc = (int8_t *)pInput->pRawData;
  int32_t loop = pInput_static_param->width * pInput_static_param->height;
  int8_t _maxim_a[16];
  if (nb_classes < UCHAR_MAX) {
    uint8_t *out = (uint8_t *)pOutput->pOutBuff;
    while(loop > 0)
    {
        vision_models_maxi_p_is8ou8(pSrc, nb_classes, nb_classes, _maxim_a, out, loop);
        pSrc+=16*nb_classes;
        out +=16;
        loop-=16;
    }
  } else {
    uint16_t *out = (uint16_t *)pOutput->pOutBuff;
    while(loop > 0)
    {
        vision_models_maxi_p_is8ou16(pSrc, nb_classes, nb_classes, _maxim_a, out, loop);
        pSrc+=8*nb_classes;
        out +=8;
        loop-=8;
    }
  }
  return error;
}
#if 0 // Not used
int32_t sseg_deeplabv3_pp_apply_color_map(sseg_deeplabv3_pp_in_t *pInput,
                                          sseg_pp_out_t *pOutput,
                                          sseg_deeplabv3_pp_static_param_t *pInput_static_param)
{
  int32_t error   = AI_SSEG_POSTPROCESS_ERROR_NO;
  uint32_t nb_classes = pInput_static_param->nb_classes;
  size_t width = pInput_static_param->width;
  size_t height = pInput_static_param->height;
  uint8_t *out = pOutput->pOutBuff;
  int32_t loop = width * height;
  out += 3 * sizeof(uint8_t) * (width * height - 1) ;

  if (nb_classes < 256) {
    uint8_t *pIdx = (uint8_t *)pOutput->pOutBuff;
    pIdx += loop - 1;
    while(loop > 0) {
        memcpy(out, &pInput_static_param->color_map[*pIdx], 3 * sizeof(uint8_t));
        out-=3 * sizeof(uint8_t);
        pIdx--;
        loop--;
    }
  } else {
    uint16_t *pIdx = (uint16_t *)pOutput->pOutBuff;
    pIdx += loop - 1;
    while(loop > 0) {
        memcpy(out, &pInput_static_param->color_map[*pIdx], 3 * sizeof(uint8_t));
        out -=3 * sizeof(uint8_t);
        pIdx--;
    }
  }
  return error;
}
#endif



/* ----------------------       Exported routines      ---------------------- */

int32_t sseg_deeplabv3_pp_reset(sseg_deeplabv3_pp_static_param_t *pInput_static_param)
{
    /* Initializations */
    //sseg_deeplabv3_pp_static_param_t->... = 0;

    return (AI_SSEG_POSTPROCESS_ERROR_NO);
}


int32_t sseg_deeplabv3_pp_process(sseg_deeplabv3_pp_in_t *pInput,
                                      sseg_pp_out_t *pOutput,
                                      sseg_deeplabv3_pp_static_param_t *pInput_static_param)
{
  int32_t error   = AI_SSEG_POSTPROCESS_ERROR_NO;

#if 0
 if (pInput_static_param->color_map) {
  /* Call argmax and directly generate colormap */
  error = sseg_deeplabv3_pp_argmax_to_colormap(pInput,
                                                  pOutput,
                                                  pInput_static_param);


 }
#else
  /* Call argmax */
  error = sseg_deeplabv3_pp_argmax(pInput,
                                   pOutput,
                                   pInput_static_param);

#if 0
 if (pInput_static_param->color_map) {
  if (error != AI_SSEG_POSTPROCESS_ERROR_NO) {
    return (error);
  }

  /* Then apply color map */
  error = sseg_deeplabv3_pp_apply_color_map(pInput,
                                               pOutput,
                                               pInput_static_param);
 }

  if (error != AI_SSEG_POSTPROCESS_ERROR_NO) {
    return (error);
  }
#endif
#endif
    return (error);
}

int32_t sseg_deeplabv3_pp_process_uint8(sseg_deeplabv3_pp_in_t *pInput,
                                        sseg_pp_out_t *pOutput,
                                        sseg_deeplabv3_pp_static_param_t *pInput_static_param)
{
  int32_t error   = AI_SSEG_POSTPROCESS_ERROR_NO;

  /* Call argmax */
  error = sseg_deeplabv3_pp_argmax_uint8(pInput,
                                   pOutput,
                                   pInput_static_param);

    return (error);
}

int32_t sseg_deeplabv3_pp_process_int8(sseg_deeplabv3_pp_in_t *pInput,
                                        sseg_pp_out_t *pOutput,
                                        sseg_deeplabv3_pp_static_param_t *pInput_static_param)
{
  int32_t error   = AI_SSEG_POSTPROCESS_ERROR_NO;

  /* Call argmax */
  error = sseg_deeplabv3_pp_argmax_int8(pInput,
                                        pOutput,
                                        pInput_static_param);

    return (error);
}

