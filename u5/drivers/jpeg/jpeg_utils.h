/**
 ******************************************************************************
 * @file    jpeg_utils.h
 * @author  MCD Application Team
 * @brief   Header for jpeg_utils.c module
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2016 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __JPEG_UTILS_H
#define __JPEG_UTILS_H

/* Includes ------------------------------------------------------------------*/
#include "jpeg_utils_conf.h"

/** @addtogroup Utilities
 * @{
 */

/** @addtogroup JPEG
 * @{
 */

/** @defgroup JPEG_Exported_Defines JPEG Exported Defines
 * @{
 */
/**
 * @}
 */

#define YCBCR_420_BLOCK_SIZE                                                   \
  384 /* YCbCr 4:2:0 MCU : 4 8x8 blocks of Y + 1 8x8 block of Cb + 1 8x8 block \
         of Cr   */
#define YCBCR_422_BLOCK_SIZE                                                   \
  256 /* YCbCr 4:2:2 MCU : 2 8x8 blocks of Y + 1 8x8 block of Cb + 1 8x8 block \
         of Cr   */
#define YCBCR_444_BLOCK_SIZE                                                   \
  192 /* YCbCr 4:4:4 MCU : 1 8x8 block of Y + 1 8x8 block of Cb + 1 8x8 block  \
         of Cr   */

#define GRAY_444_BLOCK_SIZE 64 /* GrayScale MCU : 1 8x8 block of Y */

#define CMYK_444_BLOCK_SIZE                                                    \
  256 /* CMYK MCU : 1 8x8 blocks of Cyan + 1 8x8 block Magenta + 1 8x8 block   \
         of Yellow and 1 8x8 block of BlacK */

/** @defgroup JPEG_Exported_Types JPEG Exported Types
 * @{
 */
#if (USE_JPEG_DECODER == 1)
typedef uint32_t (*JPEG_YCbCrToRGB_Convert_Function)(
    uint8_t *pInBuffer, uint8_t *pOutBuffer, uint32_t BlockIndex,
    uint32_t DataCount, uint32_t *ConvertedDataCount);
#endif

#if (USE_JPEG_ENCODER == 1)
typedef uint32_t (*JPEG_RGBToYCbCr_Convert_Function)(
    uint8_t *pInBuffer, uint8_t *pOutBuffer, uint32_t BlockIndex,
    uint32_t DataCount, uint32_t *ConvertedDataCount);
#endif
/**
 * @}
 */

/** @defgroup JPEG_Exported_Macros JPEG Exported Macros
 * @{
 */
/**
 * @}
 */

/** @defgroup JPEG_Exported_Variables JPEG Exported Variables
 * @{
 */
/**
 * @}
 */

/** @defgroup JPEG_Exported_FunctionsPrototype JPEG Exported FunctionsPrototype
 * @{
 */
void JPEG_InitColorTables(void);

#if (USE_JPEG_DECODER == 1)
HAL_StatusTypeDef
JPEG_GetDecodeColorConvertFunc(JPEG_ConfTypeDef *pJpegInfo,
                               JPEG_YCbCrToRGB_Convert_Function *pFunction,
                               uint32_t *ImageNbMCUs);
#endif

#if (USE_JPEG_ENCODER == 1)
HAL_StatusTypeDef
JPEG_GetEncodeColorConvertFunc(JPEG_ConfTypeDef *pJpegInfo,
                               JPEG_RGBToYCbCr_Convert_Function *pFunction,
                               uint32_t *ImageNbMCUs);
#endif
/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */
#endif /* __JPEG_UTILS_H */
