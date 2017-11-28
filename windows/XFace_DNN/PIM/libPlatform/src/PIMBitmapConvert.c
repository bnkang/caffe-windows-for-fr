#include "PIMTypes.h"
#include "libPlatform/inc/PIMBitmap.h"
#include "libPlatform/inc/PIMBitmapConvert.h"
#include "libPlatform/inc/PIMColor.h"
#include "libPlatform/inc/PIMMath.h"
#include "libPlatform/inc/PIMMemory.h"

#include "libPlatform/inc/PIMLog.h"
#define TAG "PIM"



/***************************************************************************************************
 * Internal Macro
 **************************************************************************************************/
#define RGB2YUV(R, G, B, Y, U, V)                           \
    (Y) = (( 66*(R) + 128*(G) +  25*(B) + 128)>>8) + 16;    \
    (U) = ((-38*(R) -  74*(G) + 112*(B) + 128 + 32768)>>8); \
    (V) = ((112*(R) -  94*(G) -  18*(B) + 128 + 32768)>>8);

#define RGB2Y__(R, G, B, Y)                                \
    (Y) = (( 66*(R) + 128*(G) +  25*(B) + 128)>>8) + 16;

#define RGB2_U_(R, G, B, U)                                \
    (U) = ((-38*(R) -  74*(G) + 112*(B) + 128 + 32768)>>8);

#define RGB2__V(R, G, B, V)                                \
    (V) = ((112*(R) -  94*(G) -  18*(B) + 128 + 32768)>>8);

#define YUV2RGB(Y, U, V, R, G, B)                                                                    \
    (R) = PIM_Math_ClipToByte((298*((Y) - 16)                   + 409*((V) - 128) + 128 ) >> 8 );    \
    (G) = PIM_Math_ClipToByte((298*((Y) - 16) - 100*((U) - 128) - 208*((V) - 128) + 128 ) >> 8 );    \
    (B) = PIM_Math_ClipToByte((298*((Y) - 16) + 516*((U) - 128)                      + 128 ) >> 8 );

#define RGB2GRAY(r, g, b) ((PIM_Uint8)(((r)*77+(g)*151+(b)*28)>>8))



/***************************************************************************************
 *  Internal method
 **************************************************************************************/
typedef struct __FormatConverter {
    PIM_ImageFormat dst_format;
    PIM_ImageFormat src_format;
    PIM_Result      (*convert)(PIM_Bitmap* dst, PIM_Bitmap* src);
} FormatConverter;

static const FormatConverter g_converter[] = {
/*  { Dst format                        Src format,                            convert                               }  */
    { PIM_IMAGE_FORMAT_GRAY,            PIM_IMAGE_FORMAT_RGB_LABEL,            PIM_BitmapConvert_RgbToGray           },
    { PIM_IMAGE_FORMAT_GRAY,            PIM_IMAGE_FORMAT_YUVPLANAR_LABEL,      PIM_BitmapConvert_YuvPlanarToGray     },
    { PIM_IMAGE_FORMAT_GRAY,            PIM_IMAGE_FORMAT_YUVPACKED_Y411,       PIM_BitmapConvert_YuvPackedY411ToGray },

    { PIM_IMAGE_FORMAT_RGB_LABEL,       PIM_IMAGE_FORMAT_GRAY,                 PIM_BitmapConvert_GrayToRgb           },    /* for OpenCV */
    { PIM_IMAGE_FORMAT_RGB_LABEL,       PIM_IMAGE_FORMAT_YUVPLANAR_LABEL,      PIM_BitmapConvert_YuvPlanarToRgb      },
    { PIM_IMAGE_FORMAT_RGB_LABEL,       PIM_IMAGE_FORMAT_YUVPACKED_Y411,       PIM_BitmapConvert_YuvPackedY411ToRgb  },
    { PIM_IMAGE_FORMAT_RGB_LABEL,       PIM_IMAGE_FORMAT_RGB_LABEL,            PIM_BitmapConvert_RgbToRgb            },

    { PIM_IMAGE_FORMAT_YUVPLANAR_LABEL, PIM_IMAGE_FORMAT_RGB_LABEL,            PIM_BitmapConvert_RgbToYuvPlanar      },
    { PIM_IMAGE_FORMAT_YUVPLANAR_LABEL, PIM_IMAGE_FORMAT_YUVPLANAR_LABEL,      PIM_BitmapConvert_YuvPlanarToYuvPlanar},

    { PIM_IMAGE_FORMAT_YUVPACKED_Y411,  PIM_IMAGE_FORMAT_RGB_LABEL,            PIM_BitmapConvert_RgbToYuvPackedY411  },

    { 0,                                0,                                     NULL                                  },
};

static const FormatConverter* getFormatConverter(PIM_Bitmap* dst, PIM_Bitmap* src)
{
    PIM_Int32 index=0;
    const FormatConverter* pConverter = NULL;

    do {
        pConverter = &g_converter[index++];

        if(( dst->imageFormat == pConverter->dst_format ) ||
           ( dst->imageFormat &  (pConverter->dst_format&0xff00) ) )
        {
            if(( src->imageFormat == pConverter->src_format ) ||
               ( src->imageFormat &  (pConverter->src_format&0xff00) ) )
            {
                return pConverter;
            }
        }
    } while(pConverter->dst_format != 0);

    return NULL;
}

/*
 * Convert one format to another. dst's buffers should be allocated by the caller.
 *  : General method.
 */
PIM_Int32 PIM_BitmapConvert(PIM_Bitmap* dst, PIM_Bitmap* src)
{
    const FormatConverter* pConverter = NULL;

    if(dst->imageFormat == src->imageFormat) {
        return PIM_Bitmap_Copy(dst, src);
    }

    pConverter = getFormatConverter(dst, src);
    if(pConverter) {
        if(pConverter->convert) {
            pConverter->convert(dst, src);
            return PIM_SUCCESS;
        }
    }

    return PIM_ERROR_NOT_SUPPORTED;
}



/*
 * supported format
 *   @ src                        @ dst                            
 *   PIM_IMAGE_FORMAT_GRAY   =>   PIM_IMAGE_FORMAT_RGB888        
 *                                PIM_IMAGE_FORMAT_BGR888
 *                                PIM_IMAGE_FORMAT_RGB8880
 *                                PIM_IMAGE_FORMAT_BGR8880
 */
PIM_Result PIM_BitmapConvert_GrayToRgb(PIM_Bitmap *dst, PIM_Bitmap *src)
{
    PIM_Int32 width  = src->width;
    PIM_Int32 height = src->height;
    PIM_Int32 y,x;
    PIM_Int32 dst_period = PIM_Bitmap_GetPeriod(dst);

    PIM_Uint8 *prgb  = dst->imageData;
    PIM_Uint8 *pgray = src->imageData;

    PIM_Int32 dst_pass = dst->stride - (dst->width*dst_period);
    PIM_Int32 src_pass = src->stride - src->width;

    for (y=0; y<height; y++) {
        for (x=0; x<width; x++) {
            prgb[0] = *pgray;
            prgb[1] = *pgray;
            prgb[2] = *pgray;
            
            pgray++;
            prgb += dst_period;
        }

        prgb  += dst_pass;
        pgray += src_pass;
    }

    return PIM_SUCCESS;
}



/*
 * supported format
 *   @ src                           @ dst                         
 *   PIM_IMAGE_FORMAT_RGB888    =>   PIM_IMAGE_FORMAT_GRAY       
 *   PIM_IMAGE_FORMAT_BGR888                             
 *   PIM_IMAGE_FORMAT_RGB8880                            
 *   PIM_IMAGE_FORMAT_BGR8880                            
 */
PIM_Result PIM_BitmapConvert_RgbToGray(PIM_Bitmap *dst, PIM_Bitmap *src)
{
    PIM_Int32 width = src->width;
    PIM_Int32 height = src->height;
    PIM_Int32 y,x;
    PIM_Int32 src_period = PIM_Bitmap_GetPeriod(src);

    PIM_Int32 dst_pass = dst->stride - dst->width;
    PIM_Int32 src_pass = src->stride - (src->width*src_period);

    PIM_Uint8 *gray;
    PIM_Uint8 *red, *green, *blue;

    switch(src->imageFormat)
    {
        case PIM_IMAGE_FORMAT_RGB888:
        case PIM_IMAGE_FORMAT_RGB8880:
            red   = &src->imageData[0];
            green = &src->imageData[1];
            blue  = &src->imageData[2];
            break;

        case PIM_IMAGE_FORMAT_BGR888:
        case PIM_IMAGE_FORMAT_BGR8880:
            red   = &src->imageData[2];
            green = &src->imageData[1];
            blue  = &src->imageData[0];
            break;

        default:
            return PIM_ERROR_NOT_SUPPORTED;
    }

    gray = dst->imageData;

    for (y=0; y<height; y++) {
        for (x=0; x<width; x++) {
            *gray++ = (PIM_Uint8)RGB2GRAY(red[0], green[0], blue[0]);
            red   += src_period;
            green += src_period;
            blue  += src_period;
        }

        gray  += dst_pass;
        red   += src_pass;
        green += src_pass;
        blue  += src_pass;
    }

    return PIM_SUCCESS;
}



/*
 * supported format
 *   @ src                           @ dst
 *   PIM_IMAGE_FORMAT_RGB888    =>   PIM_IMAGE_FORMAT_RGB888
 *   PIM_IMAGE_FORMAT_BGR888         PIM_IMAGE_FORMAT_BGR888
 *   PIM_IMAGE_FORMAT_RGB8880        PIM_IMAGE_FORMAT_RGB8880
 *   PIM_IMAGE_FORMAT_BGR8880        PIM_IMAGE_FORMAT_BGR8880
 */
PIM_Result PIM_BitmapConvert_RgbToRgb(PIM_Bitmap *dst, PIM_Bitmap *src)
{
    PIM_Int32 width = src->width;
    PIM_Int32 height = src->height;
    PIM_Int32 y,x;
    PIM_Int32 dst_period = PIM_Bitmap_GetPeriod(dst);
    PIM_Int32 src_period = PIM_Bitmap_GetPeriod(src);

    PIM_Int32 dst_pass = dst->stride - (dst->width*dst_period);
    PIM_Int32 src_pass = src->stride - (src->width*src_period);

    PIM_Uint8 *dst_r, *dst_g, *dst_b;
    PIM_Uint8 *src_r, *src_g, *src_b;

    switch(dst->imageFormat)
    {
        case PIM_IMAGE_FORMAT_RGB888:
        case PIM_IMAGE_FORMAT_RGB8880:
            dst_r = &dst->imageData[0];
            dst_g = &dst->imageData[1];
            dst_b = &dst->imageData[2];
            break;

        case PIM_IMAGE_FORMAT_BGR888:
        case PIM_IMAGE_FORMAT_BGR8880:
            dst_r = &dst->imageData[2];
            dst_g = &dst->imageData[1];
            dst_b = &dst->imageData[0];
            break;

        default:
            return PIM_ERROR_NOT_SUPPORTED;
    }

    switch(src->imageFormat)
    {
        case PIM_IMAGE_FORMAT_RGB888:
        case PIM_IMAGE_FORMAT_RGB8880:
            src_r = &src->imageData[0];
            src_g = &src->imageData[1];
            src_b = &src->imageData[2];
            break;

        case PIM_IMAGE_FORMAT_BGR888:
        case PIM_IMAGE_FORMAT_BGR8880:
            src_r = &src->imageData[2];
            src_g = &src->imageData[1];
            src_b = &src->imageData[0];
            break;

        default:
            return PIM_ERROR_NOT_SUPPORTED;
    }

    for (y=0; y<height; y++) {
        for (x=0; x<width; x++) {
            *dst_r = *src_r;
            *dst_g = *src_g;
            *dst_b = *src_b;

            dst_r += dst_period;
            dst_g += dst_period;
            dst_b += dst_period;
            src_r += src_period;
            src_g += src_period;
            src_b += src_period;
        }

        dst_r += dst_pass;
        dst_g += dst_pass;
        dst_b += dst_pass;
        src_r += src_pass;
        src_g += src_pass;
        src_b += src_pass;
    }

    return PIM_SUCCESS;
}



/*
 * supported format
 *   @ src                           @ dst                         
 *   PIM_IMAGE_FORMAT_RGB888    =>   PIM_IMAGE_FORMAT_YUVPLANAR_NV12
 *   PIM_IMAGE_FORMAT_RGB8880        PIM_IMAGE_FORMAT_YUVPLANAR_NV12
 */
PIM_Result PIM_BitmapConvert_RgbToYuvPlanar(PIM_Bitmap *dst, PIM_Bitmap *src)
{
    PIM_Int32 width = src->width;
    PIM_Int32 height = src->height;
    PIM_Int32 i,j;
	PIM_Int32 dst_stride = dst->stride;
	PIM_Int32 src_stride = src->stride;

    PIM_Int32 sample_period_w;
    PIM_Int32 sample_period_h;
    PIM_Int32 rgb_period = PIM_Bitmap_GetChannel(src);

	PIM_Uint8 *red_base, *green_base, *blue_base;
    PIM_Uint8 *red, *green, *blue;
	PIM_Uint8 *y_base, *u_base, *v_base;
    PIM_Uint8 *y, *u, *v;

    PIM_Int32 uv_period = 0;
    PIM_Int32 uv_step_width = 0;

    /*  */
    switch(src->imageFormat)
    {
        case PIM_IMAGE_FORMAT_RGB888:
        case PIM_IMAGE_FORMAT_RGB8880:
            red_base   = (PIM_Uint8*)&src->imageData[0];
            green_base = (PIM_Uint8*)&src->imageData[1];
            blue_base  = (PIM_Uint8*)&src->imageData[2];
            break;

        case PIM_IMAGE_FORMAT_BGR888:
        case PIM_IMAGE_FORMAT_BGR8880:
            red_base   = (PIM_Uint8*)&src->imageData[2];
            green_base = (PIM_Uint8*)&src->imageData[1];
            blue_base  = (PIM_Uint8*)&src->imageData[0];
            break;

        default:
            return PIM_ERROR_NOT_SUPPORTED;
    }

    /*  */
    switch(dst->imageFormat)
    {
        case PIM_IMAGE_FORMAT_YUVPLANAR_NV12:
            y_base = (PIM_Uint8*)dst->imageData;
            if(dst->auxImageData) u_base = dst->auxImageData;
            else                  u_base = y_base + dst_stride * height;
            v_base = u_base + 1;
            sample_period_w = 2;
            sample_period_h = 2;
            uv_period = 2;
            uv_step_width = dst_stride/2;
            break;

        case PIM_IMAGE_FORMAT_YUVPLANAR_NV21:
            y_base = (PIM_Uint8*)dst->imageData;
            if(dst->auxImageData) v_base = dst->auxImageData;
            else                  v_base = y_base + dst_stride * height;
            u_base = v_base + 1;
            sample_period_w = 2;
            sample_period_h = 2;
            uv_period = 2;
            uv_step_width = dst_stride/2;
            break;

        case PIM_IMAGE_FORMAT_YUVPLANAR_YV12:
            y_base = (PIM_Uint8*)dst->imageData;
            if(dst->auxImageData) v_base = dst->auxImageData;
            else                  v_base = y_base + dst_stride * height;
            u_base = v_base + (dst_stride * height)/4;
            sample_period_w = 2;
            sample_period_h = 2;
            uv_period = 1;
            uv_step_width = dst_stride/4;
            break;

        case PIM_IMAGE_FORMAT_YUVPLANAR_I420:
            y_base = (PIM_Uint8*)dst->imageData;
            if(dst->auxImageData) u_base = dst->auxImageData;
            else                  u_base = y_base + dst_stride * height;
            v_base = u_base + (dst_stride * height)/4;
            sample_period_w = 2;
            sample_period_h = 2;
            uv_period = 1;
            uv_step_width = dst_stride/4;
            break;

        default:
            return PIM_ERROR_NOT_SUPPORTED;
    }


    /* sample 2x2 */
    if(sample_period_w == 2 && sample_period_h == 2)
    {
        for (i=0; i<height; i+=sample_period_h)
        {
			if((i+1) >= height) break;	/* 마지막줄이 처리 안될거 같은데 처리가 되는것처럼 보임. 버그 소지 다분함. */

            /* first line */
			red   = &red_base  [i*src_stride];
			green = &green_base[i*src_stride];
			blue  = &blue_base [i*src_stride];
			y     = &y_base    [i*dst_stride];
			u     = &u_base    [(i&~1)*uv_step_width];
			v     = &v_base    [(i&~1)*uv_step_width];

            for (j=0; j<width; j+=sample_period_w)
            {
                RGB2Y__(red[0], green[0], blue[0], y[0]);
                RGB2_U_(red[0], green[0], blue[0], u[0]);
                RGB2__V(red[0], green[0], blue[0], v[0]);
                red  +=rgb_period;
                green+=rgb_period;
                blue +=rgb_period;
                y++;
                u+=uv_period;
                v+=uv_period;

                RGB2Y__(red[0], green[0], blue[0], y[0]);
                red  +=rgb_period;
                green+=rgb_period;
                blue +=rgb_period;
                y++;
            }


            /* second line */
			red   = &red_base  [(i+1)*src_stride];
			green = &green_base[(i+1)*src_stride];
			blue  = &blue_base [(i+1)*src_stride];
			y     = &y_base    [(i+1)*dst_stride];

            for (j=0; j<width; j+=sample_period_w)
            {
                RGB2Y__(red[0], green[0], blue[0], y[0]);
                red  +=rgb_period;
                green+=rgb_period;
                blue +=rgb_period;
                y++;

                RGB2Y__(red[0], green[0], blue[0], y[0]);
                red  +=rgb_period;
                green+=rgb_period;
                blue +=rgb_period;
                y++;
            }
        }
    } else {
        return PIM_ERROR_NOT_SUPPORTED;
    }

    return PIM_SUCCESS;    
}



/*
 * supported format
 *   @ src                           @ dst                         
 *   PIM_IMAGE_FORMAT_RGB888    =>   PIM_IMAGE_FORMAT_YUVPACKED_Y411       
 *   PIM_IMAGE_FORMAT_BGR888                             
 *   PIM_IMAGE_FORMAT_RGB8880                            
 *   PIM_IMAGE_FORMAT_BGR8880                            
 *
 *  @note width must be divisible by 4.
 */
PIM_Result PIM_BitmapConvert_RgbToYuvPackedY411(PIM_Bitmap* dst, PIM_Bitmap* src)
{
    PIM_Int32 width  = src->width;
    PIM_Int32 height = src->height;
    PIM_Int32 i, j;
    PIM_Int32 src_period = PIM_Bitmap_GetPeriod(src);

    PIM_Uint8 *rgb;
    PIM_Uint8 *yuv;

    /*  */
    if((width&0x3) != 0) {
        LOGE("wrong width(%d) -> width(%d)", width, width&(~0x3));
        return PIM_ERROR_INVALID_ARGUMENT;
    }

    rgb = src->imageData;
    yuv = dst->imageData;

    for(j=0; j<height; j++) {
        for(i=0; i<width; i+=4) { /* 4 : pixel per packed unit. */
            RGB2YUV(rgb[2], rgb[1], rgb[0], yuv[1], yuv[0], yuv[3]);
            rgb+=src_period;

            RGB2Y__(rgb[2], rgb[1], rgb[0], yuv[2]);
            rgb+=src_period;

            RGB2Y__(rgb[2], rgb[1], rgb[0], yuv[4]);
            rgb+=src_period;

            RGB2Y__(rgb[2], rgb[1], rgb[0], yuv[5]);
            rgb+=src_period;

            yuv+=6;    /* 6 : Y411 packed period */
        }
    }

    return PIM_SUCCESS;
}



/*
 * supported format
 *   @ src                                        @ dst
 *   PIM_IMAGE_FORMAT_YUVPLANAR_NV12         =>   PIM_IMAGE_FORMAT_GRAY
 *   PIM_IMAGE_FORMAT_YUVPLANAR_NV21              
 */
PIM_Result PIM_BitmapConvert_YuvPlanarToGray(PIM_Bitmap *dst, PIM_Bitmap *src)
{
    PIM_Int32 width  = src->width;
    PIM_Int32 height = src->height;
    PIM_Int32 y;

    PIM_Uint8 *pyuv  = src->imageData;
    PIM_Uint8 *pgray = dst->imageData;

    PIM_Int32 dst_stride = dst->stride;
    PIM_Int32 src_stride = src->stride;

    for (y=0; y<height; y++) {
        PIM_Memcpy(pgray, pyuv, width);
        pyuv   += src_stride;
        pgray  += dst_stride;
    }

    return PIM_SUCCESS;
}



/*
 * supported format
 *   @ src                                  @ dst
 *   PIM_IMAGE_FORMAT_YUVPLANAR_NV12   =>   PIM_IMAGE_FORMAT_RGB888 
 *   PIM_IMAGE_FORMAT_YUVPLANAR_NV21        PIM_IMAGE_FORMAT_BGR888 
 *                                          PIM_IMAGE_FORMAT_RGB8880
 *                                          PIM_IMAGE_FORMAT_BGR8880
 *
 *  @note width and height must be divisible by 2.
 */
PIM_Result PIM_BitmapConvert_YuvPlanarToRgb(PIM_Bitmap* dst, PIM_Bitmap* src)
{
    /* width / height 는 padding이 맞는 경우만 고려한다. */
    PIM_Int32 width = src->width;
    PIM_Int32 height = src->height;
    PIM_Int32 x,y;
    PIM_Int32 sample_period_w;
    PIM_Int32 sample_period_h;
    PIM_Int32 rgb_period = PIM_Bitmap_GetChannel(dst);
	PIM_Int32 dst_stride = dst->stride;
	PIM_Int32 src_stride = src->stride;

    PIM_Uint8 *red_base, *green_base, *blue_base;
    PIM_Uint8 *red, *green, *blue;
	PIM_Uint8 *py_base, *pu_base, *pv_base;
    PIM_Uint8 *py, *pu, *pv;

    PIM_Int32 uv_period = 0;
    PIM_Int32 uv_step_width = 0;

    /*  */
    switch(dst->imageFormat)
    {
        case PIM_IMAGE_FORMAT_RGB888:
        case PIM_IMAGE_FORMAT_RGB8880:
            red_base   = (PIM_Uint8*)&dst->imageData[0];
            green_base = (PIM_Uint8*)&dst->imageData[1];
            blue_base  = (PIM_Uint8*)&dst->imageData[2];
            break;

        case PIM_IMAGE_FORMAT_BGR888:
        case PIM_IMAGE_FORMAT_BGR8880:
            red_base   = (PIM_Uint8*)&dst->imageData[2];
            green_base = (PIM_Uint8*)&dst->imageData[1];
            blue_base  = (PIM_Uint8*)&dst->imageData[0];
            break;

        default:
            return PIM_ERROR_NOT_SUPPORTED;
    }

    /*  */
    switch(src->imageFormat)
    {
        case PIM_IMAGE_FORMAT_YUVPLANAR_NV12:
            py_base = (PIM_Uint8*)src->imageData;
            if(src->auxImageData) pu_base = src->auxImageData;
            else                  pu_base = py_base + src_stride * height;
            pv_base = pu_base + 1;
            sample_period_w = 2;
            sample_period_h = 2;
            uv_period = 2;
            uv_step_width = src_stride/2;
            break;

        case PIM_IMAGE_FORMAT_YUVPLANAR_NV21:
            py_base = (PIM_Uint8*)src->imageData;
            if(src->auxImageData) pv_base = src->auxImageData;
            else                  pv_base = py_base + src_stride * height;
            pu_base = pv_base + 1;
            sample_period_w = 2;
            sample_period_h = 2;
            uv_period = 2;
            uv_step_width = src_stride/2;
            break;

        case PIM_IMAGE_FORMAT_YUVPLANAR_YV12:
            py_base = (PIM_Uint8*)src->imageData;

            pv_base = py_base + src_stride * height;
            pu_base = pv_base + (src_stride * height)/4;
            sample_period_w = 2;
            sample_period_h = 2;
            uv_period = 1;
            uv_step_width = src_stride/4;
            break;

        case PIM_IMAGE_FORMAT_YUVPLANAR_I420:
            py_base = (PIM_Uint8*)src->imageData;

            pu_base = py_base + src_stride * height;
            pv_base = pu_base + (src_stride * height)/4;
            sample_period_w = 2;
            sample_period_h = 2;
            uv_period = 1;
            uv_step_width = src_stride/4;
            break;

        default:
            return PIM_ERROR_NOT_SUPPORTED;
    }


    /* sample 2x2 */
    if(sample_period_w == 2 && sample_period_h == 2)
    {
        for (y=0; y<height; y++)
        {
			red   = &red_base  [y*dst_stride];
			green = &green_base[y*dst_stride];
			blue  = &blue_base [y*dst_stride];
			py     = &py_base    [y*src_stride];
			pu     = &pu_base    [(y&~1)*uv_step_width];
			pv     = &pv_base    [(y&~1)*uv_step_width];

            for (x=0; x<width; x+=sample_period_w)
            {
                YUV2RGB(py[0], pu[0], pv[0], red[0], green[0], blue[0]);
                red  +=rgb_period;
                green+=rgb_period;
                blue +=rgb_period;
                py++;

                YUV2RGB(py[0], pu[0], pv[0], red[0], green[0], blue[0]);
                red  +=rgb_period;
                green+=rgb_period;
                blue +=rgb_period;
                py++;

                pu+=uv_period;
                pv+=uv_period;
            }
        }
    } else {
        return PIM_ERROR_NOT_SUPPORTED;
    }

    return PIM_SUCCESS;    
}

PIM_Result PIM_BitmapConvert_YuvPlanarToYuvPlanar(PIM_Bitmap *dst, PIM_Bitmap *src)
{
    PIM_Int32 width = src->width;
    PIM_Int32 height = src->height;
    PIM_Int32 i,j;
	PIM_Int32 dst_stride = dst->stride;
	PIM_Int32 src_stride = src->stride;

    PIM_Int32 sample_period_w;
    PIM_Int32 sample_period_h;

	PIM_Uint8 *dst_u_base, *dst_v_base;
    PIM_Uint8 *dst_u, *dst_v;
    PIM_Int32 dst_uv_period = 0;
    PIM_Int32 dst_uv_step_width = 0;

	PIM_Uint8 *src_u_base, *src_v_base;
    PIM_Uint8 *src_u, *src_v;
    PIM_Int32 src_uv_period = 0;
    PIM_Int32 src_uv_step_width = 0;

    /*  */
    switch(dst->imageFormat)
    {
        case PIM_IMAGE_FORMAT_YUVPLANAR_NV12:
            if(dst->auxImageData) dst_u_base = dst->auxImageData;
            else                  dst_u_base = dst->imageData + dst_stride * height;
            dst_v_base = dst_u_base + 1;
            dst_uv_period = 2;
            dst_uv_step_width = dst_stride/2;

            sample_period_w = 2;
            sample_period_h = 2;
            break;

        case PIM_IMAGE_FORMAT_YUVPLANAR_NV21:
            if(dst->auxImageData) dst_v_base = dst->auxImageData;
            else                  dst_v_base = dst->imageData + dst_stride * height;
            dst_u_base = dst_v_base + 1;
            dst_uv_period = 2;
            dst_uv_step_width = dst_stride/2;

            sample_period_w = 2;
            sample_period_h = 2;
            break;

        case PIM_IMAGE_FORMAT_YUVPLANAR_YV12:
            if(dst->auxImageData) dst_v_base = dst->auxImageData;
            else                  dst_v_base = dst->imageData + dst_stride * height;
            dst_u_base = dst_v_base + (dst_stride * height)/4;
            dst_uv_period = 1;
            dst_uv_step_width = dst_stride/4;

            sample_period_w = 2;
            sample_period_h = 2;
            break;

        case PIM_IMAGE_FORMAT_YUVPLANAR_I420:
            if(dst->auxImageData) dst_u_base = dst->auxImageData;
            else                  dst_u_base = dst->imageData + dst_stride * height;
            dst_v_base = dst_u_base + (dst_stride * height)/4;
            dst_uv_period = 1;
            dst_uv_step_width = dst_stride/4;

            sample_period_w = 2;
            sample_period_h = 2;
            break;

        default:
            return PIM_ERROR_NOT_SUPPORTED;
    }

    switch(src->imageFormat)
    {
        case PIM_IMAGE_FORMAT_YUVPLANAR_NV12:
            if(src->auxImageData) src_u_base = src->auxImageData;
            else                  src_u_base = src->imageData + src_stride * height;
            src_v_base = src_u_base + 1;
            src_uv_period = 2;
            src_uv_step_width = src_stride/2;

            sample_period_w = 2;
            sample_period_h = 2;
            break;

        case PIM_IMAGE_FORMAT_YUVPLANAR_NV21:
            if(src->auxImageData) src_v_base = src->auxImageData;
            else                  src_v_base = src->imageData + src_stride * height;
            src_u_base = src_v_base + 1;
            src_uv_period = 2;
            src_uv_step_width = src_stride/2;

            sample_period_w = 2;
            sample_period_h = 2;
            break;

        case PIM_IMAGE_FORMAT_YUVPLANAR_YV12:
            if(src->auxImageData) src_v_base = src->auxImageData;
            else                  src_v_base = src->imageData + src_stride * height;
            src_u_base = src_v_base + (src_stride * height)/4;
            src_uv_period = 1;
            src_uv_step_width = src_stride/4;

            sample_period_w = 2;
            sample_period_h = 2;
            break;

        case PIM_IMAGE_FORMAT_YUVPLANAR_I420:
            if(src->auxImageData) src_u_base = src->auxImageData;
            else                  src_u_base = src->imageData + src_stride * height;
            src_v_base = src_u_base + (src_stride * height)/4;
            src_uv_period = 1;
            src_uv_step_width = src_stride/4;

            sample_period_w = 2;
            sample_period_h = 2;
            break;

        default:
            return PIM_ERROR_NOT_SUPPORTED;
    }


    /* copy Y image */
    PIM_Memcpy(dst->imageData, src->imageData, width * height);


    /* sample 2x2 */
    if(sample_period_w == 2 && sample_period_h == 2)
    {
        for (i=0; i<height; i+=sample_period_h)
        {
			if((i+1) >= height) break;	/* 마지막줄이 처리 안될거 같은데 처리가 되는것처럼 보임. 버그 소지 다분함. */

            /* first line */
			dst_u = &dst_u_base    [(i&~1)*dst_uv_step_width];
			dst_v = &dst_v_base    [(i&~1)*dst_uv_step_width];
			src_u = &src_u_base    [(i&~1)*src_uv_step_width];
			src_v = &src_v_base    [(i&~1)*src_uv_step_width];

            for (j=0; j<width; j+=sample_period_w)
            {
                *dst_u = *src_u;
                *dst_v = *src_v;

                dst_u+=dst_uv_period;
                dst_v+=dst_uv_period;
                src_u+=src_uv_period;
                src_v+=src_uv_period;
            }
        }
    } else {
        return PIM_ERROR_NOT_SUPPORTED;
    }

    return PIM_SUCCESS;    
}



/*
 * supported format
 *   @ src                                  @ dst
 *   PIM_IMAGE_FORMAT_YUVPACKED_Y411   =>   PIM_IMAGE_FORMAT_GRAY 
 *
 *  @note width must be divisible by 4.
 */
PIM_Result PIM_BitmapConvert_YuvPackedY411ToGray(PIM_Bitmap *dst, PIM_Bitmap *src)
{
    PIM_Int32 width  = src->width;
    PIM_Int32 height = src->height;
    PIM_Int32 x,y;

    PIM_Uint8 *pyuv;
    PIM_Uint8 *pgray;

    pgray = dst->imageData;
    pyuv  = src->imageData;

    for (y=0; y<height; y++) {
        for (x=0; x<width; x+=4) {
            *pgray++ = pyuv[1];
            *pgray++ = pyuv[2];
            *pgray++ = pyuv[4];
            *pgray++ = pyuv[5];
            pyuv += 6;
        }
    }

    return PIM_SUCCESS;
}



/*
 * supported format
 *   @ src                                  @ dst
 *   PIM_IMAGE_FORMAT_YUVPACKED_Y411   =>   PIM_IMAGE_FORMAT_RGB888 
 *                                          PIM_IMAGE_FORMAT_BGR888 
 *                                          PIM_IMAGE_FORMAT_RGB8880
 *                                          PIM_IMAGE_FORMAT_BGR8880
 *
 *  @note width must be divisible by 4.
 */
PIM_Result PIM_BitmapConvert_YuvPackedY411ToRgb(PIM_Bitmap* dst, PIM_Bitmap* src)
{
    PIM_Int32 width  = src->width;
    PIM_Int32 height = src->height;
    PIM_Int32 i, j;
    PIM_Int32 dst_period = PIM_Bitmap_GetPeriod(dst);

    PIM_Uint8 *red, *green, *blue;
    PIM_Uint8 *yuv;

    /*  */
    if((width&0x3) != 0) {
        LOGE("wrong width(%d) -> width(%d)", width, width&(~0x3));
        return PIM_ERROR_INVALID_ARGUMENT;
    }


    /*  */
    switch(src->imageFormat)
    {
        case PIM_IMAGE_FORMAT_RGB888:
        case PIM_IMAGE_FORMAT_RGB8880:
            red   = (PIM_Uint8*)&src->imageData[0];
            green = (PIM_Uint8*)&src->imageData[1];
            blue  = (PIM_Uint8*)&src->imageData[2];
            break;

        case PIM_IMAGE_FORMAT_BGR888:
        case PIM_IMAGE_FORMAT_BGR8880:
            red   = (PIM_Uint8*)&src->imageData[2];
            green = (PIM_Uint8*)&src->imageData[1];
            blue  = (PIM_Uint8*)&src->imageData[0];
            break;

        default:
            return PIM_ERROR_NOT_SUPPORTED;
    }

    yuv = src->imageData;

    for(j=0; j<height; j++) {
        for(i=0; i<width; i+=4) { /* 4 : pixel per packed unit. */
            YUV2RGB(yuv[1], yuv[0], yuv[3], red[0], green[0], blue[0]);
            red  +=dst_period;
            green+=dst_period;
            blue +=dst_period;

            YUV2RGB(yuv[2], yuv[0], yuv[3], red[0], green[0], blue[0]);
            red  +=dst_period;
            green+=dst_period;
            blue +=dst_period;

            YUV2RGB(yuv[4], yuv[0], yuv[3], red[0], green[0], blue[0]);
            red  +=dst_period;
            green+=dst_period;
            blue +=dst_period;

            YUV2RGB(yuv[5], yuv[0], yuv[3], red[0], green[0], blue[0]);
            red  +=dst_period;
            green+=dst_period;
            blue +=dst_period;

            yuv+=6;    /* 6 : Y411 packed period */
        }
    }

    return PIM_SUCCESS;
}
