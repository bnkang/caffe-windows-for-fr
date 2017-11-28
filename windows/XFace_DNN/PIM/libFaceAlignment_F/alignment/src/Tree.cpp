#include "stdafx.h"

#include "Tree.h"

CTree::CTree() {
	root.depth = 1;
	root.m_left_node = NULL;
	root.m_right_node = NULL;
	root.m_upper_node = NULL;

	depth_list = NULL;
	leaf_index_list = NULL;
	fern_param_list = NULL;
	threshold_list = NULL;
	tree_children_list = NULL;
	node_cnt = 1;
}

CTree::~CTree() {
	release(*root.m_left_node);
	release(*root.m_right_node);
}

void CTree::release(CNode &node) {

	if (&node)
	{
		if (node.m_left_node) {
			release(*node.m_left_node);
			node.m_left_node = NULL;
		}

		if (node.m_right_node) {
			release(*node.m_right_node);
			node.m_right_node = NULL;
		}
	}

	delete &node;
}

/*
 * 영살을 주고 하나의 Tree에 대해서 regression 값을 얻는다.
 */
CFMat CTree::compute_delta(data_info &data, int landmark_index) {
	CFMat norm_xy, transformed_xy1, transformed_xy2;
	norm_xy.Create(1, 2);
	transformed_xy1.Create(1, 2);
	transformed_xy2.Create(1, 2);

	CNode *node = &root;
	while (node->bLeaf != 1) {
		int s = node->split(data, landmark_index);

		if (s == -1) { // left
			node = node->m_left_node;
		}
		else  { // right
			node = node->m_right_node;
		}
	}

	CFMat delta(1, 2);
	delta.m_data[0] = node->delta_x;
	delta.m_data[1] = node->delta_y;

	return delta;
}

/*
@brief local binary feature를 계산해서 최종적으로 도달한 leaf node에 해당하는 local binary feature의 index를 구한다.
*/
int CTree::compute_local_binary_feature(data_info &data, int landmark_index) 
{
	if (tree_children_list == NULL) // tree_children_list가 pointer인데 NULL 인 경우는 학습할 때임
	{
		CFMat norm_xy, transformed_xy1, transformed_xy2;
		norm_xy.Create(1, 2);
		transformed_xy1.Create(1, 2);
		transformed_xy2.Create(1, 2);

		CNode *node = &root;
		while (node->bLeaf != 1) 
		{
			int s = node->split(data, landmark_index);

			if (s == -1)  // left
			{
				node = node->m_left_node;
			}
			else // right
			{
				node = node->m_right_node;
			}
		}

		return node->leaf_index;
	}
	else // NULL이 아닌 경우는 테스트 할 때임. 이 pointer가 tree 내 모든 node들의 pixel difference를 구하기 위한 xy 좌표들의 pointer임. 한번에 transform 하기 위함
	{
		CFMat norm_xy, transformed_xy;
		norm_xy.Wrap(this->node_cnt * 2, 2, this->fern_param_list);
		transformed_xy.Create(this->node_cnt * 2, 2);
		float new_q[4];
		memcpy(new_q, data.q, sizeof(float) * 4);
		new_q[2] += data.current_pts.m_data[landmark_index * 2] - data.q[2];
		new_q[3] += data.current_pts.m_data[landmark_index * 2 + 1] - data.q[3];
		CLBFCommon::apply_similarity_transform(norm_xy, transformed_xy, new_q);

		int index = 0;
		short *left_index;
		while (1) 
		{
			left_index = tree_children_list + index * 2;
			if (*left_index == -1) 
			{
				break;
			}

			float *pts = transformed_xy.RowD(index * 2);
			int x1 = fa_max(0, fa_min(int(*pts), data.pimg->img_width - 1));
			int y1 = fa_max(0, fa_min(int(*(pts + 1)), data.pimg->img_height - 1));
			int x2 = fa_max(0, fa_min(int(*(pts + 2)), data.pimg->img_width - 1));
			int y2 = fa_max(0, fa_min(int(*(pts + 3)), data.pimg->img_height - 1));

			int diff = (int)(data.pimg->img[y1 * data.pimg->img_width + x1])
				- (int)(data.pimg->img[y2 * data.pimg->img_width + x2])
				- this->threshold_list[index];
			

			// left
			if (diff < 0) 
			{
				index = *left_index;
			}
			else // right
			{
				index = *(left_index + 1);
			}
		}

		return leaf_index_list[index];
	}
}

/*
@brief get total number of nodes
*/
int	CTree::total_number_of_nodes() {
	int total_num = 1;

	CNode *node = &root;

	if (node->m_left_node) {
		total_num += get_node_num(node->m_left_node);
	}
	if (node->m_right_node) {
		total_num += get_node_num(node->m_right_node);
	}

	return total_num;
}

int	CTree::get_node_num(CNode *node) {
	int total_num = 0;
	if (node->m_left_node == NULL && node->m_right_node == NULL)
		return 1;
	else
	{
		total_num = 1;
		total_num += get_node_num(node->m_left_node);
		total_num += get_node_num(node->m_right_node);
		return total_num;
	}
}
