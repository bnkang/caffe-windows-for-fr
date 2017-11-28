#include "stdafx.h"

#include "IGlobalRegressor.h"
#include <mmintrin.h>	//MMX
#include <xmmintrin.h>	//SSE
#include <emmintrin.h>	//SSE2
#include <tmmintrin.h>	//SSSE3
#include <smmintrin.h>	//SSE4.1
#include <immintrin.h>	//AVX

CIGlobalRegressor::CIGlobalRegressor() {
	W = NULL;
}

CIGlobalRegressor::~CIGlobalRegressor() {
	if (W) delete []W;
}

void CIGlobalRegressor::set_num_stage(int t) {
	T = t;
	W = new CIMat[T];
}

void CIGlobalRegressor::set_weight(CIMat &w, int stage) {
	//W[stage].Wrap(w.Row(), w.Col(), w.m_data);
	W[stage].Create(w.Row(), w.Col(), w.m_data);
}
void CIGlobalRegressor::size(int stage, int &h, int &w) {
	h = W[stage].Row();
	w = W[stage].Col();
}

CFMat CIGlobalRegressor::update_residual(int stage, CIMat &local_binary_features) 
{
	int i, j;
	CIMat *W_stage = W + stage;
	int *_w;
	//int *pLBF;

	CFMat _res(W_stage->Col(), 1);
	CIMat _res_int(W_stage->Col(), 1, 0);

	for( i = 0; i < local_binary_features.Row(); i++ )
	{
		_w = W_stage->RowD(local_binary_features.m_data[i]);

		int *pDst = _res_int.m_data;

#if 1		
		for (j = 0; j < W_stage->Col(); j++) 
		{
			*pDst++ += *_w++;
		}
		
#else
		// SSE
		__m128i xmm_1, xmm_2;
		for (j = 0; j < W_stage->Col() - 3; j += 4) 
		{
			xmm_1 = _mm_loadu_si128((__m128i *)_w);
			xmm_2 = _mm_loadu_si128((__m128i *)pDst);
			xmm_2 = _mm_add_epi32(xmm_1, xmm_2);
			_mm_storeu_si128((__m128i *)pDst, xmm_2);

			_w += 4;
			pDst += 4;
		}
		for (; j < W_stage->Col(); j++) 
		{
			*pDst++ += *_w++;
		}
		
#endif
	}

	float _denom = (float)(2 << 20);
	for (i = 0; i < W_stage->Col(); i++) 
	{
		_res.m_data[i] = _res_int.m_data[i] / _denom;
	}

	return _res;
}

CIMat *CIGlobalRegressor::get_weight(int stage) {
	return &W[stage];
}