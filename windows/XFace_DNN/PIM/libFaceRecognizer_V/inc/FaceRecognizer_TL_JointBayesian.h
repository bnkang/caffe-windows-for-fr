#ifndef _FACE_RECOGNIZER_TL_JOINT_BAYESIAN_H_
#define _FACE_RECOGNIZER_TL_JOINT_BAYESIAN_H_

#pragma once

#include "exp/PIM_Define_op.h"
#include "exp/PIMTypes.h"

#include "FR_Params.h"
#include "FaceAligner.h"
#include "FaceFeatureExtractor.h"

#include <vector>


#ifdef __cplusplus
extern "C" {
#endif

	using namespace std;
	
	class CFaceRecognizer_TL_JointBayesian
	{
	public:
		CFaceRecognizer_TL_JointBayesian();
		~CFaceRecognizer_TL_JointBayesian();

		PIM_Bool Load_GalleryDB();
		PIM_Bool Store_GalleryDB();

		PIM_Bool AddPerson(PIM_Bitmap *src_bitmap, PIM_DPoint *src_pts);

		PIM_Int32 Enroll(PIM_Uint8 *src_data, PIM_Int8 *szName);
		PIM_Int32 Delete(PIM_Int32 nSelectedIdx);

		PIM_Int32 SingleFace_Delete(PIM_Int32 nID);
		PIM_Int32 Person_Delete_Buffer();

		// Face Identifier		
		PIM_Int32 Who_DNN_Image(PIM_Bitmap *pBitmap, PIM_DPoint *src_pts, PIM_Int32 *nRankOneID, PIM_Float *fConfidence);

		PIM_Int32 Get_Enroll_Info(PIM_Int32 nIndex, PIM_Char *szName, PIM_Uint8 *face_data, PIM_Int32 *nFace);
		PIM_Int32 Get_Enroll_Info(PIM_Int32 nIndex, PIM_Char *szName, PIM_Uint8 *face_data);
		PIM_Int32 Get_Enroll_Cnt(PIM_Int32 nIndex);

		PIM_Int32 Get_Enroll_Candidate(PIM_Int32 *arCandidateIdx, PIM_Int32 nCandidateCnt, fr_enroll_info *reduced_enroll_info);

		PIM_Int32 Get_Face_Width();
		PIM_Int32 Get_Face_Height();

		PIM_Int32 Get_Enroll_Size();
		PIM_Int32 Get_Gallery_Size();

		void ResetPermanentID();
		void Reset_PermanentID(pdata_info fft_info);

	private:
		PIM_Float likelihood_ratio(PIM_Float *f1, PIM_Float *f2);
		void GalleryDB_Initialize();

	private:
		PIM_Int32 m_nPermanentID;
		PIM_Int32 m_nPermanentID2[10];
		PIM_Int32 m_nHitCnt;
		PIM_Int32 m_Status;
		PIM_Float m_fConfidence;
		PIM_Int32 m_GenderType;

		int nThread;

		// transfer matrix
		PIM_Float *A_sqrtlbp, *G_sqrtlbp;
		// gallery database
		fr_enroll_info m_fr_enroll;
		// Face Aligner
		CFaceAligner m_FA;
		// Feature extractor
		CFaceFeatureExtractor m_FFE;
	};

#ifdef __cplusplus
}
#endif

#endif // #ifndef _FACE_FEATURE_EXTRACTOR_H_