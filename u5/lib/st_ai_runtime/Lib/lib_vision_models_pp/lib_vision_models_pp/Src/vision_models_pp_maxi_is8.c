/*---------------------------------------------------------------------------------------------
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file in
 * the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *--------------------------------------------------------------------------------------------*/

#include "vision_models_pp.h"

#include "stdio.h"


void vision_models_maxi_p_is8ou8(int8_t *arr, uint32_t len_arr, uint32_t offset, int8_t *maxim, uint8_t *index, uint32_t parallelize)
{
#ifdef VISION_MODELS_MAXI_P_IS8OU8_MVE
  if (15*offset < UCHAR_MAX) {
    int8x16_t   s8x16_max_val = vdupq_n_s8(SCHAR_MIN);
    uint8x16_t   u8x16_max_idx = vdupq_n_u8(0);

    parallelize = MIN(parallelize, 16);
    mve_pred16_t p = vctp8q(parallelize);

    uint8x16_t u8x16_idx = vdupq_n_u8(0);
    uint8x16_t u8x16_offset = vidupq_n_u8(0,1) * (uint8_t)offset;

    for (uint8_t i = 0; i < len_arr; i++)
    {
      // load up to 16 int8
      int8x16_t s8x16_val = vldrbq_gather_offset_z_s8(arr, u8x16_offset, p);
      arr++;
      // Compare according to p to create p0
      mve_pred16_t p0 = vcmpgtq_m_s8(s8x16_val, s8x16_max_val, p);

      // according to p0: update with s8x16_val or keep s8x16_blk_minmax_val
      s8x16_max_val = vpselq_s8(s8x16_val, s8x16_max_val, p0);
      /* according to p0: store per-lane extrema indexes*/
      u8x16_max_idx = vpselq_u8(u8x16_idx, u8x16_max_idx, p0);
      u8x16_idx+=1;

    }
    vstrbq_p_u8(index,u8x16_max_idx,p);
    vstrbq_p_s8(maxim,s8x16_max_val,p);
  } else {
    uint16_t _tmpIdx[8];
    uint16_t _parallelize = MIN(8,parallelize);
    vision_models_maxi_p_is8ou16(arr,len_arr,offset,maxim,_tmpIdx,_parallelize);
    maxim+=_parallelize;
    for ( uint16_t i = 0; i < _parallelize; i++) {
      *index++ = _tmpIdx[i];
    }
    parallelize-=_parallelize;
    arr+=_parallelize*offset;
    vision_models_maxi_p_is8ou16(arr,len_arr,offset,maxim,_tmpIdx,parallelize);
    for ( uint32_t i = 0; i < parallelize; i++) {
      *index++ = _tmpIdx[i];
    }
  }
#else
  parallelize = MIN(16, parallelize);
  if(parallelize*offset < UCHAR_MAX) {
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
  } else {
    uint16_t _tmpIdx[8];
    uint16_t _parallelize = MIN(8,parallelize);
    vision_models_maxi_p_is8ou16(arr,len_arr,offset,maxim,_tmpIdx,_parallelize);
    maxim+=_parallelize;
    for ( uint16_t i = 0; i <_parallelize; i++) {
      *index++ = _tmpIdx[i];
    }
    parallelize-=_parallelize;
    arr+=_parallelize*offset;
    vision_models_maxi_p_is8ou16(arr,len_arr,offset,maxim,_tmpIdx,parallelize);
    for ( int i = 0; i < parallelize; i++) {
      *index++ = _tmpIdx[i];
    }
  }

#endif

}

void vision_models_maxi_p_is8ou16(int8_t *arr, uint32_t len_arr, uint32_t offset, int8_t *maxim, uint16_t *index, uint32_t parallelize)
{
#ifdef VISION_MODELS_MAXI_P_IS8OU16_MVE
  int16x8_t   s16x8_max_val = vdupq_n_s16(SHRT_MIN);
  uint16x8_t   u16x8_max_idx = vdupq_n_u16(0);

  parallelize = MIN(parallelize, 8);
  mve_pred16_t p = vctp16q(parallelize);

  uint16x8_t u16x8_idx = vdupq_n_u16(0);
  uint16x8_t u16x8_offset = vidupq_n_u16(0,1) * (uint16_t)offset;

  for (uint8_t i = 0; i < len_arr; i++)
  {
    // load up to 16 int8
    int16x8_t s16x8_val = vldrbq_gather_offset_z_s16(arr, u16x8_offset, p);
    arr++;
    // Compare according to p to create p0
    mve_pred16_t p0 = vcmpgtq_m_s16(s16x8_val, s16x8_max_val, p);

    // according to p0: update with s16x8_val or keep s16x8_blk_minmax_val
    s16x8_max_val = vpselq_s16(s16x8_val, s16x8_max_val, p0);
    /* according to p0: store per-lane extrema indexes*/
    u16x8_max_idx = vpselq_u16(u16x8_idx, u16x8_max_idx, p0);
    u16x8_idx+=1;

  }
  vstrhq_p_u16(index,u16x8_max_idx,p);
  vstrbq_p_s16(maxim,s16x8_max_val,p);
#else
  parallelize = MIN(8, parallelize);
  for (uint8_t k = 0; k < parallelize; k++)
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



/* return max value and it's index from a transposed array */
void vision_models_maxi_tr_p_is8ou8(int8_t *arr, uint32_t len_arr, uint32_t offset, int8_t *maxim, uint8_t *index, uint32_t parallelize)
{
  #ifdef VISION_MODELS_MAXI_TR_P_IS8OU8_MVE
  int8x16_t      s8x16_max_val = vdupq_n_s8(Q7_MIN);
  uint8x16_t     u8x16_max_idx = vdupq_n_u8(0);
  parallelize = MIN(parallelize, 16);
  mve_pred16_t    p = vctp8q(parallelize);

  uint8x16_t u8x16_idx = vdupq_n_u8(0);
  for (uint32_t i = 0; i < len_arr; i++)
  {
    // load up to 16 int8
    int8x16_t s8x16_val = vld1q_z_s8(&arr[i*offset], p);
    // Compare according to p to create p0
    mve_pred16_t p0 = vcmpgtq_m_s8(s8x16_val, s8x16_max_val, p);

    // according to p0: update with s8x16_val or keep s8x16_blk_minmax_val
    s8x16_max_val = vpselq_s8(s8x16_val, s8x16_max_val, p0);
    /* according to p0: store per-lane extrema indexes*/
    u8x16_max_idx = vpselq_u8(u8x16_idx, u8x16_max_idx, p0);
    u8x16_idx+=1;

  }
  vstrbq_p_u8((uint8_t *)index,u8x16_max_idx,p);
  vstrbq_p_s8((int8_t *)maxim,s8x16_max_val,p);
#else
  parallelize = MIN(16, parallelize);
  for (uint8_t k = 0; k < parallelize; k++)
  {
    *index = 0;
    *maxim = arr[k];
    for (uint32_t i = 1; i < len_arr; i++)
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


void vision_models_maxi_tr_p_is8ou16(int8_t *arr, uint32_t len_arr, uint32_t offset, int8_t *maxim, uint16_t *index, uint32_t parallelize)
{
#ifdef VISION_MODELS_MAXI_TR_P_IS8OU16_MVE
  int8x16_t  s8x16_max_val = vdupq_n_s8(Q7_MIN);
  uint8x16_t u8x16_max_idx = vdupq_n_u8(0);
  uint8x16_t u8x16_max_idx_blk = vdupq_n_u8(0);
  parallelize = MIN(parallelize, 16);
  mve_pred16_t    p = vctp8q(parallelize);

  int8_t idx_blk = 0;
  while (len_arr > 0)
  {
    int maxIter = MIN(len_arr, 0x100);
    uint8x16_t u8x16_blk_idx = vdupq_n_u8(0);
    uint8x16_t u8x16_blk_max_idx = vdupq_n_u8(0);
    int8x16_t  s8x16_blk_max_val = vdupq_n_s8(Q7_MIN);
    mve_pred16_t p0;
    // Process each block up to 256 i.e. 0x100
    for (int i = 0; i < maxIter; i++)
    {
      // load up to 16 int8
      int8x16_t s8x16_val = vld1q_z_s8(&arr[i*offset], p);
      // Compare according to p to create p0
      p0 = vcmpgtq_m_s8(s8x16_val, s8x16_blk_max_val, p);

      // according to p0: update with s8x16_val or keep s8x16_blk_max_val
      s8x16_blk_max_val = vpselq_s8(s8x16_val, s8x16_blk_max_val, p0);
      /* according to p0: store per-lane extrema indexes*/
      u8x16_blk_max_idx = vpselq_u8(u8x16_blk_idx, u8x16_blk_max_idx, p0);
      u8x16_blk_idx+=1;
    }
    len_arr -= 0x100;
    // Compare according to global max to create p0
    p0 = vcmpgtq_m_s8(s8x16_blk_max_val, s8x16_max_val, p);

    // update global max value
    s8x16_max_val = vpselq_s8(s8x16_blk_max_val, s8x16_max_val, p0);
    u8x16_max_idx = vpselq_u8(u8x16_blk_max_idx, u8x16_max_idx, p0);
    u8x16_max_idx_blk = vdupq_m_n_u8(u8x16_max_idx_blk, idx_blk, p0);
    idx_blk++;
  }
  // From u8 to u16 build whole index
  uint16x8x2_t   u16x8x2_idx;
  uint16x8x2_t   u16x8x2_idx_blk;
  // idx = max_idx + max_idx_blk * <block_size>
  u16x8x2_idx.val[0] = vmovlbq_u8(u8x16_max_idx);
  u16x8x2_idx_blk.val[0] = vmovlbq_u8(u8x16_max_idx_blk);
  u16x8x2_idx.val[0] = vmlaq_n_u16(u16x8x2_idx.val[0], u16x8x2_idx_blk.val[0], 256);
  // idx = max_idx + max_idx_blk * <block_size>
  u16x8x2_idx.val[1] = vmovltq_u8(u8x16_max_idx);
  u16x8x2_idx_blk.val[1] = vmovltq_u8(u8x16_max_idx_blk);
  u16x8x2_idx.val[1] = vmlaq_n_u16(u16x8x2_idx.val[1], u16x8x2_idx_blk.val[1], 256);

  vst2q_u16((uint16_t *)index,u16x8x2_idx);
  vstrbq_p_s8((int8_t *)maxim,s8x16_max_val,p);
#else
  parallelize = MIN(16, parallelize);
  for (uint16_t k = 0; k < parallelize; k++)
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
#include "stdio.h"

void vision_models_maxi_tr_p_is8ou32(int8_t *arr, uint32_t len_arr, uint32_t offset, int8_t *maxim, uint32_t *index, uint32_t parallelize)
{
#ifdef VISION_MODELS_MAXI_TR_P_IS8OU32_MVE
  int8x16_t  s8x16_max_val = vdupq_n_s8(Q7_MIN);
  parallelize = MIN(parallelize, 16);
  mve_pred16_t    p = vctp8q(parallelize);

  uint32_t offset_blk = 0;
  int8_t *pArr = arr;
  uint8x16_t u8x16_blk_max_idx = vdupq_n_u8(0);
  int8x16_t  s8x16_blk_max_val = vdupq_n_s8(Q7_MIN);

  while (len_arr > 0)
  {
    int maxIter = MIN(len_arr, 256);
    uint8x16_t u8x16_idx = vdupq_n_u8(0);
    mve_pred16_t p0;
    // Process each block up to 256 i.e. 0x100
    for (int i = 0; i < maxIter; i++)
    {
      // load up to 16 int8
      int8x16_t s8x16_val = vld1q_z_s8(pArr, p);
     // Compare according to p to create p0
      p0 = vcmpgtq_m_s8(s8x16_val, s8x16_blk_max_val, p);

      // according to p0: update with s8x16_val or keep s8x16_blk_max_val
      s8x16_blk_max_val = vpselq_s8(s8x16_val, s8x16_blk_max_val, p0);
      /* according to p0: store per-lane extrema indexes*/
      u8x16_blk_max_idx = vpselq_u8(u8x16_idx, u8x16_blk_max_idx, p0);
      u8x16_idx+=1;
      pArr+=offset;

    } // internal loop (max 256)
    len_arr -= 256;
    // Compare according to global max to create p0
    p0 = vcmpgtq_m_s8(s8x16_blk_max_val, s8x16_max_val, p);

    // update global max value
    for (uint32_t e = 0; e < parallelize; e++) {
      if (s8x16_blk_max_val[e] > s8x16_max_val[e]) {
        index[e] = offset_blk + u8x16_blk_max_idx[e];
      }
    }
    s8x16_max_val = vpselq_s8(s8x16_blk_max_val, s8x16_max_val, p0);
    offset_blk+=256;
  }
  vstrbq_p_s8((int8_t *)maxim,s8x16_max_val,p);
#else
  parallelize = MIN(16, parallelize);
  for (uint32_t p = 0; p < parallelize; p++) {

    *index = 0;
    *maxim = arr[p];

    for (uint32_t i = 1; i < len_arr; i++)
    {
      if (arr[p+i*offset] > *maxim)
      {
        *maxim = arr[p+i*offset];
        *index = (uint32_t)i;
      }
    }
    maxim++;
    index++;
  }
#endif
}

