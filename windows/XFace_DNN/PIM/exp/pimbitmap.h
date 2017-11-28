/**
 *	@file PIMBitmap.h
 *
 *  @brief  Declares PIMBitmap inteface.
 *
 *
 *  Copyright IM Lab,(http://imlab.postech.ac.kr)
 *
 *  All rights reserved.
 */

#ifndef __PIMBITMAP_H__
#define __PIMBITMAP_H__

#include "PIMTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  @brief Create bitmap.
 *
 *  @param bitmap    [OUT] : bitmap handle.
 *  @param imageData  [IN] : external image data.
 *  @param width      [IN] : image width.
 *  @param height     [IN] : image height.
 *  @param stride     [IN] : The number of bytes in "imageData" to skip between rows
 *  @param format     [IN] : image format.
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret == PIM_SUCCESS : success
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Result PIM_Bitmap_CreateRef(PIM_Bitmap *bitmap, PIM_Uint8 *imageData, PIM_Int32 width, PIM_Int32 height, PIM_Int32 stride, PIM_ImageFormat format);

/**
 *  @brief Create bitmap.
 *
 *  @param bitmap    [OUT] : bitmap handle.
 *  @param width      [IN] : image width.
 *  @param height     [IN] : image height.
 *  @param format     [IN] : image format.
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret == PIM_SUCCESS : success
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Result PIM_Bitmap_Create(PIM_Bitmap *bitmap, PIM_Int32 width, PIM_Int32 height, PIM_ImageFormat format);

/**
 *  @brief Destroy bitmap.
 *
 *  @param bitmap    [IN/OUT] : bitmap handle.
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret == PIM_SUCCESS : success
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Result PIM_Bitmap_Destroy(PIM_Bitmap *bitmap);

/**
 *  @brief Copy bitmap.
 *
 *  @param dst [IN/OUT] : bitmap handle.
 *  @param src     [IN] : bitmap handle.
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret == PIM_SUCCESS : success
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Result PIM_Bitmap_Copy(PIM_Bitmap *dst, const PIM_Bitmap *src);

/**
 *  @brief Clone bitmap.
 *
 *  @param dst    [OUT] : bitmap handle.
 *  @param src     [IN] : bitmap handle.
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret == PIM_SUCCESS : success
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Result PIM_Bitmap_Clone(PIM_Bitmap *dst, const PIM_Bitmap *src);

#ifdef __cplusplus
}
#endif

#endif /*__PIMBITMAP_H__*/
