#include "stdafx.h"

#include "libFaceRecognizer_V/inc/FaceAligner.h"

#include <memory.h>

#include "libPlatform/inc/PIMBitmap.h"
#include "libFaceRecognizer_V/Eigen/Core"
#include "libFaceRecognizer_V/Eigen/Eigen"
#include "libPlatform/inc/ImageProcess.h"


using namespace Eigen;


CFaceAligner::CFaceAligner()
{
	le_m.x = 52.0286 + 20;		le_m.y = 51.5086 + 20;
	re_m.x = 98.7894 + 20;		re_m.y = 50.7210 + 20;
	nose_m.x = 75.5818 + 20;	nose_m.y = 78.8384 + 20;
	lm_m.x = 56.0068 + 20;		lm_m.y = 102.7696 + 20;
	rm_m.x = 96.4634 + 20;		rm_m.y = 102.2152 + 20;
}

CFaceAligner::~CFaceAligner()
{
}


PIM_Int32 CFaceAligner::FaceAligner_AlignFace_ASURF(PIM_Bitmap *pBitmap, PIM_Point ptsLeftEye, PIM_Point ptsRightEye, PIM_Bitmap *pAlignFace)
{
	PIM_Point el;
	PIM_Point er;
	PIM_Bool bAlignedRet;

	el.x = ptsLeftEye.x;
	el.y = ptsLeftEye.y;
	er.x = ptsRightEye.x;
	er.y = ptsRightEye.y;


	PIM_Bitmap_CreateRef(&pAlignFace[1], NULL, 140, 168, 140, PIM_IMAGE_FORMAT_GRAY);
	bAlignedRet = affine_140_168_FIXED(pBitmap->imageData, pBitmap->width, pBitmap->height, el, er, pAlignFace[1].imageData);

	if( bAlignedRet != PIM_TRUE )
	{
		return PIM_ERROR_GENERAL;
	}

	return PIM_SUCCESS;
}

PIM_Int32 CFaceAligner::FaceAligner_AlignFace_ASURF_Gender(PIM_Bitmap *pBitmap, PIM_Point ptsLeftEye, PIM_Point ptsRightEye, PIM_Bitmap *pAlignFace)
{
	PIM_Point el;
	PIM_Point er;
	PIM_Bool bAlignedRet;

	el.x = ptsLeftEye.x;
	el.y = ptsLeftEye.y;
	er.x = ptsRightEye.x;
	er.y = ptsRightEye.y;

	PIM_Bitmap_CreateRef(&pAlignFace[0], NULL, 44, 54, 44, PIM_IMAGE_FORMAT_GRAY);
	bAlignedRet = affine_52_44_FIXED(pBitmap->imageData, pBitmap->width, pBitmap->height, el, er, pAlignFace[0].imageData);

	if( bAlignedRet != PIM_TRUE )
	{
		return PIM_ERROR_GENERAL;
	}

	PIM_Bitmap_CreateRef(&pAlignFace[1], NULL, 140, 168, 140, PIM_IMAGE_FORMAT_GRAY);
	bAlignedRet = affine_140_168_FIXED(pBitmap->imageData, pBitmap->width, pBitmap->height, el, er, pAlignFace[1].imageData);
	if( bAlignedRet != PIM_TRUE )
	{
		return PIM_ERROR_GENERAL;
	}

	return PIM_SUCCESS;
}

PIM_Int32 CFaceAligner::FaceAligner_AlignFace_TL(PIM_Bitmap *pBitmap, PIM_DPoint *ptsFittedShp, PIM_Bitmap *pAlignFace, PIM_DPoint *ptsAlignedShp)
{
	PIM_Bool bAlignedRet = PIM_FALSE;
	
	PIM_Bitmap_CreateRef(pAlignFace, NULL, 190, 190, 190, PIM_IMAGE_FORMAT_GRAY);
	bAlignedRet = alignment_TL_Bayesian(pBitmap->imageData, pBitmap->width, pBitmap->height, ptsFittedShp, pAlignFace->imageData, ptsAlignedShp);

	if( bAlignedRet != PIM_TRUE )
	{
		PIM_Bitmap_Destroy(pAlignFace);
		return PIM_ERROR_GENERAL;
	}

	return PIM_SUCCESS;
}

//PIM_Int32 CFaceAligner::FaceAligner_AlignFace_Deep(PIM_Bitmap *pBitmap, PIM_DPoint *ptsFittedShp, PIM_Bitmap *pAlignFace_95, PIM_Bitmap *pAlignFace_67, PIM_Bitmap *pAlignFace_47, PIM_DPoint *ptsAlignedShp)
PIM_Int32 CFaceAligner::FaceAligner_AlignFace_Deep(PIM_Bitmap *pBitmap, PIM_DPoint *ptsFittedShp, PIM_Bitmap *pAlignFace_95, PIM_Bitmap *pAlignFace_67, PIM_Bitmap *pAlignFace_47)
{
	PIM_Bool bAlignedRet = PIM_FALSE;
	PIM_Bitmap AlignBase = { 0 };

	PIM_Bitmap_CreateRef(&AlignBase, NULL, 190, 190, 190 * 3, PIM_IMAGE_FORMAT_BGR888);
	//bAlignedRet = alignment_RGB(pBitmap->imageData, pBitmap->width, pBitmap->height, ptsFittedShp, AlignBase.imageData, ptsAlignedShp);
	bAlignedRet = alignment_RGB(pBitmap->imageData, pBitmap->width, pBitmap->height, ptsFittedShp, AlignBase.imageData);

	if( bAlignedRet != PIM_TRUE )
	{
		PIM_Bitmap_Destroy(&AlignBase);
		return PIM_ERROR_GENERAL;
	}

	PIM_Rect rcRegion;
	rcRegion.left = 49;
	rcRegion.top = 44;
	rcRegion.right = rcRegion.left + 96;
	rcRegion.bottom = rcRegion.top + 96;

#if 1
	PIM_Bitmap temp = { 0 };
	PIM_Bitmap_CreateRef(&temp, NULL, 96, 96, 288, PIM_IMAGE_FORMAT_BGR888);
	PIM_Bitmap_CropResize(&temp, &AlignBase, &rcRegion);
	PIM_Bitmap_Resize_BI(pAlignFace_95, &temp);
	PIM_Bitmap_Resize_BI(pAlignFace_67, pAlignFace_95);
	PIM_Bitmap_Resize_BI(pAlignFace_47, pAlignFace_95);
	
	PIM_Bitmap_Destroy(&temp);

#else
	PIM_Bitmap temp = { 0 };
	PIM_Bitmap_CreateRef(&temp, NULL, 96, 96, 288, PIM_IMAGE_FORMAT_BGR888);
	Region_RGB(AlignBase.imageData, AlignBase.width, AlignBase.height, temp.imageData, rcRegion);
	Resize_BI_BGR(temp.imageData, temp.width, temp.height, pAlignFace_95->imageData, 95, 95);
	Resize_BI_BGR(pAlignFace_95->imageData, pAlignFace_95->width, pAlignFace_95->height, pAlignFace_67->imageData, 67, 67);
	Resize_BI_BGR(pAlignFace_95->imageData, pAlignFace_95->width, pAlignFace_95->height, pAlignFace_47->imageData, 47, 47);

	PIM_Bitmap_Destroy(&temp);
#endif

	PIM_Bitmap_Destroy(&AlignBase);
	
	return PIM_SUCCESS;
}

PIM_Int32 CFaceAligner::FaceAligner_AlignFace_TL_Gender(PIM_Bitmap *pBitmap, PIM_DPoint *ptsFittedShp, PIM_Bitmap *pAlignFace, PIM_DPoint *ptsAlignedShp)
{
	PIM_Bool bAlignedRet = PIM_FALSE;

	PIM_Point el, er;
#if 0
	el.x = ptsFittedShp[68].x;
	el.y = ptsFittedShp[68].y;
	er.x = ptsFittedShp[69].x;
	er.y = ptsFittedShp[69].y;
#else

	el.x = ((ptsFittedShp[36].x + ptsFittedShp[39].x) / 2) + 0.5;
	el.y = ((ptsFittedShp[36].y + ptsFittedShp[39].y) / 2) + 0.5;
	er.x = ((ptsFittedShp[42].x + ptsFittedShp[45].x) / 2) + 0.5;
	er.y = ((ptsFittedShp[42].y + ptsFittedShp[45].y) / 2) + 0.5;
#endif


	PIM_Bitmap_CreateRef(&pAlignFace[0], NULL, 190, 190, 190, PIM_IMAGE_FORMAT_GRAY);
	bAlignedRet = alignment_TL_Bayesian(pBitmap->imageData, pBitmap->width, pBitmap->height, ptsFittedShp, pAlignFace[0].imageData, ptsAlignedShp);

	if( bAlignedRet != PIM_TRUE )
	{
		PIM_Bitmap_Destroy(&pAlignFace[0]);
		return PIM_ERROR_GENERAL;
	}

	return PIM_SUCCESS;
}

PIM_Int32 CFaceAligner::affine_140_168_FIXED(PIM_Uint8 *src_data, PIM_Int32 src_w, PIM_Int32 src_h, PIM_Point ptEyeLeft, PIM_Point ptEyeRight, PIM_Uint8 *des_data)
{
	int des_x, des_y;
	int des_w = 140, des_h = 168;
	int des_x2 = 40, des_y2 = 68;
	int des_x3 = 98;

	int src_size = src_w*src_h;
	int des_size = des_w*des_h;

	int x1, x2, x3, y1, y2, y3;
	int dx1, dx2, dy1, dy2;
	int src_x, src_y, a11, a12, a21, a22;

	int l, k, a, b;
	int src_w_step, src, src_l, src_k, src_lk;
	unsigned char* pdes = des_data;
	memset(des_data, 0, des_size);


	x2 = ptEyeLeft.x, y2 = ptEyeLeft.y;
	x3 = ptEyeRight.x, y3 = ptEyeRight.y;

	if( x2 == x3 )
	{
		x2--;
	}

	dx1 = (x3 - x2) * (des_x2 << 10) / (des_x3 - des_x2);
	dy1 = dx1 * (y3 - y2) / (x3 - x2);

	if( dy1 < 0 )
	{
		dy1 = -dy1;
	}
	dy2 = dx1 * des_y2 / des_x2;
	dx2 = dy1 * dy2 / dx1;

	if( y2 > y3 )
	{
		x1 = (x2 + ((-dx1 - dx2 + 512) >> 10));
		y1 = (y2 + ((dy1 - dy2 + 512) >> 10));
	}
	else
	{
		x1 = (x2 + ((-dx1 + dx2 + 512) >> 10));
		y1 = (y2 + ((-dy1 - dy2 + 512) >> 10));
	}

	a11 = 18 * (x3 - x2), a12 = -15 * x1 + 25 * x2 - 10 * x3;
	a21 = 18 * (y3 - y2), a22 = -15 * y1 + 25 * y2 - 10 * y3;


	for( des_y = 0; des_y < des_h; des_y++ )
	{
		for( des_x = 0; des_x < des_w; des_x++ )
		{
			a = a11*des_x + a12*des_y;
			b = a21*des_x + a22*des_y;
			src_x = (a >> 10) + x1;
			src_y = (b >> 10) + y1;

			if( src_x < 0 ) { src_x = 0;/* return 0;*/ }
			if( src_x >= src_w ) { src_x = src_w - 1; /*return 0; */ }
			if( src_y < 0 ) { src_y = 0; /*return 0;*/ }
			if( src_y >= src_h ) { src_y = src_h - 1; /*return 0;*/ }

			l = src_x;
			k = src_y;
			a = a - ((a >> 10) << 10);	//항상 >= 0
			b = b - ((b >> 10) << 10);	//항상 >= 0
			src_w_step = k*src_w;
			src = src_data[src_w_step + l];
			src_l = src_data[src_w_step + l + 1];
			src_k = src_data[src_w_step + src_w + l];
			src_lk = src_data[src_w_step + src_w + l + 1];

			if( src_w_step + src_w + l + 1 < src_size )
			{
				*pdes = (unsigned char)(((src << 10) + a*(src_l - src) + b*(src_k - src) + ((a*b*(src + src_lk - src_l - src_k)) >> 10)) >> 10);
			}
			pdes++;
		}
	}

	return 1;
}

PIM_Int32 CFaceAligner::affine_52_44_FIXED(PIM_Uint8 *src_data, PIM_Int32 src_w, PIM_Int32 src_h, PIM_Point ptEyeLeft, PIM_Point ptEyeRight, PIM_Uint8 *des_data)
{
	int des_x, des_y;
	int des_w = 44, des_h = 52;
	double des_x2 = 12, des_y2 = 16;
	double des_x3 = 31;

	int src_size = src_w*src_h;
	int des_size = des_w*des_h;

	double x1, y1, x2, y2, x3, y3;

	double dx1, dx2, dy1, dy2;
	double src_x, src_y;
	double a11, a12, a21, a22;
	int l, k, src, src_l, src_k, src_lk, src_w_step, des_w_step = 0;
	double a, b;

	unsigned char* pdes = NULL;

	memset(des_data, 0, des_size);

	x2 = ptEyeLeft.x; y2 = ptEyeLeft.y;
	x3 = ptEyeRight.x; y3 = ptEyeRight.y;

	dx1 = (x3 - x2) * (des_x2) / (des_x3 - des_x2);
	dy1 = dx1 * (y3 - y2) / (x3 - x2);

	if( dy1 < 0 ) dy1 = -dy1;
	dy2 = dx1 * des_y2 / des_x2;
	dx2 = dy1 * dy2 / dx1;

	if( y2 > y3 )
	{
		x1 = (x2 + (-dx1 - dx2 + 0.5));
		y1 = (y2 + (dy1 - dy2 + 0.5));
	}
	else
	{
		x1 = (x2 + (-dx1 + dx2 + 0.5));
		y1 = (y2 + (-dy1 - dy2 + 0.5));
	}

	a11 = (1.0 / (des_x3 - des_x2)) * (x3 - x2);
	a12 = (x2 - x1 - des_x2*a11) / des_y2;
	a21 = (1.0 / (des_x3 - des_x2)) * (y3 - y2);
	a22 = (y2 - y1 - des_x2*a21) / des_y2;

	pdes = des_data;
	for( des_y = 0; des_y < des_h; des_y++ )
	{
		for( des_x = 0; des_x < des_w; des_x++ )
		{
			src_x = a11*des_x + a12*des_y + x1;
			src_y = a21*des_x + a22*des_y + y1;

			if( src_x < 0 )
			{
				src_x = 0;
				/*return 0;*/
			}
			if( src_x >= src_w )
			{
				src_x = src_w - 1;
				/*return 0;*/
			}
			if( src_y < 0 )
			{
				src_y = 0;
				/*return 0; */
			}
			if( src_y >= src_h )
			{
				src_y = src_h - 1;
				/*return 0;*/
			}

			l = (int)(src_x);
			k = (int)(src_y);
			a = src_x - l;
			b = src_y - k;
			src_w_step = k * src_w;
			src = src_data[src_w_step + l];
			src_l = src_data[src_w_step + l + 1];
			src_k = src_data[src_w_step + src_w + l];
			src_lk = src_data[src_w_step + src_w + l + 1];

			*pdes++ = (unsigned char)(src + a*(src_l - src) + b*(src_k - src) + a*b*(src + src_lk - src_l - src_k));
		}
	}

	return 1;
}


#define IMAGE_WIDTH		190
#define IMAGE_HEIGHT	190
#define LANDMARK_NUM	68
PIM_Int32 CFaceAligner::alignment_TL_Bayesian(PIM_Uint8 *src_data, PIM_Int32 src_w, PIM_Int32 src_h, PIM_DPoint *src_pts, PIM_Uint8 *des_data, PIM_DPoint *des_pts)
{
	PIM_DPoint pt_le, pt_re, pt_nose, pt_lm, pt_rm;

#if 0
	pt_le = src_pts[68];
	pt_re = src_pts[69];
#else
	pt_le.x = (src_pts[36].x + src_pts[39].x) * 0.5;
	pt_le.y = (src_pts[36].y + src_pts[39].y) * 0.5;
	pt_re.x = (src_pts[42].x + src_pts[45].x) * 0.5;
	pt_re.y = (src_pts[42].y + src_pts[45].y) * 0.5;
#endif
	pt_nose = src_pts[30];
	pt_lm.x = (src_pts[48].x + src_pts[60].x) * 0.5;
	pt_lm.y = (src_pts[48].y + src_pts[60].y) * 0.5;
	pt_rm.x = (src_pts[54].x + src_pts[64].x) * 0.5;
	pt_rm.y = (src_pts[54].y + src_pts[64].y) * 0.5;

	MatrixXf A(5, 3);
	A << le_m.x, le_m.y, 1,
		re_m.x, re_m.y, 1,
		nose_m.x, nose_m.y, 1,
		lm_m.x, lm_m.y, 1,
		rm_m.x, rm_m.y, 1;

	MatrixXf b(5, 2);
	b << pt_le.x, pt_le.y,
		pt_re.x, pt_re.y,
		pt_nose.x, pt_nose.y,
		pt_lm.x, pt_lm.y,
		pt_rm.x, pt_rm.y;

	MatrixXf inv_trans = A.jacobiSvd(ComputeThinU | ComputeThinV).solve(b);

	// Normalize the image using five points
	int src_x, src_y;
	float a11, a12, x1;
	float a21, a22, y1;
	a11 = inv_trans(0, 0); a12 = inv_trans(1, 0); x1 = inv_trans(2, 0) + 0.5;
	a21 = inv_trans(0, 1); a22 = inv_trans(1, 1); y1 = inv_trans(2, 1) + 0.5;

	unsigned char* pdes = des_data;

	for( int des_y = 0; des_y < IMAGE_HEIGHT; des_y++ )
	{
		for( int des_x = 0; des_x < IMAGE_WIDTH; des_x++ )
		{
			src_x = (a11*des_x + a12*des_y) + x1;
			src_y = (a21*des_x + a22*des_y) + y1;

			if( src_x < 0 )			{ src_x = 0; *pdes++ = 0; continue; }
			if( src_x >= src_w )	{ src_x = src_w - 1; *pdes++ = 0; continue; }
			if( src_y < 0 )			{ src_y = 0; *pdes++ = 0; continue; }
			if( src_y >= src_h )	{ src_y = src_h - 1; *pdes++ = 0; continue; }

			*pdes++ = src_data[src_y * src_w + src_x];
		}
	}

	for( int i = 0; i < LANDMARK_NUM; i++ )
	{
		float des_y = (a11*src_pts[i].y + a21*x1 - a11*y1 - a21*src_pts[i].x) / (a11*a22 - a12*a21);
		float des_x = (src_pts[i].x - a12*des_y - x1) / a11;

		des_pts[i].x = des_x;
		des_pts[i].y = des_y;
	}


	return 1;
}

//PIM_Int32 CFaceAligner::alignment_RGB(PIM_Uint8 *src_data, PIM_Int32 src_w, PIM_Int32 src_h, PIM_DPoint *src_pts, PIM_Uint8 *des_data, PIM_DPoint *des_pts)
PIM_Int32 CFaceAligner::alignment_RGB(PIM_Uint8 *src_data, PIM_Int32 src_w, PIM_Int32 src_h, PIM_DPoint *src_pts, PIM_Uint8 *des_data)
{
	PIM_DPoint pt_le, pt_re, pt_nose, pt_lm, pt_rm;

#if 0
	pt_le = src_pts[68];
	pt_re = src_pts[69];
#else
	pt_le.x = (src_pts[36].x + src_pts[39].x) * 0.5;
	pt_le.y = (src_pts[36].y + src_pts[39].y) * 0.5;
	pt_re.x = (src_pts[42].x + src_pts[45].x) * 0.5;
	pt_re.y = (src_pts[42].y + src_pts[45].y) * 0.5;
#endif
	pt_nose = src_pts[30];
	pt_lm.x = (src_pts[48].x + src_pts[60].x) * 0.5;
	pt_lm.y = (src_pts[48].y + src_pts[60].y) * 0.5;
	pt_rm.x = (src_pts[54].x + src_pts[64].x) * 0.5;
	pt_rm.y = (src_pts[54].y + src_pts[64].y) * 0.5;

	MatrixXf A(5, 3);
	A << le_m.x, le_m.y, 1,
		re_m.x, re_m.y, 1,
		nose_m.x, nose_m.y, 1,
		lm_m.x, lm_m.y, 1,
		rm_m.x, rm_m.y, 1;

	MatrixXf b(5, 2);
	b << pt_le.x, pt_le.y,
		pt_re.x, pt_re.y,
		pt_nose.x, pt_nose.y,
		pt_lm.x, pt_lm.y,
		pt_rm.x, pt_rm.y;

	MatrixXf inv_trans = A.jacobiSvd(ComputeThinU | ComputeThinV).solve(b);

	int src_x, src_y;
	float a11, a12, x1;
	float a21, a22, y1;
	a11 = inv_trans(0, 0); a12 = inv_trans(1, 0); x1 = inv_trans(2, 0) + 0.5;
	a21 = inv_trans(0, 1); a22 = inv_trans(1, 1); y1 = inv_trans(2, 1) + 0.5;

	unsigned char *pdes = des_data;

	for( int des_y = 0; des_y < IMAGE_HEIGHT; des_y++ )
	{
		for( int des_x = 0; des_x < IMAGE_WIDTH; des_x++ )
		{
			src_x = (a11*des_x + a12*des_y) + x1;
			src_y = (a21*des_x + a22*des_y) + y1;

			if( src_x < 0 )			{ src_x = 0; *pdes++ = 0; *pdes++ = 0; *pdes++ = 0; continue; }
			if( src_x >= src_w )	{ src_x = src_w - 1;  *pdes++ = 0; *pdes++ = 0; *pdes++ = 0; continue; }
			if( src_y < 0 )			{ src_y = 0; *pdes++ = 0; *pdes++ = 0; *pdes++ = 0; continue; }
			if( src_y >= src_h )	{ src_y = src_h - 1; *pdes++ = 0; *pdes++ = 0; *pdes++ = 0; continue; }

			*pdes++ = src_data[src_y * src_w * 3 + (src_x * 3)];
			*pdes++ = src_data[src_y * src_w * 3 + (src_x * 3 + 1)];
			*pdes++ = src_data[src_y * src_w * 3 + (src_x * 3 + 2)];
		}
	}

# if 0
	for( int i = 0; i < LANDMARK_NUM; i++ )
	{
		float des_y = (a11*src_pts[i].y + a21*x1 - a11*y1 - a21*src_pts[i].x) / (a11*a22 - a12*a21);
		float des_x = (src_pts[i].x - a12*des_y - x1) / a11;

		des_pts[i].x = des_x;
		des_pts[i].y = des_y;
	}
#endif

	return 1;
}