#include "stdafx.h"

#include "FGlobalRegressor.h"

CFGlobalRegressor::CFGlobalRegressor() {
	W = NULL;
}

CFGlobalRegressor::~CFGlobalRegressor() {
	if (W) delete []W;
}

void CFGlobalRegressor::set_num_stage(int t) {
	T = t;
	W = new CFMat[T];
}

void CFGlobalRegressor::set_weight(CFMat &w, int stage) {
	W[stage].CopyFrom(w);
}
void CFGlobalRegressor::size(int stage, int &h, int &w) {
	h = W[stage].Row();
	w = W[stage].Col();
}

CFMat CFGlobalRegressor::update_residual(int stage, CIMat &local_binary_features) {
	int i, j;
	CFMat &W_stage = W[stage];
	float *_w;
	//int *pLBF;
	//float *pDst;

	CFMat _res(W_stage.Col(), 1, 0);
	for (i = 0; i < local_binary_features.Row(); i++) {
		_w = W_stage.RowD(local_binary_features.m_data[i]);

		float *pDst = _res.m_data;
		for (j = 0; j < W_stage.Col(); j++) {
			*pDst++ += *_w++;
		}
	}

	return _res;

}

CFMat *CFGlobalRegressor::get_weight(int stage) {
	return &W[stage];
}