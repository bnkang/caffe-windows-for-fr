#ifndef __OPENCV_H__
#define __OPENCV_H__

#include "PIMTypes.h"

//#define __USE_OPENCV__
#ifdef __USE_OPENCV__
    #define LoadJpeg(_file_,_bitmap_)    OpenCV_LoadJpeg(_file_,_bitmap_)
    #define SaveJpeg(_file_,_bitmap_)    OpenCV_SaveJpeg(_file_,_bitmap_)

    #define ShowBitmap(_window_,_bitmap_,_flip_)    OpenCV_ShowBitmap(_window_,_bitmap_,_flip_)
#else
    #define LoadJpeg(_file_,_bitmap_)
    #define SaveJpeg(_file_,_bitmap_)

    #define ShowBitmap(_window_,_bitmap_,_flip_)
#endif/* __USE_OPENCV__ */



#ifdef __cplusplus
extern "C" {
#endif

typedef PIM_pVoid CameraHandle;
typedef PIM_pVoid RecorderHandle;
typedef PIM_pVoid VideoHandle;

/*****************************************************************************
 * WebCam
 *****************************************************************************/
extern CameraHandle OpenCV_Camera_Create(PIM_Int32* width, PIM_Int32* height);
extern PIM_Void     OpenCV_Camera_Destroy(CameraHandle* camera);
extern PIM_Result   OpenCV_Camera_ReadFrame(CameraHandle camera, PIM_Bitmap* bitmap);



/*****************************************************************************
 * Recorder
 *****************************************************************************/
extern RecorderHandle OpenCV_Recorder_Create(const PIM_Char* file_name, PIM_Int32 width, PIM_Int32 height, PIM_Bool setCodec);
extern PIM_Void       OpenCV_Recorder_Destroy(RecorderHandle* recorder);
extern PIM_Result     OpenCV_Recorder_WriteFrame(RecorderHandle recorder, PIM_Bitmap* bitmap);



/*****************************************************************************
 * Video
 *****************************************************************************/
extern VideoHandle  OpenCV_Video_Create(const PIM_Char* file_name);
extern PIM_Void     OpenCV_Video_Destroy(VideoHandle* video);
extern PIM_Result   OpenCV_Video_ReadFrame(VideoHandle video, PIM_Bitmap* bitmap);
extern PIM_Result   OpenCV_Video_SetFrameIndex(VideoHandle video, PIM_Int32 which_frame);
extern PIM_Int32    OpenCV_Video_GetFrameIndex(VideoHandle video);
extern PIM_Int32    OpenCV_Video_SetFPS(VideoHandle video, PIM_Int32 fps);
extern PIM_Int32    OpenCV_Video_GetFPS(VideoHandle video);



/*****************************************************************************
 * OpenCV common method
 *****************************************************************************/
extern PIM_Result   OpenCV_LoadJpeg(const PIM_Char* file_name, PIM_Bitmap* bitmap);
extern PIM_Result   OpenCV_SaveJpeg(const PIM_Char* file_name, PIM_Bitmap* bitmap);

/*
 * flip
 *  0x00000000 : not flip the image
 *  0x00000001 : Flip the image horizontally
 *  0x00000002 : Flip the image vertically
 */
extern PIM_Result   OpenCV_ShowBitmap(PIM_Char* window_name, PIM_Bitmap* bitmap, PIM_Uint32 flip);

/*
 * PIM_Bitmap* bitmap : not draw on bitmap
 * PIM_Uint32  color  : 0xBbGgRr.. BGR0
 */
extern PIM_Result   OpenCV_DrawRectAndShowBitmap(PIM_Char* window_name, PIM_Bitmap* bitmap, PIM_Rect* rect, PIM_Uint32 color);

extern PIM_Result   OpenCV_DrawTextAndShowBitmap(PIM_Char* window_name, PIM_Bitmap* bitmap, PIM_Point point, PIM_Uint32 color);

extern PIM_Result   OpenCV_DrawRect(PIM_Bitmap* bitmap, PIM_Rect* rect, PIM_Uint32 color);

extern PIM_Result   OpenCV_DrawText(PIM_Bitmap* bitmap, PIM_Point point, PIM_Uint32 color);

extern PIM_Uint32   OpenCV_GetRGB(PIM_Uint8 r, PIM_Uint8 g, PIM_Uint8 b);

/*
CvFont font;
11.  cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 1.0, 1.0, 0, 1, CV_AA); 
12.  cvPutText(img, "Hello World!", cvPoint(20, 20), &font, cvScalar(255, 255, 255, 0));
 */

#ifdef __cplusplus
}
#endif

#endif /*__OPENCV_H__*/
