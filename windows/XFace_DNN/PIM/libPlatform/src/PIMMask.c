#include "PIMTypes.h"
#include "libPlatform/inc/PIMBitmap.h"
#include "libPlatform/inc/PIMMath.h"
#include "libPlatform/inc/PIMMask.h"
#include "libPlatform/inc/PIMLog.h"
#define TAG "PIM"


#define CIRCLE_GRADATION 0
#define CIRCLE_SOLID     1

static PIM_Result Mask_CreateCircle(PIM_Bitmap* mask, PIM_Int32 width, PIM_Int32 height, PIM_Int32 intensity, PIM_Int32 type);

/**
 *  @brief Gray format으로 circle mask를 생성한다. Vignetting 전용.
 *
 *  @param width [IN] 
 *  @param height [IN] 
 *  @param intensity [IN] 1 ~ 3
 */
#if 0
PIM_Result PIM_Mask_CreateCircle(PIM_Bitmap* mask, PIM_Int32 width, PIM_Int32 height, PIM_Int32 intensity)
{
	PIM_Int32 ret;
	PIM_Int32 x, y;
	PIM_Int32 cx,cy;
	PIM_Int32 radius, ratio_temp;
	PIM_Int32 nMaxIntensity = 6;
	PIM_Int32 dist_y, distance;	
    PIM_Uint8* imageData = NULL;
	cx = width;
	cy = height;
	
	radius = PIM_Math_SquareInt(cx)+PIM_Math_SquareInt(cy); 

	/* 대각선을 radius로 삼을 경우, 너무 덜 어두운 감이 있어서 조금 조정함. */
	if (width >= height) 
		radius = (radius + PIM_Math_SquareInt(cx))/2;
	else 
		radius = (radius + PIM_Math_SquareInt(cy))/2;

	ratio_temp = (intensity<<8)/nMaxIntensity;

    /*  */
    ret = PIM_Bitmap_Create(mask, NULL, width, height, -1, PIM_IMAGE_FORMAT_GRAY);
    if(ret != PIM_SUCCESS) {
        return ret;
    }

    /*  */
    imageData = mask->imageData;
    for(y=0; y<height; y++) 
    {
        dist_y = PIM_Math_SquareInt(height-y);
        for(x=0; x<width; x++) 
        {
            distance = PIM_Math_SquareInt(width-x) + dist_y;

            *imageData = PIM_Math_ClipToByte(256 - distance*ratio_temp/radius);
            imageData++;
        }
    }

    return PIM_SUCCESS;
}
#else

PIM_Result PIM_Mask_CreateCircle(PIM_Bitmap* mask, PIM_Int32 width, PIM_Int32 height, PIM_Int32 intensity)
{
    return Mask_CreateCircle(mask, width, height, intensity, CIRCLE_GRADATION);
}

PIM_Result PIM_Mask_CreateSolidCircle(PIM_Bitmap* mask, PIM_Int32 width, PIM_Int32 height)
{
    return Mask_CreateCircle(mask, width, height, 0, CIRCLE_SOLID);
}

static PIM_Result Mask_CreateCircle(PIM_Bitmap* mask, PIM_Int32 width, PIM_Int32 height, PIM_Int32 intensity, PIM_Int32 type)
{
	PIM_Int32 ret;
	PIM_Int32 x, y;
	PIM_Int32 cx,cy;
	PIM_Int32 half_w,half_h;
	PIM_Int32 radius, ratio_temp;
	PIM_Int32 nMaxIntensity = 6;
	PIM_Int32 dist_y, distance;	
    PIM_Uint8* imageData = NULL;
	half_w = cx = width>>1;
	half_h = cy = height>>1;

    if(intensity < 0) {
        return PIM_ERROR_INVALID_ARGUMENT;
    }

    PIM_Bitmap_Destroy(mask);
	
	radius = PIM_Math_SquareInt(cx)+PIM_Math_SquareInt(cy); 

	/* 대각선을 radius로 삼을 경우, 너무 덜 어두운 감이 있어서 조금 조정함. */
	if (width >= height) 
		radius = (radius + PIM_Math_SquareInt(cx))/2;
	else 
		radius = (radius + PIM_Math_SquareInt(cy))/2;

    switch(type)
    {
        case CIRCLE_SOLID:
        {
            ratio_temp = 0;
        } break;

        case CIRCLE_GRADATION:
        default:
        {
            ratio_temp = (intensity<<8)/nMaxIntensity;
        } break;
    }

    /*  */
    ret = PIM_Bitmap_Create(mask, half_w, half_h, PIM_IMAGE_FORMAT_GRAY);
    if(ret != PIM_SUCCESS) {
        return ret;
    }

    /*  */
    imageData = mask->imageData;
    for(y=0; y<half_h; y++) 
    {
        dist_y = PIM_Math_SquareInt(cy-y);
        for(x=0; x<half_w; x++) 
        {
            distance = PIM_Math_SquareInt(cx-x) + dist_y;

            *imageData = PIM_Math_ClipToByte(256 - distance*ratio_temp/radius);
            imageData++;
        }
    }

    return PIM_SUCCESS;
}
#endif
