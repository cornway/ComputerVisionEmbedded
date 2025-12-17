/*---------------------------------------------------------------------------------------------
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file in
 * the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *--------------------------------------------------------------------------------------------*/

#include "vision_models_pp.h"



float32_t vision_models_sigmoid_f(float32_t x)
{
  return (1.0f / (1.0f + expf(-x)));
}


void vision_models_softmax_f(float32_t *input_x, float32_t *output_x, int32_t len_x, float32_t *tmp_x)
{
  float32_t sum = 0;

  for (int32_t i = 0; i < len_x; ++i)
  {
    tmp_x[i] = expf(input_x[i]);
    sum = sum + tmp_x[i];
  }
  sum = 1.0f / sum;
  for (int32_t i = 0; i < len_x; ++i)
  {
    tmp_x[i] *= sum;
  }
  memcpy(output_x, tmp_x, len_x * sizeof(float32_t));
}


//***************iou ********
//inline
float32_t overlap(float32_t x1, float32_t w1, float32_t x2, float32_t w2)
{
  float32_t l1 = x1 - w1 / 2;
  float32_t l2 = x2 - w2 / 2;
  float32_t left = l1 > l2 ? l1 : l2;
  float32_t r1 = x1 + w1 / 2;
  float32_t r2 = x2 + w2 / 2;
  float32_t right = r1 < r2 ? r1 : r2;
  return (right - left);
}

//inline
float32_t box_intersection(float32_t *a, float32_t *b)
{
  float32_t w = overlap(a[0], a[2], b[0], b[2]);
  float32_t h = overlap(a[1], a[3], b[1], b[3]);
  if (w < 0 || h < 0) return 0;
  float32_t area = w * h;
  return (area);
}

//inline
float32_t box_union(float32_t *a, float32_t *b)
{
  float32_t i = box_intersection(a, b);
  float32_t u = a[2] * a[3] + b[2] * b[3] - i;
  return (u);
}


float32_t vision_models_box_iou(float32_t *a, float32_t *b)
{
  float32_t I = box_intersection(a, b);
  float32_t U = box_union(a, b);
 if (I == 0 || U == 0)
  {
    return 0;
  }
  return (I / U);
}

int32_t twice_overlap_int(int32_t x1, int32_t w1, int32_t x2, int32_t w2)
{
  int32_t l1 = x1 * 2 - w1 ;
  int32_t l2 = x2 * 2 - w2;
  int32_t left = l1 > l2 ? l1 : l2;
  int32_t r1 = x1 * 2 + w1;
  int32_t r2 = x2 * 2 + w2;
  int32_t right = r1 < r2 ? r1 : r2;
  return (right - left);
}

//inline
int32_t box_intersection_is8(int32_t *a, int32_t *b)
{
  int32_t w = twice_overlap_int(a[0], a[2], b[0], b[2]);
  int32_t h = twice_overlap_int(a[1], a[3], b[1], b[3]);
  if (w < 0 || h < 0) return 0;
  int32_t area = w * h;
  return (area);
}

//inline
int32_t box_union_is8(int32_t *a, int32_t *b)
{
  int32_t i = box_intersection_is8(a, b);
  int32_t u = 4* (a[2] * a[3] + b[2] * b[3]) - i;
  return (u);
}


float32_t vision_models_box_iou_is8(int8_t *a, int8_t *b, int8_t zp)
{
  int32_t a_z[4] = {a[0]-zp, a[1]-zp, a[2]-zp, a[3]-zp};
  int32_t b_z[4] = {b[0]-zp, b[1]-zp, b[2]-zp, b[3]-zp};
  int32_t I = box_intersection_is8(a_z, b_z);
  int32_t U = box_union_is8(a_z, b_z);
  if (I == 0 || U == 0)
  {
    return 0;
  }
  float32_t ret = ((float32_t)I / (float32_t)U);
  return ret;
}
void transpose_flattened_2D(float32_t *arr, int32_t rows, int32_t cols, float32_t *tmp_x)
{
  int32_t i, j, k;
  int32_t len_r_c = rows * cols;

  for (i = 0; i < len_r_c; i++)
  {
    tmp_x[i] = arr[i];
  }

  for (i = 0; i < cols; i++)
  {
    for (j = 0; j < rows; j++)
    {
      k = i * rows + j;
      arr[k] = tmp_x[j * cols + i];
    }
  }
}


void dequantize(int32_t* arr, float32_t* tmp, int32_t n, int32_t zero_point, float32_t scale)
{
  for (int32_t i = 0; i < n; i++)
  {
    tmp[i] = (float32_t)(arr[i] - zero_point) * scale;
  }
}

