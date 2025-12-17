/*---------------------------------------------------------------------------------------------
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file in
 * the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *--------------------------------------------------------------------------------------------*/

#include "vision_models_pp.h"


/* return max value and it's index from an array */
void vision_models_maxi_if32ou32(float32_t *arr, uint32_t len_arr, float32_t *maxim, uint32_t *index)
{
#ifdef VISION_MODELS_MAXI_IF32OU32_MVE

  float32x4_t    f32x4_max_val = vdupq_n_f32(F32_MIN);
  uint32x4_t     u32x4_max_idx = vdupq_n_u32(0);


  uint32x4_t u32x4_idx = vidupq_n_u32(0,1);
  float32_t *pSrc = arr;
  int32_t iter = len_arr;
  while(iter > 0)
  {
    mve_pred16_t p = vctp32q(iter);
    // load up to 4 float32_t
    float32x4_t f32x4_val = vldrwq_z_f32(pSrc, p);
    pSrc+=4;
    // Compare according to p to create p0
    mve_pred16_t p0 = vcmpgtq_m_f32(f32x4_val, f32x4_max_val, p);

    // according to p0: update with s8x16_val or keep s8x16_blk_minmax_val
    f32x4_max_val = vpselq_f32(f32x4_val, f32x4_max_val, p0);
    /* according to p0: store per-lane extrema indexes*/
    u32x4_max_idx = vpselq_u32(u32x4_idx, u32x4_max_idx, p0);
    u32x4_idx+=4;
    iter-=4;

  }
  /*
   * Get max value across the vector
   */
  *maxim = vmaxnmvq_f32(F32_MIN, f32x4_max_val);
  /*
   * set index for lower values to max possible index
   */
  mve_pred16_t p0 = vcmpgeq_n_f32(f32x4_max_val, *maxim);
  uint32x4_t indexVec = vpselq_u32(u32x4_max_idx, vdupq_n_u32(len_arr), p0);
  /*
   * Get min index which is thus for a max value
   */
  *index = (int32_t)vminvq_u32(len_arr, indexVec);
#else
    *index = 0;
    *maxim = arr[0];

    for (uint32_t i = 1; i < len_arr; i++)
    {
      if (arr[i] > *maxim)
      {
        *maxim = arr[i];
        *index = i;
      }
    }
#endif
}

void vision_models_maxi_tr_if32ou32(float32_t *arr, uint32_t nb_elem_arr, uint32_t offset, float32_t *maxim, uint32_t *index)
{
#ifdef VISION_MODELS_MAXI_TR_IF32OU32_MVE

  float32x4_t    f32x4_max_val = vdupq_n_f32(F32_MIN);
  uint32x4_t     u32x4_max_idx = vdupq_n_u32(0);


  uint32x4_t u32x4_idx = vidupq_n_u32(0,1);
  uint32x4_t u32x4_offset = u32x4_idx * 4 * offset;

  float32_t *pSrc = arr;
  int32_t iter = nb_elem_arr;

  while(iter > 0)
  {
    mve_pred16_t p = vctp32q(iter);
    // load up to 4 float32_t
    float32x4_t f32x4_val = vldrwq_gather_offset_z_f32(pSrc, u32x4_offset, p);

    pSrc += (4 * offset);
    // Compare according to p to create p0
    mve_pred16_t p0 = vcmpgtq_m_f32(f32x4_val, f32x4_max_val, p);

    // according to p0: update with s8x16_val or keep s8x16_blk_minmax_val
    f32x4_max_val = vpselq_f32(f32x4_val, f32x4_max_val, p0);
    /* according to p0: store per-lane extrema indexes*/
    u32x4_max_idx = vpselq_u32(u32x4_idx, u32x4_max_idx, p0);
    u32x4_idx+=4;
    iter-=4;

  }
  /*
   * Get max value across the vector
   */
  *maxim = vmaxnmvq_f32(F32_MIN, f32x4_max_val);
  /*
   * set index for lower values to max possible index
   */
  mve_pred16_t p0 = vcmpgeq_n_f32(f32x4_max_val, *maxim);
  uint32x4_t indexVec = vpselq_u32(u32x4_max_idx, vdupq_n_u32(nb_elem_arr), p0);

  /*
   * Get min index which is thus for a max value
   */
  *index = (int32_t)vminvq_u32(nb_elem_arr, indexVec);
#else
    *index = 0;
    *maxim = arr[0];
    for (uint16_t i = 1; i < nb_elem_arr; i++)
    {
      if (arr[i*offset] > *maxim)
      {
        *maxim = arr[i*offset];
        *index = i;
      }
    }
    maxim++;
    index++;

#endif
}
void vision_models_maxi_p_if32ou32(float32_t *arr, uint32_t len_arr, uint32_t offset, float32_t *maxim, uint32_t *index, uint32_t parallelize)
{
#ifdef VISION_MODELS_MAXI_P_IF32OU32_MVE
  float32x4_t    f32x4_max_val = vdupq_n_f32(F32_MIN);
  uint32x4_t     u32x4_max_idx = vdupq_n_u32(0);

  parallelize = MIN(parallelize, 4);
  mve_pred16_t p = vctp32q(parallelize);

  uint32x4_t u32x4_idx = vdupq_n_u32(0);
  uint32x4_t u32x4_offset = vidupq_n_u32(0,4) * offset;

  for (uint32_t i = 0; i < len_arr; i++)
  {
    // load up to 16 int8
    float32x4_t f32x4_val = vldrwq_gather_offset_z_f32(arr, u32x4_offset, p);
    arr++;
    // Compare according to p to create p0
    mve_pred16_t p0 = vcmpgtq_m_f32(f32x4_val, f32x4_max_val, p);

    // according to p0: update with s8x16_val or keep s8x16_blk_minmax_val
    f32x4_max_val = vpselq_f32(f32x4_val, f32x4_max_val, p0);
    /* according to p0: store per-lane extrema indexes*/
    u32x4_max_idx = vpselq_u32(u32x4_idx, u32x4_max_idx, p0);
    u32x4_idx+=1;

  }
  vstrwq_p_u32(index,u32x4_max_idx,p);
  vstrwq_p_f32(maxim,f32x4_max_val,p);
#else
  parallelize = MIN(4, parallelize);
  for (uint32_t k = 0; k < parallelize; k++)
  {
    *index = 0;
    *maxim = arr[k*offset];

    for (uint32_t i = 1; i < len_arr; i++)
    {
      if (arr[k*offset+i] > *maxim)
      {
        *maxim = arr[k*offset+i];
        *index = i;
      }
    }
    maxim++;
    index++;
  }
#endif
}
void vision_models_maxi_p_if32ou16(float32_t *arr, uint32_t len_arr, uint16_t offset, float32_t *maxim, uint16_t *index, uint32_t parallelize)
{
#ifdef VISION_MODELS_MAXI_P_IF32OU16_MVE
  float32x4_t    f32x4_max_val = vdupq_n_f32(F32_MIN);
  uint32x4_t     u32x4_max_idx = vdupq_n_u32(0);

  parallelize = MIN(parallelize, 4);
  mve_pred16_t p = vctp32q(parallelize);

  uint32x4_t u32x4_idx = vdupq_n_u32(0);
  uint32x4_t u32x4_offset = vidupq_n_u32(0,4) * offset;

  for (uint32_t i = 0; i < len_arr; i++)
  {
    // load up to 16 int8
    float32x4_t f32x4_val = vldrwq_gather_offset_z_f32(arr, u32x4_offset, p);
    arr++;
    // Compare according to p to create p0
    mve_pred16_t p0 = vcmpgtq_m_f32(f32x4_val, f32x4_max_val, p);

    // according to p0: update with s8x16_val or keep s8x16_blk_minmax_val
    f32x4_max_val = vpselq_f32(f32x4_val, f32x4_max_val, p0);
    /* according to p0: store per-lane extrema indexes*/
    u32x4_max_idx = vpselq_u32(u32x4_idx, u32x4_max_idx, p0);
    u32x4_idx+=1;

  }
  vstrhq_p_u32((uint16_t *)index,u32x4_max_idx,p);
  vstrwq_p_f32(maxim,f32x4_max_val,p);
#else
  parallelize = MIN(4, parallelize);
  for (uint16_t k = 0; k < parallelize; k++)
  {
    *index = 0;
    *maxim = arr[k*offset];

    for (uint16_t i = 1; i < len_arr; i++)
    {
      if (arr[k*offset+i] > *maxim)
      {
        *maxim = arr[k*offset+i];
        *index = i;
      }
    }
    maxim++;
    index++;
  }
#endif
}
void vision_models_maxi_p_if32ou8(float32_t *arr, uint32_t len_arr, uint32_t offset, float32_t *maxim, uint8_t *index, uint32_t parallelize)
{
#ifdef VISION_MODELS_MAXI_P_IF32OU8_MVE
  float32x4_t    f32x4_max_val = vdupq_n_f32(F32_MIN);
  uint32x4_t     u32x4_max_idx = vdupq_n_u32(0);

  parallelize = MIN(parallelize, 4);
  mve_pred16_t p = vctp32q(parallelize);

  uint32x4_t u32x4_idx = vdupq_n_u32(0);
  uint32x4_t u32x4_offset = vidupq_n_u32(0,4) * offset;

  for (uint32_t i = 0; i < len_arr; i++)
  {
    // load up to 16 int8
    float32x4_t f32x4_val = vldrwq_gather_offset_z_f32(arr, u32x4_offset, p);
    arr++;
    // Compare according to p to create p0
    mve_pred16_t p0 = vcmpgtq_m_f32(f32x4_val, f32x4_max_val, p);

    // according to p0: update with s8x16_val or keep s8x16_blk_minmax_val
    f32x4_max_val = vpselq_f32(f32x4_val, f32x4_max_val, p0);
    /* according to p0: store per-lane extrema indexes*/
    u32x4_max_idx = vpselq_u32(u32x4_idx, u32x4_max_idx, p0);
    u32x4_idx+=1;

  }
  vstrbq_p_u32((uint8_t *)index,u32x4_max_idx,p);
  vstrwq_p_f32(maxim,f32x4_max_val,p);
#else
  parallelize = MIN(4, parallelize);
  for (uint8_t k = 0; k < parallelize; k++)
  {
    *index = 0;
    *maxim = arr[k*offset];

    for (uint8_t i = 1; i < len_arr; i++)
    {
      if (arr[k*offset+i] > *maxim)
      {
        *maxim = arr[k*offset+i];
        *index = i;
      }
    }
    maxim++;
    index++;
  }
#endif

}


/* return max value and it's index from a transposed array */
void vision_models_maxi_tr_p_if32ou32(float32_t *arr, uint32_t len_arr, uint32_t offset, float32_t *maxim, uint32_t *index, uint32_t parallelize)
{
#ifdef VISION_MODELS_MAXI_TR_P_IF32OU32_MVE
  float32x4_t    f32x4_max_val = vdupq_n_f32(F32_MIN);
  uint32x4_t     u32x4_max_idx = vdupq_n_u32(0);

  parallelize = MIN(parallelize, 4);
  mve_pred16_t p = vctp32q(parallelize);

  uint32x4_t u32x4_idx = vdupq_n_u32(0);
  for (uint32_t i = 0; i < len_arr; i++)
  {
    // load up to 16 int8
    float32x4_t f32x4_val = vld1q_z_f32(&arr[i*offset], p);
    // Compare according to p to create p0
    mve_pred16_t p0 = vcmpgtq_m_f32(f32x4_val, f32x4_max_val, p);

    // according to p0: update with s8x16_val or keep s8x16_blk_minmax_val
    f32x4_max_val = vpselq_f32(f32x4_val, f32x4_max_val, p0);
    /* according to p0: store per-lane extrema indexes*/
    u32x4_max_idx = vpselq_u32(u32x4_idx, u32x4_max_idx, p0);
    u32x4_idx+=1;

  }
  vstrwq_p_u32((uint32_t *)index,u32x4_max_idx,p);
  vstrwq_p_f32(maxim,f32x4_max_val,p);
#else
  parallelize = MIN(4, parallelize);
  for (uint32_t k = 0; k < parallelize; k++)
  {
    *index = 0;
    *maxim = arr[k];
    for (uint16_t i = 1; i < len_arr; i++)
    {
      if (arr[k+i*offset] > *maxim)
      {
        *maxim = arr[k+i*offset];
        *index = i;
      }
    }
    maxim++;
    index++;
  }
#endif
}



