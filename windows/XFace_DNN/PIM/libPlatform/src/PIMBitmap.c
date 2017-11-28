#include "libPlatform/inc/PIMMemory.h"
#include "libPlatform/inc/PIMLog.h"
#define TAG "PIM"

#include "libPlatform/inc/PIMRect.h"
#include "libPlatform/inc/PIMBitmap.h"
#include "libPlatform/inc/PIMColor.h"
#include "libPlatform/inc/PIMMath.h"
#include "libPlatform/inc/PIMByteStreamIO.h"


#ifdef WIN32
#define DEFAULT_ALIGN_NUM   4
#else
#define DEFAULT_ALIGN_NUM   4
#endif


#ifdef __cplusplus
extern "C" {
#endif

static PIM_Result Bitmap_CropResize_YUVPLANAR_NV(PIM_Bitmap *dst, PIM_Bitmap *src, PIM_Rect *cropRect);
static PIM_Result Bitmap_CropResize_Gray        (PIM_Bitmap *dst, PIM_Bitmap *src, PIM_Rect *cropRect);
static PIM_Result Bitmap_CropResize_RGB         (PIM_Bitmap *dst, PIM_Bitmap *src, PIM_Rect *cropRect);
static PIM_Result Bitmap_Create                 (PIM_Bitmap *bitmap, PIM_Uint8 *imageData, PIM_Uint8 *auxImageData, PIM_Int32 width, PIM_Int32 height, PIM_Int32 stride, PIM_ImageFormat format);

// to do implementation
extern PIM_Result PIM_Bitmap_ResizeNN(PIM_Bitmap *dst, PIM_Bitmap *src, PIM_Int32 sampling);

/*
 * color_type
 *  0:  color 0x......Rr
 *  1:  color 0x......Gg
 *  2:  color 0x......Bb
 *  3:  color 0x......Aa
 *  4:  color 0xAaRrGgBb
 */
PIM_Uint32 Get_Pixel_RGB(PIM_Bitmap *bitmap, PIM_Int32 x, PIM_Int32 y, Color_Type type)
{
    PIM_Uint32 color = 0;
    PIM_Int32 stride  = bitmap->stride;
    PIM_Uint8* imageData = bitmap->imageData;
    PIM_Int32  channel = PIM_Bitmap_GetChannel(bitmap);

    switch(bitmap->imageFormat)
    {
        case PIM_IMAGE_FORMAT_RGB888:
        case PIM_IMAGE_FORMAT_RGB8880:
        {
            if(type >= Color_Type_ERed && type <= Color_Type_EBlur) {
                color = imageData[stride * y + x * channel + type];
            } else if(type == Color_Type_ERGB) {
                color = (imageData[stride * y + x * channel + 0] << 16) |
                        (imageData[stride * y + x * channel + 1] <<  8) |
                        (imageData[stride * y + x * channel + 2]      );
            } else {
                color = 0xff;
            }
        } break;

        case PIM_IMAGE_FORMAT_BGR888:
        case PIM_IMAGE_FORMAT_BGR8880:
        {
            if(type >= Color_Type_ERed && type <= Color_Type_EBlur) {
                color = imageData[stride * y + x * channel + (2-type)];
            } else if (type == Color_Type_ERGB) {
                color = (imageData[stride * y + x * channel + 0]      ) |
                        (imageData[stride * y + x * channel + 1] <<  8) |
                        (imageData[stride * y + x * channel + 2] << 16);
            } else {
                color = 0xff;
            }
        } break;

        default:
            color = PIM_ERROR_NOT_SUPPORTED;
            break;
    }

    return color;
}


PIM_Uint32 Set_Pixel_RGB(PIM_Bitmap *bitmap, PIM_Int32 x, PIM_Int32 y, Color_Type type, PIM_Uint32 color)
{
    PIM_Int32 stride = bitmap->stride;
    PIM_Uint8* imageData = bitmap->imageData;
    PIM_Int32  channel = PIM_Bitmap_GetChannel(bitmap);

    switch(bitmap->imageFormat)
    {
        case PIM_IMAGE_FORMAT_RGB888:
        case PIM_IMAGE_FORMAT_RGB8880:
        {
            if(type >= Color_Type_ERed && type <= Color_Type_EBlur) {
                imageData[stride * y + x * channel + type] = color;
            } else if (type == Color_Type_ERGB) {
                imageData[stride * y + x * channel + 0] = color>>16&0x000000ff;
                imageData[stride * y + x * channel + 1] = color>> 8&0x000000ff;
                imageData[stride * y + x * channel + 2] = color    &0x000000ff;
            } else {
                color = 0xff;
            }
        } break;

        case PIM_IMAGE_FORMAT_BGR888:
        case PIM_IMAGE_FORMAT_BGR8880:
        {
            if(type >= Color_Type_ERed && type <= Color_Type_EBlur) {
                imageData[stride * y + x * channel + (2-type)] = color;
            } else if (type == Color_Type_ERGB) {
                imageData[stride * y + x * channel + 0] = color    &0x000000ff;
                imageData[stride * y + x * channel + 1] = color>> 8&0x000000ff;
                imageData[stride * y + x * channel + 2] = color>>16&0x000000ff;
            } else {
                color = 0xff;
            }
        } break;

        default:
            return PIM_ERROR_NOT_SUPPORTED;
    }

    return PIM_SUCCESS;
}



/*
 *  @brief 
 *
 *  @param bitmap[OUT] 
 *  @param imageData[IN] From outside the allocated memory
 *  @param width[IN] image width
 *  @param height[IN] image height
 *  @param stride[IN] The number of bytes in "imageData" to skip between rows
 *  @param format[IN] 
 *
 *  @return ret != PIM_SUCCESS : fail, error code.
 *  @return ret == PIM_SUCCESS : success
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
PIM_Result PIM_Bitmap_CreateRef(PIM_Bitmap* bitmap, PIM_Uint8* imageData, PIM_Int32 width, PIM_Int32 height, PIM_Int32 stride, PIM_ImageFormat format)
{
    return Bitmap_Create(bitmap, imageData, NULL, width, height, stride, format);
}

PIM_Result PIM_Bitmap_CreateYuvRef(PIM_Bitmap *bitmap, PIM_Uint8 *y_data, PIM_Uint8 *uv_data, PIM_Int32 width, PIM_Int32 height, PIM_Int32 stride, PIM_ImageFormat format)
{
    return Bitmap_Create(bitmap, y_data, uv_data, width, height, stride, format);
}

static PIM_Result Bitmap_Create(PIM_Bitmap *bitmap, PIM_Uint8 *imageData, PIM_Uint8 *auxImageData, PIM_Int32 width, PIM_Int32 height, PIM_Int32 stride, PIM_ImageFormat format)
{
    PIM_Size_t size;
    PIM_Int32 align_num = DEFAULT_ALIGN_NUM;
    if (bitmap == NULL) return PIM_ERROR_INVALID_ARGUMENT;

    PIM_Memset(bitmap, 0, sizeof(PIM_Bitmap));

    /* common set */
    bitmap->width       = width;
    bitmap->height      = height;
    bitmap->imageFormat = format;
    bitmap->align       = align_num;

    /* if the value of the stride is valid, align value is ignored. */
    if(stride > 0) {
        bitmap->stride = stride;
    } else {
        bitmap->stride = PIM_Bitmap_GetStride(format, width, align_num);
    }

    /* allocate memory */
    if(imageData == NULL) {
        switch(format)
        {
            case PIM_IMAGE_FORMAT_GRAY:
            case PIM_IMAGE_FORMAT_RGB888:
            case PIM_IMAGE_FORMAT_RGB8880:
            case PIM_IMAGE_FORMAT_RGBA8888:
            case PIM_IMAGE_FORMAT_BGR888:
            case PIM_IMAGE_FORMAT_BGR8880:
            case PIM_IMAGE_FORMAT_YUVPACKED_Y411:
            {
                size = bitmap->stride * height;
                bitmap->imageData = (PIM_Uint8 *)PIM_Calloc(1, size);
            } break;

            case PIM_IMAGE_FORMAT_RGB_INDEX:
            {
                size = bitmap->stride * height;
                bitmap->imageData    = (PIM_Uint8 *)PIM_Calloc(1, size);
                bitmap->auxImageData = (PIM_Uint8 *)PIM_Calloc(1, 256*4); /* palete */
            } break;

            case PIM_IMAGE_FORMAT_YUVPLANAR_NV12:
            case PIM_IMAGE_FORMAT_YUVPLANAR_NV21:
            case PIM_IMAGE_FORMAT_YUVPLANAR_YV12:
            case PIM_IMAGE_FORMAT_YUVPLANAR_I420:
            {
                size = bitmap->stride * height * 3 / 2;
                bitmap->imageData = (PIM_Uint8 *)PIM_Calloc(1, size);
				PIM_Memset(bitmap->imageData + bitmap->stride*height, 0x80, bitmap->stride*height/2);
            } break;

            default:
                return PIM_ERROR_NOT_SUPPORTED;
        }
    } else {
        bitmap->imageData = imageData;
        bitmap->auxImageData = auxImageData;
        bitmap->reference = 1;
    }

    return PIM_SUCCESS;
}



/*
 *
 */
PIM_Result PIM_Bitmap_Create(PIM_Bitmap* bitmap, PIM_Int32 width, PIM_Int32 height, PIM_ImageFormat format)
{
    if (bitmap == NULL) return PIM_ERROR_INVALID_ARGUMENT;

    return Bitmap_Create(bitmap, NULL, NULL, width, height, 0, format);
}



/*
 *
 */
PIM_Result PIM_Bitmap_Recreate(PIM_Bitmap* bitmap, PIM_Int32 width, PIM_Int32 height, PIM_ImageFormat format)
{
    if (bitmap == NULL) return PIM_ERROR_INVALID_ARGUMENT;

    if(bitmap->width == width && bitmap->height == height && bitmap->imageData != NULL) {
        return PIM_SUCCESS;
    }

    PIM_Bitmap_Destroy(bitmap);

    return Bitmap_Create(bitmap, NULL, NULL, width, height, 0, format);
}



/*
 *
 */
PIM_Result PIM_Bitmap_Destroy(PIM_Bitmap* bitmap)
{
    if (!bitmap) return PIM_ERROR_INVALID_ARGUMENT;

    /*  */
    if(bitmap->reference == PIM_TRUE) {
        PIM_Memset(bitmap, 0, sizeof(PIM_Bitmap));
        return PIM_SUCCESS;
    }

    /*  */
    if(bitmap->imageData ) {
        PIM_Free(bitmap->imageData);
        bitmap->imageData = NULL;
    }

    /*  */
    if(bitmap->auxImageData ) {
        PIM_Free(bitmap->auxImageData);
        bitmap->auxImageData = NULL;
    }

    PIM_Memset(bitmap, 0, sizeof(PIM_Bitmap));

    return PIM_SUCCESS;
}



/*
 *  @brief copy only "image data".
 *  @note Image of the property must be the same. (width / height / format)
 */
PIM_Result PIM_Bitmap_Copy(PIM_Bitmap *dst, PIM_Bitmap *src)
{
    PIM_Int32 y;
    PIM_Int32 width, height;
    PIM_Int32 dst_stride;
    PIM_Int32 src_stride;
    PIM_Uint8* pDst = NULL;
    PIM_Uint8* pSrc = NULL;
    PIM_ImageFormat format;
    PIM_Int32 channel = PIM_Bitmap_GetChannel(src);

    if(dst->width != src->width || dst->height != src->height || dst->imageFormat != src->imageFormat) {
        return PIM_ERROR_INVALID_ARGUMENT;
    }

    width = dst->width;
    height = dst->height;
    dst_stride = dst->stride;
    src_stride = src->stride;
    format = src->imageFormat;

    switch(format)
    {
        case PIM_IMAGE_FORMAT_RGB_INDEX:    /* be careful, no break */
            PIM_Memcpy(dst->auxImageData, src->auxImageData, 256);

	    case PIM_IMAGE_FORMAT_GRAY:
        case PIM_IMAGE_FORMAT_RGB888:
        case PIM_IMAGE_FORMAT_RGB8880:
        case PIM_IMAGE_FORMAT_RGBA8888:
        case PIM_IMAGE_FORMAT_BGR888:
        case PIM_IMAGE_FORMAT_BGR8880:
        case PIM_IMAGE_FORMAT_YUVPACKED_Y411:
        {
            pDst = dst->imageData;
            pSrc = src->imageData;

            for(y=0; y<height; y++) {
                PIM_Memcpy(pDst, pSrc, width * channel);

                pDst += dst_stride;
                pSrc += src_stride;
            }
        } break;

        case PIM_IMAGE_FORMAT_YUVPLANAR_NV12:
        case PIM_IMAGE_FORMAT_YUVPLANAR_NV21:
        case PIM_IMAGE_FORMAT_YUVPLANAR_YV12:
        case PIM_IMAGE_FORMAT_YUVPLANAR_I420:
        {
            PIM_Int32 half_h = height>>1;

            /* Y */
            pDst = dst->imageData;
            pSrc = src->imageData;

            for(y=0; y<height; y++) {
                PIM_Memcpy(pDst, pSrc, width);

                pDst += dst_stride;
                pSrc += src_stride;
            }

            /* UV */
            if(dst->auxImageData) pDst = dst->auxImageData;
            else                  pDst = dst->imageData + dst->stride * dst->height;
            if(src->auxImageData) pSrc = src->auxImageData;
            else                  pSrc = src->imageData + src->stride * src->height;

            for(y=0; y<half_h; y++) {
                PIM_Memcpy(pDst, pSrc, width);

                pDst += dst_stride;
                pSrc += src_stride;
            }
        } break;

        default:
            return PIM_ERROR_NOT_SUPPORTED;
    }

    return PIM_SUCCESS;
}



/*
 *
 */
PIM_Result PIM_Bitmap_Clone(PIM_Bitmap *dst, PIM_Bitmap *src)
{
    PIM_Result ret = PIM_SUCCESS;

    if(dst == src) return PIM_SUCCESS;

    PIM_Bitmap_Destroy(dst);
    ret = PIM_Bitmap_Create(dst, src->width, src->height, src->imageFormat);
    if (ret == PIM_SUCCESS)
    {
        ret = PIM_Bitmap_Copy(dst, src);
    }

    return ret;
}



/*
 *
 */
PIM_Int32 PIM_Bitmap_GetBitPerPixel(PIM_Bitmap* bitmap)
{
    PIM_Int32 bitsPerPixel;
    switch (bitmap->imageFormat) {
        case PIM_IMAGE_FORMAT_GRAY:    
        case PIM_IMAGE_FORMAT_RGB_INDEX:    
            bitsPerPixel = 8;
            break;

        case PIM_IMAGE_FORMAT_RGB888:    
        case PIM_IMAGE_FORMAT_BGR888:
            bitsPerPixel = 24;
            break;
#if 0
        case PIM_ImageFormat_ERgb565L:
        case PIM_ImageFormat_ERgb565B:
            bitsPerPixel = 16;
            break;
#endif
        case PIM_IMAGE_FORMAT_BGR8880:
        case PIM_IMAGE_FORMAT_RGB8880:
        case PIM_IMAGE_FORMAT_RGBA8888:
        //case PIM_ImageFormat_EBgr0888:
            bitsPerPixel = 32;
            break;
#if 0
        case PIM_ImageFormat_EYuv422YUYV:
        case PIM_ImageFormat_EYuv422UYVY:
        case PIM_ImageFormat_EYuv422Plane:
            bitsPerPixel = 16;
            break;
#endif
        //case PIM_ImageFormat_EYuv420IYUV:
        case PIM_IMAGE_FORMAT_YUVPLANAR_NV12:
        case PIM_IMAGE_FORMAT_YUVPLANAR_NV21:
        case PIM_IMAGE_FORMAT_YUVPLANAR_YV12:
        case PIM_IMAGE_FORMAT_YUVPLANAR_I420:
        //case PIM_IMAGE_FORMAT_YUVPLANAR_NV12_SPLIT:
        //case PIM_IMAGE_FORMAT_YUVPLANAR_NV21_SPLIT:
            bitsPerPixel = 12;
            break;
        default:
            return PIM_ERROR_NOT_SUPPORTED;
    }

    return bitsPerPixel;
}



/*
 *
 */
PIM_Int32 PIM_Bitmap_GetChannel(PIM_Bitmap* bitmap)
{
    PIM_Int32 channel;
    channel = PIM_Bitmap_GetBitPerPixel(bitmap) / 8;
    return channel;
}


/* RGB or YUV(y) */
PIM_Int32 PIM_Bitmap_GetPeriod(PIM_Bitmap *bitmap)
{
    switch(bitmap->imageFormat) {
        case PIM_IMAGE_FORMAT_GRAY:
            return 1;

#if 0
        case PIM_ImageFormat_ERgb565:
        case PIM_ImageFormat_EYuv422YUYV:
        case PIM_ImageFormat_EYuv422UYVY:
            return 2;
#endif

        case PIM_IMAGE_FORMAT_RGB888:
        case PIM_IMAGE_FORMAT_BGR888:
        //case PIM_ImageFormat_EYuv444:
            return 3;

        case PIM_IMAGE_FORMAT_BGR8880:
        case PIM_IMAGE_FORMAT_RGB8880:
        case PIM_IMAGE_FORMAT_RGBA8888:
            return 4;

        //case PIM_ImageFormat_EYuv422YYUV:
        //case PIM_ImageFormat_EYuv422Plane:
        //case PIM_ImageFormat_EYuv420IYUV:
        //case PIM_IMAGE_FORMAT_YUVPLANAR_NV12:
        //case PIM_IMAGE_FORMAT_YUVPLANAR_NV21:
        //case PIM_IMAGE_FORMAT_YUVPLANAR_NV12_SPLIT:
        //case PIM_IMAGE_FORMAT_YUVPLANAR_NV21_SPLIT:
        //case PIM_ImageFormat_EYuvPacked_Y41P:
        //case PIM_ImageFormat_EYuvPacked_Y411:

        default:
            return PIM_ERROR_GENERAL;
    }
}



/*
 *  @brief 이미지 포맷과 width/align_num 을 계산하여 stride를 리턴한다. 
 *  @note padding은 실제 data_length를 기준으로 한다.
 */
PIM_Int32 PIM_Bitmap_GetStride(PIM_ImageFormat format, PIM_Int32 width, PIM_Int32 align_num)
{
    PIM_Int32 width_length;
    PIM_Int32 stride;
    PIM_Int32 mod_num;

    if(align_num <= 0) align_num = DEFAULT_ALIGN_NUM;

    switch(format)
    {
        case PIM_IMAGE_FORMAT_RGB888:
        case PIM_IMAGE_FORMAT_BGR888:
        case PIM_IMAGE_FORMAT_YUVPACKED_Y411:
        {
            width_length = width * 3;
        } break;

        case PIM_IMAGE_FORMAT_RGB8880:
        case PIM_IMAGE_FORMAT_RGBA8888:
        case PIM_IMAGE_FORMAT_BGR8880:
        {
            width_length = width * 4;
        } break;

        case PIM_IMAGE_FORMAT_RGB_INDEX:
        case PIM_IMAGE_FORMAT_GRAY:
        case PIM_IMAGE_FORMAT_YUVPLANAR_NV12:
        case PIM_IMAGE_FORMAT_YUVPLANAR_NV21:
        case PIM_IMAGE_FORMAT_YUVPLANAR_YV12:
        case PIM_IMAGE_FORMAT_YUVPLANAR_I420:
        {
            width_length = width * 1;
        } break;

        default:
            return PIM_ERROR_NOT_SUPPORTED;
    }

    mod_num = width_length % align_num;
    if(mod_num) stride = width_length + (align_num - mod_num);
    else        stride = width_length;

    return stride;
}



/*
 *  @brief size of the allocated memory is returned.
 */
PIM_Int32 PIM_Bitmap_GetImageSize(PIM_Bitmap *bitmap)
{
    PIM_Int32 imageSize = 0;

    switch(bitmap->imageFormat)
    {
	    case PIM_IMAGE_FORMAT_GRAY:
        case PIM_IMAGE_FORMAT_RGB888:
        case PIM_IMAGE_FORMAT_RGB8880:
        case PIM_IMAGE_FORMAT_RGBA8888:
        case PIM_IMAGE_FORMAT_BGR888:
        case PIM_IMAGE_FORMAT_BGR8880:
            imageSize = bitmap->stride * bitmap->height;
            break;

        case PIM_IMAGE_FORMAT_YUVPLANAR_NV12:
        case PIM_IMAGE_FORMAT_YUVPLANAR_NV21:
        case PIM_IMAGE_FORMAT_YUVPLANAR_YV12:
        case PIM_IMAGE_FORMAT_YUVPLANAR_I420:
            imageSize = bitmap->stride * bitmap->height * 3 / 2;
            break;

        default:
            imageSize = PIM_ERROR_NOT_SUPPORTED;
            break;
    }

    return imageSize;
}



/*
 *
 */
PIM_Bool PIM_Bitmap_isValid(PIM_Bitmap* bitmap)
{
    if(bitmap == NULL) return PIM_FALSE;
    if(bitmap->imageData == NULL) return PIM_FALSE;
    if(bitmap->width <= 0 || bitmap->height <= 0) return PIM_FALSE;

    return PIM_TRUE;
}

PIM_Bool PIM_Bitmap_isValidPoint(PIM_Bitmap *bitmap, PIM_Point *pos)
{
    PIM_Rect bitmap_rect = {0};
    PIM_Rect_SetRect(&bitmap_rect, 0, 0, bitmap->width-1, bitmap->height-1);

    return PIM_Rect_IsInRect(pos->x, pos->y, &bitmap_rect);
}

PIM_Bool PIM_Bitmap_isValidRect(PIM_Bitmap *bitmap, PIM_Rect *rect)
{
    PIM_Rect bitmap_rect = {0};
    PIM_Rect_SetRect(&bitmap_rect, 0, 0, bitmap->width-1, bitmap->height-1);

    return PIM_Rect_Intersects(&bitmap_rect, rect);
}



/*
 *
 */
#define BIT_SHIFT 10
PIM_Result PIM_Bitmap_Resize(PIM_Bitmap* dst, PIM_Bitmap* src)
{
    PIM_Int32 channel;
    PIM_Int32 src_w = src->width;
    PIM_Int32 src_h = src->height;
    PIM_Int32 des_w = dst->width;
    PIM_Int32 des_h = dst->height;
    PIM_Uint32 ratio_w, ratio_h;
    PIM_Int32 x, y;

    PIM_Uint8 *pSrc = src->imageData;
    PIM_Uint8 *pDst = dst->imageData;
    PIM_Uint8 *src_buf, *dst_buf;

#ifndef __LOG_INFO__	
	PIM_Uint32 time;
	time = PIM_Device_msClock();
#endif

    if( !des_w || !des_h ) return PIM_ERROR_INVALID_ARGUMENT;
    ratio_w = (src_w << BIT_SHIFT) / des_w;
    ratio_h = (src_h << BIT_SHIFT) / des_h;

    if(!pSrc) return PIM_ERROR_NULL_POINTER;
    if(!pDst) return PIM_ERROR_NULL_POINTER;

    channel = PIM_Bitmap_GetChannel(src);

    switch(src->imageFormat)
    {
        case PIM_IMAGE_FORMAT_YUVPLANAR_NV12:
        case PIM_IMAGE_FORMAT_YUVPLANAR_NV21:
        {
            PIM_Int32 index = 0;

            /* y */
            for(y=0; y<des_h; y++)
            {
                src_buf = &pSrc[((ratio_h*y)>>BIT_SHIFT)*src->stride];
                dst_buf = &pDst[y                       *dst->stride];

                *dst_buf++ = src_buf[0];
                for(x=1; x<des_w; x++) {
                    *dst_buf++ = src_buf[(ratio_w*(x+1))>>BIT_SHIFT];
                }
            }

            /* uv */
            if(src->auxImageData == NULL) pSrc = src->imageData + src->stride*src->height;
            else                          pSrc = src->auxImageData;

            if(dst->auxImageData == NULL) pDst = dst->imageData + dst->stride*dst->height;
            else                          pDst = dst->auxImageData;

            des_h = ((des_h+1)&~1)/2;
            des_w = ((des_w+1)&~1)/2;

            for(y=0; y<des_h; y++)
            {
                src_buf = &pSrc[((ratio_h*y)>>BIT_SHIFT)*src->stride];
                dst_buf = &pDst[y                       *dst->stride];

                /* x == 0 */
                dst_buf[0] = src_buf[0];
                dst_buf[1] = src_buf[1];
                dst_buf+=2;

                /* x > 0 */
                for(x=1; x<des_w; x++) {
                    index = ((ratio_w*(x))>>BIT_SHIFT)*2;
                    dst_buf[0] = src_buf[index  ];
                    dst_buf[1] = src_buf[index+1];

                    dst_buf+=2;
                }
            }
        } break;

        case PIM_IMAGE_FORMAT_GRAY:
        case PIM_IMAGE_FORMAT_RGB_INDEX:
        {
            for(y=0; y<des_h; y++)
            {
                src_buf = &pSrc[((ratio_h*y)>>BIT_SHIFT)*src->stride];
                dst_buf = &pDst[y                       *dst->stride];

                *dst_buf++ = src_buf[0];
                for(x=1; x<des_w; x++) {
                    *dst_buf++ = src_buf[(ratio_w*(x))>>BIT_SHIFT];
                }
            }
        } break;

        case PIM_IMAGE_FORMAT_RGB888:
        case PIM_IMAGE_FORMAT_RGB8880:
        case PIM_IMAGE_FORMAT_BGR888:
        case PIM_IMAGE_FORMAT_BGR8880:
        {
            PIM_Int32 index = 0;
            for(y=0; y<des_h; y++)
            {
                src_buf = &pSrc[((ratio_h*y)>>BIT_SHIFT)*src->stride];
                dst_buf = &pDst[y                       *dst->stride];

                /* x == 0 */
                dst_buf[0] = src_buf[0];
                dst_buf[1] = src_buf[1];
                dst_buf[2] = src_buf[2];
                dst_buf+=channel;

                /* x > 0 */
                for(x=1; x<des_w; x++) {
                    index = ((ratio_w*(x+1))>>BIT_SHIFT)*channel;
                    dst_buf[0] = src_buf[index  ];
                    dst_buf[1] = src_buf[index+1];
                    dst_buf[2] = src_buf[index+2];

                    dst_buf+=channel;
                }
            }
        } break;

        case PIM_IMAGE_FORMAT_RGBA8888:
        {
            PIM_Int32 index = 0;
            for(y=0; y<des_h; y++)
            {
                src_buf = &pSrc[((ratio_h*y)>>BIT_SHIFT)*src->stride];
                dst_buf = &pDst[y                       *dst->stride];

                /* x == 0 */
                dst_buf[0] = src_buf[0];
                dst_buf[1] = src_buf[1];
                dst_buf[2] = src_buf[2];
                dst_buf[3] = src_buf[3];
                dst_buf+=channel;

                /* x > 0 */
                for(x=0; x<des_w; x++) {
                    index = ((ratio_w*(x))>>BIT_SHIFT)*channel;
                    dst_buf[0] = src_buf[index  ];
                    dst_buf[1] = src_buf[index+1];
                    dst_buf[2] = src_buf[index+2];
                    dst_buf[3] = src_buf[index+3];

                    dst_buf+=channel;
                }
            }
        } break;
		
        default:
            return PIM_ERROR_NOT_SUPPORTED;
    }
#ifndef __LOG_INFO__	
//	LOGD("PIM_Bitmap_Resize(%d->%d) elapsed time = %d ms",src->imageFormat, dst->imageFormat, PIM_Device_msClock() - time);
#endif
    return PIM_SUCCESS;
}



/*
 *  @note PIM_Bitmap_Resize_BI는 PIM_Bitmap_CropResize으로 대체가능하다.
 */
PIM_Result PIM_Bitmap_Resize_BI(PIM_Bitmap *dstbitmap, PIM_Bitmap* srcbitmap)
{
    PIM_Rect rect = {0};
    PIM_Rect_SetRect(&rect, 0, 0, srcbitmap->width-1, srcbitmap->height-1);
    return PIM_Bitmap_CropResize(dstbitmap, srcbitmap, &rect);
}

PIM_Result PIM_Bitmap_RotateVersatile(PIM_Bitmap *dst, PIM_Bitmap *src, PIM_Point *ref_point, PIM_Int32 angle)
{
	const PIM_Int32 cosT = PIM_Math_CosS(angle);
	const PIM_Int32 sinT = PIM_Math_SinS(angle);

	PIM_Int32 dst_height = (((src->width * PIM_Math_Abs(sinT)) + (src->height * PIM_Math_Abs(cosT))) >> PIM_Math_KCosSBaseShifter) -2;
	PIM_Int32 dst_width = (((src->height * PIM_Math_Abs(sinT)) + (src->width * PIM_Math_Abs(cosT))) >> PIM_Math_KCosSBaseShifter) -2;

	PIM_Int32 x1, y1;
	PIM_Int32 mx = dst_width/2;
	PIM_Int32 my = dst_height/2;
	PIM_Int32 msx = (src->width)/2;
	PIM_Int32 msy = (src->height)/2;
	PIM_Int32 x, y;
	
	PIM_Int32 temp_x, temp_y;
	PIM_Int32 temp_xs = -mx * cosT + my * sinT;
	PIM_Int32 temp_ys = -mx * sinT - my * cosT;
	
	PIM_Point temp_point;

    PIM_Uint8* pDstData = NULL;
    PIM_Uint8* pSrcData = NULL;
    PIM_Int32  channel = PIM_Bitmap_GetChannel(src);
    PIM_Int32  src_width, src_height;
    PIM_Int32  dst_stride, src_stride;

	if (PIM_Bitmap_Recreate(dst, dst_width, dst_height, src->imageFormat) != PIM_SUCCESS) return PIM_ERROR_NOMEM;

    src_width  = src->width;
    src_height = src->height;
    src_stride = src->stride;
    dst_stride = dst->stride;


	for(y=0; y<dst_height; y++) {
		
		temp_x = temp_xs;
		temp_y = temp_ys;
		
		for(x=0; x<dst_width; x++) {
			
			x1 = (temp_x >> PIM_Math_KCosSBaseShifter) + msx;
			y1 = (temp_y >> PIM_Math_KCosSBaseShifter) + msy;
			
			if	((0 <= x1) && (x1 <= src_width - 1) && (0 <= y1) && (y1 <= src_height - 1)) {
                pDstData = &dst->imageData[y*dst_stride + x*channel];
                pSrcData = &src->imageData[y1*src_stride + x1*channel];

                PIM_Memcpy(pDstData, pSrcData, channel);

				//dst->imageData[y*dst_width + x] = src->imageData[y1*src_width + x1];
			} else {
				pDstData = &dst->imageData[y*dst_stride + x*channel];
                PIM_Memset(pDstData, 0, channel);
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



/*
 *
 */
PIM_Result PIM_Bitmap_Crop(PIM_Bitmap *dst, PIM_Bitmap *src, PIM_Rect *rect)
{
    PIM_Int32 i;
    PIM_Uint8 *pSrc = src->imageData;
    PIM_Uint8 *pDst = dst->imageData;
    PIM_Int32 channel = PIM_Bitmap_GetChannel(src);

    if(!pSrc) return PIM_ERROR_NULL_POINTER;
    if(!pDst) return PIM_ERROR_NULL_POINTER;

    switch (dst->imageFormat)
    {
        case PIM_IMAGE_FORMAT_GRAY:
        case PIM_IMAGE_FORMAT_RGB888:
        case PIM_IMAGE_FORMAT_BGR888:
        case PIM_IMAGE_FORMAT_BGR8880:
        case PIM_IMAGE_FORMAT_RGB8880:
        case PIM_IMAGE_FORMAT_RGBA8888:
        {
            pSrc += (src->stride * rect->top) + (rect->left * channel);
            for (i=0; i<dst->height; i++) {
                PIM_Memcpy(pDst, pSrc, dst->width*channel);

                pSrc += src->stride;
                pDst += dst->stride;
            }
        } break;

        case PIM_IMAGE_FORMAT_YUVPLANAR_NV21:
        case PIM_IMAGE_FORMAT_YUVPLANAR_NV12:
        {
            pSrc += (rect->top*src->stride+rect->left);
            for (i=0; i<dst->height; i++) {
                PIM_Memcpy(pDst, pSrc, dst->width);
                pSrc += src->stride;
                pDst += dst->stride;
            }

            pSrc = src->imageData + src->stride*src->height + ((rect->top/2)*src->stride + ((rect->left+1)&~1));
            for (i=0; i<dst->height/2; i++) {
                PIM_Memcpy(pDst, pSrc, (dst->width+1)&~1);
                pSrc += src->stride;
                pDst += dst->stride;
            }
        } break;

        default:
            return PIM_ERROR_NOT_SUPPORTED;
    }/* end switch */

    return PIM_SUCCESS;
}



/*
 *  @brief src이미지에서 특정 영역을 dst이미지로 Crop&Resize한다.
 *  @note crop한 이미지는 dst이미지의 크기에 맞춰 resize된다.
 *
 *  @param src_rect [IN] crop area of "src image"
 */
PIM_Result PIM_Bitmap_CropResize(PIM_Bitmap *dst, PIM_Bitmap *src, PIM_Rect *cropRect)
{
    switch(src->imageFormat)
    {
        case PIM_IMAGE_FORMAT_YUVPLANAR_NV12:
        //case PIM_IMAGE_FORMAT_YUVPLANAR_NV12_SPLIT:
        case PIM_IMAGE_FORMAT_YUVPLANAR_NV21:
        //case PIM_IMAGE_FORMAT_YUVPLANAR_NV21_SPLIT:
            return Bitmap_CropResize_YUVPLANAR_NV(dst, src, cropRect);

        case PIM_IMAGE_FORMAT_GRAY:
        case PIM_IMAGE_FORMAT_RGB_INDEX:
            return Bitmap_CropResize_Gray(dst, src, cropRect);

        case PIM_IMAGE_FORMAT_RGB888:
        case PIM_IMAGE_FORMAT_RGB8880:
        case PIM_IMAGE_FORMAT_RGBA8888:
        case PIM_IMAGE_FORMAT_BGR888:
        case PIM_IMAGE_FORMAT_BGR8880:
            return Bitmap_CropResize_RGB(dst, src, cropRect);

        default:
            return PIM_ERROR_NOT_SUPPORTED;
    }

    return PIM_SUCCESS;
}

#define LINEAR_CALC(val,aa,bb,cc,dd,x_diff,y_diff)      \
    val = (PIM_Uint8) ((                                \
            (aa) * (1024 - x_diff) * (1024 - y_diff) +  \
            (bb) * x_diff * (1024 - y_diff) +           \
            (cc) * y_diff * (1024 - x_diff) +           \
            (dd) * x_diff * y_diff                      \
            ) >> 20);

static PIM_Result Bitmap_CropResize_YUVPLANAR_NV(PIM_Bitmap *dst, PIM_Bitmap *src, PIM_Rect *cropRect)
{
    PIM_Result ret = PIM_SUCCESS;

    PIM_Uint8 *src_pixels = src->imageData;
    PIM_Uint8 *dst_pixels = dst->imageData;
    
    PIM_Int32 x_ratio = (PIM_Int32) ((PIM_Rect_GetWidth(cropRect) << 10) / dst->width);
    //PIM_Int32 x_ratio = (PIM_Int32) ((PIM_Rect_GetWidth(cropRect) << 10) / dst->stride);
    PIM_Int32 y_ratio = (PIM_Int32) ((PIM_Rect_GetHeight(cropRect) << 10) / dst->height);

    PIM_Int32 A, B, C, D, xr, yr;
    long x, y, x_diff, y_diff;
    PIM_Int32 i, j;
    PIM_Int32 index, h_index, uv_index;
    PIM_Int32 dst_pass;

    /* Y */
    y = 0;
    dst_pass = dst->stride - dst->width;
    for (i=0; i<(PIM_Int32)dst->height; i++) {
        yr = (PIM_Int32) (y >> 10);
        y_diff = y - (yr << 10);
        h_index = (cropRect->top + yr) * src->stride;

        x = 0;
        for (j=0; j<(PIM_Int32)dst->width; j++) {
            xr = (PIM_Int32) (x >> 10);
            x_diff = x - (xr << 10);
            index = h_index + (cropRect->left + xr);

            // range is 0 to 255 thus bitwise AND with 0xff
            A = src_pixels[index];
            B = src_pixels[index+1];
            C = src_pixels[index+src->stride];
            D = src_pixels[index+src->stride+1];

            // Y = A(1-wd)(1-ht) + B(wd)(1-ht) + C(ht)(1-wd) + D(wd)(ht)
            LINEAR_CALC(*dst_pixels, A, B, C, D, x_diff, y_diff);

            dst_pixels++;

            x += x_ratio;
        }
        dst_pixels += dst_pass;
        y += y_ratio;
    }
    
    /* UV */
    y = 0;
    uv_index = src->stride * src->height;
    //dst_pass &= ~1; /* 버그 주의 */
    //dst_pass = dst->stride - ((dst->width+1)&~1)*2;
    dst_pass = dst->stride - ((dst->width+1)&~1);
    for (i=0; i<(PIM_Int32)dst->height; i+=2) {
        yr = (PIM_Int32) (y >> 10);
        y_diff = y - (yr << 10);
        h_index = (cropRect->top/2 + yr) * src->stride;

        x = 0;
        for (j=0; j<(PIM_Int32)dst->width; j+=2) {
            xr = (PIM_Int32) (x >> 10);
            x_diff = x - (xr << 10);
            index = uv_index + h_index + (cropRect->left/2 + xr) * 2;

            // range is 0 to 255 thus bitwise AND with 0xff
            A = src_pixels[index] & 0xff ;
            B = src_pixels[index+2] & 0xff ;
            C = src_pixels[index+src->stride] & 0xff ;
            D = src_pixels[index+src->stride+2] & 0xff ;

            LINEAR_CALC(*dst_pixels, A, B, C, D, x_diff, y_diff);
            dst_pixels++;

            A = src_pixels[index+1] & 0xff ;
            B = src_pixels[index+3] & 0xff ;
            C = src_pixels[index+src->stride+1] & 0xff ;
            D = src_pixels[index+src->stride+3] & 0xff ;

            LINEAR_CALC(*dst_pixels, A, B, C, D, x_diff, y_diff);
            dst_pixels++;

            x += x_ratio;
        }
        dst_pixels += dst_pass;
        y += (y_ratio);
    }

    return ret;
}

static PIM_Result Bitmap_CropResize_Gray(PIM_Bitmap *dst, PIM_Bitmap *src, PIM_Rect *cropRect)
{
    PIM_Result ret = PIM_SUCCESS;

    PIM_Uint8 *src_pixels = src->imageData;
    PIM_Uint8 *dst_pixels = dst->imageData;

    PIM_Int32 x_ratio = (PIM_Int32) ((PIM_Rect_GetWidth(cropRect) << 10) / dst->width);
    PIM_Int32 y_ratio = (PIM_Int32) ((PIM_Rect_GetHeight(cropRect) << 10) / dst->height);

    PIM_Int32 A, B, C, D, xr, yr;
    long x, y, x_diff, y_diff;
    PIM_Int32 i, j;
    PIM_Int32 index, h_index;
    PIM_Int32 dst_pass = dst->stride - dst->width;

    y = 0;
    for (i=0; i<(PIM_Int32)dst->height; i++) {
        yr = (PIM_Int32) (y >> 10);
        y_diff = y - (yr << 10);
        h_index = (cropRect->top + yr) * src->stride;
        x = 0;

        if((yr+1) >= src->height) { // warning!!! bug
            for (j=0; j<(PIM_Int32)dst->width; j++) {
                xr = (PIM_Int32) (x >> 10);
                x_diff = x - (xr << 10);
                index = h_index + (cropRect->left + xr);

                // range is 0 to 255 thus bitwise AND with 0xff
                A = src_pixels[index];
                B = src_pixels[index+1];
                C = src_pixels[index];
                D = src_pixels[index+1];
                LINEAR_CALC(dst_pixels[0], A, B, C, D, x_diff, y_diff);

                dst_pixels++;

                x += x_ratio;
            }
        } else {
            for (j=0; j<(PIM_Int32)dst->width; j++) {
                xr = (PIM_Int32) (x >> 10);
                x_diff = x - (xr << 10);
                index = h_index + (cropRect->left + xr);

                // range is 0 to 255 thus bitwise AND with 0xff
                A = src_pixels[index];
                B = src_pixels[index+1];
                C = src_pixels[index+src->stride];
                D = src_pixels[index+src->stride+1];
                LINEAR_CALC(dst_pixels[0], A, B, C, D, x_diff, y_diff);

                dst_pixels++;

                x += x_ratio;
            }
        }

        dst_pixels += dst_pass;

        y += y_ratio;
    }

    return ret;
}

static PIM_Result Bitmap_CropResize_RGB(PIM_Bitmap *dst, PIM_Bitmap *src, PIM_Rect *cropRect)
{
    PIM_Result ret = PIM_SUCCESS;
    PIM_Int32 channel = PIM_Bitmap_GetChannel(src);

    PIM_Uint8 *src_pixels = src->imageData;
    PIM_Uint8 *dst_pixels = dst->imageData;

    PIM_Int32 x_ratio = (PIM_Int32) ((PIM_Rect_GetWidth(cropRect) << 10) / dst->width);
    PIM_Int32 y_ratio = (PIM_Int32) ((PIM_Rect_GetHeight(cropRect) << 10) / dst->height);

    PIM_Int32 A, B, C, D, xr, yr;
    long x, y, x_diff, y_diff;
    PIM_Int32 i, j;
    PIM_Int32 index, h_index;

    y = 0;
    for (i=0; i<(PIM_Int32)dst->height; i++) {
        yr = (PIM_Int32) (y >> 10);
        y_diff = y - (yr << 10);
        h_index = (cropRect->top + yr) * src->stride;
        x = 0;

        if((yr+1) >= src->height) { // warning!!! bug
            for (j=0; j<(PIM_Int32)dst->width; j++) {
                xr = (PIM_Int32) (x >> 10);
                x_diff = x - (xr << 10);
                index = h_index + (cropRect->left + xr)*channel;

                // range is 0 to 255 thus bitwise AND with 0xff
                A = src_pixels[index+0];
                B = src_pixels[index+channel+0];
                C = src_pixels[index+0];
                D = src_pixels[index+channel+0];
                LINEAR_CALC(dst_pixels[0], A, B, C, D, x_diff, y_diff);

                A = src_pixels[index+1];
                B = src_pixels[index+channel+1];
                C = src_pixels[index+1];
                D = src_pixels[index+channel+1];
                LINEAR_CALC(dst_pixels[1], A, B, C, D, x_diff, y_diff);

                A = src_pixels[index+2];
                B = src_pixels[index+channel+2];
                C = src_pixels[index+2];
                D = src_pixels[index+channel+2];
                LINEAR_CALC(dst_pixels[2], A, B, C, D, x_diff, y_diff);

                dst_pixels += channel;

                x += x_ratio;
            }
        } else {
            for (j=0; j<(PIM_Int32)dst->width; j++) {
                xr = (PIM_Int32) (x >> 10);
                x_diff = x - (xr << 10);
                index = h_index + (cropRect->left + xr)*channel;

                // range is 0 to 255 thus bitwise AND with 0xff
                A = src_pixels[index+0];
                B = src_pixels[index+channel+0];
                C = src_pixels[index+src->stride+0];
                D = src_pixels[index+src->stride+channel+0];
                LINEAR_CALC(dst_pixels[0], A, B, C, D, x_diff, y_diff);

                A = src_pixels[index+1];
                B = src_pixels[index+channel+1];
                C = src_pixels[index+src->stride+1];
                D = src_pixels[index+src->stride+channel+1];
                LINEAR_CALC(dst_pixels[1], A, B, C, D, x_diff, y_diff);

                A = src_pixels[index+2];
                B = src_pixels[index+channel+2];
                C = src_pixels[index+src->stride+2];
                D = src_pixels[index+src->stride+channel+2];
                LINEAR_CALC(dst_pixels[2], A, B, C, D, x_diff, y_diff);

                dst_pixels += channel;

                x += x_ratio;
            }
        }

        dst_pixels += (dst->stride - dst->width*channel);

        y += y_ratio;
    }

    return ret;
}

/* 90-degree clockwise rotated */
static PIM_Result Bitmap_Rotate_CW90(PIM_Bitmap* dst, PIM_Bitmap* src)
{
    PIM_Int32 channel = PIM_Bitmap_GetChannel(dst);
    PIM_Uint8 *src_buf = src->imageData;
    PIM_Uint8 *dst_buf = dst->imageData;
    PIM_Uint8 *pSrc, *pDst;
    PIM_Int32 x, y;
    PIM_Int32 dst_w, dst_h;
    PIM_Int32 src_w, src_h;
    PIM_Int32 dst_stride, src_stride;

    if(!src_buf) return PIM_ERROR_NULL_POINTER;
    if(!dst_buf) return PIM_ERROR_NULL_POINTER;

    /*  */
    dst_w = dst->width  = src->height;
    dst_h = dst->height = src->width;
    dst_stride = dst->stride = dst_w*channel;
    src_w = src->width;
    src_h = src->height;
    src_stride = src->stride;

    switch(src->imageFormat)
    {
        case PIM_IMAGE_FORMAT_GRAY:
        case PIM_IMAGE_FORMAT_RGB_INDEX:
        {
            for (y=0; y<src_h; y++)
            {
                src_buf = &src->imageData[y*src_stride];
                dst_buf = &dst->imageData[(dst_w-1)-y];

                for (x=0; x<src_w; x++)
                {
                    *dst_buf = *src_buf;

                    src_buf++;
                    dst_buf+=dst_stride;
                }
            }
        } break;

        case PIM_IMAGE_FORMAT_RGB888:
        case PIM_IMAGE_FORMAT_RGB8880:
        case PIM_IMAGE_FORMAT_RGBA8888:
        case PIM_IMAGE_FORMAT_BGR888:
        case PIM_IMAGE_FORMAT_BGR8880:
        {
            for (y=0; y<src_h; y++)
            {
                src_buf = &src->imageData[y*src_stride];
                dst_buf = &dst->imageData[((dst_w-1)-y)*channel];

                for (x=0; x<src_w; x++)
                {
                    dst_buf[0] = src_buf[0];
                    dst_buf[1] = src_buf[1];
                    dst_buf[2] = src_buf[2];

                    src_buf+=channel;
                    dst_buf+=dst_stride;
                }
            }
        } break;

        case PIM_IMAGE_FORMAT_YUVPLANAR_NV21:
        case PIM_IMAGE_FORMAT_YUVPLANAR_NV12:
        {
            /* Y */
            pSrc = src->imageData;
            pDst = dst->imageData;

            for(y=0; y<src_h; y++)
            {
                src_buf = &pSrc[y*src_stride];
                dst_buf = &pDst[(dst_w-1)-y];

                for (x=0; x<src_w; x++)
                {
                    *dst_buf = *src_buf;

                    src_buf++;
                    dst_buf+=dst_stride;
                }
            }

            /* UV */
            if(src->auxImageData) pSrc = src->auxImageData;
            else                  pSrc = &src->imageData[src_h*src_stride];
            if(dst->auxImageData) pDst = dst->auxImageData;
            else                  pDst = &dst->imageData[dst_h*dst_stride];

            src_h = ((src_h+1)&~1)/2;
            src_w = ((src_w+1)&~1)/2;
            dst_h = ((dst_h+1)&~1)/2;
            dst_w = ((dst_w+1)&~1)/2;

            for(y=0; y<src_h; y++)
            {
                src_buf = &pSrc[y*src_stride];
                dst_buf = &pDst[((dst_w-1)-y)*2];

                for (x=0; x<src_w; x++)
                {
                    dst_buf[0] = src_buf[0];
                    dst_buf[1] = src_buf[1];

                    src_buf+=2;
                    dst_buf+=dst_stride;
                }
            }
        } break;

        default:
            return PIM_ERROR_NOT_SUPPORTED;
    }


    return PIM_SUCCESS;
}

/* 90-degree counterclockwise rotated */
static PIM_Result Bitmap_Rotate_CW270(PIM_Bitmap* dst, PIM_Bitmap* src)
{
    PIM_Int32 channel = PIM_Bitmap_GetChannel(dst);
    PIM_Uint8 *src_buf = src->imageData;
    PIM_Uint8 *dst_buf = dst->imageData;
    PIM_Uint8 *pSrc, *pDst;
    PIM_Int32 x, y;
    PIM_Int32 dst_w, dst_h;
    PIM_Int32 src_w, src_h;
    PIM_Int32 dst_stride, src_stride;

    if(!src_buf) return PIM_ERROR_NULL_POINTER;
    if(!dst_buf) return PIM_ERROR_NULL_POINTER;

    /*  */
    dst_w = dst->width  = src->height;
    dst_h = dst->height = src->width;
    dst_stride = dst->stride = dst_w*channel;
    src_w = src->width;
    src_h = src->height;
    src_stride = src->stride;

    switch(src->imageFormat)
    {
        case PIM_IMAGE_FORMAT_GRAY:
        case PIM_IMAGE_FORMAT_RGB_INDEX:
        {
            pSrc = src->imageData;
            pDst = dst->imageData;

            for (y=0; y<src_h; y++)
            {
                src_buf = &pSrc[            y*src_stride];
                dst_buf = &pDst[y + (dst_h-1)*dst_stride];

                for (x=0; x<src_w; x++)
                {
                    *dst_buf = *src_buf;

                    src_buf++;
                    dst_buf-=dst_stride;
                }
            }
        } break;

        case PIM_IMAGE_FORMAT_RGB888:
        case PIM_IMAGE_FORMAT_RGB8880:
        case PIM_IMAGE_FORMAT_RGBA8888:
        case PIM_IMAGE_FORMAT_BGR888:
        case PIM_IMAGE_FORMAT_BGR8880:
        {
            pSrc = src->imageData;
            pDst = dst->imageData;

            for (y=0; y<src_h; y++)
            {
                src_buf = &pSrc[                    y*src_stride];
                dst_buf = &pDst[y*channel + (dst_h-1)*dst_stride];

                for (x=0; x<src_w; x++)
                {
                    dst_buf[0] = src_buf[0];
                    dst_buf[1] = src_buf[1];
                    dst_buf[2] = src_buf[2];

                    src_buf+=channel;
                    dst_buf-=dst_stride;
                }
            }
        } break;

        case PIM_IMAGE_FORMAT_YUVPLANAR_NV12:
        case PIM_IMAGE_FORMAT_YUVPLANAR_NV21:
        {
            /* Y */
            pSrc = src->imageData;
            pDst = dst->imageData;

            for(y=0; y<src_h; y++)
            {
                src_buf = &pSrc[            y*src_stride];
                dst_buf = &pDst[y + (dst_h-1)*dst_stride];

                for (x=0; x<src_w; x++)
                {
                    *dst_buf = *src_buf;

                    src_buf++;
                    dst_buf-=dst_stride;
                }
            }

            /* UV */
            if(src->auxImageData) pSrc = src->auxImageData;
            else                  pSrc = &src->imageData[src_h*src_stride];
            if(dst->auxImageData) pDst = dst->auxImageData;
            else                  pDst = &dst->imageData[dst_h*dst_stride];

            src_h = ((src_h+1)&~1)/2;
            src_w = ((src_w+1)&~1)/2;
            dst_h = ((dst_h+1)&~1)/2;
            dst_w = ((dst_w+1)&~1)/2;

            for(y=0; y<src_h; y++)
            {
                src_buf = &pSrc[              y*src_stride];
                dst_buf = &pDst[y*2 + (dst_h-1)*dst_stride];

                for (x=0; x<src_w; x++)
                {
                    dst_buf[0] = src_buf[0];
                    dst_buf[1] = src_buf[1];

                    src_buf+=2;
                    dst_buf-=dst_stride;
                }
            }
        } break;

        default:
            return PIM_ERROR_NOT_SUPPORTED;

    }

    return PIM_SUCCESS;
}



/* 180-degree clockwise rotated */
static PIM_Result Bitmap_Rotate_CW180(PIM_Bitmap* dst, PIM_Bitmap* src)
{
    PIM_Int32 channel = PIM_Bitmap_GetChannel(dst);
    PIM_Uint8 *src_buf = src->imageData;
    PIM_Uint8 *dst_buf = dst->imageData;
    PIM_Uint8 *pSrc, *pDst;
    PIM_Int32 x, y;
    PIM_Int32 dst_w, dst_h;
    PIM_Int32 src_w, src_h;
    PIM_Int32 dst_stride, src_stride;

    if(!src_buf) return PIM_ERROR_NULL_POINTER;
    if(!dst_buf) return PIM_ERROR_NULL_POINTER;

    /*  */
    dst_w = dst->width;
    dst_h = dst->height;
    dst_stride = dst->stride;
    src_w = src->width;
    src_h = src->height;
    src_stride = src->stride;
	
    switch(src->imageFormat)
    {
        case PIM_IMAGE_FORMAT_GRAY:
        case PIM_IMAGE_FORMAT_RGB_INDEX:
        {
            pSrc = src->imageData;
            pDst = dst->imageData;

            for (y=0; y<src_h; y++)
            {
                src_buf = &pSrc[                      y*src_stride];
                dst_buf = &pDst[(dst_w-1) + (dst_h-1-y)*dst_stride];

                for (x=0; x<src_w; x++)
                {
                    *dst_buf = *src_buf;

                    src_buf++;
                    dst_buf--;
                }
            }
        } break;

        case PIM_IMAGE_FORMAT_RGB888:
        case PIM_IMAGE_FORMAT_RGB8880:
        case PIM_IMAGE_FORMAT_RGBA8888:
        case PIM_IMAGE_FORMAT_BGR888:
        case PIM_IMAGE_FORMAT_BGR8880:
        {
            pSrc = src->imageData;
            pDst = dst->imageData;

            for (y=0; y<src_h; y++)
            {
                src_buf = &pSrc[                              y*src_stride];
                dst_buf = &pDst[(dst_w-1)*channel + (dst_h-1-y)*dst_stride];

                for (x=0; x<src_w; x++)
                {
                    dst_buf[0] = src_buf[0];
                    dst_buf[1] = src_buf[1];
                    dst_buf[2] = src_buf[2];

                    src_buf+=channel;
                    dst_buf-=channel;
                }
            }
        } break;

        case PIM_IMAGE_FORMAT_YUVPLANAR_NV12:
        case PIM_IMAGE_FORMAT_YUVPLANAR_NV21:
        {
            /* Y */
            pSrc = src->imageData;
            pDst = dst->imageData;

            for(y=0; y<src_h; y++)
            {
                src_buf = &pSrc[                      y*src_stride];
                dst_buf = &pDst[(dst_w-1) + (dst_h-1-y)*dst_stride];

                for (x=0; x<src_w; x++)
                {
                    *dst_buf = *src_buf;

                    src_buf++;
                    dst_buf--;
                }
            }

            /* UV */
            if(src->auxImageData) pSrc = src->auxImageData;
            else                  pSrc = &src->imageData[src_h*src_stride];
            if(dst->auxImageData) pDst = dst->auxImageData;
            else                  pDst = &dst->imageData[dst_h*dst_stride];

            src_h = ((src_h+1)&~1)/2;
            src_w = ((src_w+1)&~1)/2;
            dst_h = ((dst_h+1)&~1)/2;
            dst_w = ((dst_w+1)&~1)/2;

            for(y=0; y<src_h; y++)
            {
                src_buf = &pSrc[                        y*src_stride];
                dst_buf = &pDst[(dst_w-1)*2 + (dst_h-1-y)*dst_stride];

                for (x=0; x<src_w; x++)
                {
                    dst_buf[0] = src_buf[0];
                    dst_buf[1] = src_buf[1];

                    src_buf+=2;
                    dst_buf-=2;
                }
            }
        } break;

        default:
            return PIM_ERROR_NOT_SUPPORTED;
    }

    return PIM_SUCCESS;
}



/*
 *
 */
PIM_Result PIM_Bitmap_Rotate(PIM_Bitmap* dst, PIM_Bitmap* src, PIM_ImageOrientation ori)
{
    switch(ori)
    {
        case PIM_IMAGE_ORIENTATION_CW90:
            return Bitmap_Rotate_CW90(dst, src);

        case PIM_IMAGE_ORIENTATION_CW270:
            return Bitmap_Rotate_CW270(dst, src);

        case PIM_IMAGE_ORIENTATION_CW180:
            return Bitmap_Rotate_CW180(dst, src);

        default:
            return PIM_ERROR_NOT_SUPPORTED;
    }
}



/*
 *
 */
#define SWAP(A,B,temp)    temp = A; A = B; B = temp;
PIM_Result PIM_Bitmap_HFlip(PIM_Bitmap *bitmap)
{
    PIM_Int32 width = bitmap->width;
    PIM_Int32 height = bitmap->height;
    PIM_Uint8 *data_p, tmp;
    PIM_Int32 i, j;
    PIM_Int32 stride = bitmap->stride;

    switch(bitmap->imageFormat)
    {
	    case PIM_IMAGE_FORMAT_GRAY:
		{
			PIM_Int32 channel = PIM_Bitmap_GetChannel(bitmap);
			PIM_Uint8 *front_data, *rear_data;

			for(j=0; j<height; j++) {
				front_data = &bitmap->imageData[(j*stride)                    ]; 
				rear_data  = &bitmap->imageData[(j*stride) + (width-1)*channel]; 

				for(i=0; i<width/2; i++) {
					SWAP(front_data[0], rear_data[0], tmp);

					front_data += channel;
					rear_data  -= channel;
				}
			}
		} break;

        case PIM_IMAGE_FORMAT_RGB888:
        case PIM_IMAGE_FORMAT_RGB8880:
        case PIM_IMAGE_FORMAT_BGR888:
        case PIM_IMAGE_FORMAT_BGR8880:
        {
            PIM_Int32 channel = PIM_Bitmap_GetChannel(bitmap);
            PIM_Uint8 *front_data, *rear_data;

            for(j=0; j<height; j++) {
                front_data = &bitmap->imageData[(j*stride)                    ]; 
                rear_data  = &bitmap->imageData[(j*stride) + (width-1)*channel]; 

                for(i=0; i<width/2; i++) {
                    SWAP(front_data[0], rear_data[0], tmp);
                    SWAP(front_data[1], rear_data[1], tmp);
                    SWAP(front_data[2], rear_data[2], tmp);

                    front_data += channel;
                    rear_data  -= channel;
                }
            }
        } break;

        case PIM_IMAGE_FORMAT_RGBA8888:
        {
            PIM_Int32 channel = PIM_Bitmap_GetChannel(bitmap);
            PIM_Uint8 *front_data, *rear_data;

            for(j=0; j<height; j++) {
                front_data = &bitmap->imageData[(j*stride)                    ]; 
                rear_data  = &bitmap->imageData[(j*stride) + (width-1)*channel]; 

                for(i=0; i<width/2; i++) {
                    SWAP(front_data[0], rear_data[0], tmp);
                    SWAP(front_data[1], rear_data[1], tmp);
                    SWAP(front_data[2], rear_data[2], tmp);
                    SWAP(front_data[3], rear_data[3], tmp);

                    front_data += channel;
                    rear_data  -= channel;
                }
            }
        } break;

        case PIM_IMAGE_FORMAT_YUVPLANAR_NV12:
        case PIM_IMAGE_FORMAT_YUVPLANAR_NV21:
        {
            /* Y */
            data_p = bitmap->imageData;
            for(j=0; j<height; j++) {
                for(i=0; i<width/2; i++) {
                    tmp = data_p[(j*width) + (i)]; 
                    data_p[(j*width) + (i)] = data_p[(j*width) + (width-i-1)]; 
                    data_p[(j*width) + (width-i-1)] = tmp; 
                }
            }

            /* UV */
            if(bitmap->auxImageData) data_p = bitmap->auxImageData;
            else                     data_p = bitmap->imageData + width*height;

            for(j=0; j<height/2; j++) {
                for(i=0; i<width/4; i++) {
                    tmp = data_p[(j*width) + (i*2)]; 
                    data_p[(j*width) + (i*2)] = data_p[(j*width) + (width-i*2-2)]; 
                    data_p[(j*width) + (width-i*2-2)] = tmp; 

                    tmp = data_p[(j*width) + (i*2+1)]; 
                    data_p[(j*width) + (i*2+1)] = data_p[(j*width) + (width-i*2-1)]; 
                    data_p[(j*width) + (width-i*2-1)] = tmp; 
                }
            }
        } break;

        default:
            return PIM_ERROR_NOT_SUPPORTED;
    }

    return PIM_SUCCESS;
}



/*
 *
 */
PIM_Result PIM_Bitmap_VFlip(PIM_Bitmap *bitmap)
{
    PIM_Int32 width = bitmap->width;
    PIM_Int32 height = bitmap->height;
    PIM_Uint8 *data_p, tmp;
    PIM_Int32 i, j;
    PIM_Int32 stride = bitmap->stride;
    PIM_Int32 channel = PIM_Bitmap_GetChannel(bitmap);
    PIM_Uint8 *front_data, *rear_data;

    switch(bitmap->imageFormat)
    {
		case PIM_IMAGE_FORMAT_GRAY:
		{
			for(j=0; j<height/2; j++) {
				front_data = &bitmap->imageData[j           *stride]; 
				rear_data  = &bitmap->imageData[(height-j-1)*stride]; 

				for(i=0; i<width; i++) {
					SWAP(front_data[0], rear_data[0], tmp);

					front_data += channel;
					rear_data  += channel;
				}
			}
		} break;

        case PIM_IMAGE_FORMAT_RGB888:
        case PIM_IMAGE_FORMAT_RGB8880:
        case PIM_IMAGE_FORMAT_BGR888:
        case PIM_IMAGE_FORMAT_BGR8880:
        {

            for(j=0; j<height/2; j++) {
                front_data = &bitmap->imageData[j           *stride]; 
                rear_data  = &bitmap->imageData[(height-j-1)*stride]; 

                for(i=0; i<width; i++) {
                    SWAP(front_data[0], rear_data[0], tmp);
                    SWAP(front_data[1], rear_data[1], tmp);
                    SWAP(front_data[2], rear_data[2], tmp);

                    front_data += channel;
                    rear_data  += channel;
                }
            }
        } break;

        case PIM_IMAGE_FORMAT_RGBA8888:
        {

            for(j=0; j<height/2; j++) {
                front_data = &bitmap->imageData[j           *stride]; 
                rear_data  = &bitmap->imageData[(height-j-1)*stride]; 

                for(i=0; i<width; i++) {
                    SWAP(front_data[0], rear_data[0], tmp);
                    SWAP(front_data[1], rear_data[1], tmp);
                    SWAP(front_data[2], rear_data[2], tmp);
                    SWAP(front_data[3], rear_data[3], tmp);

                    front_data += channel;
                    rear_data  += channel;
                }
            }
        } break;

        case PIM_IMAGE_FORMAT_YUVPLANAR_NV12:
        case PIM_IMAGE_FORMAT_YUVPLANAR_NV21:
        {
            /* Y */
            data_p = bitmap->imageData;
            for(j=0; j<height/2; j++) {
                for(i=0; i<width; i++) {
                    tmp = data_p[j*width + i]; 
                    data_p[j*width + i] = data_p[(height-j-1)*width + i]; 
                    data_p[(height-j-1)*width + i] = tmp; 
                }
            }

            /* UV */
            if(bitmap->auxImageData) data_p = bitmap->auxImageData;
            else                     data_p = bitmap->imageData + width*height;

            for(j=0; j<height/4; j++) {
                for(i=0; i<width; i++) {
                    tmp = data_p[j*width + i]; 
                    data_p[j*width + i] = data_p[(height/2-j-1)*width + i]; 
                    data_p[(height/2-j-1)*width + i] = tmp; 
                }
            }
        } break;

        default:
            return PIM_ERROR_NOT_SUPPORTED;
    }

    return PIM_SUCCESS;
}



#ifndef __CODE_SIZE_OPTIMIZE__
PIM_Result PIM_Bitmap_Rotate_RightBottom_7(PIM_Bitmap* dst, PIM_Bitmap* src)
{
    PIM_Int32 channel = PIM_Bitmap_GetChannel(dst);
    PIM_Uint8 *buf_temp = src->imageData;
    PIM_Uint8 *dst_buf = dst->imageData;
    PIM_Int32 new_width, new_height;
    PIM_Int32 i, j, cnt=0, idx,new_wh;
    PIM_Result ret = PIM_SUCCESS;

    if(!buf_temp) return PIM_ERROR_NULL_POINTER;
    if(!dst_buf) return PIM_ERROR_NULL_POINTER;


    dst->width = new_width = src->height;
    dst->height = new_height = src->width;
    new_wh = new_width*new_height;

    switch(src->imageFormat)
    {
        case PIM_IMAGE_FORMAT_GRAY:
        {
            for (j=new_width-1; j>=0; j--) {
                for (i=new_height-1; i>=0; i--) {
                
                    idx = (i*new_width+j);
                    dst_buf[idx] = buf_temp[cnt++];
                }
            }
        } break;

        case PIM_IMAGE_FORMAT_RGB888:
        case PIM_IMAGE_FORMAT_BGR888:
        {
            for (j=new_width-1; j>=0; j--) {
                for (i=new_height-1; i>=0; i--) {
                
                    idx = (i*new_width+j)*3;
                    dst_buf[idx+0 ] = buf_temp[cnt++];
                    dst_buf[idx+1 ] = buf_temp[cnt++];
                    dst_buf[idx+2 ] = buf_temp[cnt++];
                }
            }
        } break;

        case PIM_IMAGE_FORMAT_BGR8880:
        case PIM_IMAGE_FORMAT_RGB8880:
        {
            for (j=new_width-1; j>=0; j--) {
                for (i=new_height-1; i>=0; i--) {
                
                    idx = (i*new_width+j)*4;
                    dst_buf[idx+0 ] = buf_temp[cnt++];
                    dst_buf[idx+1 ] = buf_temp[cnt++];
                    dst_buf[idx+2 ] = buf_temp[cnt++];
                    dst_buf[idx+3 ] = buf_temp[cnt++];
                }
            }
        } break;

    default:
        ret = PIM_ERROR_NOT_SUPPORTED;

    }

    return ret;
}


PIM_Result PIM_Bitmap_Rotate_TopRightBottom_2(PIM_Bitmap* dst, PIM_Bitmap* src)
{
    PIM_Int32 channel = PIM_Bitmap_GetChannel(dst);
    PIM_Uint8 *buf_temp = src->imageData;
    PIM_Uint8 *dst_buf = dst->imageData;
    PIM_Int32 new_width, new_height;
    PIM_Int32 i, j, cnt=0, idx;
    PIM_Result ret = PIM_SUCCESS;

    if(!buf_temp) return PIM_ERROR_NULL_POINTER;
    if(!dst_buf) return PIM_ERROR_NULL_POINTER;

    dst->height = new_height = src->height;
    dst->width = new_width = src->width;
    
    switch(src->imageFormat)
    {
        case PIM_IMAGE_FORMAT_GRAY:
        {
            for (i=0; i<new_height; i++) {
                for (j=new_width-1; j>=0; j--) {
                    idx = (i*new_width+j);
                    dst_buf[idx] = buf_temp[cnt++];
                }
            }
        } break;

        case PIM_IMAGE_FORMAT_RGB888:
        case PIM_IMAGE_FORMAT_BGR888:
        {
            for (i=0; i<new_height; i++) {
                for (j=new_width-1; j>=0; j--) {
                    idx = (i*new_width+j)*3;
                    dst_buf[idx+0 ] = buf_temp[cnt++];
                    dst_buf[idx+1 ] = buf_temp[cnt++];
                    dst_buf[idx+2 ] = buf_temp[cnt++];
                }
            }
        }
        break;
        case PIM_IMAGE_FORMAT_BGR8880:
        case PIM_IMAGE_FORMAT_RGB8880:
        {
            for (i=0; i<new_height; i++) {
                for (j=new_width-1; j>=0; j--) {
                    idx = (i*new_width+j)*4;
                    dst_buf[idx+0 ] = buf_temp[cnt++];
                    dst_buf[idx+1 ] = buf_temp[cnt++];
                    dst_buf[idx+2 ] = buf_temp[cnt++];
                    dst_buf[idx+3 ] = buf_temp[cnt++];
                }
            }
        } break;

        case PIM_IMAGE_FORMAT_YUVPLANAR_NV21://paul717 (20090408)
        {
            for (i=0; i<new_height; i++) {
                for (j=new_width-1; j>=0; j--) {
                    idx = (i*new_width+j);
                    dst_buf[idx] = buf_temp[cnt++];
                }
            }
            cnt = 0;
            dst_buf += new_width*new_height;
            buf_temp += new_width*new_height;
            for (i=0; i<(new_height/2); i++) {
                for (j=(new_width-2); j>=0; j-=2) {
                    idx = ((i*new_width+j)) & ~1;
                    dst_buf[idx] = buf_temp[cnt++];
                    dst_buf[idx+1] = buf_temp[cnt++];
                }
            }
        } break;

#if 0
        case PIM_IMAGE_FORMAT_YUVPLANAR_NV12_SPLIT://paul717 (20090408)
        case PIM_IMAGE_FORMAT_YUVPLANAR_NV21_SPLIT:
        {
            for (i=0; i<new_height; i++) {
                for (j=new_width-1; j>=0; j--) {
                    idx = (i*new_width+j);
                    dst_buf[idx] = buf_temp[cnt++];
                }
            }
            cnt = 0;
            dst_buf = dst->auxImageData;
            buf_temp = src->auxImageData;
            for (i=0; i<(new_height/2); i++) {
                for (j=(new_width-2); j>=0; j-=2) {
                    idx = ((i*new_width+j)) & ~1;
                    dst_buf[idx] = buf_temp[cnt++];
                    dst_buf[idx+1] = buf_temp[cnt++];
                }
            }
        }
        break;
#endif

        default:
            ret = PIM_ERROR_NOT_SUPPORTED;
    }

    return ret;
}


PIM_Result PIM_Bitmap_Rotate_BottomLeft_4(PIM_Bitmap* dst, PIM_Bitmap* src)
{
    PIM_Int32 channel = PIM_Bitmap_GetChannel(dst);
    PIM_Uint8 *buf_temp = src->imageData;
    PIM_Uint8 *dst_buf = dst->imageData;
    PIM_Int32 new_width, new_height;
    PIM_Int32 i, j, cnt=0, idx;
    PIM_Result ret = PIM_SUCCESS;

    if(!buf_temp) return PIM_ERROR_NULL_POINTER;
    if(!dst_buf) return PIM_ERROR_NULL_POINTER;

    dst->height = new_height = src->height;
    dst->width  = new_width  = src->width;
        
    switch(src->imageFormat)
    {
        case PIM_IMAGE_FORMAT_GRAY:
        {
            for (i=new_height-1; i>=0; i--) {
                for (j=0; j<new_width; j++) {
                    idx = (i*new_width+j);
                    dst_buf[idx] = buf_temp[cnt++];
                }
            }
        } break;

        case PIM_IMAGE_FORMAT_RGB888:
        case PIM_IMAGE_FORMAT_BGR888:
        {
            for (i=new_height-1; i>=0; i--) {
                for (j=0; j<new_width; j++) {
                    idx = (i*new_width+j)*3;
                    dst_buf[idx+0 ] = buf_temp[cnt++];
                    dst_buf[idx+1 ] = buf_temp[cnt++];
                    dst_buf[idx+2 ] = buf_temp[cnt++];
                }
            }
        } break;

        case PIM_IMAGE_FORMAT_BGR8880:
        case PIM_IMAGE_FORMAT_RGB8880:
        {
            for (i=new_height-1; i>=0; i--) {
                for (j=0; j<new_width; j++) {
                    idx = (i*new_width+j)*4;
                    dst_buf[idx+0 ] = buf_temp[cnt++];
                    dst_buf[idx+1 ] = buf_temp[cnt++];
                    dst_buf[idx+2 ] = buf_temp[cnt++];
                    dst_buf[idx+3 ] = buf_temp[cnt++];
                }
            }
        } break;

        case PIM_IMAGE_FORMAT_YUVPLANAR_NV21://paul717 (20090408)
        {
            for (i=new_height-1; i>=0; i--) {
                for (j=0; j<new_width; j++) {
                    idx = (i*new_width+j);
                    dst_buf[idx] = buf_temp[cnt++];
                }
            }
            cnt = 0;
            dst_buf += new_width*new_height;
            buf_temp += new_width*new_height;
            for (i=(new_height/2)-1; i>=0; i--) {
                for (j=0; j<new_width; j+=2) {
                    idx = ((i*new_width+j)) & ~1;
                    dst_buf[idx] = buf_temp[cnt++];
                    dst_buf[idx+1] = buf_temp[cnt++];
                }
            }
        } break;

#if 0
    case PIM_IMAGE_FORMAT_YUVPLANAR_NV12_SPLIT://paul717 (20090408)
    case PIM_IMAGE_FORMAT_YUVPLANAR_NV21_SPLIT:
        {
            for (i=new_height-1; i>=0; i--) {
                for (j=0; j<new_width; j++) {
                    idx = (i*new_width+j);
                    dst_buf[idx] = buf_temp[cnt++];
                }
            }
            cnt = 0;
            dst_buf = dst->auxImageData;
            buf_temp = src->auxImageData;
            for (i=(new_height/2)-1; i>=0; i--) {
                for (j=0; j<new_width; j+=2) {
                    idx = ((i*new_width+j)) & ~1;
                    dst_buf[idx] = buf_temp[cnt++];
                    dst_buf[idx+1] = buf_temp[cnt++];
                }
            }
        }
        break;
#endif
        default:
            ret = PIM_ERROR_NOT_SUPPORTED;
    }

    return ret;
}

PIM_Result PIM_Bitmap_Rotate_LeftTop_5(PIM_Bitmap* dst, PIM_Bitmap* src)
{
    PIM_Int32 channel = PIM_Bitmap_GetChannel(dst);
    PIM_Uint8 *buf_temp = src->imageData;
    PIM_Uint8 *dst_buf = dst->imageData;
    PIM_Int32 new_width, new_height;
    PIM_Int32 i, j, cnt=0, idx;
    PIM_Result ret = PIM_SUCCESS;

    if(!buf_temp) return PIM_ERROR_NULL_POINTER;
    if(!dst_buf) return PIM_ERROR_NULL_POINTER;

    dst->height = new_height = src->width;
    dst->width = new_width = src->height;
    
    switch(src->imageFormat)
    {
        case PIM_IMAGE_FORMAT_GRAY:
        {
            for (j=0; j<new_width; j++) {
                for (i=0; i<new_height; i++) {
                    idx = (i*new_width+j);
                    dst_buf[idx] = buf_temp[cnt++];
                }
            }
        } break;

        case PIM_IMAGE_FORMAT_RGB888:
        case PIM_IMAGE_FORMAT_BGR888:
        {

            for (j=0; j<new_width; j++) {
                for (i=0; i<new_height; i++) {
                    idx = (i*new_width+j)*3;
                    dst_buf[idx+0 ] = buf_temp[cnt++];
                    dst_buf[idx+1 ] = buf_temp[cnt++];
                    dst_buf[idx+2 ] = buf_temp[cnt++];
                }
            }
        } break;

        case PIM_IMAGE_FORMAT_BGR8880:
        case PIM_IMAGE_FORMAT_RGB8880:
        {

            for (j=0; j<new_width; j++) {
                for (i=0; i<new_height; i++) {
                    idx = (i*new_width+j)*4;
                    dst_buf[idx+0 ] = buf_temp[cnt++];
                    dst_buf[idx+1 ] = buf_temp[cnt++];
                    dst_buf[idx+2 ] = buf_temp[cnt++];
                    dst_buf[idx+3 ] = buf_temp[cnt++];
                }
            }
        } break;

        default:
            ret = PIM_ERROR_NOT_SUPPORTED;
    }

    return ret;
}
#endif /*__CODE_SIZE_OPTIMIZE__*/


/* exclude data pointer value (imageData, auxImageData) */
#define BITMAP_HEADER_LENGTH (sizeof(PIM_Bitmap) - sizeof(PIM_Uint8*)*2)

PIM_Int32 PIM_Bitmap_CalcSize(PIM_Bitmap *bitmap)
{
    PIM_Int32 bitmapSize = 0;

    /* header size */
    bitmapSize += BITMAP_HEADER_LENGTH;

    /* imageData size */
    bitmapSize += (bitmap->stride * bitmap->height);

    return bitmapSize;
}

PIM_Result PIM_Bitmap_Write(PIM_Uint8 *out, PIM_Bitmap *bitmap)
{
    PIM_Int32 writeSize = 0;
    PIM_Int32 y = 0;
    PIM_Uint8* pDst = NULL;
    PIM_Uint8* pSrc = NULL;
    PIM_Int32 channel = PIM_Bitmap_GetChannel(bitmap);

    pDst = out;

    /* write header of PIM_Bitmap */
    PIM_Memcpy(pDst, bitmap, BITMAP_HEADER_LENGTH);
    pDst += BITMAP_HEADER_LENGTH;
    writeSize += BITMAP_HEADER_LENGTH;

    /* write imageData of PIM_Bitmap */
    for(y=0; y<bitmap->height; y++) {
        PIM_Memcpy(pDst, pSrc, bitmap->width*channel);

        pDst += bitmap->stride;
        pSrc += bitmap->stride;
        writeSize += bitmap->stride;
    }

    //return writeSize;
    return PIM_SUCCESS;
}

/*
 *  @note bitmap 에 이미지가 항당되어 있으면 안된다.
 */
PIM_Result PIM_Bitmap_Read(PIM_Bitmap *bitmap, PIM_Uint8 *in)
{
    PIM_Result ret = PIM_SUCCESS;  /* success or error code */
    PIM_Int32 y = 0;
    PIM_Uint8* pDst = NULL;
    PIM_Uint8* pSrc = NULL;
    PIM_Bitmap tempBitmap = {0};
    //PIM_Int32 channel = PIM_Bitmap_GetChannel(bitmap);

    pSrc = in;
    pDst = (PIM_Uint8*)&tempBitmap;

    /* read header of PIM_Bitmap */
    PIM_Memcpy(pDst, pSrc, BITMAP_HEADER_LENGTH);
    pSrc += BITMAP_HEADER_LENGTH;
    //ret += BITMAP_HEADER_LENGTH;

    /* set bitmap */
    if(bitmap->reference == 1) /* 외부에서 할당한 bitmap은 해제가 불가하다. */
    {
        if(bitmap->width != tempBitmap.width ||
           bitmap->height != tempBitmap.height ||
           bitmap->imageFormat != tempBitmap.imageFormat)
        {
            LOGE("read bitmap size(%dx%d) format(%x)", tempBitmap.width, tempBitmap.height, tempBitmap.imageFormat);
            LOGE("dst  bitmap size(%dx%d) format(%x)", bitmap->width, bitmap->height, bitmap->imageFormat);
            return PIM_ERROR_INVALID_ARGUMENT;
        }
    }
    else
    {
        PIM_Bitmap_Destroy(bitmap);
        ret = PIM_Bitmap_Create(bitmap, tempBitmap.width, tempBitmap.height, tempBitmap.imageFormat);
        if(ret != PIM_SUCCESS) {
            LOGE("PIM_Bitmap_Create ret(%d)", ret);
            return ret;
        }
    }

    /* write imageData of PIM_Bitmap */
    pDst = bitmap->imageData;
    for(y=0; y<bitmap->height; y++) {
        PIM_Memcpy(pDst, pSrc, tempBitmap.stride);

        pDst += bitmap->stride;
        pSrc += tempBitmap.stride;
        //ret += bitmap->stride;
    }

    return ret;
}


/********************************************************************
 * Index Image. (PIM_IMAGE_FORMAT_RGBA_INDEX)
 *******************************************************************/
#define MAX_INDEX_IMAGE_SIZE (200*1024)
PIM_Result PIM_Bitmap_MappingIndexImage(PIM_List *pBitmapList, PIM_Uint8 *index_image)
{
	PIM_Int32 i = 0;
    PIM_Int32 numImages = 0;
    PIM_Result ret = PIM_SUCCESS;
    PIM_Uint8 magic[4];
	PIM_ByteStream *bs = NULL;

    if(pBitmapList == NULL || index_image == NULL) {
        return PIM_ERROR_INVALID_ARGUMENT;
    }

    PIM_List_RemoveAll(pBitmapList);

	bs = PIM_ByteStream_Create((PIM_Uint8 *)index_image, MAX_INDEX_IMAGE_SIZE);

    /* check magic number */
    for(i=0; i<4; i++) magic[i] = PIM_ByteStream_ReadUint8(bs);

    if(magic[0] != 0x01 || magic[1] != 0xAF || magic[2] != 0xAC || magic[3] != 0xE1) {
		/* Wrong magic */
        LOGE("wrong magic");
		goto END_PROCESS;
	}

    /* [Not Used] image name. */
	for (i=0; i<12; i++) PIM_ByteStream_ReadUint8(bs);

    /* get count of images */
	numImages = PIM_ByteStream_ReadInt32(bs);

    /* set bitmap */
	for (i=0; i<numImages; i++) {
        PIM_Bitmap bitmap = {0};

		bitmap.width        = PIM_ByteStream_ReadInt32(bs);
		bitmap.height       = PIM_ByteStream_ReadInt32(bs);
		bitmap.auxImageData = index_image + PIM_ByteStream_ReadInt32(bs);
		bitmap.imageData    = index_image + PIM_ByteStream_ReadInt32(bs);
        bitmap.imageFormat  = PIM_IMAGE_FORMAT_RGB_INDEX;

		bitmap.stride       = bitmap.width;
		bitmap.reference    = PIM_TRUE;

        PIM_List_AddTail(pBitmapList, (PIM_pVoid)&bitmap);
	}
	
END_PROCESS:
	PIM_ByteStream_Destroy(bs);

    return ret;
}

PIM_Result PIM_Bitmap_FixRatioResize(PIM_Bitmap* dst, PIM_Bitmap* src, PIM_Int32 limit_size)
{
    PIM_Result ret = PIM_SUCCESS;

    if(limit_size < src->width*src->height) {
        PIM_Int32 width, height;
        double ratio = 0.0f;

        /*  */
        width  = src->width;
        height = src->height;
        ratio = PIM_Device_Sqrt((double)limit_size/(double)(width*height));
        width  = width  * ratio;
        height = height * ratio;
        width  &= (~1);
        height &= (~1);

        /*  */
        PIM_Bitmap_Destroy(dst);
        PIM_Bitmap_Create(dst, width, height, src->imageFormat);
        ret = PIM_Bitmap_Resize_BI(dst, src);
    } else {
        ret = PIM_Bitmap_Clone(dst, src);
    }

    return ret;
}

#ifdef __cplusplus
}
#endif
