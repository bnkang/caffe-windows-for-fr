
#ifndef _FACE_ALIGNER_H_
#define _FACE_ALIGNER_H_

#pragma once

#include "exp/PIMTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

	using namespace std;

	class CFaceAligner
	{
	public:
		CFaceAligner();
		~CFaceAligner();

		PIM_Int32 FaceAligner_AlignFace_ASURF(PIM_Bitmap *pBitmap, PIM_Point ptsLeftEye, PIM_Point ptsRightEye, PIM_Bitmap *pAlignFace);
		PIM_Int32 FaceAligner_AlignFace_ASURF_Gender(PIM_Bitmap *pBitmap, PIM_Point ptsLeftEye, PIM_Point ptsRightEye, PIM_Bitmap *pAlignFace);
		PIM_Int32 FaceAligner_AlignFace_TL(PIM_Bitmap *pBitmap, PIM_DPoint *ptsFittedShp, PIM_Bitmap *pAlignFace, PIM_DPoint *ptsAlignedShp);
		PIM_Int32 FaceAligner_AlignFace_TL_Gender(PIM_Bitmap *pBitmap, PIM_DPoint *ptsFittedShp, PIM_Bitmap *pAlignFace, PIM_DPoint *ptsAlignedShp);
		//PIM_Int32 FaceAligner_AlignFace_Deep(PIM_Bitmap *pBitmap, PIM_DPoint *ptsFittedShp, PIM_Bitmap *pAlignFace_95, PIM_Bitmap *pAlignFace_67, PIM_Bitmap *pAlignFace_47, PIM_DPoint *ptsAlignedShp);
		PIM_Int32 FaceAligner_AlignFace_Deep(PIM_Bitmap *pBitmap, PIM_DPoint *ptsFittedShp, PIM_Bitmap *pAlignFace_95, PIM_Bitmap *pAlignFace_67, PIM_Bitmap *pAlignFace_47);

	private:
		PIM_Int32 affine_52_44_FIXED(PIM_Uint8 *src_data, PIM_Int32 src_w, PIM_Int32 src_h, PIM_Point ptEyeLeft, PIM_Point ptEyeRight, PIM_Uint8 *des_data);
		PIM_Int32 affine_140_168_FIXED(PIM_Uint8 *src_data, PIM_Int32 src_w, PIM_Int32 src_h, PIM_Point ptEyeLeft, PIM_Point ptEyeRight, PIM_Uint8 *des_data);
		PIM_Int32 alignment_TL_Bayesian(PIM_Uint8 *image_data, PIM_Int32 nWidth, PIM_Int32 nHeight, PIM_DPoint *ptsShp, PIM_Uint8 *aligned_face, PIM_DPoint *aligned_shp);
		//PIM_Int32 alignment_RGB(PIM_Uint8 *src_data, PIM_Int32 src_w, PIM_Int32 src_h, PIM_DPoint *src_pts, PIM_Uint8 *des_data, PIM_DPoint *des_pts);
		PIM_Int32 alignment_RGB(PIM_Uint8 *src_data, PIM_Int32 src_w, PIM_Int32 src_h, PIM_DPoint *src_pts, PIM_Uint8 *des_data);

		PIM_DPoint le_m, re_m, nose_m, lm_m, rm_m;
	};


#ifdef __cplusplus
}
#endif

#endif // #ifndef _FACE_ALIGNER_H_
