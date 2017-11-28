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
	int index;		// tree 내에서 각 node의 index
	short leaf_index;							// leaf node일때 leaf node의 순서를 나타냄 (tree에서 왼쪽 부터 1)
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