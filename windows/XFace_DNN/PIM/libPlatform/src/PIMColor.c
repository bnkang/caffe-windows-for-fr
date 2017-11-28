#include "PIMTypes.h"
#include "libPlatform/inc/PIMBitmap.h"
#include "libPlatform/inc/PIMColor.h"
#include "libPlatform/inc/PIMMath.h"
#include "libPlatform/inc/PIMMemory.h"

#include "libPlatform/inc/PIMLog.h"
#define TAG "PIM"



/***************************************************************************************************
 * 
 **************************************************************************************************/
static PIM_Uint32 PIM_Color_ConvertHueToRgb(PIM_Int32 M1, PIM_Int32 M2, PIM_Int32 Hue, PIM_Int32* Channel);

// Returns RGB565 format of the given color as packed to two bytes' format.
PIM_Uint32 PIM_Color_GetRgb565(PIM_Color c)
{
    return ((c >> 8) & 0xF800) | ((c >> 5) & 0x07E0) | ((c >> 3) & 0x1F);
}

// Returns YUV format of the given color as '0YUV' format.
PIM_Uint32 PIM_Color_GetYUV(PIM_Color c)
{
    PIM_Uint32 y, uv;

    y = PIM_Color_GetY(c);
    uv = PIM_Color_GetUV(c);

    return (y << 16) | uv;
}

// Returns only Y
PIM_Uint32 PIM_Color_GetY(PIM_Color c)
{
    PIM_Uint32 y;
    y = ((66*((c >> 16) & 0xFF) + 128*((c >> 8) & 0xFF) +  25*(c & 0xFF) + 128)>>8) + 16;

    return y;
}

// Returns UV as '00UV' format.
PIM_Uint32 PIM_Color_GetUV(PIM_Color c)
{
    PIM_Uint32 u, v;
    u = (-38*((c >> 16) & 0xFF) -  74*((c >> 8) & 0xFF) + 112*(c & 0xFF) + 128 + 32768)>>8;
    v = (112*((c >> 16) & 0xFF) -  94*((c >> 8) & 0xFF) -  18*(c & 0xFF) + 128 + 32768)>>8;

    return (u << 8) | v;
}



PIM_Uint32 PIM_Color_ConvertBgr888ToHls(PIM_Int32* Hue, PIM_Int32* Lumination, PIM_Int32* Saturation,
                                        PIM_Uint8 Blue, PIM_Uint8 Green, PIM_Uint8 Red)
{
    PIM_Int32 Delta;
    PIM_Int32 Max, Min;
    PIM_Int32 Redf, Greenf, Bluef;
    PIM_Int32 HueT;
     
    Redf    = Red   << 8;
    Greenf  = Green << 8;
    Bluef   = Blue  << 8; 
     
    Max     = PIM_Math_Max(PIM_Math_Max(Redf, Greenf), Bluef);
    Min     = PIM_Math_Min(PIM_Math_Min(Redf, Greenf), Bluef);
     
    *Hue        = 0;
    *Lumination = (Max + Min)>>1;
    *Saturation = 0;
     
    if (Max == Min)
        return PIM_SUCCESS;
     
    Delta = (Max - Min);
     
    if (*Lumination < 32640)    *Saturation = (Delta<<8) / ((Max + Min)>>8);
    else                        *Saturation = (Delta<<8) / ((130560 - Max - Min)>>8);
     
    if (Redf == Max)           HueT = ((Greenf - Bluef)<<8) / (Delta>>8);
    else if (Greenf == Max)    HueT = 130560 + ((Bluef - Redf)<<8) / (Delta>>8);
    else                       HueT = 261120 + ((Redf - Greenf)<<8) / (Delta>>8);

    HueT /= 6;
    if(HueT < 0) HueT += 65280;
    *Hue = HueT;
    return PIM_SUCCESS;

}

PIM_Uint32 PIM_Color_ConvertHlsToBgr888(PIM_Uint8* Red, PIM_Uint8* Green, PIM_Uint8* Blue,
                                        PIM_Int32 Hue, PIM_Int32 Lumination, PIM_Int32 Saturation)
{
    PIM_Int32 M1, M2;
    PIM_Int32 Redf, Greenf, Bluef;

    if(Saturation>65280)    Saturation = 65280;
    else if (Saturation<0)    Saturation = 0;
     
    if (Saturation == 0) {
        Redf    = Lumination;
        Greenf  = Lumination;
        Bluef   = Lumination;
    } else {
        if (Lumination <= 32640)
            M2 = ((Lumination>>4) * ((65280 + Saturation)>>4))/255;
        else
            M2 = Lumination + Saturation - ((Lumination>>4) * (Saturation>>4))/255;

        M1 = (2 * Lumination - M2);

        PIM_Color_ConvertHueToRgb(M1, M2, Hue + 21760, &Redf);
        PIM_Color_ConvertHueToRgb(M1, M2, Hue, &Greenf);
        PIM_Color_ConvertHueToRgb(M1, M2, Hue - 21760, &Bluef);
    }
     
    *Red    = (Redf>>8)&0xff;
    *Blue   = (Bluef>>8)&0xff;
    *Green  = (Greenf>>8)&0xff;
    
    return PIM_SUCCESS;
}



/**
  *   Only Used in PIM_Color_ConvertHlsToBgr888()
  */
PIM_Uint32 PIM_Color_ConvertHueToRgb(PIM_Int32 M1, PIM_Int32 M2, PIM_Int32 Hue, PIM_Int32* Channel)
{
    if (Hue < 0)            Hue += 65280;
    else if (Hue > 65280)   Hue -= 65280;
     
    if ((6 * Hue) < 65280)          *Channel = (M1 + ((M2 - M1) * Hue * 6)/65280);
    else if ((2 * Hue) < 65280)     *Channel = (M2);
    else if ((3 * Hue) < 130560)    *Channel = (M1 + ((M2 - M1) * (43520 - Hue) * 6)/65280);
    else                            *Channel = (M1);
    return PIM_SUCCESS;
}



/** Convert RGB888 to HSL.
 *
 * @param xH [out] hue(휘도) [0, 255<<8)
 * @param xS [out] saturation(포화도) [0, 255<<8]
 * @param xL [out] lightness(명도) [0, 255<<8]
 * @param R [in] red   [0, 255]
 * @param G [in] green [0, 255]
 * @param B [in] blue  [0, 255]
 */
void PIM_Color_RGB2HSL(PIM_Int32 *xH, PIM_Int32 *xS, PIM_Int32 *xL, PIM_Uint8 R, PIM_Uint8 G, PIM_Uint8 B)
{
    PIM_Int32 nMax, nMin;
    PIM_Int32 nC, xC, nS;
    PIM_Int32 xHueT;

	nMax = PIM_Math_Max(PIM_Math_Max(R, G), B);
	nMin = PIM_Math_Min(PIM_Math_Min(R, G), B);
	nS = nMax + nMin;

    *xL = (nS << 7);

    if (nMax == nMin)
    {
        *xH = 0;
        *xS = 0;

        return;
    }

	nC = nMax-nMin;
    xC = (nC<<16);

    if (*xL < 32640)    *xS = (xC) / (nS);
    else                *xS = (xC) / (510 - nS);

    if (R == nMax)         xHueT = ((G - B)<<16) / (nC);
    else if (G == nMax)    xHueT = 130560 + ((B - R)<<16) / (nC);
    else                   xHueT = 261120 + ((R - G)<<16) / (nC);

    xHueT /= 6;
    if(xHueT < 0) xHueT += 65280;

    *xH = xHueT;
}



/** Convert HSL to RGB888.
 *
 * @param R [out] red   [0, 255]
 * @param G [out] green [0, 255]
 * @param B [out] blue  [0, 255]
 * @param xH [in] hue(휘도) [0, 255<<8). 범위를 벗어나는 경우는 보정함 (circular).
 * @param xS [in] saturation(포화도) [0, 255<<8]. 범위를 벗어나는 경우는 Crop.
 * @param xL [in] lightness(명도) [0, 255<<8]. 범위를 벗어나는 경우 Crop.
 */
void PIM_Color_HSL2RGB(PIM_Uint8 *R, PIM_Uint8 *G, PIM_Uint8 *B, PIM_Int32 xH, PIM_Int32 xS, PIM_Int32 xL)
{
    PIM_Int32 M1, M2, M3;
    PIM_Int32 xR, xG, xB;
    PIM_Int32 xHueT;

    if(xS>65280)    xS = 65280;
    else if (xS<0)    xS = 0;

    if(xL>65280)    xL = 65280;
    else if (xL<0)    xL = 0;

    if (xS == 0) {
		*R  = (xL>>8)&0xff;
		*G  = *R ;
		*B  = *R ;
		return;
    } 
	
    if (xL <= 32640)
        M2 = ((xL>>4) * ((65280 + xS)>>4))/255;
    else
        M2 = xL + xS - ((xL>>4) * (xS>>4))/255;

    M1 = (2 * xL - M2);
	M3 = M2-M1;

	while (xH < 0)        xH += 65280;
	while (xH > 65280)    xH -= 65280;
   
	xHueT = (xH > 43520)?(xH-43520):(xH + 21760);
	if ((xHueT) < 10880)         xR = (M1 + (M3 * xHueT)/10880);
	else if ((xHueT) < 32640)    xR = (M2);
	else if ((xHueT) < 43520)    xR = (M1 + (M3 * (43520 - xHueT))/10880);
	else                         xR = (M1);

	xHueT = (xH); 
	if ((xHueT) < 10880)         xG = (M1 + (M3 * xHueT)/10880);
	else if ((xHueT) < 32640)    xG = (M2);
	else if ((xHueT) < 43520)    xG = (M1 + (M3 * (43520 - xHueT))/10880);
	else                         xG = (M1);

	xHueT = (xH < 21760)?(xH+43520):(xH - 21760);
	if ((xHueT) < 10880)         xB = (M1 + (M3 * xHueT)/10880);
	else if ((xHueT) < 32640)    xB = (M2);
	else if ((xHueT) < 43520)	 xB = (M1 + (M3 * (43520 - xHueT))/10880);
	else                         xB = (M1);

    *R  = (xR>>8)&0xff;
    *G  = (xG>>8)&0xff;
    *B  = (xB>>8)&0xff;
}


/**
 *    color dodge mode로 블렌딩 하되, 노출 보정(밝기 보정) 효과를 얻을 수 있는 parameter를 추가하였음.
 *
 *    @param base[in] base color to blend. [0,255]
 *    @param color[in] color to blend. [0,255]
 *    @param white [in] white value. (0,255] // 낮을 수록 노출을 낮추는 효과가 있음. 0이면 값을 정의할 수 없음.
 *    @param width[in] 
 */
#ifndef BLEND_METHOD_DEFINE
PIM_Uint8 PIM_Color_BlendColorDodge2(PIM_Uint8 base, PIM_Uint8 color, PIM_Uint8 white)
{
    return (PIM_Uint8)((color<255)?PIM_Math_ClipToByte(((PIM_Int32)white*base)/(255-color)):(255));
}

PIM_Uint8 PIM_Color_BlendSoftLight(PIM_Uint8 base, PIM_Uint8 blend)
{
    return ((PIM_Uint8)((base < 128)?((2*((blend>>1)+64)*(PIM_Int32)base)/255): (255-((2 * (255-((blend>>1)+64)) * (PIM_Int32)(255-base)) /255))));
}

PIM_Uint8 PIM_Color_BlendOverlay (PIM_Uint8 base, PIM_Uint8 blend)
{
    return ((PIM_Uint8)((base < 128) ? (2 * blend * (PIM_Int32)base / 255):(255 - 2 * (255 - blend) * (PIM_Int32)(255 - base) / 255)));
}

PIM_Uint8 PIM_Color_BlendExclusion (PIM_Uint8 base, PIM_Uint8 blend)
{
    return PIM_Math_ClipToByte(base + blend - 2 * base * blend / 255);
}

PIM_Uint8 PIM_Color_BlendScreen(PIM_Uint8 base, PIM_Uint8 blend)
{
    return PIM_Math_ClipToByte((255 - (((255 - base) * (255 - blend)) >> 8)));
}
#endif
