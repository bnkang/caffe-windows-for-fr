
#include "PIMTypes.h"
#include "libPlatform/inc/PIMMemory.h"

#include "libPlatform/inc/ByteImage.h"
#include "libPlatform/inc/PIMBitmap.h"
#include "libPlatform/inc/PIMBitmapConvert.h"

#include "libPlatform/inc/PIMColor.h"
#include "libPlatform/inc/PIMMath.h"
#include "libPlatform/inc/PIMRect.h"
#include "libPlatform/inc/ImageProcess.h"
#include "libPlatform/inc/PIMLog.h"

#define TAG "ByteImage"

static PIM_Int32 API_Bitmap_Recreate(PIM_Bitmap* bitmap, PIM_Int32 width, PIM_Int32 height);


const PIM_Int32 sin_shift10[73] = {   -0,   -89,  -178, -265, -350, -433, -512 ,-587 ,-658 ,-724 ,-784 ,-839 ,-887 ,-928 ,-962 ,-989 ,-1008 ,-1020 ,-1024 ,-1020 ,-1008 ,-989 ,-962 ,-928 ,-887 ,-839 ,-784 ,-724 ,-658 ,-587 ,-512 ,-433 ,-350 ,-265 ,-178 ,-89 ,    0 ,  89 , 178 ,265 ,350 ,433 ,512 ,587 ,658 ,724 ,784 ,839 ,887 ,928 ,962 ,989 ,1008 ,1020 ,1024 ,1020 ,1008 , 989 , 962 , 928 ,887 ,839 ,784 ,724 ,658 ,587 ,512 ,433 ,350 ,265 ,178 ,89 ,0};
const PIM_Int32 cos_shift10[73] = {-1024, -1020, -1008, -989, -962, -928, -887 ,-839 ,-784 ,-724 ,-658 ,-587 ,-512 ,-433 ,-350 ,-265 ,-178    ,-89 ,    0 ,   89 ,  178 , 265 , 350 , 433 , 512 , 587 , 658 , 724 , 784 , 839 , 887 , 928 , 962 , 989 ,1008 ,1020 ,1024 ,1020 ,1008 ,989 ,962 ,928 ,887 ,839 ,784 ,724 ,658 ,587 ,512 ,433 ,350 ,265 ,178  ,89   ,0    , -89 ,-178 ,-265 ,-350 ,-433 ,-512 ,-587 ,-658 ,-724 ,-784 ,-839 ,-887 ,-928 ,-962 ,-989 ,-1008 ,-1020 ,-1024};

/* 080130 cbHeo : this function uses fliped angle. */
PIM_Result PIM_ByteImage_RotatePreview(PIM_Bitmap *dst, PIM_Bitmap *src, PIM_Int32 angle)
{
#define SHIFT_BASE 10
    const PIM_Int32 cosT = cos_shift10[angle/5+36];
    const PIM_Int32 sinT = sin_shift10[angle/5+36];

    PIM_Int32 result_height = PIM_Math_Max(src->height, src->width);
    PIM_Int32 result_width = result_height;


    PIM_Int32 x1, y1;
    PIM_Int32 mx = result_width/2;
    PIM_Int32 my = result_height/2;
    PIM_Int32 msx = (src->width)/2;
    PIM_Int32 msy = (src->height)/2;
    PIM_Int32 x, y;
    
    PIM_Int32 temp_x, temp_y;
    

    PIM_Int32 temp_xs = -mx * cosT - my * sinT;
    PIM_Int32 temp_ys = mx * sinT - my * cosT;

    if(dst->width != result_width || dst->height != result_height) {
        PIM_Result ret;
        PIM_Bitmap_Destroy(dst);
        ret = PIM_Bitmap_Create(dst, result_width, result_height, PIM_IMAGE_FORMAT_GRAY);
        if(ret != PIM_SUCCESS) {
            return ret;
        }
    }

    /* Modified rotation : courtesy of POLPHIT */
    for(y=0; y<dst->height; y++) {
        
        temp_x = temp_xs;
        temp_y = temp_ys;
        
        for(x=0; x<dst->width; x++) {
            
            x1 = (temp_x >> SHIFT_BASE) + msx;
            y1 = (temp_y >> SHIFT_BASE) + msy;
            
            if  ((0 <= x1) && (x1 <= src->width - 1) && (0 <= y1) && (y1 <= src->height - 1))
                SET_GRAY(dst, x, y, GET_GRAY(src, x1, y1));

            else
                SET_GRAY(dst, x, y, 0);
                
            temp_x += cosT;
            temp_y -= sinT;
        }
        
        temp_xs += sinT;
        temp_ys += cosT;
    }

    return PIM_SUCCESS;
}



PIM_Result PIM_ByteImage_RotateResize_NN(PIM_Bitmap *dst, PIM_Bitmap *src, PIM_Int32 angle)
{
	PIM_Int32 i;
	PIM_Int32 utx=0, uty=0;
	PIM_Int32 uda_x;
	PIM_Int32 uda_y;
	PIM_Uint8 *pSrc, *pDst;
	PIM_Uint8 *src_buf;

    PIM_Int32 src_w, src_h, src_stride;
    PIM_Int32 dst_w, dst_h, dst_stride;

    pSrc = src->imageData;
    src_w = src->width;
    src_h = src->height;
    src_stride = src->stride;

    pDst = dst->imageData;
    dst_w = dst->width;
    dst_h = dst->height;
    dst_stride = dst->stride;

	if ( !angle ) {
		uda_x = (src_w << 10) / dst_w;
		uda_y = (src_h << 10) / dst_h;
		src_buf = pSrc;

		while(dst_h--) {
			utx = 0;
			i = dst_w;
			while(i--) {
				*pDst++ = src_buf[utx>>10];
				utx += uda_x;
			}
			uty += uda_y;		
			src_buf = pSrc + (uty>>10)*src_stride;
		}
	} else if(angle == 90) {
		uda_x = (src_w << 10) / dst_h;
		uda_y = (src_h << 10) / dst_w;
		pSrc = src->imageData + (src_h-1) * src_stride;
		src_buf = pSrc;

		while(dst_h--) {
			uty = 0;
			i = dst_w;
			while(i--) {
				*pDst++ = src_buf[(uty>>10)*src_stride];
				uty += uda_y;
			}		
			utx += uda_x;	
			src_buf = pSrc + (utx>>10);			
		}

	} else if(angle == -90){
		uda_x = (src_w << 10) / dst_h;
		uda_y = (src_h << 10) / dst_w;
		pSrc = src->imageData + (src_w-1);
		src_buf = pSrc;

		while(dst_h--) {
			uty = 0;
			i = dst_w;
			while(i--) {
				*pDst++ = src_buf[(uty>>10)*src_stride];		
				uty += uda_y;
			}		
			utx += uda_x;	
			src_buf = pSrc - (utx>>10);			
		}
	} 

    return PIM_SUCCESS;
}


//PIM_Void PIM_ByteImage_RotateCrop(PIM_Uint8 *src_data, PIM_Int32 src_w, PIM_Int32 src_h, PIM_Rect* crop_area, PIM_Int32 angle, PIM_Uint8 *des_data, PIM_Int32 des_w, PIM_Int32 des_h)
PIM_Result PIM_ByteImage_RotateCrop(PIM_Bitmap *dst, PIM_Bitmap *src, PIM_Rect *crop_area, PIM_Int32 angle)
{
    PIM_Int32 i;

    const PIM_Int32 cosT = cos_shift10[-angle/5+36];
    const PIM_Int32 sinT = sin_shift10[-angle/5+36];
    const PIM_Int32 inv_ratio_sft10_x_cost = cosT;
    const PIM_Int32 inv_ratio_sft10_x_sint = sinT;
    
    const PIM_Int32 scx_sft10 = (crop_area->left + crop_area->right)<<9;
    const PIM_Int32 scy_sft10 = (crop_area->top + crop_area->bottom)<<9;

    PIM_Uint8 *src_buf;
    PIM_Uint8 *dst_buf;
    PIM_Int32 src_w, src_h, src_stride;
    PIM_Int32 dst_w, dst_h, dst_stride;

    const PIM_Int32 dcx = dst->width>>1;
    const PIM_Int32 dcy = dst->height>>1;

    PIM_Int32 inv_ratio_sft10_x_sint_x_dy = -inv_ratio_sft10_x_sint*dcy;
    PIM_Int32 inv_ratio_sft10_x_cost_x_dy = -inv_ratio_sft10_x_cost*dcy;


    src_w = src->width;
    src_h = src->height;
    src_stride = src->stride;
    src_buf = src->imageData;

    dst_w = dst->width;
    dst_h = dst->height;
    dst_stride = dst->stride;
    dst_buf = dst->imageData;
    
    while(dst_h--) 
    {
        PIM_Int32 inv_ratio_sft10_x_sint_x_dx = -inv_ratio_sft10_x_sint*dcx;
        PIM_Int32 inv_ratio_sft10_x_cost_x_dx = -inv_ratio_sft10_x_cost*dcx;

        i = dst_w;
        while(i--) 
        {
            PIM_Int32 X = (scx_sft10 + inv_ratio_sft10_x_cost_x_dx - inv_ratio_sft10_x_sint_x_dy + 512)>>10;
            PIM_Int32 Y = (scy_sft10 + inv_ratio_sft10_x_sint_x_dx + inv_ratio_sft10_x_cost_x_dy + 512)>>10;
            
            if( X >= 0 && X < src_w && Y >= 0 && Y < src_h )
                *dst_buf++ = src_buf[src_stride*Y+X];
            else
                *dst_buf++ = 0;

            inv_ratio_sft10_x_sint_x_dx += inv_ratio_sft10_x_sint;
            inv_ratio_sft10_x_cost_x_dx += inv_ratio_sft10_x_cost;
        }
        inv_ratio_sft10_x_sint_x_dy += inv_ratio_sft10_x_sint;
        inv_ratio_sft10_x_cost_x_dy += inv_ratio_sft10_x_cost;
    }

    return PIM_SUCCESS;
}

//PIM_Void PIM_ByteImage_RotateCropResize_NN(PIM_Uint8 *src_data, PIM_Int32 src_w, PIM_Int32 src_h, PIM_Rect* crop_area, PIM_Int32 angle, PIM_Int32 ratio_sft10, PIM_Uint8 *des_data, PIM_Int32 des_w, PIM_Int32 des_h)
PIM_Result PIM_ByteImage_RotateCropResize_NN(PIM_Bitmap *dst, PIM_Bitmap *src, PIM_Rect* crop_area, PIM_Int32 angle, PIM_Int32 ratio_sft10)
{
    PIM_Int32 i;

    const PIM_Int32 inv_ratio_sft10 = 1048576/ratio_sft10;//1024*1024/ratio_sft10;
    const PIM_Int32 cosT = cos_shift10[-angle/5+36];
    const PIM_Int32 sinT = sin_shift10[-angle/5+36];
    const PIM_Int32 inv_ratio_sft10_x_cost = inv_ratio_sft10*cosT;
    const PIM_Int32 inv_ratio_sft10_x_sint = inv_ratio_sft10*sinT;
    
    const PIM_Int32 scx_sft20 = (crop_area->left + crop_area->right)<<19;
    const PIM_Int32 scy_sft20 = (crop_area->top + crop_area->bottom)<<19;
    PIM_Int32 dcx = dst->width>>1;
    PIM_Int32 dcy = dst->height>>1;

    PIM_Int32 inv_ratio_sft10_x_sint_x_dy = -inv_ratio_sft10_x_sint*dcy;
    PIM_Int32 inv_ratio_sft10_x_cost_x_dy = -inv_ratio_sft10_x_cost*dcy;

	PIM_Uint8 *pSrc, *pDst;
	PIM_Uint8 *dst_buf;

    PIM_Int32 src_w, src_h, src_stride;
    PIM_Int32 dst_w, dst_h, dst_stride;

    pSrc = src->imageData;
    src_w = src->width;
    src_h = src->height;
    src_stride = src->stride;

    pDst = dst->imageData;
    dst_w = dst->width;
    dst_h = dst->height;
    dst_stride = dst->stride;

    
    while(dst_h--) 
    {
        PIM_Int32 inv_ratio_sft10_x_sint_x_dx = -inv_ratio_sft10_x_sint*dcx;
        PIM_Int32 inv_ratio_sft10_x_cost_x_dx = -inv_ratio_sft10_x_cost*dcx;

        dst_buf = &pDst[dst_stride*(dst->height - dst_h - 1)];

        i = dst_w;
        while(i--) 
        {
            PIM_Int32 X = (scx_sft20 + inv_ratio_sft10_x_cost_x_dx - inv_ratio_sft10_x_sint_x_dy + 524288)>>20;
            PIM_Int32 Y = (scy_sft20 + inv_ratio_sft10_x_sint_x_dx + inv_ratio_sft10_x_cost_x_dy + 524288)>>20;
            
            if( X >= 0 && X < src_w && Y >= 0 && Y < src_h )
                *dst_buf++ = pSrc[src_stride*Y+X];
            else
                *dst_buf++ = 0;

            inv_ratio_sft10_x_sint_x_dx += inv_ratio_sft10_x_sint;
            inv_ratio_sft10_x_cost_x_dx += inv_ratio_sft10_x_cost;
        }
        inv_ratio_sft10_x_sint_x_dy += inv_ratio_sft10_x_sint;
        inv_ratio_sft10_x_cost_x_dy += inv_ratio_sft10_x_cost;
    }

    return PIM_SUCCESS;
}


PIM_Result PIM_ByteImage_CropMargin_Align(PIM_Bitmap *dst, PIM_Bitmap *src, PIM_Rect *crop_area)
{
    PIM_Int32 i;
    PIM_Uint32 ni;
        
    PIM_Uint32 height = PIM_Rect_GetHeight(crop_area);
    PIM_Uint32 width = PIM_Rect_GetWidth(crop_area);
    
    PIM_Int32 real_left = crop_area->left;
    PIM_Int32 real_right = crop_area->right;
    PIM_Uint32 real_w = width;
    PIM_Uint32 left_dummy = 0;
    
    //PIM_Bitmap_Recreate2(dst, width, height, width, PIM_IMAGE_FORMAT_GRAY);
    API_Bitmap_Recreate(dst, width, height);
    
    if(real_left < 0 || real_right >= src->width)
    {
        if( real_left < 0 )
        {
            left_dummy = -real_left;
            real_left = 0;
        }
        if( real_right >= src->width ) real_right = src->width - 1;
        
        real_w = real_right - real_left;
    }
    
    for( i=crop_area->top, ni=0; ni<height; i++, ni++ )
    {
		PIM_Memset(dst->imageData + (ni*dst->stride), 0, width * sizeof(PIM_Uint8));
        if( i >= 0 && i < src->height )
        {   /* inside picture */
            PIM_Uint8 *dst_buf = Get_GrayPtr(dst, left_dummy, ni);
            PIM_Uint8 *src_buf = Get_GrayPtr(src, real_left, i);
            PIM_Memcpy( dst_buf, src_buf, real_w * sizeof(PIM_Uint8) );
        }
    } 

    return PIM_SUCCESS;
}

#ifndef __CODE_SIZE_OPTIMIZE__
PIM_Result PIM_ByteImage_CropMargin(PIM_Bitmap *dst, PIM_Bitmap *src, PIM_Rect *crop_area)
{
    PIM_Int32 src_y;
    PIM_Int32 dst_y;
    PIM_Int32 height = PIM_Rect_GetHeight(crop_area);
    PIM_Int32 width  = PIM_Rect_GetWidth(crop_area);

    if (crop_area->left > crop_area->right || crop_area->top > crop_area->bottom) return PIM_ERROR_INVALID_ARGUMENT;
    if (PIM_ByteImage_SetSize(dst, width, height) != PIM_SUCCESS) return PIM_ERROR_NOMEM;

    for (src_y=crop_area->top, dst_y=0; dst_y<height; src_y++, dst_y++) {
        PIM_Memcpy(Get_GrayPtr(dst, 0, dst_y), Get_GrayPtr(src, crop_area->left, src_y), width * sizeof(PIM_Uint8));
    } 
    
    return PIM_SUCCESS;
}
#endif/*__CODE_SIZE_OPTIMIZE__*/



//PIM_Void PIM_ByteImage_CropResizeMargin_NN(PIM_Uint8 *src_data, PIM_Int32 src_w, PIM_Int32 src_h, PIM_Rect* crop_area, PIM_Uint8 *des_data, PIM_Int32 des_w, PIM_Int32 des_h)
PIM_Result PIM_ByteImage_CropResizeMargin_NN(PIM_Bitmap *dst, PIM_Bitmap *src, PIM_Rect* crop_area)
{
    PIM_Int32 w, h;
    PIM_Int32 crop_w = PIM_Rect_GetWidth(crop_area);
    PIM_Int32 crop_h = PIM_Rect_GetHeight(crop_area);
    PIM_Int32 utx, uty=0;
    PIM_Int32 uda_x, uda_y;
    PIM_Int32 real_left, real_right, real_top, real_bottom, left_dummy=0, top_dummy=0;
    PIM_Int32 real_left_dummy, real_top_dummy, real_right_end, real_bottom_end;
    PIM_Int32 real_w, real_h;
    PIM_Uint8 *pstart_data;
    PIM_Uint8 *psrc_data;
    PIM_Uint8 *pdes_data;

    PIM_Int32 dst_w = dst->width;
    PIM_Int32 dst_h = dst->height;
    PIM_Int32 dst_stride = dst->stride;
    PIM_Int32 src_w = src->width;
    PIM_Int32 src_h = src->height;
    PIM_Int32 src_stride = src->stride;
    
    PIM_Memset(dst->imageData, 0, dst_stride*dst_h);  // 
    uda_x = (crop_w << 10) / dst_w;
    uda_y = (crop_h << 10) / dst_h;

    if( crop_area->left < 0) {
        left_dummy = -crop_area->left;
        real_left = 0;
    } else {
        real_left = crop_area->left;
    }
    if( crop_area->top < 0 ){
        top_dummy = -crop_area->top;
        real_top = 0;
    } else {
        real_top = crop_area->top;
    }
    real_right  = PIM_Math_Min(crop_area->right,  src_w-1);
    real_bottom = PIM_Math_Min(crop_area->bottom, src_h-1);

    pstart_data = src->imageData + src_stride*real_top + real_left;
    psrc_data = pstart_data;

    real_left_dummy = left_dummy * dst_w / crop_w;
    real_top_dummy  = top_dummy  * dst_h / crop_h;
    real_w          = (real_right  - real_left) * dst_w / crop_w;
    real_h          = (real_bottom - real_top)  * dst_h / crop_h;
    real_right_end  = PIM_Math_Min(dst_w, real_w+real_left_dummy +1);
    real_bottom_end = PIM_Math_Min(dst_h, real_h+real_top_dummy  +1);

    pdes_data  = dst->imageData + dst_stride*real_top_dummy + real_left_dummy; 
    
    for(h=real_top_dummy; h<real_bottom_end ;h++) {
        utx = 0; 
        pdes_data = dst->imageData + dst_stride*h + real_left_dummy;
        for(w=real_left_dummy; w<real_right_end; w++) {
            *pdes_data++ = *(psrc_data + (utx>>10));        
            utx += uda_x;
        }
        uty += uda_y;       
        
        psrc_data = pstart_data + (uty>>10)*src_stride;
    }

    return PIM_SUCCESS;
}



PIM_Result PIM_ByteImage_CropResize_NN(PIM_Bitmap *dst, PIM_Bitmap *src, PIM_Rect* crop_area)
{
    PIM_Int32 i;
    PIM_Int32 utx, uty=0;
    PIM_Int32 dst_w = dst->width;
    PIM_Int32 dst_h = dst->height;
    PIM_Int32 src_w = src->width;
    PIM_Int32 src_stride = src->stride;

    PIM_Int32 uda_x = (PIM_Rect_GetWidth(crop_area) << 10) / dst_w;
    PIM_Int32 uda_y = (PIM_Rect_GetHeight(crop_area) << 10) / dst_h;
    PIM_Uint8 *pstart_data = src->imageData + (crop_area->top * src_w) + crop_area->left;
    PIM_Uint8 *psrc_data = pstart_data;
    PIM_Uint8 *pdst_data = dst->imageData;

    while(dst_h--) {
        utx = 0;
        i = dst_w;
        while(i--) {
            *pdst_data++ = *(psrc_data + (utx>>10));        
            utx += uda_x;
        }       
        uty += uda_y;       
        psrc_data = pstart_data + (uty>>10)*src_stride;
    }

    return PIM_SUCCESS;
}



#ifndef __CODE_SIZE_OPTIMIZE__
PIM_Result PIM_ByteImage_Init(PIM_Bitmap *pThis)
{
    if (!pThis) return PIM_ERROR_NULL_POINTER;
    
    PIM_Memset(pThis, 0, sizeof(PIM_Bitmap));
    return PIM_SUCCESS;
}

PIM_Result PIM_ByteImage_SetSize(PIM_Bitmap *This, PIM_Int32 w, PIM_Int32 h)
{
    PIM_Result ret = PIM_SUCCESS;
    
    if (h == This->height && w == This->width && This->imageData != NULL != NULL) return PIM_SUCCESS;
    
    PIM_Bitmap_Destroy(This);
    
#if 0
    This->height = h;
    This->width = w;

    This->imageData = (PIM_Uint8 *)PIM_Malloc(sizeof(PIM_Uint8) * h * w);
    if (This->imageData == NULL) return PIM_ERROR_NOMEM;
#else
	ret = PIM_Bitmap_Create(This, w, h, PIM_IMAGE_FORMAT_GRAY);
#endif
    
    return ret; 
}
#endif/*__CODE_SIZE_OPTIMIZE__*/

//PIM_Void ByteImage::Load(PIM_Bitmap *bitmap)
PIM_Result PIM_ByteImage_Load(PIM_Bitmap *dst, PIM_Bitmap *src, PIM_Int32 N)
{
    PIM_Bitmap resizeBitmap = {0};
    PIM_Result  result = PIM_SUCCESS;
    PIM_Int32  width = 0;
    PIM_Int32  height = 0;

    if (N < 1) return PIM_ERROR_INVALID_ARGUMENT;


	width =  src->width / N;
    height = src->height / N;

    /* align change */
    width  = PIM_Bitmap_GetStride(PIM_IMAGE_FORMAT_GRAY, width,  4);
    height = PIM_Bitmap_GetStride(PIM_IMAGE_FORMAT_GRAY, height, 4);

    /*  */
    if (height != dst->height || width != dst->width || dst->imageData == NULL)
	{
		PIM_Bitmap_Destroy(dst);
		if (PIM_Bitmap_Create(dst, width, height, PIM_IMAGE_FORMAT_GRAY) != PIM_SUCCESS) return PIM_ERROR_NOMEM;
	}

    /*  */
    switch(src->imageFormat)
    {
        case PIM_IMAGE_FORMAT_GRAY:
			if (N > 1) 
				result = PIM_Bitmap_Resize(dst, src);
            else       
				result = PIM_Bitmap_Copy(dst, src);

            break;

		case PIM_IMAGE_FORMAT_YUVPLANAR_NV12:
		case PIM_IMAGE_FORMAT_YUVPLANAR_NV21: 
			{
				PIM_Bitmap src_tmp = {0,};
				PIM_Bitmap_CreateRef(&src_tmp, src->imageData, src->width, src->height, src->width, PIM_IMAGE_FORMAT_GRAY);
				if (N > 1) 
					result = PIM_Bitmap_Resize(dst, &src_tmp);
				else       
					result = PIM_Bitmap_Copy(dst, &src_tmp);

				PIM_Bitmap_Destroy(&src_tmp);
				break;
			}
        default:
        {
			PIM_Bitmap processBitmap = {0};
			if (N > 1) {
                if (PIM_Bitmap_Create(&resizeBitmap, width, height, src->imageFormat) != PIM_SUCCESS) return PIM_ERROR_NOMEM;
                PIM_Bitmap_Resize(&resizeBitmap, src);
                processBitmap = resizeBitmap;			
			} 
			// added 20111110 by neebok : for making image width to a multiple of 4 when width is smaller than 640 {
			else if(width != src->width || height != src->height) {
				if (PIM_Bitmap_Create(&resizeBitmap, width, height, src->imageFormat) != PIM_SUCCESS) return PIM_ERROR_NOMEM;
				PIM_Bitmap_Resize(&resizeBitmap, src);
				processBitmap = resizeBitmap;
            }
			// } added 20111110 by neebok 
			else {
                processBitmap = *src;
            }

			// removed 20111110 by neebok {
// 			/* side effect : align size */
// 			if(width != src->width || height != src->height) {
// 				processBitmap.width = width;
// 				processBitmap.height = height;
// 			}
			// } removed 20111110 by neebok
            result = PIM_BitmapConvert(dst, &processBitmap);
        } break;
    }

    /* clear resource */
    PIM_Bitmap_Destroy(&resizeBitmap);

    return result;
}



#ifndef __CODE_SIZE_OPTIMIZE__
// 070829 goodsong created
PIM_Result PIM_ByteImage_Clone( const PIM_Bitmap *This, PIM_Bitmap *dst )
{
    PIM_Uint32 w = This->width;
    PIM_Uint32 h = This->height;
    PIM_Uint32 len = w*h;
    PIM_Result ret;

    if(dst == This) return PIM_SUCCESS;
   
    //ret = PIM_Bitmap_Recreate2( dst, w, h, w, PIM_IMAGE_FORMAT_GRAY );
    ret = API_Bitmap_Recreate(dst, w, h);
    if( ret != PIM_SUCCESS ) return ret;
    PIM_Memcpy( dst->imageData, This->imageData, len*sizeof(PIM_Uint8) );
    return PIM_SUCCESS;
}
#endif/*__CODE_SIZE_OPTIMIZE__*/

PIM_Result PIM_ByteImage_LoadResizedFromByteStream(PIM_Bitmap *dst, PIM_Bitmap *src,
         PIM_Int32 ratio_nom_h, PIM_Int32 ratio_denom_h, PIM_Int32 ratio_nom_w, PIM_Int32 ratio_denom_w)
{
    PIM_Int32 dst_h = (src->height*ratio_nom_h/ratio_denom_h)&~1;
    PIM_Int32 dst_w = (src->width *ratio_nom_w/ratio_denom_w)&~1;
    PIM_Result ret;

    //ret = PIM_Bitmap_Recreate2(dst, dst_w, dst_h, dst_w, PIM_IMAGE_FORMAT_GRAY);
    ret = API_Bitmap_Recreate(dst, dst_w, dst_h);
    if (ret != PIM_SUCCESS) return ret;

    PIM_Bitmap_Resize(dst, src);
    return PIM_SUCCESS;
}

PIM_Result PIM_ByteImage_RotateVersatile(const PIM_Bitmap *pthis, PIM_Bitmap *result, PIM_Point *ref_point, PIM_Int32 angle)
{
	const PIM_Int32 cosT = PIM_Math_CosS(angle);
	const PIM_Int32 sinT = PIM_Math_SinS(angle);

	PIM_Int32 result_height = (((pthis->width * PIM_Math_Abs(sinT)) + (pthis->height * PIM_Math_Abs(cosT))) >> PIM_Math_KCosSBaseShifter) -2;
	PIM_Int32 result_width = (((pthis->height * PIM_Math_Abs(sinT)) + (pthis->width * PIM_Math_Abs(cosT))) >> PIM_Math_KCosSBaseShifter) -2;

	PIM_Int32 x1, y1;
	PIM_Int32 mx = result_width/2;
	PIM_Int32 my = result_height/2;
	PIM_Int32 msx = (pthis->width)/2;
	PIM_Int32 msy = (pthis->height)/2;
	PIM_Int32 x, y;
	
	PIM_Int32 temp_x, temp_y;
	PIM_Int32 temp_xs = -mx * cosT + my * sinT;
	PIM_Int32 temp_ys = -mx * sinT - my * cosT;
	
	PIM_Point temp_point;

	//if (PIM_Bitmap_Recreate2(result, result_width, result_height, result_width, PIM_IMAGE_FORMAT_GRAY) != PIM_SUCCESS) return PIM_ERROR_NOMEM;
	if (API_Bitmap_Recreate(result, result_width, result_height) != PIM_SUCCESS) return PIM_ERROR_NOMEM;


	for(y=0; y<result->height; y++) {
		
		temp_x = temp_xs;
		temp_y = temp_ys;
		
		for(x=0; x<result->width; x++) {
			
			x1 = (temp_x >> PIM_Math_KCosSBaseShifter) + msx;
			y1 = (temp_y >> PIM_Math_KCosSBaseShifter) + msy;
			
			if	((0 <= x1) && (x1 <= pthis->width - 1) && (0 <= y1) && (y1 <= pthis->height - 1)) {
				//result->data[y][x] = pthis->data[y1][x1];
				result->imageData[y*result->width + x] = pthis->imageData[y1*pthis->width + x1];
			} else {
				result->imageData[y*result->width + x] = 0;
			}
				
			temp_x += cosT;
			temp_y += sinT;
		}
		
		temp_xs -= sinT;
		temp_ys += cosT;
	}

	ref_point->x -= msx;
	ref_point->y -= msy;
	temp_point.x = (ref_point->x * cosT + ref_point->y * sinT) >> PIM_Math_KCosSBaseShifter;
	temp_point.y = (-1 * ref_point->x * sinT + ref_point->y * cosT) >> PIM_Math_KCosSBaseShifter;
	
	ref_point->x = temp_point.x + mx;
	ref_point->y = temp_point.y + my;

	return PIM_SUCCESS;
}

PIM_Uint32 PIM_ByteImage_GetBrightness(PIM_Bitmap* dst, PIM_Rect * crop_area)
{
#define SAMPLE_NUM 16
	PIM_Int32 i, j = SAMPLE_NUM;
	PIM_Int32 utx, uty=0;
	PIM_Int32 sum=0;
	PIM_Int32 src_w = dst->width;
	PIM_Int32 uda_x = (PIM_Rect_GetWidth(crop_area) << 10) / SAMPLE_NUM;
	PIM_Int32 uda_y = (PIM_Rect_GetHeight(crop_area) << 10) / SAMPLE_NUM;
	PIM_Uint8 *pstart_data = dst->imageData + (crop_area->top * src_w) + crop_area->left;
	PIM_Uint8 *psrc_data = pstart_data;

	while(j--) {
		utx = 0;
		i = SAMPLE_NUM;
		while(i--) {
			sum += *(psrc_data + (utx>>10));		
			utx += uda_x;
		}		
		uty += uda_y;		
		psrc_data = pstart_data + (uty>>10)*src_w;
	}

	return sum>>8;
}

static PIM_Int32 API_Bitmap_Recreate(PIM_Bitmap* bitmap, PIM_Int32 width, PIM_Int32 height)
{
    PIM_Int32 ret = PIM_SUCCESS;

    if(bitmap->width != width || bitmap->height != height || bitmap->imageData == NULL) {
        PIM_Bitmap_Destroy(bitmap);
        PIM_Bitmap_CreateRef(bitmap, NULL, width, height, width, PIM_IMAGE_FORMAT_GRAY);
    }

    return ret;
}
