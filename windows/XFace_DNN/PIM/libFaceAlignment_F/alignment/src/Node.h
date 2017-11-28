#ifndef _NODE_H_
#define _NODE_H_

#include "LBFCommon.h"
#include <vector>
using namespace std;


class CNode {
public :
	bool bLeaf;
	int depth;
	float x1, x2, y1, y2, thr;
	int index;		// tree ������ �� node�� index
	short leaf_index;							// leaf node�϶� leaf node�� ������ ��Ÿ�� (tree���� ���� ���� 1)
	CNode *m_left_node, *m_right_node;
	CNode *m_upper_node;
	vector<int> sampleIndexVector;
	float delta_x, delta_y;

public :
	CNode();
	~CNode();

	int split(data_info &data, int landmark_index);
};

#endif