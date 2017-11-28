/*
 * Copyright IM Lab,(http://imlab.postech.ac.kr)
 * 
 * All rights reserved.
 * 
 */

#ifndef __PIMBITMAP_H__
#define __PIMBITMAP_H__

#include "PIMTypes.h"
#include "PIMList.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  @brief color_type ( 0:red, 1:green, 2:blue, 3:alpha, 4:rgb)
 */
typedef enum {
	Color_Type_ERed,
	Color_Type_EGreen,
	Color_Type_EBlur,
	Color_Type_EAlpha,
	Color_Type_ERGB
} Color_Type;

/**
 *  @brief 
 *
 *  @param bitmap [IN] : 
 *  @param x [IN] : x-coordinate
 *  @param y [IN] : y-coordinate
 *  @param type [IN] : 
 *
 *  @return ret : 
 */
extern PIM_Uint32 Get_Pixel_RGB(PIM_Bitmap *bitmap, PIM_Int32 x, PIM_Int32 y, Color_Type type);

#define Get_R(_bitmap_,_x_,_y_)		Get_Pixel_RGB(_bitmap_,_x_,_y_,Color_Type_ERed)
#define Get_G(_bitmap_,_x_,_y_)		Get_Pixel_RGB(_bitmap_,_x_,_y_,Color_Type_EGreen)
#define Get_B(_bitmap_,_x_,_y_)		Get_Pixel_RGB(_bitmap_,_x_,_y_,Color_Type_EBlur)
#define Get_A(_bitmap_,_x_,_y_)		Get_Pixel_RGB(_bitmap_,_x_,_y_,Color_Type_EAlpha)
#define Get_RGB(_bitmap_,_x_,_y_)	Get_Pixel_RGB(_bitmap_,_x_,_y_,Color_Type_ERGB)


/**
 *  @brief 
 *
 *  @param bitmap [IN] : 
 *  @param x [IN] : x-coordinate
 *  @param y [IN] : y-coordinate
 *  @param type [IN] : 
 *
 *  @return ret : 
 */
extern PIM_Uint32 Set_Pixel_RGB(PIM_Bitmap *bitmap, PIM_Int32 x, PIM_Int32 y, Color_Type type, PIM_Uint32 color);

#define Set_R(_bitmap_,_x_,_y_,_color_)		Set_Pixel_RGB(_bitmap_,_x_,_y_,Color_Type_ERed,_color_)
#define Set_G(_bitmap_,_x_,_y_,_color_)		Set_Pixel_RGB(_bitmap_,_x_,_y_,Color_Type_EGreen,_color_)
#define Set_B(_bitmap_,_x_,_y_,_color_)		Set_Pixel_RGB(_bitmap_,_x_,_y_,Color_Type_EBlur,_color_)
#define Set_A(_bitmap_,_x_,_y_,_color_)		Set_Pixel_RGB(_bitmap_,_x_,_y_,Color_Type_EAlpha,_color_)
#define Set_RGB(_bitmap_,_x_,_y_,_color_)	Set_Pixel_RGB(_bitmap_,_x_,_y_,Color_Type_ERGB,_color_)


/**
 *  @brief 
 */
#define Get_GrayPtr(bitmap, x, y) (bitmap->imageData + (((y)*bitmap->stride) + (x)))

/**
 *  @brief 
 */
#define GET_GRAY(bitmap, x, y) (*Get_GrayPtr((bitmap), (x), (y)))

/**
 *  @brief 
 */
#define SET_GRAY(bitmap, x, y, value) (*(bitmap->imageData + (((y)*bitmap->stride) + (x)))) = value

/**
 *  @brief PIM_Bitmap_CreateRef creates the bitmap object that not allocates memory and refers to external image data(parameter 2).
 *  @note The reference field of PIM_Bitmap type sets ¡®1¡¯ by this function.
 *
 *  @param bitmap [OUT] : On return, contains the pointer to created PIM_Bitmap object.
 *  @param imageData [IN] : external image data
 *  @param width [IN] : image width
 *  @param height [IN] : image height
 *  @param stride [IN] : The number of bytes in "imageData" to skip between rows
 *  @param format [IN] : image format
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret == PIM_SUCCESS : success
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Result PIM_Bitmap_CreateRef(PIM_Bitmap *bitmap, PIM_Uint8 *imageData, PIM_Int32 width, PIM_Int32 height, PIM_Int32 stride, PIM_ImageFormat format);

/**
 *  @brief PIM_Bitmap_CreateYuvRef creates the bitmap object that not allocates memory and refers to external image data(parameter 2).
 *  @note The reference field of PIM_Bitmap type sets ¡®1¡¯ by this function.
 *
 *  @param bitmap [OUT] : On return, contains the pointer to created PIM_Bitmap object.
 *  @param y_data  [IN] : external y image data
 *  @param uv_data [IN] : external uv image data
 *  @param width   [IN] : image width
 *  @param height  [IN] : image height
 *  @param stride  [IN] : The number of bytes in "y_data" to skip between rows
 *  @param format  [IN] : image format
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret == PIM_SUCCESS : success
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Result PIM_Bitmap_CreateYuvRef(PIM_Bitmap *bitmap, PIM_Uint8 *y_data, PIM_Uint8 *uv_data, PIM_Int32 width, PIM_Int32 height, PIM_Int32 stride, PIM_ImageFormat format);

/**
 *  @brief PIM_Bitmap_Create creates the bitmap object that allocates memory for image data.
 *
 *  @param bitmap [OUT] : On return, contains the pointer to created PIM_Bitmap object.
 *  @param width [IN] : image width
 *  @param height [IN] : image height
 *  @param format [IN] : image format
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret == PIM_SUCCESS : success
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Result PIM_Bitmap_Create(PIM_Bitmap *bitmap, PIM_Int32 width, PIM_Int32 height, PIM_ImageFormat format);

/**
 *  @brief PIM_Bitmap_Recreate creates the bitmap object that allocates memory for image data.
 *  @note If bitmap is allocated and property values are different, bitmap is reassigned.
 *
 *  @param bitmap [IN/OUT] : On return, contains the pointer to created PIM_Bitmap object.
 *  @param width [IN] : image width
 *  @param height [IN] : image height
 *  @param format [IN] : image format
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret == PIM_SUCCESS : success
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Result PIM_Bitmap_Recreate(PIM_Bitmap *bitmap, PIM_Int32 width, PIM_Int32 height, PIM_ImageFormat format);

/**
 *  @brief release resources of the bitmap.
 *
 *  @param bitmap [IN/OUT] : On return, contains the pointer to destroyed PIM_Bitmap object
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret == PIM_SUCCESS : success
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Result PIM_Bitmap_Destroy(PIM_Bitmap *bitmap);

/**
 *  @brief 
 *
 *  @param dst [IN] : a destination image
 *  @param src [IN] : a source image
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret == PIM_SUCCESS : success
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Result PIM_Bitmap_Copy(PIM_Bitmap *dst, PIM_Bitmap *src);

/**
 *  @brief The dst is allocated memory block for image data inside the PIM_Bitmap_clone.  PIM_Bitmap_clone copies all values from src to dst
 *
 *  @param dst [IN] : a destination image
 *  @param src [IN] : a source image
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret == PIM_SUCCESS : success
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Result PIM_Bitmap_Clone(PIM_Bitmap *dst, PIM_Bitmap *src);



/*********************************************************************************************
 * internal function
 ********************************************************************************************/

/**
 *  @brief 
 *
 *  @param bitmap [IN] : 
 *
 *  @return ret : 
 */
extern PIM_Int32 PIM_Bitmap_GetBitPerPixel(PIM_Bitmap *bitmap);

/**
 *  @brief 
 *
 *  @param bitmap [IN] : 
 *
 *  @return ret : 
 */
extern PIM_Int32 PIM_Bitmap_GetChannel(PIM_Bitmap *bitmap);

/**
 *  @brief 
 *
 *  @param bitmap [IN] : 
 *
 *  @return ret : 
 */
extern PIM_Int32 PIM_Bitmap_GetPeriod(PIM_Bitmap *bitmap); /* RGB or YUV(y) */

/**
 *  @brief 
 *
 *  @param format [IN] : 
 *  @param width [IN] : 
 *  @param align_num [IN] : 
 *
 *  @return ret : 
 */
extern PIM_Int32 PIM_Bitmap_GetStride(PIM_ImageFormat format, PIM_Int32 width, PIM_Int32 align_num);

/**
 *  @brief 
 *
 *  @param bitmap [IN] : 
 *
 *  @return ret : 
 */
extern PIM_Int32 PIM_Bitmap_GetImageSize(PIM_Bitmap *bitmap); // G

/**
 *  @brief Check the validity of the image
 *
 *  @param bitmap [IN] : image
 *
 *  @return : If valid, return PIM_TRUE. If not valid, return PIM_FALSE.
 */
extern PIM_Bool PIM_Bitmap_isValid     (PIM_Bitmap *bitmap);

/**
 *  @brief check the selected point is available in the image.
 *
 *  @param bitmap [IN] : image
 *  @param pos [IN] : selected point
 *
 *  @return : If valid, return PIM_TRUE. If not valid, return PIM_FALSE.
 */
extern PIM_Bool PIM_Bitmap_isValidPoint(PIM_Bitmap *bitmap, PIM_Point *pos);

/**
 *  @brief check the selected area is available in the image.
 *
 *  @param bitmap [IN] : image
 *  @param rect [IN] : selected area
 *
 *  @return : If valid, return PIM_TRUE. If not valid, return PIM_FALSE.
 */
extern PIM_Bool PIM_Bitmap_isValidRect (PIM_Bitmap *bitmap, PIM_Rect *rect);

/**
 *  @brief By sampling "src image" is copied to "dst image".
 *
 *  @param dst [IN] : a destination image
 *  @param src [IN] : a source image
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret == PIM_SUCCESS : success
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Result PIM_Bitmap_Resize(PIM_Bitmap* dst, PIM_Bitmap* src);

/**
 *  @brief By resizing "src image" is copied to "dst image".
 *
 *  @param dst [IN] : a destination image
 *  @param src [IN] : a source image
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret == PIM_SUCCESS : success
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Result PIM_Bitmap_Resize_BI(PIM_Bitmap *dst, PIM_Bitmap *src);

/**
 *  @brief "dst image" is copied to a specific area of "src image".
 *  @note size of src_rect and "dst image" should be the same size. (not resized) 
 *
 *  @param dst [IN] : a destination image
 *  @param src [IN] : a source image
 *  @param src_rect [IN] : need to crop area of "src image"
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret == PIM_SUCCESS : success
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Result PIM_Bitmap_Crop(PIM_Bitmap *dst, PIM_Bitmap *src, PIM_Rect *src_rect);

/**
 *  @brief Crop a 'src_rect' region of the 'src' image. resize Cropped image to fit the 'dst' image.
 *
 *  @param dst      [OUT] : a destination image
 *  @param src      [IN]  : a source image
 *  @param src_rect [IN]  : crop area of "src image"
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret == PIM_SUCCESS : success
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Result PIM_Bitmap_CropResize(PIM_Bitmap *dst, PIM_Bitmap *src, PIM_Rect *src_rect);

/**
 *  @brief rotate the image.
 *
 *  @param dst [OUT] : a destination image
 *  @param src [IN]  : a source image
 *  @param ori [IN]  : the direction of rotation
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret == PIM_SUCCESS : success
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 *  @see PIM_ImageOrientation : see PIMTypes.h
 */
extern PIM_Result PIM_Bitmap_Rotate(PIM_Bitmap *dst, PIM_Bitmap *src, PIM_ImageOrientation ori);

/**
 *  @brief rotate the image.
 *
 *  @param dst       [OUT]    : a destination image
 *  @param src       [IN]     : a source image
 *  @param ref_point [IN/OUT] : 
 *  @param angle     [IN]     : 
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret == PIM_SUCCESS : success
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Result PIM_Bitmap_RotateVersatile(PIM_Bitmap *dst, PIM_Bitmap *src, PIM_Point *ref_point, PIM_Int32 angle);

/**
 *  @brief rotate right and left of the image
 *
 *  @param bitmap [IN] : image
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret == PIM_SUCCESS : success
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Result PIM_Bitmap_HFlip(PIM_Bitmap *bitmap);

/**
 *  @brief rotate the image upside down
 *
 *  @param bitmap [IN] : image
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret == PIM_SUCCESS : success
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Result PIM_Bitmap_VFlip(PIM_Bitmap *bitmap);

/**
 *  @brief 
 *
 *  @param bitmap [IN] : 
 *
 *  @return ret : the size of bitmap include header and imageData.
 */
extern PIM_Int32 PIM_Bitmap_CalcSize(PIM_Bitmap *bitmap);

/**
 *  @brief copy the bitmap data to the out buffer.
 *
 *  @param out [OUT] : allocated memory block.
 *  @param bitmap [IN] : image
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret == PIM_SUCCESS : success
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Result PIM_Bitmap_Write(PIM_Uint8 *out, PIM_Bitmap *bitmap);

/**
 *  @brief copy the in buffer to the bitmap.
 *
 *  @param bitmap [OUT] : created image.
 *  @param in [IN] : 
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret == PIM_SUCCESS : success
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Result PIM_Bitmap_Read(PIM_Bitmap *bitmap, PIM_Uint8 *in);

/**
 *  @brief 
 *
 *  @param pBitmapList [IN/OUT] : index image list.
 *  @param index_image [IN]     : index image resource.
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret == PIM_SUCCESS : success
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Result PIM_Bitmap_MappingIndexImage(PIM_List *pBitmapList, PIM_Uint8 *index_image);



#ifdef __cplusplus
}
#endif

#endif /*__PIMBITMAP_H__*/
