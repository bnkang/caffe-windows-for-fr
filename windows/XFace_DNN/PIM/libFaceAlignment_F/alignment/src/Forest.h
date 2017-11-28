#ifndef _FOREST_H_
#define _FOREST_H_

#include "Forest.h"
#include "Tree.h"

class CForest {
public :
	CForest();
	~CForest();

public:
	void initialize(int numberofTrees, int vcnt);
	CTree *GetTree(int landmark_index, int tree_index);
	void compute_residual(unsigned char *src, int width, int height, float *q, float current_x, float current_y, float &residual_x, float &residual_y);
	void set_local_binary_feature_dim(int dim);

public:
	CTree **m_trees;
	int m_nTrees; 
	int m_vcnt;
	int lbf_dim;
};

#endif
