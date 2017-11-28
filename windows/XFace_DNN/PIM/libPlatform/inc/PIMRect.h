/*
 * Copyright IM Lab,(http://imlab.postech.ac.kr)
 * 
 * All rights reserved.
 * 
 */

#ifndef __PIMRECT_H__
#define __PIMRECT_H__

#include "PIMTypes.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 *  @brief set rect.
 *
 *  @param rect [OUT] : a destination rectangle
 *  @param left [IN] : left position of the Rectangular
 *  @param top [IN] : top  position of the Rectangular
 *  @param right [IN] : right position of the Rectangular
 *  @param bottom [IN] : bottom position of the Rectangular
 */
extern PIM_Void   PIM_Rect_SetRect(PIM_Rect* rect, PIM_Int32 left, PIM_Int32 top, PIM_Int32 right, PIM_Int32 bottom); 

/**
 *  @brief get width of rectangle
 *
 *  @param rect [IN] : rectangle
 *
 *  @return width of rectangle
 */
extern PIM_Int32  PIM_Rect_GetWidth(const PIM_Rect *rect);

/**
 *  @brief get height of rectangle
 *
 *  @param rect [IN] : rectangle
 *
 *  @return height of rectangle
 */
extern PIM_Int32  PIM_Rect_GetHeight(const PIM_Rect *rect);

/**
 *  @brief get size of the rectangle area
 *
 *  @param rect [IN] : rectangle
 *
 *  @return size of the rectangle area
 */
extern PIM_Int32  PIM_Rect_GetArea(const PIM_Rect *rect);

/**
 *  @brief set the rectangle based on the point position
 *
 *  @param rect [OUT] : rectangle
 *  @param sp [IN] : Based coordinates
 *  @param wh [IN] : one side length of the rectangle
 */
extern PIM_Void PIM_Rect_SetCenterPoint(PIM_Rect *dr, const PIM_Point* sp, PIM_Int32 wh);

/**
 *  @brief get the coordinates of the rectangle
 *
 *  @param dp [OUT] : the coordinates of the rectangle
 *  @param sr [IN] : rectangle
 */
extern PIM_Void PIM_Rect_GetCenterPoint(PIM_Point *dp, const PIM_Rect *sr);

/**
 *  @brief check the coordinates are included in the rectangular area
 *
 *  @param x [IN] : x-coordinate
 *  @param y [IN] : y-coordinate
 *  @param rect [IN] : based area
 *
 *  @return If included, return value is PIM_TRUE. otherwise return value is PIM_FALSE
 */
extern PIM_Bool PIM_Rect_IsInRect(const PIM_Int32 x, const PIM_Int32 y, const PIM_Rect* rect);

/**
 *  @brief check the 'rect' are included in the 'base_rect' area
 *
 *  @param base_rect [IN] : based area
 *  @param rect [IN] : rectangle
 *
 *  @return If overlapped , the return value is PIM_TRUE. otherwise the return value is PIM_FALSE
 */
extern PIM_Bool PIM_Rect_Intersects(const PIM_Rect *base_rect, const PIM_Rect *rect);

/**
 *  @brief get the area contains both 'rect1' and 'rect2'
 *
 *  @param result [OUT] : result rectangle
 *  @param rect1 [IN] : rectangle
 *  @param rect2 [IN] : rectangle
 */
extern PIM_Void PIM_Rect_GetIntersectionRect(PIM_Rect *result, const PIM_Rect *rect1, const PIM_Rect *rect2);

/**
 *  @brief get the size of the overlapping area
 *
 *  @param rect1 [IN] : rectangle
 *  @param rect2 [IN] : rectangle
 *
 *  @return the size of the overlapping area
 */
extern PIM_Int32 PIM_Rect_GetIntersectionArea(const PIM_Rect *rect1, const PIM_Rect *rect2);

/**
 *  @brief get the rectangles that contain both 'rect1' and 'rect2'.
 *
 *  @param result [OUT] : result rectangle
 *  @param rect1 [IN] : rectangle
 *  @param rect2 [IN] : rectangle
 */
extern PIM_Void PIM_Rect_GetBoundingRect(PIM_Rect *result, const PIM_Rect *rect1, const PIM_Rect *rect2);

/**
 *  @brief a portion of the area to remove outside
 *
 *  @param rect [IN/OUT] : result rectangle
 *  @param bound [IN] : bound rectangle
 */
extern PIM_Void PIM_Rect_ClipToBound(PIM_Rect *rect, const PIM_Rect *bound);

/**
 *  @brief 
 *
 *  @param dst [OUT] : a destination rectangle
 *  @param src [IN] : a source rectangle
 *  @param cp [IN] : based coordinate
 *  @param angle [IN] : rotate angle (0' ~ 359')
 */
extern PIM_Void PIM_Rect_Rotate(PIM_Rect* dst, PIM_Rect* src, PIM_Point* cp, PIM_Int32 angle360);

/**
 *  @brief the rectangle is adjusted to fit the percentage.
 *
 *  @param dst [OUT] : a destination rectangle
 *  @param src [IN] : a source rectangle
 *  @param percent [IN] : ratio (default : 100)
 */
extern PIM_Void PIM_Rect_Margin(PIM_Rect* dst, PIM_Rect* src, PIM_Int32 percent);

/**
 *  @brief move a rectangle
 *
 *  @param dst [OUT] : a destination rectangle
 *  @param offset_x [IN] : Increase or decrease the x-coordinate
 *  @param offset_y [IN] : Increase or decrease the y-coordinate
 */
extern PIM_Void PIM_Rect_Move(PIM_Rect* dst, PIM_Int32 offset_x, PIM_Int32 offset_y);



#ifdef __cplusplus
}
#endif


#endif /*__PIMRECT_H__*/

