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

void vision_models_maxi_iu8ou8(uint8_t *arr, uint32_t len_arr, uint8_t *pMaxim, uint8_t *pIndex)
{
#ifdef VISION_MODELS_MAXI_IU8OU8_MVE
  uint8_t maxValue = 0;
  uint8_t *pSrc = arr;
  uint8x16_t u8x16_max_val = vdupq_n_u8(0);
  uint8x16_t u8x16_max_idx = vdupq_n_u8(0);
  int32_t iter = len_arr;
  uint8x16_t u8x16_idx = vidupq_n_u8(0,1);
  uint8_t index =  0xFF;
  while(iter > 0)
  {
    mve_pred16_t p = vctp8q(iter);
    // load up to 16 int8
    uint8x16_t u8x16_val = vld1q_z_u8(pSrc, p);
    pSrc+=16;
    // Compare according to p to create p0
    mve_pred16_t p0 = vcmpcsq_m_u8(u8x16_val, u8x16_max_val, p);

    u8x16_max_val = vpselq_u8(u8x16_val, u8x16_max_val, p0);
    u8x16_max_idx = vpselq_u8(u8x16_idx, u8x16_max_idx, p0);

    u8x16_idx+=16;
    iter-=16;
  }

  maxValue = vmaxvq_u8(maxValue, u8x16_max_val);
  // get index for which value is equal to maxvalue and then get lower index
  mve_pred16_t p0 = vcmpeqq_n_u8(u8x16_max_val, maxValue);

  u8x16_max_idx = vpselq_u8( u8x16_max_idx, vdupq_n_u8(0xFF), p0);
  /*
   * Get min index which is thus for a max value
  */
  index = vminvq_p_u8(0xFF, u8x16_max_idx,p0);

  *pMaxim = maxValue;
  *pIndex = index;
#else
  *pIndex = 0;
  *pMaxim = arr[0];

  for (uint32_t i = 1; i < len_arr; i++)
  {
    if (arr[i] > *pMaxim)
    {
      *pMaxim = arr[i];
      *pIndex = i;
    }
  }
#endif
}

void vision_models_maxi_iu8ou16(uint8_t *arr, uint32_t len_arr, uint8_t *pMaxim, uint16_t *pIndex)
{
#ifdef VISION_MODELS_MAXI_IU8OU16_MVE
  uint8_t maxValue = 0;
  uint8_t maxValueLoc = 0;
  int32_t index = 0;

  int32_t iterLoop = len_arr;
  uint8_t *pSrc = arr;
  uint16_t indexBlk = 0;
  while (iterLoop > 0)
  {
    uint8x16_t u8x16_max_val = vdupq_n_u8(0);
    uint8x16_t u8x16_max_idx = vdupq_n_u8(len_arr);
    int32_t iter = MIN(len_arr,0x100);
    iterLoop -= iter;
    uint8x16_t u8x16_idx = vidupq_n_u8(0,1);
    uint8_t indexLoc =  0xFF;
    while(iter > 0)
    {
      mve_pred16_t p = vctp8q(iter);
      // load up to 16 int8
      uint8x16_t u8x16_val = vld1q_z_u8(pSrc, p);
      pSrc+=16;
      // Compare according to p to create p0
      mve_pred16_t p0 = vcmpcsq_m_u8(u8x16_val, u8x16_max_val, p);

      u8x16_max_val = vpselq_u8(u8x16_val, u8x16_max_val, p0);
      u8x16_max_idx = vpselq_u8(u8x16_idx, u8x16_max_idx, p0);

      u8x16_idx+=16;
      iter-=16;

    }

    maxValueLoc = vmaxvq_u8(0, u8x16_max_val);
    mve_pred16_t p0 = vcmpeqq_n_u8(u8x16_max_val, maxValueLoc);

    u8x16_max_idx = vpselq_u8( u8x16_max_idx, vdupq_n_u8(0xFF), p0);
    /*
     * Get min index which is thus for a max value
    */
    indexLoc = vminvq_p_u8(0xFF, u8x16_max_idx,p0);

    if (maxValueLoc > maxValue)
    {
      maxValue = maxValueLoc;
      index = indexLoc + indexBlk;
    }
    indexBlk += (1<<8);
  }
  *pMaxim = maxValue;
  *pIndex = index;
#else
  *pIndex = 0;
  *pMaxim = arr[0];

  for (uint32_t i = 1; i < len_arr; i++)
  {
    if (arr[i] > *pMaxim)
    {
      *pMaxim = arr[i];
      *pIndex = i;
    }
  }
#endif
}
void vision_models_maxi_p_iu8ou8(uint8_t *arr, uint32_t len_arr, uint32_t offset, uint8_t *maxim, uint8_t *index, uint32_t parallelize)
{
#ifdef VISION_MODELS_MAXI_IU8OU8_MVE
  if (15*offset < UCHAR_MAX) {
    uint8x16_t   u8x16_max_val = vdupq_n_u8(0);
    uint8x16_t   u8x16_max_idx = vdupq_n_u8(0);

    parallelize = MIN(parallelize, 16);
    mve_pred16_t p = vctp8q(parallelize);

    uint8x16_t u8x16_idx = vdupq_n_u8(0);
    uint8x16_t u8x16_offset = vidupq_n_u8(0,1) * (uint8_t)offset;

    for (uint8_t i = 0; i < len_arr; i++)
    {
      // load up to 16 int8
      uint8x16_t u8x16_val = vldrbq_gather_offset_z_u8(arr, u8x16_offset, p);
      arr++;
      // Compare according to p to create p0
      mve_pred16_t p0 = vcmphiq_m_u8(u8x16_val, u8x16_max_val, p);

      // according to p0: update with s8x16_val or keep s8x16_blk_minmax_val
      u8x16_max_val = vpselq_u8(u8x16_val, u8x16_max_val, p0);
      /* according to p0: store per-lane extrema indexes*/
      u8x16_max_idx = vpselq_u8(u8x16_idx, u8x16_max_idx, p0);
      u8x16_idx+=1;

    }
    vstrbq_p_u8((uint8_t *)index,u8x16_max_idx,p);
    vstrbq_p_u8(maxim,u8x16_max_val,p);
  } else {
    uint16_t _tmpIdx[8];
    uint16_t _parallelize = MIN(8,parallelize);
    vision_models_maxi_p_iu8ou16(arr,len_arr,offset,maxim,_tmpIdx,_parallelize);
    maxim+=_parallelize;
    for ( int i = 0; i < _parallelize; i++) {
      *index++ = _tmpIdx[i];
    }
    parallelize-=_parallelize;
    arr+=_parallelize*offset;
    vision_models_maxi_p_iu8ou16(arr,len_arr,offset,maxim,_tmpIdx,parallelize);
    for (uint32_t i = 0; i < parallelize; i++) {
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
    vision_models_maxi_p_iu8ou16(arr,len_arr,offset,maxim,_tmpIdx,_parallelize);
    maxim+=_parallelize;
    for ( uint16_t i = 0; i <_parallelize; i++) {
      *index++ = _tmpIdx[i];
    }
    parallelize-=_parallelize;
    arr+=_parallelize*offset;
    vision_models_maxi_p_iu8ou16(arr,len_arr,offset,maxim,_tmpIdx,parallelize);
    for ( int i = 0; i < parallelize; i++) {
      *index++ = _tmpIdx[i];
    }
  }
#endif
}
void vision_models_maxi_p_iu8ou16(uint8_t *arr, uint32_t len_arr, uint32_t offset, uint8_t *maxim, uint16_t *index, uint32_t parallelize)
{
#ifdef VISION_MODELS_MAXI_P_IU8OU16_MVE
  uint16x8_t   u16x8_max_val = vdupq_n_u16(0);
  uint16x8_t   u16x8_max_idx = vdupq_n_u16(0);

  parallelize = MIN(parallelize, 8);
  mve_pred16_t p = vctp16q(parallelize);

  uint16x8_t u16x8_idx = vdupq_n_u16(0);
  uint16x8_t u16x8_offset = vidupq_n_u16(0,1) * (uint16_t)offset;

  for (uint8_t i = 0; i < len_arr; i++)
  {
    // load up to 16 int8
    uint16x8_t u16x8_val = vldrbq_gather_offset_z_u16(arr, u16x8_offset, p);
    arr++;
    // Compare according to p to create p0
    mve_pred16_t p0 = vcmphiq_m_u16(u16x8_val, u16x8_max_val, p);

    // according to p0: update with s16x8_val or keep s16x8_blk_minmax_val
    u16x8_max_val = vpselq_u16(u16x8_val, u16x8_max_val, p0);
    /* according to p0: store per-lane extrema indexes*/
    u16x8_max_idx = vpselq_u16(u16x8_idx, u16x8_max_idx, p0);
    u16x8_idx+=1;

  }
  vstrhq_p_u16((uint16_t *)index,u16x8_max_idx,p);
  vstrbq_p_u16(maxim,u16x8_max_val,p);
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

void vision_models_maxi_iu8ou32(uint8_t *arr, uint32_t len_arr, uint8_t *maxim, uint32_t *index)
{
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
}

