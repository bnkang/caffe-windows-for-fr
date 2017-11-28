#include "stdafx.h"

#include "LBFCommon.h"
//#include "cv.h"
//#include "highgui.h"
// #include "ImageProcessing.h"
// #include "ImageProcess.h"

#include "../../common/src/FA_ImageProcess.h"

// #include <io.h>
#include <stdio.h>
#include "FMat.h"

//using namespace cv;



CLBFCommon::CLBFCommon() {

}

CLBFCommon::~CLBFCommon() {

}



/*
@brief similarity transform의 paramter를 구한다. (from s0 to s1)
@param[in]	"CFMat"		&s1			- 입력 좌표
@param[in]	"CFMat"		&s0			- reference 좌표
@param[in]	"float"	q			- similarity transform의 parameter
*/
void CLBFCommon::compute_similarity(CFMat& s1, CFMat& s0, float *q)
{
	int vcnt = s1.Row();
	// B = AX
	CFMat A;
	A.Create(vcnt * 2, 4, (float)0);
	CFMat  B, X;

	B.Wrap(vcnt * 2, 1, s1.m_data);
	X.Wrap(4, 1, q);

	float *row, x0, y0;
	for (int i = 0; i<vcnt; i++) {
		x0 = s0.m_data[i * 2];
		y0 = s0.m_data[i * 2 + 1];

		row = A.m_data + (i * 2) * 4;
		row[0] = x0; row[1] = y0; row[2] = 1; row[3] = 0;

		row = A.m_data + (i * 2 + 1) * 4;
		row[0] = y0; row[1] = -x0; row[2] = 0; row[3] = 1;
	}


	SolveLSE(A, B, X);

	//q[0] -= 1;

	return;
}

/*
@brief similarity transform parameter의 inverse를 구한다.
*/
void CLBFCommon::compute_inverse_similarity(float *q2, float *inv_q2)
{
	CFMat similarity_param, inv_similarity_param;
	similarity_param.Create(3, 3);

	similarity_param[0][0] = q2[0];		similarity_param[0][1] = q2[1];		similarity_param[0][2] = q2[2];
	similarity_param[1][0] = -q2[1];	similarity_param[1][1] = q2[0];		similarity_param[1][2] = q2[3];
	similarity_param[2][0] = 0;			similarity_param[2][1] = 0;			similarity_param[2][2] = 1;

	inv_similarity_param.Create(3, 3);
	inv_similarity_param.IsInverse(similarity_param);

	inv_q2[0] = inv_similarity_param[0][0];
	inv_q2[1] = inv_similarity_param[0][1];
	inv_q2[2] = inv_similarity_param[0][2];
	inv_q2[3] = inv_similarity_param[1][2];
}

/*
@brief apply similarity transform to s0, and save the result to s1
*/
void CLBFCommon::apply_similarity_transform(CFMat& s0, CFMat& s1, float *q2)
{
	int vcnt = s0.Row();

	if (s0.Row() != s1.Row() || s0.Col() != s1.Col()) {
		s1.Create(s0.Row(), s0.Col());
	}

	float *src = s0.m_data, *dst = s1.m_data;
	float src_x, src_y;
	int i;

	// SIMD를 해볼려고 했으나 잘 안됨
#if 1	
	float a = q2[0];
	float b = q2[1];
	float mb = -q2[1];
	float tx = q2[2];
	float ty = q2[3];
	for (i = 0; i < vcnt; i++) 
	{
		src_x = *src++;
		src_y = *src++;
		*dst++ = a * src_x + b * src_y + tx;
		*dst++ = mb * src_x + a * src_y + ty;
	}
#else
	CFMat s0_x(s0.Row(), 1), s0_y(s0.Row(), 1);
	CFMat s1_x(s0.Row(), 1), s1_y(s0.Row(), 1);
	float *dst_x = s0_x.m_data;
	float *dst_y = s0_y.m_data;
	float *src = s0.m_data;
	/*for (i = 0; i < vcnt; i++) {
		*dst_x++ = *src++;
		*dst_y++ = *src++;
	}*/

	__m128 xmm_x, xmm_y, xmm_a, xmm_b, xmm_mb, xmm_tx, xmm_ty;
	xmm_a = _mm_set1_ps(q2[0]);
	xmm_b = _mm_set1_ps(q2[1]);
	xmm_mb = _mm_set1_ps(-q2[1]);
	xmm_tx = _mm_set1_ps(q2[2]);
	xmm_ty = _mm_set1_ps(q2[3]);
	dst_x = s1_x.m_data;
	dst_y = s1_y.m_data;
	float *src_x = s0.m_data;
	float *src_y = s0.m_data + 1;
	for (i = 0; i < vcnt - 3; i += 4) 
	{
		float vector_x[] = { *(src_x), *(src_x + 2), *(src_x + 4), *(src_x + 6)};
		float vector_y[] = { *(src_y), *(src_y + 2), *(src_y + 4), *(src_y + 6) };
		xmm_x = _mm_loadu_ps(vector_x);
		xmm_y = _mm_loadu_ps(vector_y);
		__m128 m1 = _mm_mul_ps(xmm_a, xmm_x);
		__m128 m2 = _mm_fmadd_ps(xmm_b, xmm_y, m1);
		m2 = _mm_add_ps(m2, xmm_tx);
		_mm_storeu_ps(dst_x, m2);


		m1 = _mm_mul_ps(xmm_mb, xmm_x);
		m2 = _mm_fmadd_ps(xmm_a, xmm_y, m1);
		m2 = _mm_add_ps(m2, xmm_ty);
		_mm_storeu_ps(dst_y, m2);


		src_x += 8;
		src_y += 8;
		dst_x += 4;
		dst_y += 4;

		/*src_x = *src;
		src_y = *(src + 1);
		*dst = q2[0] * src_x + q2[1] * src_y + q2[2];
		*(dst + 1) = -q2[1] * src_x + q2[0] * src_y + q2[3];
		dst += 2;
		src += 2;*/
	}
	for (; i < vcnt; i++) 
	{
		*dst_x++ = q2[0] * *src_x + q2[1] * *src_y + q2[2];
		*dst_y++ = -q2[1] * *src_x + q2[0] * *src_y + q2[3];
		src_x++;
		src_y++;
	}

	// copy
	float *dst = s1.m_data;
	src_x = s1_x.m_data;
	src_y = s1_y.m_data;
	for (i = 0; i < vcnt; i++) 
	{
		*dst++ = *src_x++;
		*dst++ = *src_y++;
	}
#endif
}

/*
@brief shape의 bounding box를 계산한다.
*/
FMyRect CLBFCommon::compute_bbox(CFMat &pts) 
{
	int i;
	FMyRect rect;
	float *dst = pts.m_data;
	rect.left = INT_MAX;
	rect.right = INT_MIN;
	rect.top = INT_MAX;
	rect.bottom = INT_MIN;
	for( i = 0; i < VCNT; i++ )
	{
		if( rect.left > *dst )
		{
			rect.left = *dst;
		}
		if( rect.right < *dst )
		{
			rect.right = *dst;
		}
		dst++;
		if( rect.top > *dst )
		{
			rect.top = *dst;
		}
		if( rect.bottom < *dst )
		{
			rect.bottom = *dst;
		}
		dst++;
	}

	return rect;
}


