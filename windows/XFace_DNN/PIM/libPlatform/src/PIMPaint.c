#include "libPlatform/inc/PIMMemory.h"
#include "libPlatform/inc/PIMMath.h"
#include "libPlatform/inc/PIMMask.h"
#include "libPlatform/inc/PIMBitmap.h"
#include "libPlatform/inc/PIMPaint.h"

#include "libPlatform/inc/PIMLog.h"
#define TAG "PIM"

#ifdef __cplusplus
extern "C" {
#endif



/*******************************************************************************************
 * Internal Method
 ******************************************************************************************/
static PIM_Int32 Paint_DrawPoint_RGB(PIM_Bitmap *bitmap, PIM_Int32 x, PIM_Int32 y, PIM_Color color);
static PIM_Int32 Paint_DrawRect_RGB (PIM_Bitmap *bitmap, PIM_Rect *rect, PIM_Color color);
static PIM_Int32 Paint_DrawVLine_RGB(PIM_Bitmap *bitmap, PIM_Int32 x_fix, PIM_Int32 y_start, PIM_Int32 y_end, PIM_Color color);
static PIM_Int32 Paint_DrawHLine_RGB(PIM_Bitmap *bitmap, PIM_Int32 x_start, PIM_Int32 x_end, PIM_Int32 y_fix, PIM_Color color);



PIM_Result PIM_Paint_DrawPoint(PIM_Bitmap *bitmap, PIM_Int32 x, PIM_Int32 y, PIM_Color color)
{
	PIM_Int32 cpos;
	PIM_Int32 uvpos;
	PIM_Uint32 conv_color;
    PIM_Point pos = {x, y};

	if (PIM_Bitmap_isValid(bitmap) == PIM_FALSE) return PIM_ERROR_INVALID_ARGUMENT;
	if (PIM_Bitmap_isValidPoint(bitmap, &pos) == PIM_FALSE) return PIM_ERROR_INVALID_ARGUMENT;

	switch (bitmap->imageFormat)
    {
		case PIM_IMAGE_FORMAT_YUVPLANAR_NV21:
			cpos = (y * bitmap->stride + x);
			uvpos = (bitmap->stride * bitmap->height) + ((y/2) * bitmap->stride + x);
			conv_color = PIM_Color_GetYUV(color);
			bitmap->imageData[cpos] = (PIM_Uint8)(conv_color >> 16);
			bitmap->imageData[uvpos] = (PIM_Uint8)(conv_color);
			bitmap->imageData[uvpos+1] = (PIM_Uint8)(conv_color >> 8);
			break;

		case PIM_IMAGE_FORMAT_BGR888:
		case PIM_IMAGE_FORMAT_BGR8880:
		case PIM_IMAGE_FORMAT_RGB888:
		case PIM_IMAGE_FORMAT_RGB8880:
			Paint_DrawPoint_RGB(bitmap, x, y, color);
			break;
			
		default:
			return PIM_ERROR_NOT_SUPPORTED;
	}
	return PIM_SUCCESS;
}



PIM_Result PIM_Paint_DrawRect(PIM_Bitmap *bitmap, PIM_Rect *rect, PIM_Color color)
{
	if (PIM_Bitmap_isValid(bitmap) == PIM_FALSE) return PIM_ERROR_INVALID_ARGUMENT;
	if (color == PIM_Color_KNone) return PIM_SUCCESS;

	switch (bitmap->imageFormat)
    {
		case PIM_IMAGE_FORMAT_YUVPLANAR_NV21:
		case PIM_IMAGE_FORMAT_YUVPLANAR_NV12:
			PIM_Paint_DrawVLine(bitmap, rect->left,  rect->top,    rect->bottom, color);
			PIM_Paint_DrawVLine(bitmap, rect->right, rect->top,    rect->bottom, color);
			PIM_Paint_DrawHLine(bitmap, rect->left,  rect->top,    rect->right,  color);
			PIM_Paint_DrawHLine(bitmap, rect->left,  rect->bottom, rect->right,  color);
            break;

		case PIM_IMAGE_FORMAT_BGR888:
		case PIM_IMAGE_FORMAT_BGR8880:
		case PIM_IMAGE_FORMAT_RGB888:
		case PIM_IMAGE_FORMAT_RGB8880:
            Paint_DrawRect_RGB(bitmap, rect, color);
            break;

		default:
			return PIM_ERROR_NOT_SUPPORTED;
	}
	return PIM_SUCCESS;
}



// Returns number of characters written.
PIM_Result PIM_Paint_DrawString(PIM_Bitmap *bitmap, const PIM_Point *pos, const PIM_String str, PIM_Color chcolor, PIM_Size *fontsize)
{
#if 0
	PIM_Int32 i, j, k;
	PIM_Int32 x, y;		// current pos (x, y)
	PIM_Int32 cpos;		// currnet offset in bytestream
	PIM_Int32 uvpos;
	PIM_Int32 size = PIM_Util_Strlen(str);
	PIM_Uint32 conv_color;
	const PIM_Int32 ch_width = 8;
	const PIM_Int32 ch_height = 16;
	PIM_Uint32 font_row_pixels;
	PIM_Int32 written_ch = 0;
	PIM_Int32 font_w, font_h;		// font width and font height
	
	if (!bitmap || !pos || !str) return PIM_ERROR_NULL_POINTER; 

	if (fontsize == NULL) {
		font_w = font_h = 1;
	} else {
		font_w = fontsize->width, font_h = fontsize->height;
	}

	x = pos->x;
	y = pos->y;

	if (y < 0 || (y + ch_height) >= (PIM_Int32)(bitmap->height)) return 0;

	switch (bitmap->imageFormat) {

		case PIM_ImageFormat_ERgb565L:
			conv_color = PIM_Color_GetRgb565(chcolor);

			for (i=0; i<size; i++, x+=ch_width) {
				if ((x + ch_width) >= (PIM_Int32)(bitmap->width)) break;
				if (x < 0) continue;

				written_ch++;
				cpos = (y * bitmap->width + x) * 2;
				for (j=0; j<ch_height; j++) {
					cpos += (2 * ch_width);
					font_row_pixels = PIM_Font_EngFont[(PIM_Int32)(str[i]) * ch_height + j];
					for (k=0; k<ch_width; k++) {
						cpos -= 2;
						if (font_row_pixels & 1) {
							bitmap->imageData[cpos] = (PIM_Uint8)(conv_color);
							bitmap->imageData[cpos+1] = (PIM_Uint8)(conv_color >> 8);
						}
						font_row_pixels >>= 1;
					}
					cpos += bitmap->width * 2;
				}
			}
			break;
		case PIM_IMAGE_FORMAT_RGB888:

			conv_color = (PIM_Int32)chcolor;

			for (i=0; i<size; i++, x+=ch_width) {
				if ((x + ch_width) >= (PIM_Int32)(bitmap->width)) break;
				if (x < 0) continue;

				written_ch++;
				cpos = (y * bitmap->width + x) * 3;
				for (j=0; j<ch_height; j++) {
					cpos += (3 * ch_width);
					font_row_pixels = PIM_Font_EngFont[(PIM_Int32)(str[i]) * ch_height + j];
					for (k=0; k<ch_width; k++) {
						cpos -= 3;
						if (font_row_pixels & 1) {
							bitmap->imageData[cpos] = (PIM_Uint8)(conv_color);
							bitmap->imageData[cpos+1] = (PIM_Uint8)(conv_color >> 8);
							bitmap->imageData[cpos+2] = (PIM_Uint8)(conv_color >> 16);
						}
						font_row_pixels >>= 1;
					}
					cpos += bitmap->width * 3;
				}
			}
			break;
		case PIM_IMAGE_FORMAT_BGR888:

			conv_color = (PIM_Int32)chcolor;

			for (i=0; i<size; i++, x+=ch_width) {
				if ((x + ch_width) >= (PIM_Int32)(bitmap->width)) break;
				if (x < 0) continue;

				written_ch++;
				cpos = (y * bitmap->width + x) * 3;
				for (j=0; j<ch_height; j++) {
					cpos += (3 * ch_width);
					font_row_pixels = PIM_Font_EngFont[(PIM_Int32)(str[i]) * ch_height + j];
					for (k=0; k<ch_width; k++) {
						cpos -= 3;
						if (font_row_pixels & 1) {
							bitmap->imageData[cpos+2] = (PIM_Uint8)(conv_color);
							bitmap->imageData[cpos+1] = (PIM_Uint8)(conv_color >> 8);
							bitmap->imageData[cpos  ] = (PIM_Uint8)(conv_color >> 16);
						}
						font_row_pixels >>= 1;
					}
					cpos += bitmap->width * 3;
				}
			}
			break;
		case PIM_IMAGE_FORMAT_BGR8880:
		case PIM_IMAGE_FORMAT_RGB8880:
			{
			PIM_Uint32 *origin = bitmap->imageData;
			PIM_Uint32 *pos;
			conv_color = chcolor;

			for (i=0; i<size; i++, x+=ch_width) {
				if ((x + ch_width) >= (PIM_Int32)(bitmap->width)) break;
				if (x < 0) continue;

				written_ch++;
				pos = origin + (y * bitmap->width + x);
				for (j=0; j<ch_height; j++) {
					pos += ch_width;
					font_row_pixels = PIM_Font_EngFont[(PIM_Int32)(str[i]) * ch_height + j];
					for (k=0; k<ch_width; k++) {
						pos--;
						if (font_row_pixels & 1) {
							*pos = conv_color;
						}
						font_row_pixels >>= 1;
					}
					pos += bitmap->width;
				}
			}
			
			PIM_Image_ReleaseImagePointer(bitmap);
			break;
			}
		case PIM_IMAGE_FORMAT_YUVPLANAR_NV21:
			conv_color = PIM_Color_GetYUV(chcolor);

			for (i=0; i<size; i++, x+=ch_width) {
				if ((x + ch_width) >= (PIM_Int32)(bitmap->width)) break;
				if (x < 0) continue;

				written_ch++;
				cpos = (y * bitmap->width + x);				// pos of Y component
				uvpos = (bitmap->width * bitmap->height) + ((y/2) * bitmap->width + (((x+1)>>1)<<1));	// pos of V component
				for (j=0; j<ch_height; j++) {
					cpos += (ch_width);
					font_row_pixels = PIM_Font_EngFont[(PIM_Int32)(str[i]) * ch_height + j];
					for (k=0; k<ch_width; k++) {
						cpos -= 1;
						if (font_row_pixels & 1) {
							bitmap->imageData[cpos] = (PIM_Uint8)(conv_color >> 16);		// Y
						}
						font_row_pixels >>= 1;
					}

					uvpos += (ch_width);
					font_row_pixels = PIM_Font_EngFont[(PIM_Int32)(str[i]) * ch_height + j];
					for (k=0; k<ch_width/2; k++) {
						uvpos -= 2;
						if (font_row_pixels & 1) {
							bitmap->imageData[uvpos] = (PIM_Uint8)(conv_color);			// V
							bitmap->imageData[uvpos+1] = (PIM_Uint8)(conv_color >> 8);	// U
						}
						font_row_pixels >>= 2;
					}

					cpos += (bitmap->width );
					if (j & 0x01) uvpos += (bitmap->width );
				}
			}
			break;

		case PIM_ImageFormat_EYuv422YUYV:
			conv_color = PIM_Color_GetYUV(chcolor);
			
			for (i=0; i<size; i++, x+=ch_width*font_w) {
				if ((x + ch_width*font_w) >= (PIM_Int32)(bitmap->width)) break;
				if (x < 0) continue;

				written_ch++;
				cpos = (y * bitmap->width + x) * 2;		// pos of Y component
				for (j=0; j<ch_height; j++) {

					PIM_Int32 m;

					for (m=0; m<font_h; m++) {
						font_row_pixels = PIM_Font_EngFont[(PIM_Int32)(str[i]) * ch_height + j];	
						cpos += 2 * ch_width * font_w;
						for (k=0; k<ch_width; k++) {
							cpos -= (2 * font_w);
							if (font_row_pixels & 1) {
								PIM_Int32 l;
								for (l=0; l<font_w; l++) {
									bitmap->imageData[cpos + l*2] = (PIM_Uint8)(conv_color >> 16);		// Y
									bitmap->imageData[cpos + l*2 + 1] = (((cpos + l*2 + 1) & 0x03) == 1) ? (PIM_Uint8)(conv_color >> 8) /*U*/ :(PIM_Uint8)(conv_color); /*V*/
								}
							}
							font_row_pixels >>= 1;
						}
						cpos += (bitmap->width * 2);
					}				
				}

			}

		default:
			return PIM_ERROR_NOT_SUPPORTED;
	}
	return written_ch;	
#endif

	return PIM_ERROR_NOT_SUPPORTED;
}

PIM_Result PIM_Paint_DrawVLine(PIM_Bitmap *bitmap, PIM_Int32 x_fix, PIM_Int32 y_start, PIM_Int32 y_end, PIM_Color color)
{
	PIM_Int32 cpos;
	PIM_Int32 uvpos;
	PIM_Uint32 conv_color;
	PIM_Int32 i;
	PIM_Int32 start, end;

	if (PIM_Bitmap_isValid(bitmap) == PIM_FALSE) return PIM_ERROR_INVALID_ARGUMENT;
	if (x_fix < 0 || x_fix >= (PIM_Int32)bitmap->width) return PIM_ERROR_INVALID_ARGUMENT;

	start = PIM_Math_Max(0, y_start);
	end = PIM_Math_Min(y_end, bitmap->height - 1);

	if (start >= (PIM_Int32)bitmap->height || end <= 0) return PIM_ERROR_INVALID_ARGUMENT;

	switch (bitmap->imageFormat)
    {
		case PIM_IMAGE_FORMAT_YUVPLANAR_NV21:
			conv_color = PIM_Color_GetYUV(color);
			cpos = (start * bitmap->stride + x_fix);								// pos of Y component
			uvpos = (bitmap->stride * bitmap->height) + ((start/2) * bitmap->stride + ((x_fix>>1)<<1));	// pos of V component

			for (i=start; i<=end; i++) {
				bitmap->imageData[cpos] = (PIM_Uint8)(conv_color >> 16);		// Y
				cpos += (bitmap->stride);
			}
			for (i=start; i<=end; i+=2) {
				bitmap->imageData[uvpos] = (PIM_Uint8)(conv_color);			// V
				bitmap->imageData[uvpos+1] = (PIM_Uint8)(conv_color >> 8);	// U
				uvpos += (bitmap->stride);		
			}
			break;

		case PIM_IMAGE_FORMAT_BGR888:
		case PIM_IMAGE_FORMAT_BGR8880:
		case PIM_IMAGE_FORMAT_RGB888:
		case PIM_IMAGE_FORMAT_RGB8880:
			Paint_DrawVLine_RGB(bitmap, x_fix, y_start, y_end, color);

		default:
			return PIM_ERROR_NOT_SUPPORTED;
	}
	return PIM_SUCCESS;
}

PIM_Result PIM_Paint_DrawHLine(PIM_Bitmap *bitmap, PIM_Int32 x_start, PIM_Int32 y_fix, PIM_Int32 x_end, PIM_Color color)
{
	PIM_Int32 cpos;
	PIM_Int32 uvpos;
	PIM_Uint32 conv_color;
	PIM_Int32 i;
	PIM_Int32 start, end;

	if (PIM_Bitmap_isValid(bitmap) == PIM_FALSE) return PIM_ERROR_INVALID_ARGUMENT;
	if (y_fix < 0 || y_fix >= (PIM_Int32)bitmap->height) return PIM_ERROR_INVALID_ARGUMENT;

	start = PIM_Math_Max(0, x_start);
	end = PIM_Math_Min(x_end, bitmap->width - 1);

	if (start >= (PIM_Int32)bitmap->width || end < 0) return PIM_ERROR_INVALID_ARGUMENT;

	switch (bitmap->imageFormat)
    {
		case PIM_IMAGE_FORMAT_YUVPLANAR_NV21:
			conv_color = PIM_Color_GetYUV(color);
			cpos = (y_fix * bitmap->stride + start);								// pos of Y component
			uvpos = (bitmap->stride * bitmap->height) + ((y_fix/2) * bitmap->stride + ((start>>1)<<1));	// pos of V component

			for (i=start; i<=end; i++) {
				bitmap->imageData[cpos++] = (PIM_Uint8)(conv_color >> 16);	// Y				
			}
			for (i=start; i<=(end+start)/2; i++) {
				bitmap->imageData[uvpos++] = (PIM_Uint8)(conv_color);			// V
				bitmap->imageData[uvpos++] = (PIM_Uint8)(conv_color >> 8);	// U
			}
			break;

		case PIM_IMAGE_FORMAT_BGR888:
		case PIM_IMAGE_FORMAT_BGR8880:
		case PIM_IMAGE_FORMAT_RGB888:
		case PIM_IMAGE_FORMAT_RGB8880:
			Paint_DrawHLine_RGB(bitmap, x_start, y_fix, x_end, color);

		default:
			return PIM_ERROR_NOT_SUPPORTED;
	}
	return PIM_SUCCESS;
}

PIM_Result PIM_Paint_DrawLine(PIM_Bitmap *bitmap, PIM_Int32 x_start, PIM_Int32 y_start, PIM_Int32 x_end, PIM_Int32 y_end, PIM_Color color)
{
	PIM_Int32 left, left_y, right, right_y;
	PIM_Int32 length_x, length_y;
	PIM_Int32 i,j = 0;
	PIM_Int32 step_x;
	PIM_Int32 unit_x;

	if (PIM_Bitmap_isValid(bitmap) == PIM_FALSE) return PIM_ERROR_INVALID_ARGUMENT;

	if( x_start < x_end ) { 
		left = x_start;
		left_y = y_start;
		right = x_end;
		right_y = y_end;
	} else {
		left = x_end;
		left_y = y_end;
		right = x_start;
		right_y = y_start;
	}

	length_x = right - left;
	length_y = right_y - left_y;

	if( length_y != 0 ) 
		unit_x = (length_x<<10) /length_y;
	else {
		unit_x = (length_x<<10) ;
		length_y = 1;
	}

	if( length_y > 0 ) {
		step_x = 1;
	}else {
		step_x = -1;
		left += PIM_Math_Abs(length_x) ;
	}

	switch(bitmap->imageFormat) 
	{
	case PIM_IMAGE_FORMAT_BGR8880:
	case PIM_IMAGE_FORMAT_RGB8880:
		{
			PIM_Uint8 *buf = bitmap->imageData;
			PIM_Uint8 *dst ;
			PIM_Uint8 r,g,b,a;
			PIM_Int32 min_y = PIM_Math_Min(y_start, y_end);
			PIM_Int32 u_x = unit_x>>10 == 0 ? 1 : PIM_Math_Abs(unit_x>>10);//PIM_Math_Max(1, unit_x>>10);
			a = (color>>24) & 0xFF;
			r = (color>>16) & 0xFF;
			g = (color>>8) & 0xFF;
			b = (color) & 0xFF;
			for(i= 0 ; i!= PIM_Math_Abs(length_y); i++) {
				dst = buf + (bitmap->stride*(i+min_y)) + (left+((unit_x*i)>>10))*4;
				for(j=0;j!=u_x+1 ;j++) {
					dst[0] = b;
					dst[1] = g;
					dst[2] = r;
					dst[3] = a;

					dst += 4*step_x;
				}
			}
		}
		break;
	case PIM_IMAGE_FORMAT_BGR888:
	case PIM_IMAGE_FORMAT_RGB888:
		{
			PIM_Uint8 *buf = bitmap->imageData;
			PIM_Uint8 *dst ;
			PIM_Uint8 r,g,b,a;
			PIM_Int32 min_y = PIM_Math_Min(y_start, y_end);
			PIM_Int32 u_x = unit_x>>10 == 0 ? 1 : PIM_Math_Abs(unit_x>>10);//PIM_Math_Max(1, unit_x>>10);
			a = (color>>24) & 0xFF;
			r = (color>>16) & 0xFF;
			g = (color>>8) & 0xFF;
			b = (color) & 0xFF;
			for(i= 0 ; i!= PIM_Math_Abs(length_y); i++) {
				dst = buf + (bitmap->stride*(i+min_y))+ (left+((unit_x*i)>>10))*3;
				for(j=0;j!=u_x+1 ;j++) {
					dst[0] = b;
					dst[1] = g;
					dst[2] = r;
					dst += 3*step_x;
				}
			}
		}
		break;
	default:
		return PIM_ERROR_NOT_SUPPORTED;
	}

	return PIM_SUCCESS;
}

PIM_Result PIM_Paint_FillRect(PIM_Bitmap *bitmap, PIM_Rect *rect, PIM_Color color, PIM_Int32 transparency)
{
	PIM_Int32 cpos;
	PIM_Int32 uvpos;
	PIM_Uint32 conv_color;
	PIM_Int32 i, j;
	PIM_Int32 start_x, end_x;
	PIM_Int32 start_y, end_y;
	PIM_Int32 r_trans = 256- transparency;

	if (!bitmap || !bitmap->imageData) return PIM_ERROR_NULL_POINTER;

	start_x = PIM_Math_Max(0, rect->left);
	end_x = PIM_Math_Min(bitmap->width - 1, rect->right);
	start_y = PIM_Math_Max(0, rect->top);
	end_y = PIM_Math_Min(bitmap->height - 1, rect->bottom);

	switch (bitmap->imageFormat) {
		case PIM_IMAGE_FORMAT_RGB888:
			{
			PIM_Uint8 r = (color>>16)&0xFF , g = (color>>8) & 0xFF, b= (color&0xFF);
			
			for (i=start_y; i<=end_y; i++) {
				cpos = i * bitmap->stride + start_x * 3;						
				for (j=start_x; j<=end_x; j++) {
					bitmap->imageData[cpos] = (r * r_trans + bitmap->imageData[cpos] * transparency)>>8;
					cpos++;
					bitmap->imageData[cpos] = (g * r_trans + bitmap->imageData[cpos] * transparency)>>8;
					cpos++;
					bitmap->imageData[cpos] = (b * r_trans + bitmap->imageData[cpos] * transparency)>>8;					
					cpos++;
				}
			}
			break;
			}
		case PIM_IMAGE_FORMAT_BGR888:
			{
			PIM_Uint8 r = (color>>16)&0xFF , g = (color>>8) & 0xFF, b= (color&0xFF);
			
			for (i=start_y; i<=end_y; i++) {
				cpos = i * bitmap->stride + start_x * 3;						
				for (j=start_x; j<=end_x; j++) {
					bitmap->imageData[cpos] = (b * r_trans + bitmap->imageData[cpos] * transparency)>>8;
					cpos++;
					bitmap->imageData[cpos] = (g * r_trans + bitmap->imageData[cpos] * transparency)>>8;
					cpos++;
					bitmap->imageData[cpos] = (r * r_trans + bitmap->imageData[cpos] * transparency)>>8;					
					cpos++;
				}
			}
			break;
			}
		case PIM_IMAGE_FORMAT_BGR8880:
		case PIM_IMAGE_FORMAT_RGB8880:
			{
			PIM_Uint8 r = (color>>16)&0xFF , g = (color>>8) & 0xFF, b= (color&0xFF);
			
			for (i=start_y; i<=end_y; i++) {
				cpos = i * bitmap->stride + start_x * 4;						
				for (j=start_x; j<=end_x; j++) {
					bitmap->imageData[cpos] = (b * r_trans + bitmap->imageData[cpos] * transparency)>>8;
					cpos++;
					bitmap->imageData[cpos] = (g * r_trans + bitmap->imageData[cpos] * transparency)>>8;
					cpos++;
					bitmap->imageData[cpos] = (r * r_trans + bitmap->imageData[cpos] * transparency)>>8;					
					cpos++;
					cpos++;
				}
			}
			break;
			}
		case PIM_IMAGE_FORMAT_YUVPLANAR_NV12:
		case PIM_IMAGE_FORMAT_YUVPLANAR_NV21:
			conv_color = PIM_Color_GetYUV(color);

			for (i=start_y; i<=end_y; i++) {
				cpos = (i * bitmap->stride + start_x);					// pos of Y component
				for (j=start_x; j<=end_x; j++) {
					bitmap->imageData[cpos] = (PIM_Uint8)(( (conv_color >> 16)*r_trans + bitmap->imageData[cpos]*transparency)>> 8);	// Y
					cpos++;
				}
			}

			{
				start_x &= 0xFFFE;		// to point V component
				for (i=(start_y>>1); i<=(end_y)>>1; i++) {
					uvpos = (bitmap->stride * bitmap->height) + (i * bitmap->stride + start_x);	// pos of V component
					for (j=start_x; j<=end_x; j+=2) {
						bitmap->imageData[uvpos] = (PIM_Uint8)(( (conv_color     )*r_trans + bitmap->imageData[uvpos]*transparency)>> 8);	// V
						uvpos++;
						bitmap->imageData[uvpos] = (PIM_Uint8)(( (conv_color >> 8)*r_trans + bitmap->imageData[uvpos]*transparency)>> 8);	// U
						uvpos++;
					}
				}


			}
			
			break;

		default:
			return PIM_ERROR_NOT_SUPPORTED;
	}
	return PIM_SUCCESS;
}


PIM_Result PIM_Paint_DrawBitmap(PIM_Bitmap *dst, const PIM_Bitmap *src, PIM_Point *pos, PIM_Size *size)
{
	return PIM_ERROR_NOT_SUPPORTED;
}


typedef struct __OverlayAttr {
    PIM_Bitmap *dst_bitmap;
    PIM_Bitmap *src_bitmap;
	PIM_Int32  work_width, work_height;
	PIM_Point  dst_pos;
	PIM_Point  src_pos;
    PIM_Int32  transparency;
    PIM_Color  transparent_color;
} __OverlayAttr;

static PIM_Result Paint_Overlay_RGB                (__OverlayAttr *attr);
static PIM_Result Paint_Overlay_RGBA               (__OverlayAttr *attr);
static PIM_Result Paint_Overlay_RGBIndexToRGB      (__OverlayAttr *attr);
static PIM_Result Paint_Overlay_RGBIndexToYUVPLANAR(__OverlayAttr *attr);

static PIM_Result Paint_Overlay_RGB(__OverlayAttr *attr)
{
	PIM_Result ret = PIM_SUCCESS;
    PIM_Int32  x, y;

    PIM_Bitmap *layee = attr->dst_bitmap;
    PIM_Bitmap *layer = attr->src_bitmap;
    PIM_Int32  transparency = attr->transparency;
    PIM_Int32  work_width  = attr->work_width;
    PIM_Int32  work_height = attr->work_height;

    PIM_Uint8  r = (attr->transparent_color>>16)& 0xFF;
    PIM_Uint8  g = (attr->transparent_color>>8) & 0xFF;
    PIM_Uint8  b = (attr->transparent_color)    & 0xFF;

    PIM_Int32  dst_channel = PIM_Bitmap_GetChannel(attr->dst_bitmap);
    PIM_Int32  src_channel = PIM_Bitmap_GetChannel(attr->src_bitmap);
    PIM_Int32  dst_step = layee->stride - work_width * dst_channel;
    PIM_Int32  src_step = layer->stride - work_width * src_channel;

    PIM_Int32  sh_d = 256-transparency;

    PIM_Uint8 *pDstData = layee->imageData + layee->stride*attr->dst_pos.y + attr->dst_pos.x * dst_channel;
    PIM_Uint8 *pSrcData = layer->imageData + layer->stride*attr->src_pos.y + attr->src_pos.x * src_channel;

    for(y=0;y<work_height; y++) 
    {
        for(x=0; x<work_width; x++) 
        {
            if( !(pSrcData[0] == r && pSrcData[1] == g && pSrcData[2] == b) ) 
            {
                pDstData[0] = (PIM_Uint8)((pDstData[0]*sh_d + pSrcData[0]*transparency)>> 8);
                pDstData[1] = (PIM_Uint8)((pDstData[1]*sh_d + pSrcData[1]*transparency)>> 8);
                pDstData[2] = (PIM_Uint8)((pDstData[2]*sh_d + pSrcData[2]*transparency)>> 8);
            }
            pDstData += dst_channel;
            pSrcData += src_channel;

        }
        pDstData += dst_step;
        pSrcData += src_step;
    }

    return ret;
}

static PIM_Result Paint_Overlay_RGBA(__OverlayAttr *attr)
{
	PIM_Result ret = PIM_SUCCESS;
    PIM_Int32  x, y;

    PIM_Bitmap *layee = attr->dst_bitmap;
    PIM_Bitmap *layer = attr->src_bitmap;
    PIM_Int32  transparency = attr->transparency;
    PIM_Int32  work_width  = attr->work_width;
    PIM_Int32  work_height = attr->work_height;

    PIM_Int32  dst_channel = PIM_Bitmap_GetChannel(attr->dst_bitmap);
    PIM_Int32  src_channel = PIM_Bitmap_GetChannel(attr->src_bitmap);
    PIM_Int32  dst_step = layee->stride - work_width * dst_channel;
    PIM_Int32  src_step = layer->stride - work_width * src_channel;

    PIM_Int32  sh_d = 256-transparency;

    PIM_Uint8 *pDstData = layee->imageData + layee->stride*attr->dst_pos.y + attr->dst_pos.x * dst_channel;
    PIM_Uint8 *pSrcData = layer->imageData + layer->stride*attr->src_pos.y + attr->src_pos.x * src_channel;

    for(y=0;y<work_height; y++) 
    {
        for(x=0; x<work_width; x++) 
        {
            //if( !(pSrcData[0] == r && pSrcData[1] == g && pSrcData[2] == b) ) 
            //{
            sh_d = 256 - pSrcData[3];
                pDstData[0] = PIM_Math_ClipToByte((pDstData[0]*sh_d + pSrcData[0]*256)>> 8 );
                pDstData[1] = PIM_Math_ClipToByte((pDstData[1]*sh_d + pSrcData[1]*256)>> 8 );
                pDstData[2] = PIM_Math_ClipToByte((pDstData[2]*sh_d + pSrcData[2]*256)>> 8 );
                //pDstData[3] = PIM_Math_ClipToByte((pDstData[3]*sh_d + pSrcData[3]*transparency)>> 8 );
            //}

            pDstData += dst_channel;
            pSrcData += src_channel;

        }
        pDstData += dst_step;
        pSrcData += src_step;
    }

    return ret;
}

static PIM_Result Paint_Overlay_RGBIndexToRGB(__OverlayAttr *attr)
{
	PIM_Result ret = PIM_SUCCESS;
    PIM_Int32  x, y;
    PIM_Uint8 *pDstR, *pDstG, *pDstB;
    PIM_Uint8 srcR, srcG, srcB;

    PIM_Bitmap *layee = attr->dst_bitmap;
    PIM_Bitmap *layer = attr->src_bitmap;
    PIM_Int32  transparency = attr->transparency;
    PIM_Int32  work_width  = attr->work_width;
    PIM_Int32  work_height = attr->work_height;

    PIM_Uint8  r = (attr->transparent_color>>16)& 0xFF;
    PIM_Uint8  g = (attr->transparent_color>>8) & 0xFF;
    PIM_Uint8  b = (attr->transparent_color)    & 0xFF;

    PIM_Int32  dst_channel = PIM_Bitmap_GetChannel(attr->dst_bitmap);
    PIM_Int32  src_channel = PIM_Bitmap_GetChannel(attr->src_bitmap);
    PIM_Int32  dst_step = layee->stride - work_width * dst_channel;
    PIM_Int32  src_step = layer->stride - work_width * src_channel;

    PIM_Int32  sh_d = 256-transparency;

    PIM_Uint8 *pDstData = layee->imageData + layee->stride*attr->dst_pos.y + attr->dst_pos.x * dst_channel;
    PIM_Uint8 *pSrcData = layer->imageData + layer->stride*attr->src_pos.y + attr->src_pos.x * src_channel;
    PIM_Uint8 *palete = layer->auxImageData;

    /*  */
    switch(layee->imageFormat)
    {
        case PIM_IMAGE_FORMAT_RGB888:
        case PIM_IMAGE_FORMAT_RGB8880:
            pDstR = &pDstData[0];
            pDstG = &pDstData[1];
            pDstB = &pDstData[2];
            break;

        case PIM_IMAGE_FORMAT_BGR888:
        case PIM_IMAGE_FORMAT_BGR8880:
            pDstR = &pDstData[2];
            pDstG = &pDstData[1];
            pDstB = &pDstData[0];
            break;

        default:
            return PIM_ERROR_NOT_SUPPORTED;
    }

    for(y=0;y<work_height; y++) 
    {
        for(x=0; x<work_width; x++) 
        {
            if(pSrcData[0] >= 1)
            {
                srcR = palete[pSrcData[0] * 4 + 1];
                srcG = palete[pSrcData[0] * 4 + 2];
                srcB = palete[pSrcData[0] * 4 + 3];

                // regard totally green color as transparent
                if( !(srcR == r && srcG == g && srcB == b) ) 
                {
                    *pDstR = (PIM_Uint8)((pDstR[0]*sh_d + srcR*transparency)>> 8 );
                    *pDstG = (PIM_Uint8)((pDstG[1]*sh_d + srcG*transparency)>> 8 );
                    *pDstB = (PIM_Uint8)((pDstB[2]*sh_d + srcB*transparency)>> 8 );
                }
            }

            pDstR += dst_channel;
            pDstG += dst_channel;
            pDstB += dst_channel;
            pSrcData   += src_channel;

        }
        pDstR += dst_step;
        pDstG += dst_step;
        pDstB += dst_step;
        pSrcData   += src_step;
    }

    return ret;
}

static PIM_Result Paint_Overlay_RGBIndexToYUVPLANAR(__OverlayAttr *attr)
{
	PIM_Result ret = PIM_SUCCESS;
    PIM_Int32  x, y;
    PIM_Uint8 srcR, srcG, srcB;

    PIM_Bitmap *layee = attr->dst_bitmap;
    PIM_Bitmap *layer = attr->src_bitmap;
    PIM_Int32  work_width  = attr->work_width;
    PIM_Int32  work_height = attr->work_height;

    PIM_Uint8  r = (attr->transparent_color>>16)& 0xFF;
    PIM_Uint8  g = (attr->transparent_color>>8) & 0xFF;
    PIM_Uint8  b = (attr->transparent_color)    & 0xFF;
    PIM_Uint32 color;

    PIM_Int32  dst_y_step  = layee->stride - work_width;
    PIM_Int32  dst_uv_step = layee->stride - work_width;
    PIM_Int32  src_step    = layer->stride - work_width;

    PIM_Uint8 *pDstY    = layee->imageData + layee->stride*attr->dst_pos.y + attr->dst_pos.x;
    PIM_Uint8 *pDstUV   = layee->imageData + layee->width * layee->height;// + layee->stride*(attr->dst_pos.y/2) + (attr->dst_pos.x&(~1));
    PIM_Uint8 *pSrcData = layer->imageData + layer->stride*attr->src_pos.y + attr->src_pos.x;
    PIM_Uint8 *palete = layer->auxImageData;

    /*  */
    for (y=0; y<work_height; y++)
    {
        if( y & 0x01 ) {
            /* Y + UV */
            for (x=0; x<work_width; x++)
            {
                if(pSrcData[0] >= 1)
                {
                    srcR = palete[pSrcData[0] * 4 + 1];
                    srcG = palete[pSrcData[0] * 4 + 2];
                    srcB = palete[pSrcData[0] * 4 + 3];

                    if( !(srcR == r && srcG == g && srcB == b) ) 
                    {
                        color = PIM_Color_GetY(srcR<<16 | srcG<<8 | srcB);
                        *pDstY = (PIM_Uint8)(color & 0xFF);

                        if(x&1) {
                            color = PIM_Color_GetUV(srcR<<16 | srcG<<8 | srcB);
                            pDstUV[0] = (PIM_Uint8)((color>>8)&0xFF);
                            pDstUV[1] = (PIM_Uint8)((color)&0xFF);
                        }
                    }
                }

                pDstUV++;
                pDstY++;
                pSrcData++;
            }

            pDstUV += dst_uv_step;
        } else {
            /* Y */
            for (x=0; x<work_width; x++)
            {
                if(pSrcData[0] >= 1)
                {
                    srcR = palete[pSrcData[0] * 4 + 1];
                    srcG = palete[pSrcData[0] * 4 + 2];
                    srcB = palete[pSrcData[0] * 4 + 3];

                    if( !(srcR == r && srcG == g && srcB == b) ) 
                    {
                        color = PIM_Color_GetY( srcR<<16 | srcG<<8 | srcB);
                        *pDstY = (PIM_Uint8)(color & 0xFF);
                    }
                }
                pDstY++;
                pSrcData++;
            }
        }

        pDstY += dst_y_step;
        pSrcData += src_step;
    }

    return ret;
}


PIM_Result PIM_Paint_Overlay(PIM_Bitmap* layee, PIM_Bitmap* layer, PIM_Int32 x, PIM_Int32 y, PIM_Int32 transparency, PIM_Color transparent_color)
{
	PIM_Result ret = PIM_SUCCESS;
    __OverlayAttr overlayAttr = {0};
	PIM_Int32 work_width, work_height;
	PIM_Int32 layer_start_x, layer_start_y;
	if( !layee || !layer ) return PIM_ERROR_NULL_POINTER;
	if( transparency < 0 || transparency > 256 ) return PIM_ERROR_INVALID_ARGUMENT;
	if(layer->imageFormat != layee->imageFormat && layer->imageFormat != PIM_IMAGE_FORMAT_RGB_INDEX) return PIM_ERROR_NOT_SUPPORTED;


	/* Check boundary */
	if( x < 0 ) {
		work_width = x + layer->width;
		layer_start_x = -x;
		x = 0;
	} else if( x+layer->width > layee->width ) {
		work_width = layee->width - x;
		layer_start_x = 0;
	} else {
		work_width = layer->width;
		layer_start_x = 0;
	}

	if( y < 0 ) {
		work_height = y + layer->height;
		layer_start_y = -y;
		y = 0;
	} else if( y+layer->height > layee->height ) {
		work_height = layee->height - y;
		layer_start_y = 0;
	} else {
		work_height = layer->height;
		layer_start_y = 0;
	}

    /*  */
    overlayAttr.dst_bitmap  = layee;
    overlayAttr.src_bitmap  = layer;
    overlayAttr.work_width  = work_width;
    overlayAttr.work_height = work_height;
    overlayAttr.dst_pos.x   = x;
    overlayAttr.dst_pos.y   = y;
    overlayAttr.src_pos.x   = layer_start_x;
    overlayAttr.src_pos.y   = layer_start_y;
    overlayAttr.transparency = transparency;
    overlayAttr.transparent_color = transparent_color;
	

	switch(layer->imageFormat)
    {
		case PIM_IMAGE_FORMAT_RGB888:
		case PIM_IMAGE_FORMAT_RGB8880:
		case PIM_IMAGE_FORMAT_BGR888:
		case PIM_IMAGE_FORMAT_BGR8880:
        {
            ret = Paint_Overlay_RGB(&overlayAttr);
        } break;

		case PIM_IMAGE_FORMAT_RGBA8888:
        {
            ret = Paint_Overlay_RGBA(&overlayAttr);
        } break;

		case PIM_IMAGE_FORMAT_RGB_INDEX:
        {
            if(layee->imageFormat & PIM_IMAGE_FORMAT_RGB_LABEL) {
                ret = Paint_Overlay_RGBIndexToRGB(&overlayAttr);
            } else if(layee->imageFormat & PIM_IMAGE_FORMAT_YUVPLANAR_LABEL) {
                ret = Paint_Overlay_RGBIndexToYUVPLANAR(&overlayAttr);
            } else {
                ret = PIM_ERROR_NOT_SUPPORTED;
            }
        } break;

		default:
			ret = PIM_ERROR_NOT_SUPPORTED;
		}



	return ret;
}



PIM_Result PIM_Paint_MaskingOverlay(PIM_Bitmap* dst, PIM_Bitmap* src, int x, int y, PIM_Bool isCircle)
{
	PIM_Result ret = PIM_SUCCESS;
	int i,j;
	int lw = src->width;
	int lh = src->height;
	int channel = PIM_Bitmap_GetChannel(dst);
	PIM_Uint8* mask = NULL;
	PIM_Bitmap circle_resized = {0};

	if( dst->imageFormat != src->imageFormat ) {
		return PIM_ERROR_INVALID_ARGUMENT;
	}

	if(isCircle == PIM_TRUE) {
#if 0
        PIM_Bitmap circle_res = {0};
        PIM_Bitmap_CreateRef(&circle_res, _CIRCLEMASK, CIRCLEMASK_W, CIRCLEMASK_H, CIRCLEMASK_W, PIM_IMAGE_FORMAT_GRAY);
		PIM_Bitmap_Create(&circle_resized, lw, lh, PIM_IMAGE_FORMAT_GRAY);
		//Resize_BI(_CIRCLEMASK, CIRCLEMASK_W, CIRCLEMASK_H, circle_resized.imageData, circle_resized.width, circle_resized.height);
		PIM_Bitmap_Resize_BI(&circle_resized, &circle_res);
#else
        PIM_Mask_CreateSolidCircle(&circle_resized, lw, lw);
#endif
		mask = circle_resized.imageData;
	}

	switch( dst->imageFormat ) {
		case PIM_IMAGE_FORMAT_RGB888:
		case PIM_IMAGE_FORMAT_RGB8880:
		case PIM_IMAGE_FORMAT_BGR888:
		case PIM_IMAGE_FORMAT_BGR8880:
			{
				unsigned char *dst_buf = dst->imageData;
				unsigned char *src_buf = src->imageData;

				dst_buf = dst_buf + (y* dst->stride) + x*channel;
				if(mask) {
					for(j=0; j<lh; j++) {
						for(i=0;i<lw; i++) {
							dst_buf[0] = (*mask*src_buf[0] + (256-*mask)*dst_buf[0])>>8 ;
							dst_buf[1] = (*mask*src_buf[1] + (256-*mask)*dst_buf[1])>>8 ;
							dst_buf[2] = (*mask*src_buf[2] + (256-*mask)*dst_buf[2])>>8 ;
							dst_buf += channel;
							src_buf += channel;
							mask++;
						}
						dst_buf += (dst->stride - lw*channel);
                        mask += (circle_resized.stride - lw);
					}
				} else {
					for(j=0; j<lh; j++) {
						for(i=0;i<lw; i++) {
							dst_buf[0] = src_buf[0];
							dst_buf[1] = src_buf[1];
							dst_buf[2] = src_buf[2];
							dst_buf += channel;
							src_buf += channel;
						}
						dst_buf += (dst->stride - lw*channel);
					}
				}
			}
			break;

		default:
			ret = PIM_ERROR_NOT_SUPPORTED;
			break;
	}

	PIM_Bitmap_Destroy(&circle_resized);

	return ret;
}



/*******************************************************************************************
 * Internal Method
 ******************************************************************************************/
PIM_Result Paint_DrawPoint_RGB(PIM_Bitmap *bitmap, PIM_Int32 x, PIM_Int32 y, PIM_Color color)
{
	PIM_Result ret=PIM_SUCCESS;

	if (!bitmap || !bitmap->imageData) return PIM_ERROR_NULL_POINTER;
	if (x < 0 || y < 0 || x >= (PIM_Int32)bitmap->width || y >= (PIM_Int32)bitmap->height) return PIM_ERROR_INVALID_ARGUMENT;

	ret = (PIM_Int32)Set_RGB(bitmap, x, y, color);
	if(ret < PIM_SUCCESS) return ret;

	return PIM_SUCCESS;
}



PIM_Result Paint_DrawRect_RGB(PIM_Bitmap *bitmap, PIM_Rect *rect, PIM_Color color)
{
	if (!bitmap || !bitmap->imageData || !rect) return PIM_ERROR_NULL_POINTER;
	if (color == PIM_Color_KNone) return PIM_SUCCESS;

	Paint_DrawHLine_RGB(bitmap, rect->left,  rect->right, rect->top,    color);
	Paint_DrawHLine_RGB(bitmap, rect->left,  rect->right, rect->bottom, color);
	Paint_DrawVLine_RGB(bitmap, rect->left,  rect->top,   rect->bottom, color);
	Paint_DrawVLine_RGB(bitmap, rect->right, rect->top,   rect->bottom, color);

	return PIM_SUCCESS;
}



/* y_start <= y_end */
PIM_Result Paint_DrawVLine_RGB(PIM_Bitmap *bitmap, PIM_Int32 x_fix, PIM_Int32 y_start, PIM_Int32 y_end, PIM_Color color)
{
	PIM_Int32 y;
	PIM_Int32 width  = bitmap->width;
	PIM_Int32 height = bitmap->height;
	PIM_ImageFormat format = bitmap->imageFormat;

	/*  */
	if(y_start > y_end) return PIM_ERROR_INVALID_ARGUMENT;
	if(x_fix < 0 || x_fix >= width) return PIM_ERROR_INVALID_ARGUMENT;

	switch(format)
	{
	case PIM_IMAGE_FORMAT_BGR888:
	case PIM_IMAGE_FORMAT_BGR8880:
	case PIM_IMAGE_FORMAT_RGB888:
	case PIM_IMAGE_FORMAT_RGB8880:
		break;

	default:
		return PIM_ERROR_NOT_SUPPORTED;
	}

	/*  */
	y_start = (y_start<0)?0:y_start;
	y_end   = (y_end>=height)?height-1:y_end;

	for(y=y_start; y<y_end; y++) {
		Set_RGB(bitmap, x_fix, y, color);
	}

	return PIM_SUCCESS;
}



PIM_Result Paint_DrawHLine_RGB(PIM_Bitmap *bitmap, PIM_Int32 x_start, PIM_Int32 x_end, PIM_Int32 y_fix, PIM_Color color)
{
	PIM_Int32 width  = bitmap->width;
	PIM_Int32 height = bitmap->height;
	PIM_Int32 x;
	PIM_ImageFormat format = bitmap->imageFormat;

	/*  */
	if(x_start > x_end) return PIM_ERROR_INVALID_ARGUMENT;
	if(y_fix < 0 || y_fix >= height) return PIM_ERROR_INVALID_ARGUMENT;

	switch(format)
	{
	case PIM_IMAGE_FORMAT_BGR888:
	case PIM_IMAGE_FORMAT_BGR8880:
	case PIM_IMAGE_FORMAT_RGB888:
	case PIM_IMAGE_FORMAT_RGB8880:
		break;

	default:
		return PIM_ERROR_NOT_SUPPORTED;
	}

	/*  */
	x_start = (x_start<0)?0:x_start;
	x_end   = (x_end>=width)?width-1:x_end;

	for(x=x_start; x<x_end; x++) {
		Set_RGB(bitmap, x, y_fix, color);
	}

	return PIM_SUCCESS;
}



#ifdef __cplusplus
}
#endif
