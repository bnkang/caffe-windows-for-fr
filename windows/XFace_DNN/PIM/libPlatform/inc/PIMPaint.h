/*
 * Copyright IM Lab,(http://imlab.postech.ac.kr)
 * 
 * All rights reserved.
 * 
 */

#ifndef __PIMPAINT_H__
#define __PIMPAINT_H__

#include "PIMTypes.h"
#include "PIMColor.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  @brief draw a point
 *
 *  @param bitmap [IN/OUT] : image
 *  @param x [IN] : x-coordinate
 *  @param y [IN] : x-coordinate
 *  @param color [IN] : line color
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret == PIM_SUCCESS : success
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Result PIM_Paint_DrawPoint(PIM_Bitmap *bitmap, PIM_Int32 x, PIM_Int32 y, PIM_Color color);

/**
 *  @brief draw a hollow rectangle
 *
 *  @param bitmap [IN/OUT] : image
 *  @param rect [IN] : selected area
 *  @param color [IN] : line color
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret == PIM_SUCCESS : success
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Result PIM_Paint_DrawRect(PIM_Bitmap *bitmap, PIM_Rect *rect, PIM_Color color);

/**
 *  @brief draw a string
 *
 *  @param bitmap [IN/OUT] : image
 *  @param pos [IN] : the position of the drawing string
 *  @param str [IN] : string
 *  @param chcolor [IN] : string color
 *  @param fontsize [IN] : font size. (Not available)
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret == PIM_SUCCESS : success
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Result PIM_Paint_DrawString(PIM_Bitmap *bitmap, const PIM_Point *pos, const PIM_String str, PIM_Color chcolor, PIM_Size *fontsize);

/**
 *  @brief draw a vertical line
 *
 *  @param bitmap [IN/OUT] : image
 *  @param x_fix [IN] : x-coordinate
 *  @param y_start [IN] : y-coordinate
 *  @param y_end [IN] : y-coordinate
 *  @param color [IN] : line color
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret == PIM_SUCCESS : success
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Result PIM_Paint_DrawVLine(PIM_Bitmap *bitmap, PIM_Int32 x_fix, PIM_Int32 y_start, PIM_Int32 y_end, PIM_Color color);

/**
 *  @brief draw a horizontal line.
 *
 *  @param bitmap [IN/OUT] : image
 *  @param x_start [IN] : x-coordinate
 *  @param y_fix [IN] : y-coordinate
 *  @param x_end [IN] : x-coordinate
 *  @param color [IN] : line color
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret == PIM_SUCCESS : success
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Result PIM_Paint_DrawHLine(PIM_Bitmap *bitmap, PIM_Int32 x_start, PIM_Int32 y_fix, PIM_Int32 x_end, PIM_Color color);

/**
 *  @brief draw line. (Including the diagonal)
 *
 *  @param bitmap [IN/OUT] : image
 *  @param x_start [IN] : x-coordinate
 *  @param y_start [IN] : y-coordinate
 *  @param x_end [IN] : x-coordinate
 *  @param y_end [IN] : y-coordinate
 *  @param color [IN] : line color
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret == PIM_SUCCESS : success
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Result PIM_Paint_DrawLine(PIM_Bitmap *bitmap, PIM_Int32 x_start, PIM_Int32 y_start, PIM_Int32 x_end, PIM_Int32 y_end, PIM_Color color);

/**
 *  @brief draw the rectangle in filled
 *
 *  @param bitmap [IN/OUT] : image
 *  @param rect [IN] : selected area
 *  @param color [IN] : rect color
 *  @param transparency [IN] : 
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret == PIM_SUCCESS : success
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Result PIM_Paint_FillRect(PIM_Bitmap *bitmap, PIM_Rect *rect, PIM_Color color, PIM_Int32 transparency);

/**
 *  @brief draw the source image on the destination image.
 *
 *  @param dst [OUT] : a destination image
 *  @param src [IN] : a source image
 *  @param pos [IN] : 
 *  @param size [IN] : 
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret == PIM_SUCCESS : success
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Result PIM_Paint_DrawBitmap(PIM_Bitmap *dst, const PIM_Bitmap *src, PIM_Point *pos, PIM_Size *size);

/**
 *  @brief draw the source image on the destination image.
 *
 *  @param layee [OUT] : a destination image
 *  @param layer [IN] : a source image
 *  @param x [IN] : x-coordinate
 *  @param y [IN] : y-coordinate
 *  @param transparency [IN] : 
 *  @param transparent_color [IN] : 
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret == PIM_SUCCESS : success
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Result PIM_Paint_Overlay(PIM_Bitmap* layee, PIM_Bitmap* layer, PIM_Int32 x, PIM_Int32 y, PIM_Int32 transparency, PIM_Color transparent_color);

/**
 *  @brief draw some of the source image on the destination image.
 *
 *  @param dst [OUT] : a destination image
 *  @param src [IN] : a source image
 *  @param x [IN] : x-coordinate
 *  @param y [IN] : y-coordinate
 *  @param isCircle [IN] : 
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret == PIM_SUCCESS : success
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Result PIM_Paint_MaskingOverlay(PIM_Bitmap* dst, PIM_Bitmap* src, int x, int y, PIM_Bool isCircle);


#if 0
extern PIM_Int32 PIM_Paint_DrawFillRect(PIM_Bitmap *This, PIM_Rect *rect, PIM_Color color, PIM_Int32 transparency);
extern PIM_Int32 PIM_Paint_DrawLine(PIM_Bitmap *This, PIM_Int32 x_start, PIM_Int32 y_start, PIM_Int32 x_end, PIM_Int32 y_end, PIM_Color color);
extern PIM_Int32 PIM_Paint_DrawBitmap(PIM_Bitmap *This, const PIM_Bitmap *bitmap, PIM_Point *pos, PIM_Size *size);
extern PIM_Int32 PIM_Paint_DrawString(PIM_Bitmap *This, const PIM_Point *pos, const PIM_String str, PIM_Color chcolor, PIM_Size *fontsize);
extern PIM_Int32 PIM_Paint_Overlay(PIM_Bitmap* layee, PIM_Bitmap* layer, PIM_Int32 x, PIM_Int32 y, PIM_Int32 transparency, PIM_Color transparent_color);
#endif

#ifdef __cplusplus
}
#endif

#endif /*__PIMPAINT_H__*/
