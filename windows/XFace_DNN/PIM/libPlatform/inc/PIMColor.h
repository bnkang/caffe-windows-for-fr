/*
 * Copyright IM Lab,(http://imlab.postech.ac.kr)
 * 
 * All rights reserved.
 * 
 */

#ifndef __PIMCOLOR_H__
#define __PIMCOLOR_H__

#include "PIMTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef PIM_Uint32 PIM_Rgb;                                        
typedef PIM_Rgb PIM_Color;                                         

#define PIM_Color_KWhite        0x00FFFFFF                           
#define PIM_Color_KBlack        0x00000000                           
#define PIM_Color_KRed          0x00FF0000                           
#define PIM_Color_KGreen        0x0000FF00                           
#define PIM_Color_KBlue         0x000000F0        // 0x0000FF        
#define PIM_Color_KYellow       0x0000FFFF                           
#define PIM_Color_KCyan         0x00FFFF00                           
#define PIM_Color_KOrange       0x00FF8400                           
#define PIM_Color_KNone         0x00123456        // don't draw Color

#define RGB24_TO_GRAY(r, g, b) ((PIM_Uint8)(((r)+(g)+(b))/3))



/**
 *  @brief
 *
 *  @param color [IN] : 
 *
 *  @return
 */
extern PIM_Uint32 PIM_Color_GetYUV(PIM_Color color);

/**
 *  @brief
 *
 *  @param color [IN] : 
 *
 *  @return
 */
extern PIM_Uint32 PIM_Color_GetY(PIM_Color color);

/**
 *  @brief
 *
 *  @param color [IN] : 
 *
 *  @return
 */
extern PIM_Uint32 PIM_Color_GetUV(PIM_Color color);

/**
 *  @brief Convert RGB888 to HSL.
 *
 *  @param xH [OUT] : hue(휘도) [0, 255<<8)
 *  @param xS [OUT] : saturation(포화도) [0, 255<<8]
 *  @param xL [OUT] : lightness(명도) [0, 255<<8]
 *  @param R [IN] : red   [0, 255]
 *  @param G [IN] : green [0, 255]
 *  @param B [IN] : blue  [0, 255]
 */
extern PIM_Void PIM_Color_RGB2HSL(PIM_Int32 *xH, PIM_Int32 *xS, PIM_Int32 *xL, PIM_Uint8 R, PIM_Uint8 G, PIM_Uint8 B);


/**
 *  @brief Convert HSL to RGB888.
 *
 *  @param R [OUT] : red   [0, 255]
 *  @param G [OUT] : green [0, 255]
 *  @param B [OUT] : blue  [0, 255]
 *  @param xH [IN] : hue(휘도) [0, 255<<8). 범위를 벗어나는 경우는 보정함 (circular).
 *  @param xS [IN] : saturation(포화도) [0, 255<<8]. 범위를 벗어나는 경우는 Crop.
 *  @param xL [IN] : lightness(명도) [0, 255<<8]. 범위를 벗어나는 경우 Crop.
 */
extern PIM_Void PIM_Color_HSL2RGB(PIM_Uint8 *R, PIM_Uint8 *G, PIM_Uint8 *B, PIM_Int32 xH, PIM_Int32 xS, PIM_Int32 xL);

/* 아래 알고리즘들은 RGB 이외에서의 정상적인 동작을 보장할 수 없음. */
//#define BLEND_METHOD_DEFINE

#ifndef BLEND_METHOD_DEFINE
extern PIM_Uint8 PIM_Color_BlendColorDodge2(PIM_Uint8 base, PIM_Uint8 color, PIM_Uint8 white);
extern PIM_Uint8 PIM_Color_BlendSoftLight(PIM_Uint8 base, PIM_Uint8 blend);
extern PIM_Uint8 PIM_Color_BlendOverlay (PIM_Uint8 base, PIM_Uint8 blend);
extern PIM_Uint8 PIM_Color_BlendExclusion (PIM_Uint8 base, PIM_Uint8 blend);
extern PIM_Uint8 PIM_Color_BlendScreen(PIM_Uint8 base, PIM_Uint8 blend);
#else
#define PIM_Color_BlendColorDodge2(_base_,_color_,_white_)    \
			 (PIM_Uint8)(((_color_)<255)?PIM_Math_ClipToByte(((PIM_Int32)(_white_)*(_base_))/(255-(_color_))):(255))

#define PIM_Color_BlendSoftLight(_base_, _blend_)    \
			 ((PIM_Uint8)(((_base_) < 128)?((2*(((_blend_)>>1)+64)*(PIM_Int32)(_base_))/255): (255-((2 * (255-(((_blend_)>>1)+64)) * (PIM_Int32)(255-(_base_))) /255))))

#define PIM_Color_BlendOverlay(_base_, _blend_)    \
			 ((PIM_Uint8)(((_base_) < 128) ? (2 * (_blend_) * (PIM_Int32)(_base_) / 255):(255 - 2 * (255 - (_blend_)) * (PIM_Int32)(255 - (_base_)) / 255)))

#define PIM_Color_BlendExclusion(_base_, _blend_)    \
			PIM_Math_ClipToByte((_base_) + (_blend_) - 2 * (_base_) * (_blend_) / 255)

#define PIM_Color_BlendScreen(_base_, _blend_)    \
			PIM_Math_ClipToByte((255 - (((255 - (_base_)) * (255 - (_blend_))) >> 8)))
#endif/* end #ifndef BLEND_METHOD_DEFINE */

#ifdef __cplusplus
}
#endif

#endif /*__PIMCOLOR_H__*/
