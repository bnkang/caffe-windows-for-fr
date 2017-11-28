/*
 * Copyright IM Lab,(http://imlab.postech.ac.kr)
 * 
 * All rights reserved.
 * 
 */

#ifndef __PIMPOINT_H__
#define __PIMPOINT_H__

#include "PIMTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  @brief measure the distance between two points.
 *
 *  @param p1 [IN] : point
 *  @param p2 [IN] : point
 *
 *  @return : the destance between two points
 */
extern PIM_Int32 PIM_Point_Dist(const PIM_Point* p1, const PIM_Point* p2);

/**
 *  @brief 'dst point' rotated by this function will be rotated by -angle360 actually.
 *
 *  @param dst [OUT] : a destination point
 *  @param src [IN] : a source point
 *  @param zerop [IN] : Based coordinates
 *  @param angle360 [IN] : rotate angle (0' ~ 359')
 */
extern PIM_Void PIM_Point_Rotate(PIM_Point* dst, PIM_Point *src, PIM_Point *zerop, PIM_Int32 angle360);

/**
 *  @brief move a point
 *
 *  @param dst [IN/OUT] : point
 *  @param offset_x [IN] : Increase or decrease the x-coordinate
 *  @param offset_y [IN] : Increase or decrease the y-coordinate
 */
extern PIM_Void PIM_Point_Move(PIM_Point* dst, PIM_Int32 offset_x, PIM_Int32 offset_y);



#ifdef __cplusplus
}
#endif


#endif /*__PIMPOINT_H__*/
