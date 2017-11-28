/*
 * Copyright IM Lab,(http://imlab.postech.ac.kr)
 * 
 * All rights reserved.
 * 
 */

#ifndef __PIMMASK_H__
#define __PIMMASK_H__

#include "PIMTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  @brief 
 *
 *  @param mask [IN/OUT] : 
 *  @param width [IN] : 
 *  @param height [IN] : 
 *  @param intensity [IN] : 
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret == PIM_SUCCESS : success
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Result PIM_Mask_CreateCircle(PIM_Bitmap* mask, PIM_Int32 width, PIM_Int32 height, PIM_Int32 intensity);

extern PIM_Result PIM_Mask_CreateSolidCircle(PIM_Bitmap* mask, PIM_Int32 width, PIM_Int32 height);

/**
 *  @brief 
 */
extern PIM_Void PIM_Mask_Clear();

#ifdef __cplusplus
}
#endif

#endif /*__PIMMASK_H__*/
