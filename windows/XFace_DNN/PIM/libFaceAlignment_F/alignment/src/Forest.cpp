#include "stdafx.h"

#include "Forest.h"

CForest::CForest() {
}

void CForest::initialize(int numberofTrees, int vcnt) {
	m_nTrees = numberofTrees;
	m_vcnt = vcnt;

	m_trees = new CTree*[vcnt];

	for (int i = 0; i < vcnt; i++) {
		m_trees[i] = new CTree[numberofTrees];
	}
}

CForest::~CForest() {
	for (int i = 0; i < m_vcnt; i++) {
		delete []m_trees[i];
	}
	delete []m_trees;
}

void CForest::compute_residual(unsigned char *src, int width, int height, float *q, float current_x, float current_y, float &residual_x, float &residual_y)
{
	/*int i = m_nTrees;

	float result = 0;
	float *temp_residual_x = new float[m_nTrees];
	float *temp_residual_y = new float[m_nTrees];

	residual_x = 0; residual_y = 0;
	for (i = 0 ; i < m_nTrees ; i++) {
		m_trees[i].compute_residual(src, width, height, q, current_x, current_y, temp_residual_x[i], temp_residual_y[i]);
		residual_x += temp_residual_x[i];
		residual_y += temp_residual_y[i];
	}
	residual_x /= m_nTrees;
	residual_y /= m_nTrees;

	delete temp_residual_x;
	delete temp_residual_y;*/
}

CTree *CForest::GetTree(int landmark_index, int tree_index) {
	return m_trees[landmark_index] + tree_index;
	//return m_trees + idx;
	//return NULL;
}

void CForest::set_local_binary_feature_dim(int dim) {
	lbf_dim = dim;
}