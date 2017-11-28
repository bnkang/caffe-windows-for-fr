#ifndef _TREE_H_
#define _TREE_H_

#include "Node.h"
//#include "define_op.h"
#include "../../../exp/PIM_Define_op.h"

#include "FMat.h"
#include "LBFCommon.h"

class CTree {
public:
	CTree();
	~CTree();

	//int compute_delta(unsigned char *src, int img_width, int img_height, float *q, CFMat &current_pts);
	CFMat	compute_delta(data_info &data, int landmark_index);
	int		compute_local_binary_feature(data_info &data, int landmark_index);
	int		total_number_of_nodes();

private:
	void release(CNode &node);
	int	 get_node_num(CNode *node);

public:
	CNode root;

	short *depth_list;
	short *leaf_index_list;
	float *fern_param_list;
	short *threshold_list;
	short *tree_children_list;
	int node_cnt;
};

#endif