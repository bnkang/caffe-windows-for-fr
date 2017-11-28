/**
 *	@file PIMTypes.h
 *
 *  @brief  Declares basic data types and structure for PIMworks libraries.
 *
 *  
 *  Copyright IM Lab,(http://imlab.postech.ac.kr)
 *
 *  All rights reserved.
 */



#ifndef __PIMTYPES_H__
#define __PIMTYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef int PIM_Bool;
typedef unsigned int PIM_Uint32;
typedef int PIM_Int32;
typedef char PIM_Int8;
typedef short PIM_Int16;
typedef unsigned short PIM_Uint16;
typedef unsigned char PIM_Uint8;
typedef char PIM_Char;
typedef long PIM_Long;
typedef float PIM_Float;
typedef double PIM_Double;

typedef void  PIM_Void;
typedef void *PIM_pVoid;

typedef unsigned char *PIM_String;
typedef unsigned int PIM_Time_t;
typedef unsigned int PIM_Size_t;
typedef PIM_Uint32 PIM_Clock_t;

#define PIM_TRUE 1
#define PIM_FALSE 0

#ifndef NULL
#define NULL 0
#endif

#define PIM_KMaxInt32   0x7fffffff
#define PIM_KMinInt32   (PIM_Int32)0x80000000
#define PIM_KMaxUint32  0xffffffffu

#define PIM_CLOCKS_PER_SEC      1000                /**< 1 clock is 1 ms. */

/**
 *  @struct PIM_Rect PIMTypes.h "PIMTypes.h"
 *  @brief four integer coordinates for a rectangle
 */
typedef struct PIM_Rect
{
    PIM_Int32 left;
    PIM_Int32 top;
    PIM_Int32 right;
    PIM_Int32 bottom;
} PIM_Rect;

/**
 *  @struct PIM_Size PIMTypes.h "PIMTypes.h"
 *  @brief two integer for size (width and height dimensions)
 */
typedef struct PIM_Size
{
    PIM_Int32 width;
    PIM_Int32 height;
} PIM_Size;

/**
 *  @struct PIM_Point PIMTypes.h "PIMTypes.h"
 *  @brief two integer coordinates
 */
typedef struct PIM_Point
{
    PIM_Int32 x;
    PIM_Int32 y;
} PIM_Point;

typedef struct PIM_DPoint
{
	PIM_Double x;
	PIM_Double y;
} PIM_DPoint;

/**
 *  @struct PIM_3D_Point PIMTypes.h "PIMTypes.h"
 *  @brief three integer coordinates
 */
typedef struct PIM_3D_Point
{
    PIM_Int32 x;
    PIM_Int32 y;
    PIM_Int32 z;
} PIM_3D_Point;


/**
 *  @enum PIM_ImageFormat PIMTypes.h "PIMTypes.h"
 *  @brief image format
 */
typedef enum {
    PIM_IMAGE_FORMAT_GRAY       = 0x001,    /**< Gray image (1 channel)                                          */
    PIM_IMAGE_FORMAT_RGB_INDEX  = 0x002,    /**< @this format can be used only resource. Index image (4 channel, not use alpha channel) */

    PIM_IMAGE_FORMAT_RGB_LABEL  = 0x100,    /**< General RGB (used for bitmasking)              */
    PIM_IMAGE_FORMAT_RGB888     = 0x101,
    PIM_IMAGE_FORMAT_BGR888     = 0x102,
    PIM_IMAGE_FORMAT_RGB8880    = 0x103,
    PIM_IMAGE_FORMAT_BGR8880    = 0x104,
    PIM_IMAGE_FORMAT_RGBA8888   = 0x105,    /**< @this format can be used only Overlay function */

    /*
     * http://www.fourcc.org/yuv.php
     *  1. Yuv Packed format : 0x200 ~ 0x2ff
     *  2. Yuv Planar format : 0x400 ~ 0x4ff
     *                                                    [FOURCC in Hex] description
     */
    PIM_IMAGE_FORMAT_YUVPACKED_LABEL       = 0x200,    /* ******** YUV Packed format ******** */
    PIM_IMAGE_FORMAT_YUVPACKED_Y411        = 0x201,    /* [0x31313459] YUV 4:1:1 with a packed, 6 byte/4 pixel macroblock structure.          * 
                                                        *                        (U2 Y0 Y1 V2 Y2 Y3) : Macropixel = 6 bytes = 4 image pixels. *
                                                        * @this format can be used only FaceDetector                                          */

    PIM_IMAGE_FORMAT_YUVPLANAR_LABEL       = 0x400,    /* ******** YUV Planar format ******** */
    PIM_IMAGE_FORMAT_YUVPLANAR_NV12        = 0x401,    /* [0x3231564E] 8-bit Y plane followed by an interleaved U/V plane with 2x2 subsampling (PIM_ImageFormat_EYuv420NV12)    */
    PIM_IMAGE_FORMAT_YUVPLANAR_NV21        = 0x402,    /* [0x3132564E] As NV12 with U and V reversed in the interleaved plane                  (PIM_ImageFormat_EYuv420NV21)    */
    PIM_IMAGE_FORMAT_YUVPLANAR_YV12        = 0x403,    /* [0x32315659] 8 bit Y plane followed by 8 bit 2x2 subsampled V and U planes.                                           */
    PIM_IMAGE_FORMAT_YUVPLANAR_I420        = 0x404,    /* [0x30323449] 8 bit Y plane followed by 8 bit 2x2 subsampled U and V planes.                                           */

} PIM_ImageFormat;


/* normal    CW90          CW180     CW270      */
/*                                              */
/* 888888    8888888888        88    88         */
/* 88            88  88        88    88  88     */
/* 8888              88      8888    8888888888 */
/* 88                          88               */
/* 88                      888888               */

/**
 *  @enum PIM_ImageOrientation PIMTypes.h "PIMTypes.h"
 *  @brief image orientation
 */
typedef enum {
    PIM_IMAGE_ORIENTATION_NORMAL, /**< PIM_ImageOrientation_ETopLeft     = 1, : Normal case                        */
    PIM_IMAGE_ORIENTATION_CW90,   /**< PIM_ImageOrientation_ELeftBottom  = 8  : 90-degree clockwise rotated        */
    PIM_IMAGE_ORIENTATION_CW180,  /**< PIM_ImageOrientation_EBottomRight = 3, : 180-degree clockwise rotated       */
    PIM_IMAGE_ORIENTATION_CW270   /**< PIM_ImageOrientation_ERightTop    = 6, : 90-degree counterclockwise rotated */

} PIM_ImageOrientation;


/**
 *  @struct PIM_Bitmap PIMTypes.h "PIMTypes.h"
 *  @brief general image structure in the PIM libraries
 */
typedef struct PIM_Bitmap
{
    PIM_Int32 width;
    PIM_Int32 height;
    PIM_ImageFormat imageFormat;
    PIM_ImageOrientation orientation;

    PIM_Int32  align;                   /**< Alignment of image rows (default:4) */
    PIM_Int32  stride;                  /**< The number of bytes in "imageData" to skip between rows */
    PIM_Int32  reference;               /**< if reference is 1, From outside the allocated memory */

    PIM_Uint8 *imageData;
    PIM_Uint8 *auxImageData;            /**< Pointer to auxiliary image data (Usually ignored) */
} PIM_Bitmap;


/**
 *  @struct PIM_Version PIMTypes.h "PIMTypes.h"
 *  @brief Structure representing Library version. Version is represented as AA.BB.CC.DD
 */
typedef struct PIM_Version
{
    PIM_Uint16 major;        /**< Major version 0xAABB */
    PIM_Uint16 minor;        /**< Minor version 0xCCDD */
    PIM_Char*  config;       /**< Configuration string */
    PIM_Char*  copyright;    /**< Copyright string     */
} PIM_Version;


typedef PIM_pVoid PIM_pFile;

/**
 *  @enum PIM_File_SeekMode PIMTypes.h "PIMTypes.h"
 *  @brief seek mode.
 */
typedef enum {
    PIM_FSEEK_EBEGIN = 0,
    PIM_FSEEK_ECUR,
    PIM_FSEEK_EEND
} PIM_File_SeekMode;

/**
 *  @enum PIM_File_OpenMode PIMTypes.h "PIMTypes.h"
 *  @brief file open mode
 */
typedef enum {
    PIM_FOPEN_EBINARY_READ,
    PIM_FOPEN_EBINARY_WRITE,
    PIM_FOPEN_EBINARY_NEW_WRITE,
    PIM_FOPEN_EBINARY_APPEND,
    PIM_FOPEN_ETEXT_READ,
    PIM_FOPEN_ETEXT_WRITE,
    PIM_FOPEN_ETEXT_NEW_WRITE,
    PIM_FOPEN_ETEXT_APPEND,
} PIM_File_OpenMode;

/**
 *  @enum PIM_Result PIMTypes.h "PIMTypes.h"
 *  @brief common return value.
 */
typedef enum {
    PIM_SUCCESS                  = 0,
    PIM_ERROR_GENERAL            = -1,
    PIM_ERROR_NOT_SUPPORTED      = -2,
    PIM_ERROR_NULL_POINTER       = -3,
    PIM_ERROR_INVALID_ARGUMENT   = -4,
    PIM_ERROR_INTERRUPTED        = -5,
    PIM_ERROR_BUFFER_OVERFLOW    = -6,
    PIM_ERROR_INVALID_STATUS     = -7,
	PIM_ERROR_NOMEM              = -8,

    PIM_ERROR_EXPIRED            = -32,
    
    PIM_ERROR_IO                 = -100,
    PIM_ERROR_FILE_NOT_FOUND     = -101,

	// face recognition only
	PIM_ERROR_NOT_CREATED        = -300,
	PIM_ERROR_ALREADY_EXISTS     = -301,
} PIM_Result;


/**
 *  @struct PIM_Tm PIMTypes.h "PIMTypes.h"
 *  @brief Corresponds to struct tm in std C.
 */
typedef struct PIM_Tm PIM_Tm;
struct PIM_Tm
{
    PIM_Int32 tm_sec;       /**< 0 ~ 60 */ 
    PIM_Int32 tm_min;       /**< 0 ~ 59 */ 
    PIM_Int32 tm_hour;      /**< 0 ~ 23 */ 
    PIM_Int32 tm_mday;      /**< 1 ~ 31 */ 
    PIM_Int32 tm_mon;       /**< 0 ~ 11 */ 
    PIM_Int32 tm_year;      /**< year - 1900 */ 
    PIM_Int32 tm_wday;      /**< Not used */ 
    PIM_Int32 tm_yday;      /**< Not used */ 
    PIM_Int32 tm_isdst;     /**< Not used */
};


/**
 *  @struct PIM_DateTime PIMTypes.h "PIMTypes.h"
 *  @brief Human friendly date and time structure. Preferred to PIM_Tm in most cases.
 */
typedef struct PIM_DateTime
{
	PIM_Int32 year;         /**< Any value */ 
    PIM_Int32 month;        /**< 1 ~ 12 */ 
    PIM_Int32 day;          /**< 1 ~ 31 */ 
    PIM_Int32 hour;         /**< 0 ~ 23 */ 
    PIM_Int32 minute;       /**< 0 ~ 59 */ 
    PIM_Int32 second;       /**< 0 ~ 59 */ 
    PIM_Int32 milisec;      /**< 0 ~ 999 */ 
} PIM_DateTime;

/**
 *  @struct PIM_Timeval PIMTypes.h "PIMTypes.h"
 *  @brief Corresponds to struct timeval in std C.
 */
typedef struct PIM_Timeval
{
    PIM_Time_t tv_sec;
    unsigned int tv_usec;
} PIM_Timeval;

/**
 *  @struct PIM_Timezone PIMTypes.h "PIMTypes.h"
 *  @brief Corresponds to struct timezone in std C. Currently reserved.
 */
typedef struct PIM_Timezone
{
    PIM_Int32 tz_minuteswest;
    PIM_Int32 tz_dsttime;
} PIM_Timezone;


#ifdef __cplusplus
}
#endif

#endif /*__PIMTYPES_H__*/

