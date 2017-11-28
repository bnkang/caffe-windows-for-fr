#include "stdafx.h"
#include "FaceRejector.h"
#include "../../common/src/FA_ImageProcess.h"
#include "Face_Rejector_parameters.h"

CFaceRejector::CFaceRejector() {
	int t;
	unsigned short *pLBPFeatureY, *pLBPFeatureX;
	pLBPFeatureY = m_nFaceRejectorLBPFeatureY, pLBPFeatureX = m_nFaceRejectorLBPFeatureX;
	nLBPFeatureGlobalPosition = new int[FACE_REJECTOR_LBP_FEATURE_SIZE];
	int *pLBPFeatureGlobalPosition = nLBPFeatureGlobalPosition;

	for (t = 0; t<FACE_REJECTOR_LBP_FEATURE_SIZE;) {
		*pLBPFeatureGlobalPosition++ = *pLBPFeatureY++ * FACE_REJECTOR_LBP_WINDOW_WIDTH + *pLBPFeatureX++;
		*pLBPFeatureGlobalPosition++ = *pLBPFeatureY++ * FACE_REJECTOR_LBP_WINDOW_WIDTH + *pLBPFeatureX++;
		t += 2;
	}
}

CFaceRejector::~CFaceRejector() {
	free(nLBPFeatureGlobalPosition);
}

bool CFaceRejector::is_face(data_info &data) {
	unsigned char *norm_img = (unsigned char *)malloc(sizeof(unsigned char) * 20 * 20);

	if (normalize(data, norm_img)) {
		bool b_face = classify(norm_img);
		free(norm_img);

		return b_face;
	}
	else {
		free(norm_img);
		return false;
	}
}

bool CFaceRejector::normalize(data_info &data, unsigned char *des_data) {
	int i;
	float left_eye[2] = { 0, }, right_eye[2] = { 0, };
	for (i = 0; i < LEFT_EYE_IDX_CNT; i++) {
		left_eye[0] += data.current_pts.m_data[LEFT_EYE_IDX[i] * 2];
		left_eye[1] += data.current_pts.m_data[LEFT_EYE_IDX[i] * 2 + 1];
	}
	left_eye[0] /= LEFT_EYE_IDX_CNT;
	left_eye[1] /= LEFT_EYE_IDX_CNT;

	for (i = 0; i < RIGHT_EYE_IDX_CNT; i++) {
		right_eye[0] += data.current_pts.m_data[RIGHT_EYE_IDX[i] * 2];
		right_eye[1] += data.current_pts.m_data[RIGHT_EYE_IDX[i] * 2 + 1];
	}
	right_eye[0] /= RIGHT_EYE_IDX_CNT;
	right_eye[1] /= RIGHT_EYE_IDX_CNT;

	SMyPoint le, re;
	le.x = left_eye[0];
	le.y = left_eye[1];
	re.x = right_eye[0];
	re.y = right_eye[1];

	double eye_error = sqrt(double((le.x - re.x) * (le.x - re.x) + (le.y - re.y) * (le.y - re.y)));
	//printf("Eye error = %f\n", eye_error);
	if( eye_error < 10.0 ) {
		return false;
	}

	FMyRect norm_box = CLBFCommon::compute_bbox(data.current_norm_pts);
	CFMat bound_rect(4, 2), t_bound_rect(4, 2);
	bound_rect.m_data[0 * 2] = norm_box.left;
	bound_rect.m_data[0 * 2 + 1] = norm_box.top;
	bound_rect.m_data[1 * 2] = norm_box.right;
	bound_rect.m_data[1 * 2 + 1] = norm_box.top;
	bound_rect.m_data[2 * 2] = norm_box.left;
	bound_rect.m_data[2 * 2 + 1] = norm_box.bottom;
	bound_rect.m_data[3 * 2] = norm_box.right;
	bound_rect.m_data[3 * 2 + 1] = norm_box.bottom;


	CLBFCommon::apply_similarity_transform(bound_rect, t_bound_rect, data.q);

	CFMat norm_rect(4, 2);
	norm_rect.m_data[0 * 2] = 0;
	norm_rect.m_data[0 * 2 + 1] = 0;
	norm_rect.m_data[1 * 2] = 20;
	norm_rect.m_data[1 * 2 + 1] = 0;
	norm_rect.m_data[2 * 2] = 0;
	norm_rect.m_data[2 * 2 + 1] = 20;
	norm_rect.m_data[3 * 2] = 20;
	norm_rect.m_data[3 * 2 + 1] = 20;


	CFMat A(8, 6);
	for (i = 0; i < 4; i++) {
		float *dst = A.RowD(i * 2);
		*dst = norm_rect.m_data[i * 2];
		*(dst + 1) = norm_rect.m_data[i * 2 + 1];
		*(dst + 2) = 0;
		*(dst + 3) = 0;
		*(dst + 4) = 1;
		*(dst + 5) = 0;

		*(dst + 6) = 0;
		*(dst + 7) = 0;
		*(dst + 8) = norm_rect.m_data[i * 2];
		*(dst + 9) = norm_rect.m_data[i * 2 + 1];
		*(dst + 10) = 0;
		*(dst + 11) = 1;
	}

	CFMat B(8, 1);
	B.m_data[0] = t_bound_rect.m_data[0 * 2];
	B.m_data[1] = t_bound_rect.m_data[0 * 2 + 1];
	B.m_data[2] = t_bound_rect.m_data[1 * 2];
	B.m_data[3] = t_bound_rect.m_data[1 * 2 + 1];
	B.m_data[4] = t_bound_rect.m_data[2 * 2];
	B.m_data[5] = t_bound_rect.m_data[2 * 2 + 1];
	B.m_data[6] = t_bound_rect.m_data[3 * 2];
	B.m_data[7] = t_bound_rect.m_data[3 * 2 + 1];

	CFMat x(6, 1);
	SolveLSE(A, B, x);

	{
		memset(des_data, 0, 20 * 20);

		double src_x, src_y;
		double a, b;
		int l, k, src, src_l, src_k, src_lk, src_w_step, des_w_step = 0;
		int src_w = data.pimg->img_width;
		unsigned char *pdes = des_data;
		unsigned char *src_data = data.pimg->img;
		for (int des_y = 0; des_y < 20; des_y++) {
			for (int des_x = 0; des_x < 20; des_x++) {
				src_x = x.m_data[0] * des_x + x.m_data[1] * des_y + x.m_data[4];
				src_y = x.m_data[2] * des_x + x.m_data[3] * des_y + x.m_data[5];

				if (src_x < 0 || src_x >= data.pimg->img_width - 1 || src_y < 0 || src_y >= data.pimg->img_height - 1)
				{
					*pdes++ = 0;
				}
				else
				{
					l = (int)(src_x);
					k = (int)(src_y);
					a = src_x - l;
					b = src_y - k;
					src_w_step = k*data.pimg->img_width;
					src = src_data[src_w_step + l];
					src_l = src_data[src_w_step + l + 1];
					src_k = src_data[src_w_step + src_w + l];
					src_lk = src_data[src_w_step + src_w + l + 1];

					*pdes++ = (unsigned char)(src + a*(src_l - src) + b*(src_k - src) +
						a*b*(src + src_lk - src_l - src_k));
				}
			}
		}
	}


	//face_normalization_20_20(data.pimg->img, des_data, data.pimg->img_width, data.pimg->img_height, le, re);
	return true;
}

bool CFaceRejector::classify(unsigned char* nSrcData)
{
	int t;
	int is_face;
	int nLBPAlphaCount = 0;
	int nLBPConfidence, nLBPOneCascadeFeatureSize;
	unsigned char *nLBPData;
	unsigned char *pLBPData;
	//unsigned short *pLBPFeatureY, *pLBPFeatureX;
	int *pLBPFeatureGlobalPosition = nLBPFeatureGlobalPosition;
	nLBPData = (unsigned char*)malloc(sizeof(unsigned char)*FACE_REJECTOR_LBP_WINDOW_WIDTH*FACE_REJECTOR_LBP_WINDOW_HEIGHT);
	Image2LBP8_1(nSrcData, FACE_REJECTOR_LBP_WINDOW_WIDTH, FACE_REJECTOR_LBP_WINDOW_HEIGHT, nLBPData);
	pLBPData = nLBPData;

	nLBPAlphaCount = 0;

	is_face = false;

	nLBPConfidence = 0;
	nLBPOneCascadeFeatureSize = FACE_REJECTOR_LBP_FEATURE_SIZE;

	// 각 cascade에 해당하는 feature들의 MCT값에 대한 합을 구함
	for (t = 0; t<nLBPOneCascadeFeatureSize;) {
		nLBPConfidence += m_nFaceRejectorLBPAlpha[(nLBPAlphaCount + *(pLBPData + *pLBPFeatureGlobalPosition++))];
		nLBPAlphaCount += FACE_REJECTOR_LBP_VALUE_RANGE;

		nLBPConfidence += m_nFaceRejectorLBPAlpha[(nLBPAlphaCount + *(pLBPData + *pLBPFeatureGlobalPosition++))];
		nLBPAlphaCount += FACE_REJECTOR_LBP_VALUE_RANGE;
		t += 2;
	}

	//printf("face confidence - %d\n", nLBPConfidence);
	if (nLBPConfidence < FACE_REJECTOR_LBP_THRESHOLD + 4000)
		is_face = true;

	free(nLBPData);
	

	return is_face;
}
