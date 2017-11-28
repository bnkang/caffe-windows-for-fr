#include "stdafx.h"

#include <stdio.h>
#include <memory.h>
#include "libPlatform/inc/PIMBitmap.h"
#include "libFaceRecognizer_V/inc/FaceRecognizer_TL_JointBayesian.h"


#include "libPlatform/inc/ImageProcess.h"
#include "libFaceRecognizer_V/Eigen/Core"
#include "libFaceRecognizer_V/Eigen/Eigen"

#include "caffe/util/benchmark.hpp"


#define _OPENMP

#ifdef _OPENMP
#include <omp.h>
#endif


using namespace std;
using namespace Eigen;



CFaceRecognizer_TL_JointBayesian::CFaceRecognizer_TL_JointBayesian()
{
	// Read the resources from dat files
	FILE* fp = NULL;
	fp = fopen(DNN_TL_MAT_PATH, "rb");
	if (fp == NULL)
	{
		printf("error open files \n");
		return;
	}

	A_sqrtlbp = (PIM_Float *)malloc(1024 * 1024 * sizeof(PIM_Float));
	G_sqrtlbp = (PIM_Float *)malloc(1024 * 1024 * sizeof(PIM_Float));

	fread(A_sqrtlbp, sizeof(PIM_Float), 1024 * 1024, fp);
	fread(G_sqrtlbp, sizeof(PIM_Float), 1024 * 1024, fp);
	fclose(fp);

	GalleryDB_Initialize();

	nThread = NUM_THREADS;// omp_get_max_threads();
	omp_set_num_threads(nThread);
}

CFaceRecognizer_TL_JointBayesian::~CFaceRecognizer_TL_JointBayesian()
{
	free(A_sqrtlbp);
	free(G_sqrtlbp);
}

void CFaceRecognizer_TL_JointBayesian::GalleryDB_Initialize()
{
	m_fr_enroll.size = 0;
	m_fr_enroll.person_count = 0;
	memset(m_fr_enroll.id, 0, sizeof(int)*FR_ENROLL_MAX_SIZE);
	memset(m_fr_enroll.aligned_face, 0, sizeof(unsigned char)*ALIGN_IMG_WIDTH*ALIGN_IMG_HEIGHT * 3 * FR_ENROLL_MAX_SIZE);
	memset(m_fr_enroll.feature, 0.0f, sizeof(float)*FR_ENROLL_MAX_SIZE*PCA_FEATURE_DIM);
}


void CFaceRecognizer_TL_JointBayesian::ResetPermanentID()
{
	m_nPermanentID = -1;
	m_nHitCnt = 0;
	m_fConfidence = -99999.9f;
	m_GenderType = -1;
}

void CFaceRecognizer_TL_JointBayesian::Reset_PermanentID(pdata_info fft_info)
{
	for (int i = 0; i < MAX_FACE_NUM; i++)
	{
		fft_info[i].current_pts.Clear();
		fft_info[i].initial_pts.Clear();
		fft_info[i].current_norm_pts.Clear();

		memset(fft_info[i].q, 0.0, sizeof(float)* 4);
		memset(fft_info[i].inv_q, 0.0, sizeof(float)* 4);
		fft_info[i].nROP = 0;
		fft_info[i].nHitCnt = -1;
		fft_info[i].nUnknownHitCnt = -1;
		fft_info[i].nRecogID = -2;
		fft_info[i].nRankOneID = -2;
		fft_info[i].fConfidence = 0.0;
		fft_info[i].nGenderType = -1;
		fft_info[i].nPermanentID = -1;
		fft_info[i].TRACKING_STATUS = 0;
		memset(fft_info[i].nRecogIDs, -2, sizeof(int)* 20);
	}
}

PIM_Bool CFaceRecognizer_TL_JointBayesian::Load_GalleryDB()
{
	FILE *fp = fopen(FR_GALLERY_DB_PATH, "rb");

	if (fp == NULL)
	{
		printf("FaceRecognizer_Load_GalleryDB() : file load error\n");
		return PIM_FALSE;
	}

	if (fread(&m_fr_enroll, sizeof(fr_enroll_info), 1, fp) != 1)
	{
		printf("FaceRecognizer_Load_GalleryDB() : file read error\n");
		fclose(fp);
		return PIM_FALSE;
	}

	fclose(fp);

	return PIM_TRUE;
}

PIM_Bool CFaceRecognizer_TL_JointBayesian::Store_GalleryDB()
{
	FILE *fp = fopen(FR_GALLERY_DB_PATH, "wb");
	if (fp == NULL)
	{
		printf("FaceRecognizer_Store_GalleryDB() : file store error\n");
		fclose(fp);
		return PIM_FALSE;
	}

	if (fwrite(&m_fr_enroll, sizeof(fr_enroll_info), 1, fp) != 1)
	{
		printf("FaceRecognizer_Store_GalleryDB() : file write error\n");
		fclose(fp);
		return PIM_FALSE;
	}

	fclose(fp);

	return PIM_TRUE;
}


PIM_Bool CFaceRecognizer_TL_JointBayesian::AddPerson(PIM_Bitmap *src_bitmap, PIM_DPoint *src_pts)
{
	if (src_bitmap == NULL || src_pts == NULL)
	{
		printf("error.\n");
		return PIM_FALSE;
	}

	PIM_Int32 id = 0;
	PIM_Int32 count = 0;

	PIM_Float feature[PCA_FEATURE_DIM];

	PIM_Bitmap aligned_face_95 = { 0 };
	PIM_Bitmap aligned_face_67 = { 0 };
	PIM_Bitmap aligned_face_47 = { 0 };

	for (int i = 0; i < m_fr_enroll.size; i++)
	{
		if (strcmp(m_fr_enroll.name[i], "UNNAMED_PERSON") == 0)
		{
			id = m_fr_enroll.id[i];
			count++;
		}
	}

	if (m_fr_enroll.size >= FR_ENROLL_MAX_SIZE)
	{
		return -3;
	}

	PIM_Bitmap_CreateRef(&aligned_face_95, NULL, 95, 95, 95 * 3, PIM_IMAGE_FORMAT_BGR888);
	PIM_Bitmap_CreateRef(&aligned_face_67, NULL, 67, 67, 67 * 3, PIM_IMAGE_FORMAT_BGR888);
	PIM_Bitmap_CreateRef(&aligned_face_47, NULL, 47, 47, 47 * 3, PIM_IMAGE_FORMAT_BGR888);

	if (m_FA.FaceAligner_AlignFace_Deep(src_bitmap, src_pts, &aligned_face_95, &aligned_face_67, &aligned_face_47) != PIM_SUCCESS)
	{
		printf("Face Alignment error.\n");
		PIM_Bitmap_Destroy(&aligned_face_47);
		PIM_Bitmap_Destroy(&aligned_face_67);
		PIM_Bitmap_Destroy(&aligned_face_95);
		return PIM_FALSE;
	}
	memcpy(m_fr_enroll.aligned_face[m_fr_enroll.size], aligned_face_95.imageData, sizeof(PIM_Uint8)*ALIGN_IMG_WIDTH*ALIGN_IMG_HEIGHT * 3);

	if (m_FFE.FeatureExtract(&aligned_face_95, &aligned_face_67, &aligned_face_47, feature) != PIM_SUCCESS)
	{
		printf("Feature extract error.\n");
		PIM_Bitmap_Destroy(&aligned_face_47);
		PIM_Bitmap_Destroy(&aligned_face_67);
		PIM_Bitmap_Destroy(&aligned_face_95);
		return PIM_FALSE;
	}


	memcpy(m_fr_enroll.feature[m_fr_enroll.size], feature, sizeof(PIM_Float)* PCA_FEATURE_DIM);
	strcpy(m_fr_enroll.name[m_fr_enroll.size], "UNNAMED_PERSON");

	if (id == 0)
	{
		m_fr_enroll.id[m_fr_enroll.size] = m_fr_enroll.person_count + 1;
		m_fr_enroll.person_count++;
	}
	else
	{
		m_fr_enroll.id[m_fr_enroll.size] = id;
	}
	m_fr_enroll.size++;


	PIM_Bitmap_Destroy(&aligned_face_47);
	PIM_Bitmap_Destroy(&aligned_face_67);
	PIM_Bitmap_Destroy(&aligned_face_95);

	return PIM_TRUE;
}

PIM_Int32 CFaceRecognizer_TL_JointBayesian::Enroll(PIM_Uint8 *src_data, PIM_Int8 *szName)
{
	int i, count = 0, id = -1;
	int bExist = 0;

	for (i = 0; i < m_fr_enroll.size; i++)
	{
		if (strcmp(m_fr_enroll.name[i], szName) == 0)
		{
			id = m_fr_enroll.id[i];
			bExist = 1;
			break;
		}
	}

	for (i = 0; i < m_fr_enroll.size; i++)
	{
		if (strcmp(m_fr_enroll.name[i], "UNNAMED_PERSON") == 0)
		{
			strcpy(m_fr_enroll.name[i], szName);
			count++;

			// if existing name, add only one new feature
			if (bExist)
			{
				m_fr_enroll.id[i] = id;
				count = FR_ENROLL_IMAGE_PER_PERSON;
				break;
			}
		}
	}

	count = 0;
	for (i = 0; i < m_fr_enroll.size; i++)
	{
		if (m_fr_enroll.id[i] == id)
		{
			count++;
		}
	}

	// if existing name, delete the oldest face of the name
	if (count > FR_ENROLL_IMAGE_PER_PERSON)
	{
		SingleFace_Delete(id);
	}
	Person_Delete_Buffer();

	if (bExist)
	{
		m_fr_enroll.person_count--;
	}

	return PIM_TRUE;
}


PIM_Int32 CFaceRecognizer_TL_JointBayesian::Delete(PIM_Int32 nSelectedIdx)
{
	int i, j, count = 0;
	int nID = nSelectedIdx + 1;

	for (i = 0; i < m_fr_enroll.size; i++)
	{
		if (m_fr_enroll.id[i] == nSelectedIdx + 1)
		{
			m_fr_enroll.id[i] = 0;
			count++;
		}
	}

	for (i = 0; i < m_fr_enroll.size; i++)
	{
		if (m_fr_enroll.id[i] == 0)
		{
			for (j = i + 1; j < m_fr_enroll.size; j++)
			{
				if (m_fr_enroll.id[j] != 0)
				{
					if (m_fr_enroll.id[j] < nID)
					{
						m_fr_enroll.id[i] = m_fr_enroll.id[j];
					}
					else
					{
						m_fr_enroll.id[i] = m_fr_enroll.id[j] - 1;
					}

					strcpy(m_fr_enroll.name[i], m_fr_enroll.name[j]);
					memcpy(m_fr_enroll.feature[i], m_fr_enroll.feature[j], sizeof(PIM_Float)*PCA_FEATURE_DIM);
					memcpy(m_fr_enroll.aligned_face[i], m_fr_enroll.aligned_face[j], sizeof(PIM_Uint8)*ALIGN_IMG_WIDTH*ALIGN_IMG_HEIGHT * 3);

					m_fr_enroll.id[j] = 0;
					break;
				}
			}
			if (j == m_fr_enroll.size)
			{
				break;
			}
		}
		else
		{
			if (m_fr_enroll.id[i] > nID)
			{
				m_fr_enroll.id[i] = m_fr_enroll.id[i] - 1;
			}
		}
	}

	m_fr_enroll.size -= count;
	m_fr_enroll.person_count -= count > 0 ? 1 : 0;

	return 0;
}

PIM_Int32 CFaceRecognizer_TL_JointBayesian::SingleFace_Delete(PIM_Int32 nID)
{
	int i, j, count = 0;

	for (i = 0; i < m_fr_enroll.size; i++)
	{
		if (m_fr_enroll.id[i] == nID)
		{
			if (count++ == 1)
			{
				m_fr_enroll.id[i] = 0;
				count = 1;
				break;
			}
			//count++;
			//break;
		}
	}

	if (count == 0)
	{
		return count;
	}

	for (i = 0; i < m_fr_enroll.size; i++)
	{
		if (m_fr_enroll.id[i] == 0)
		{
			for (j = i + 1; j < m_fr_enroll.size; j++)
			{
				if (m_fr_enroll.id[j] != 0)
				{
					m_fr_enroll.id[i] = m_fr_enroll.id[j];
					strcpy(m_fr_enroll.name[i], m_fr_enroll.name[j]);
					memcpy(m_fr_enroll.feature[i], m_fr_enroll.feature[j], sizeof(PIM_Float)*PCA_FEATURE_DIM);
					memcpy(m_fr_enroll.aligned_face[i], m_fr_enroll.aligned_face[j], sizeof(PIM_Uint8)*ALIGN_IMG_WIDTH*ALIGN_IMG_HEIGHT * 3);
					m_fr_enroll.id[j] = 0;
					break;
				}
			}
			if (j == m_fr_enroll.size)
			{
				break;
			}
		}
	}
	m_fr_enroll.size -= count;


	return count;
}

PIM_Int32 CFaceRecognizer_TL_JointBayesian::Person_Delete_Buffer()
{
	int i, count;
	count = 0;

	for (i = 0; i < m_fr_enroll.size; i++)
	{
		if (strcmp(m_fr_enroll.name[i], "UNNAMED_PERSON") == 0)
		{
			m_fr_enroll.id[i] = 0;
			strcpy(m_fr_enroll.name[i], "");
			count++;
		}
	}
	m_fr_enroll.size -= count;
	m_fr_enroll.person_count -= count > 0 ? 1 : 0;

	return count;	//	returns number of deleted faces
}


PIM_Int32 CFaceRecognizer_TL_JointBayesian::Get_Enroll_Info(PIM_Int32 nIndex, PIM_Char *szName, PIM_Uint8 *face_data, PIM_Int32 *nFace)
{
	int i, j;

	int nFaceNum = 0;
	for (i = 0; i < m_fr_enroll.size; i++)
	{
		if (m_fr_enroll.id[i] == nIndex + 1)
		{
			if (szName)
			{
				sprintf(szName, "%s", m_fr_enroll.name[i]);
			}
			if (face_data)
			{
				memcpy(face_data, m_fr_enroll.aligned_face[i], sizeof(PIM_Uint8)*ALIGN_IMG_WIDTH*ALIGN_IMG_HEIGHT * 3);
				nFaceNum++;
			}

			for (j = 0; j < m_fr_enroll.size; j++)
			{
				if (m_fr_enroll.id[j] == nIndex + 1)
				{
					(*nFace)++;
				}
			}

			if (nFaceNum == 1)
				return 1;
		}
	}

	return 0;
}


PIM_Int32 CFaceRecognizer_TL_JointBayesian::Get_Enroll_Cnt(PIM_Int32 nIndex)
{
	int nCnt = 0;
	for (int i = 0; i < m_fr_enroll.size; i++)
	{
		if (m_fr_enroll.id[i] == nIndex)
		{
			nCnt++;
		}
	}

	return nCnt;
}


PIM_Int32 CFaceRecognizer_TL_JointBayesian::Get_Enroll_Info(PIM_Int32 nIndex, PIM_Char *szName, PIM_Uint8 *face_data)
{
	int i;

	for (i = 0; i < m_fr_enroll.size; i++)
	{
		if (m_fr_enroll.id[i] == nIndex)
		{
			if (szName)
			{
				sprintf(szName, "%s", m_fr_enroll.name[i]);
			}
			if (face_data)
			{
				memcpy(face_data, m_fr_enroll.aligned_face[i], sizeof(PIM_Uint8)*ALIGN_IMG_WIDTH*ALIGN_IMG_HEIGHT * 3);
			}

			return 1;
		}
	}

	return 0;
}

PIM_Int32 CFaceRecognizer_TL_JointBayesian::Get_Face_Width()
{
	return ALIGN_IMG_WIDTH;
}

PIM_Int32 CFaceRecognizer_TL_JointBayesian::Get_Face_Height()
{
	return ALIGN_IMG_HEIGHT;
}

PIM_Int32 CFaceRecognizer_TL_JointBayesian::Get_Enroll_Size()
{
	return m_fr_enroll.person_count;
}

PIM_Int32 CFaceRecognizer_TL_JointBayesian::Get_Gallery_Size()
{
	return m_fr_enroll.size;
}


PIM_Int32 CFaceRecognizer_TL_JointBayesian::Who_DNN_Image(PIM_Bitmap *pBitmap, PIM_DPoint *src_pts, PIM_Int32 *nRankOneID, PIM_Float *fConfidence)
{
// 	Timer total_timer;
// 	Timer align_timer;
// 	Timer extract_timer;
// 	Timer recognition_timer;
// 	Timer omp_timer;

	PIM_Int32 nRecogIdx = -1;
	PIM_Int32 nR1Idx = -1;

	//total_timer.Start();

	PIM_Float reduced_feature[PCA_FEATURE_DIM];
	memset(reduced_feature, 0, sizeof(PIM_Float)*PCA_FEATURE_DIM);

	PIM_Bitmap aligned_face_95 = { 0 };
	PIM_Bitmap aligned_face_67 = { 0 };
	PIM_Bitmap aligned_face_47 = { 0 };

	if (pBitmap == NULL || src_pts == NULL)
	{
		printf("input params are null.\n");
		return -2;
	}

	if (m_fr_enroll.person_count < 1 || m_fr_enroll.size < 1)
	{
		printf("# of enrolled persons is zero.\n");
		return -10;
	}


	PIM_Bitmap_CreateRef(&aligned_face_95, NULL, 95, 95, 95 * 3, PIM_IMAGE_FORMAT_BGR888);
	PIM_Bitmap_CreateRef(&aligned_face_67, NULL, 67, 67, 67 * 3, PIM_IMAGE_FORMAT_BGR888);
	PIM_Bitmap_CreateRef(&aligned_face_47, NULL, 47, 47, 47 * 3, PIM_IMAGE_FORMAT_BGR888);

	//align_timer.Start();
	if (m_FA.FaceAligner_AlignFace_Deep(pBitmap, src_pts, &aligned_face_95, &aligned_face_67, &aligned_face_47) != PIM_SUCCESS)
	{
		printf("Face Alignment error.\n");
		PIM_Bitmap_Destroy(&aligned_face_47);
		PIM_Bitmap_Destroy(&aligned_face_67);
		PIM_Bitmap_Destroy(&aligned_face_95);
		return -3;
	}
	//align_timer.Stop();


	//extract_timer.Start();
	if (m_FFE.FeatureExtract(&aligned_face_95, &aligned_face_67, &aligned_face_47, reduced_feature) != PIM_SUCCESS)
	{
		printf("Feature extraction error.\n");
		PIM_Bitmap_Destroy(&aligned_face_47);
		PIM_Bitmap_Destroy(&aligned_face_67);
		PIM_Bitmap_Destroy(&aligned_face_95);
		return -4;
	}
	//extract_timer.Stop();

	PIM_Float fConf = 0.0;
	PIM_Int32 nMaxIndex = -1;
	PIM_Float fMaxConf = -9999999999.9;

	//recognition_timer.Start();
#if 1

#if 0
	float *maxf = (float *)malloc(sizeof(float)*nThread);
	memset(maxf, 0, sizeof(float)*nThread);

	int *maxI = (int *)malloc(sizeof(int)*nThread);
	memset(maxI, 0, sizeof(int)*nThread);
#else
	float maxf[NUM_THREADS] = { 0, };
	int maxI[NUM_THREADS] = { 0, };

#endif
	int tid;
	int i;

	//omp_timer.Start();
#pragma omp parallel private(fConf, tid)
	{
		tid = omp_get_thread_num();
#pragma omp for
		for (i = 0; i < m_fr_enroll.size; ++i)
		{

			fConf = likelihood_ratio(reduced_feature, m_fr_enroll.feature[i]);
			if (maxf[tid] < fConf)
			{
				maxf[tid] = fConf;
				maxI[tid] = i;
			}
		}

	}
	//omp_timer.Stop();

	for (int i = 0; i < NUM_THREADS; ++i)
	{
		if (fMaxConf < maxf[i])
		{
			fMaxConf = maxf[i];
			//nRecogIdx = maxI[i];
			nMaxIndex = maxI[i];
		}
	}

#if 0
	if (maxf != NULL)
	{
		free(maxf);
	}
	maxf = NULL;
	if (maxI != NULL)
	{
		free(maxI);
	}
	maxI = NULL;
#endif
#else


	for (register int i = 0; i < m_fr_enroll.size; i++)
	{

		fConf = likelihood_ratio(reduced_feature, m_fr_enroll.feature[i]);

		{
			if (fMaxConf <= fConf)
			{
				fMaxConf = fConf;
				nMaxIndex = i;
			}
		}

	}
#endif

	if (fMaxConf < FR_IDENTIFIER_THR)
	{
		nRecogIdx = -1;
	}
	else
	{
		nRecogIdx = m_fr_enroll.id[nMaxIndex];
	}

	nR1Idx = m_fr_enroll.id[nMaxIndex];

	//recognition_timer.Stop();

	*fConfidence = fMaxConf;
	*nRankOneID = nR1Idx;// reduced_enroll_info->id[nMaxIndex];

	//total_timer.Stop();

// 	printf("Align processing: %lf ms.\n", align_timer.MilliSeconds());
// 	printf("Extract processing: %lf ms.\n", extract_timer.MilliSeconds());
// 	printf("Recognition processing: %lf ms.\n", recognition_timer.MilliSeconds());
// 	printf("\tOpenMP processig: %lf ms.\n", omp_timer.MilliSeconds());
// 	printf("Total processing time: %lf ms.\n", total_timer.MilliSeconds());

	PIM_Bitmap_Destroy(&aligned_face_47);
	PIM_Bitmap_Destroy(&aligned_face_67);
	PIM_Bitmap_Destroy(&aligned_face_95);

	return nRecogIdx;
}


PIM_Int32 CFaceRecognizer_TL_JointBayesian::Get_Enroll_Candidate(PIM_Int32 *arCandidateIdx, PIM_Int32 nCandidateCnt, fr_enroll_info *reduced_enroll_info)
{
	int nTotalSubCnt = 0;

	for (int i = 0; i < nCandidateCnt; i++)
	{
		int nCnt = Get_Enroll_Cnt(arCandidateIdx[i]);
		nTotalSubCnt += nCnt;
	}

	reduced_enroll_info->size = nTotalSubCnt;
	reduced_enroll_info->person_count = nCandidateCnt;

	int k = 0;
	for (int i = 0; i < nCandidateCnt; i++)
	{
		for (int j = 0; j < m_fr_enroll.size && k < nTotalSubCnt; j++)
		{
			if (arCandidateIdx[i] == m_fr_enroll.id[j])
			{
				reduced_enroll_info->id[k] = m_fr_enroll.id[j];
				memcpy(reduced_enroll_info->feature[k], m_fr_enroll.feature[j], sizeof(PIM_Float)* PCA_FEATURE_DIM);
				memcpy(reduced_enroll_info->aligned_face[k], m_fr_enroll.aligned_face[j], sizeof(PIM_Uint8)*ALIGN_IMG_WIDTH*ALIGN_IMG_HEIGHT * 3);
				//strcpy(reduced_enroll_info->name[k], m_fr_enroll.name[j]);
				strcpy_s(reduced_enroll_info->name[k], 32, m_fr_enroll.name[j]);
				k++;
			}
		}
	}

	return nTotalSubCnt;
}


PIM_Float CFaceRecognizer_TL_JointBayesian::likelihood_ratio(PIM_Float *f1, PIM_Float *f2)
{
	PIM_Float temp1[PCA_FEATURE_DIM];
	PIM_Float temp2[PCA_FEATURE_DIM];
	PIM_Float temp3[PCA_FEATURE_DIM];

	// xAx';
	PIM_Float *pProjVec = A_sqrtlbp;
	PIM_Float *pProjVec2 = G_sqrtlbp;
	PIM_Float *pFeature1 = temp1;
	PIM_Float *pFeature2 = temp2;
	PIM_Float *pFeature3 = temp3;
	PIM_Float s1 = 0.0;
	PIM_Float s2 = 0.0;
	PIM_Float s3 = 0.0;

	for (register int bn = 0; bn < PCA_FEATURE_DIM; bn++)
	{
		PIM_Float *pULBPFeatureVec1 = f1;
		PIM_Float *pULBPFeatureVec2 = f2;

		*pFeature1 = 0;
		*pFeature2 = 0;
		*pFeature3 = 0;

		for (register int bn2 = 0; bn2 < PCA_FEATURE_DIM; bn2++)
		{
			*pFeature1 += *pULBPFeatureVec1 * *pProjVec;
			*pFeature2 += *pULBPFeatureVec2++ * *pProjVec++;
			*pFeature3 += *pULBPFeatureVec1++ * *pProjVec2++;
		}
		s1 += *pFeature1 * f1[bn];
		s2 += *pFeature2 * f2[bn];
		s3 += *pFeature3 * f2[bn];

		pFeature1++;
		pFeature2++;
		pFeature3++;
	}

	return s1 + s2 - 2 * s3;
}




