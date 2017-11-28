/*
 * Copyright IM Lab,(http://imlab.postech.ac.kr)
 * 
 * All rights reserved.
 * 
 */

#ifndef __PIMBITMAPCONVERT_H__
#define __PIMBITMAPCONVERT_H__

#include "PIMTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  @brief Convert one format to another. Typically used
 *  @note dst's buffers should be allocated by the caller.
 *
 *  @param dst [IN] : a destination image
 *  @param src [IN] : a source image
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret == PIM_SUCCESS : success
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Result PIM_BitmapConvert(PIM_Bitmap *dst, PIM_Bitmap *src);

/*
 * supported format
 *   @ src                        @ dst
 *   PIM_IMAGE_FORMAT_GRAY   =>   PIM_IMAGE_FORMAT_RGB888        
 *                                PIM_IMAGE_FORMAT_BGR888
 *                                PIM_IMAGE_FORMAT_RGB8880
 *                                PIM_IMAGE_FORMAT_BGR8880
 */
extern PIM_Result PIM_BitmapConvert_GrayToRgb(PIM_Bitmap* dst, PIM_Bitmap* src);

/*
 * supported format
 *   @ src                           @ dst
 *   PIM_IMAGE_FORMAT_RGB888    =>   PIM_IMAGE_FORMAT_GRAY       
 *   PIM_IMAGE_FORMAT_BGR888                             
 *   PIM_IMAGE_FORMAT_RGB8880                            
 *   PIM_IMAGE_FORMAT_BGR8880                            
 */
extern PIM_Result PIM_BitmapConvert_RgbToGray(PIM_Bitmap* dst, PIM_Bitmap* src);

/*
 * supported format
 *   @ src                           @ dst
 *   PIM_IMAGE_FORMAT_RGB888    =>   PIM_IMAGE_FORMAT_RGB888
 *   PIM_IMAGE_FORMAT_BGR888         PIM_IMAGE_FORMAT_BGR888
 *   PIM_IMAGE_FORMAT_RGB8880        PIM_IMAGE_FORMAT_RGB8880
 *   PIM_IMAGE_FORMAT_BGR8880        PIM_IMAGE_FORMAT_BGR8880
 */
extern PIM_Result PIM_BitmapConvert_RgbToRgb(PIM_Bitmap *dst, PIM_Bitmap *src);

/*
 * supported format
 *   @ src                           @ dst
 *   PIM_IMAGE_FORMAT_RGB888    =>   PIM_IMAGE_FORMAT_YUVPLANAR_NV12
 *   PIM_IMAGE_FORMAT_BGR888         PIM_IMAGE_FORMAT_YUVPLANAR_NV21_SPLIT
 *   PIM_IMAGE_FORMAT_RGB8880        PIM_IMAGE_FORMAT_YUVPLANAR_NV12
 *   PIM_IMAGE_FORMAT_BGR8880        PIM_IMAGE_FORMAT_YUVPLANAR_NV21_SPLIT
 */
extern PIM_Result PIM_BitmapConvert_RgbToYuvPlanar(PIM_Bitmap* dst, PIM_Bitmap* src);

/*
 * supported format
 *   @ src                           @ dst
 *   PIM_IMAGE_FORMAT_RGB888    =>   PIM_IMAGE_FORMAT_YUVPACKED_Y411       
 *   PIM_IMAGE_FORMAT_BGR888                             
 *   PIM_IMAGE_FORMAT_RGB8880                            
 *   PIM_IMAGE_FORMAT_BGR8880                            
 */
extern PIM_Result PIM_BitmapConvert_RgbToYuvPackedY411(PIM_Bitmap* dst, PIM_Bitmap* src);

/*
 * supported format
 *   @ src                                        @ dst
 *   PIM_IMAGE_FORMAT_YUVPLANAR_NV12         =>   PIM_IMAGE_FORMAT_GRAY
 *   PIM_IMAGE_FORMAT_YUVPLANAR_NV21_SPLIT        
 *   PIM_IMAGE_FORMAT_YUVPLANAR_NV12              
 *   PIM_IMAGE_FORMAT_YUVPLANAR_NV21_SPLIT        
 */
extern PIM_Result PIM_BitmapConvert_YuvPlanarToGray(PIM_Bitmap* dst, PIM_Bitmap* src);

/*
 * supported format
 *   @ src                                        @ dst
 *   PIM_IMAGE_FORMAT_YUVPLANAR_NV12         =>   PIM_IMAGE_FORMAT_RGB888 
 *   PIM_IMAGE_FORMAT_YUVPLANAR_NV21_SPLIT        PIM_IMAGE_FORMAT_BGR888 
 *   PIM_IMAGE_FORMAT_YUVPLANAR_NV12              PIM_IMAGE_FORMAT_RGB8880
 *   PIM_IMAGE_FORMAT_YUVPLANAR_NV21_SPLIT        PIM_IMAGE_FORMAT_BGR8880
 */
extern PIM_Result PIM_BitmapConvert_YuvPlanarToRgb(PIM_Bitmap* dst, PIM_Bitmap* src);

extern PIM_Result PIM_BitmapConvert_YuvPlanarToYuvPlanar(PIM_Bitmap* dst, PIM_Bitmap* src);

/*
 * supported format
 *   @ src                                  @ dst
 *   PIM_IMAGE_FORMAT_YUVPACKED_Y411   =>   PIM_IMAGE_FORMAT_GRAY 
 */
extern PIM_Result PIM_BitmapConvert_YuvPackedY411ToGray(PIM_Bitmap* dst, PIM_Bitmap* src);

/*
 * supported format
 *   @ src                                  @ dst
 *   PIM_IMAGE_FORMAT_YUVPACKED_Y411   =>   PIM_IMAGE_FORMAT_RGB888 
 *                                          PIM_IMAGE_FORMAT_BGR888 
 *                                          PIM_IMAGE_FORMAT_RGB8880
 *                                          PIM_IMAGE_FORMAT_BGR8880
 */
extern PIM_Result PIM_BitmapConvert_YuvPackedY411ToRgb(PIM_Bitmap* dst, PIM_Bitmap* src);



#ifdef __cplusplus
}
#endif

#endif /*__PIMBITMAPCONVERT_H__*/
