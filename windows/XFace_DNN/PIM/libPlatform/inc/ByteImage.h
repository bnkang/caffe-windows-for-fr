/*
 * Copyright IM Lab,(http://imlab.postech.ac.kr)
 * 
 * All rights reserved.
 * 
 */

#ifndef __BYTEIMAGE_H__
#define __BYTEIMAGE_H__

#include "PIMTypes.h"

#ifdef __cplusplus 
extern "C" { 
#endif

/**
 *  @brief 
 */
extern const PIM_Int32 sin_shift10[73];
extern const PIM_Int32 cos_shift10[73];

/**
 *  @brief By rotating "src image" is copied to "dst image".
 *
 *  @param dst [OUT] : a destination image
 *  @param src [IN] : a source image
 *  @param angle [IN] : rotate angle (0' ~ 359')
 *
 *  @return ret == PIM_SUCCESS : success
 *  @return ret <  PIM_SUCCESS : fail, error code.
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Result PIM_ByteImage_RotatePreview   (PIM_Bitmap *dst, PIM_Bitmap *src, PIM_Int32 angle);

/**
 *  @brief By rotating & resizing "src image" is copied to "dst image".
 *
 *  @param dst [OUT] : a destination image
 *  @param src [IN] : a source image
 *  @param angle [IN] : rotate angle (0' ~ 359')
 *
 *  @return ret == PIM_SUCCESS : success
 *  @return ret <  PIM_SUCCESS : fail, error code.
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Result PIM_ByteImage_RotateResize_NN (PIM_Bitmap *dst, PIM_Bitmap *src, PIM_Int32 angle);

/**
 *  @brief By rotating & cropping "src image" is copied to "dst image".
 *
 *  @param dst [OUT] : a destination image
 *  @param src [IN] : a source image
 *  @param crop_area [IN] : 
 *  @param angle [IN] : 
 *
 *  @return ret == PIM_SUCCESS : success
 *  @return ret <  PIM_SUCCESS : fail, error code.
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Result PIM_ByteImage_RotateCrop      (PIM_Bitmap *dst, PIM_Bitmap *src, PIM_Rect *crop_area, PIM_Int32 angle);

/**
 *  @brief By rotating & cropping & resizing "src image" is copied to "dst image".
 *
 *  @param dst [OUT] : a destination image
 *  @param src [IN] : a source image
 *  @param crop_area [IN] : select area
 *  @param angle [IN] : rotate angle (0' ~ 359')
 *  @param ratio_sft10 [IN] : 
 *
 *  @return ret == PIM_SUCCESS : success
 *  @return ret <  PIM_SUCCESS : fail, error code.
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Result PIM_ByteImage_RotateCropResize_NN(PIM_Bitmap *dst, PIM_Bitmap *src, PIM_Rect* crop_area, PIM_Int32 angle, PIM_Int32 ratio_sft10);

/**
 *  @brief it copy the selected areas of the source image to the destination image.
 *
 *  @param dst [OUT] : a destination image
 *  @param src [IN] : a source image
 *  @param crop_area [IN] : select area
 *
 *  @return ret == PIM_SUCCESS : success
 *  @return ret <  PIM_SUCCESS : fail, error code.
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Result PIM_ByteImage_CropMargin_Align    (PIM_Bitmap *dst, PIM_Bitmap *src, PIM_Rect *crop_area);

/**
 *  @brief it copy the selected areas of the source image to the destination image.
 *
 *  @param dst [OUT] : a destination image
 *  @param src [IN] : a source image
 *  @param crop_area [IN] : select area
 *
 *  @return ret == PIM_SUCCESS : success
 *  @return ret <  PIM_SUCCESS : fail, error code.
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Result PIM_ByteImage_CropMargin          (PIM_Bitmap *dst, PIM_Bitmap *src, PIM_Rect *crop_area);

/**
 *  @brief it copy the selected areas of the source image to the destination image.
 *
 *  @param dst [OUT] : a destination image
 *  @param src [IN] : a source image
 *  @param crop_area [IN] : select area
 *
 *  @return ret == PIM_SUCCESS : success
 *  @return ret <  PIM_SUCCESS : fail, error code.
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Result PIM_ByteImage_CropResizeMargin_NN (PIM_Bitmap *dst, PIM_Bitmap *src, PIM_Rect *crop_area);

/**
 *  @brief it copy the selected areas of the source image to the destination image.
 *
 *  @param dst [OUT] : a destination image
 *  @param src [IN] : a source image
 *  @param crop_area [IN] : select area
 *
 *  @return ret == PIM_SUCCESS : success
 *  @return ret <  PIM_SUCCESS : fail, error code.
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Result PIM_ByteImage_CropResize_NN       (PIM_Bitmap *dst, PIM_Bitmap *src, PIM_Rect *crop_area);

/**
 *  @brief �ҽ� �̹����� ũ�⸦ N���� 1�� �ٿ� dst �̹����� �����Ѵ�.
 *
 *  @param dst [OUT] : a destination image
 *  @param src [IN] : a source image
 *  @param N [IN] : 
 *
 *  @return ret == PIM_SUCCESS : success
 *  @return ret <  PIM_SUCCESS : fail, error code.
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Result PIM_ByteImage_Load(PIM_Bitmap *dst, PIM_Bitmap *src, PIM_Int32 N);

/**
 *  @brief By resizing "src image" is copied to "dst image".
 *
 *  @param dst [OUT] : a destination image
 *  @param src [IN] : a source image
 *  @param ratio_nom_h [IN] : 
 *  @param ratio_denom_h [IN] : 
 *  @param ratio_nom_2 [IN] : 
 *  @param ratio_denom_2 [IN] : 
 *
 *  @return ret == PIM_SUCCESS : success
 *  @return ret <  PIM_SUCCESS : fail, error code.
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Result PIM_ByteImage_LoadResizedFromByteStream(PIM_Bitmap *dst, PIM_Bitmap *src, PIM_Int32 ratio_nom_h, PIM_Int32 ratio_denom_h, PIM_Int32 ratio_nom_w, PIM_Int32 ratio_denom_w);

/**
 *  @brief get the brightness of the selected area.
 *
 *  @param bitmap [IN] : a image.
 *  @param crop_area [IN] : select area
 *
 *  @return ret : 0 ~ 255 brightness
 */
extern PIM_Uint32 PIM_ByteImage_GetBrightness(PIM_Bitmap *bitmap, PIM_Rect *crop_area);

#ifdef __cplusplus 
}
#endif

#endif /* __BYTEIMAGE_H__ */

