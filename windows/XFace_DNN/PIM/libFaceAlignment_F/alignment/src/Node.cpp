#include "stdafx.h"

#include "Node.h"

CNode::CNode() {
	bLeaf = 1;
	depth = -1;
	m_left_node = NULL;
	m_right_node = NULL;
	m_upper_node = NULL;
	x1 = -1;
	x2 = -1;
	y1 = -1;
	y2 = -1;
	thr = 0;
	leaf_index = -1;
	index = -1;
}

CNode::~CNode() {

}

int CNode::split(data_info &data, int landmark_index) {
	CFMat norm_xy, transformed_xy1, transformed_xy2;
	norm_xy.Create(1, 2);
	transformed_xy1.Create(1, 2);
	transformed_xy2.Create(1, 2);

	norm_xy.m_data[0] = this->x1;
	norm_xy.m_data[1] = this->y1;
	CLBFCommon::apply_similarity_transform(norm_xy, transformed_xy1, data.q);

	norm_xy.m_data[0] = this->x2;
	norm_xy.m_data[1] = this->y2;
	CLBFCommon::apply_similarity_transform(norm_xy, transformed_xy2, data.q);

	transformed_xy1.m_data[0] += data.current_pts.m_data[landmark_index * 2] - data.q[2];
	transformed_xy1.m_data[1] += data.current_pts.m_data[landmark_index * 2 + 1] - data.q[3];
	transformed_xy2.m_data[0] += data.current_pts.m_data[landmark_index * 2] - data.q[2];
	transformed_xy2.m_data[1] += data.current_pts.m_data[landmark_index * 2 + 1] - data.q[3];

	int x1 = fa_max(0, fa_min(int(transformed_xy1.m_data[0]), data.pimg->img_width - 1));
	int y1 = fa_max(0, fa_min(int(transformed_xy1.m_data[1]), data.pimg->img_height - 1));
	int x2 = fa_max(0, fa_min(int(transformed_xy2.m_data[0]), data.pimg->img_width - 1));
	int y2 = fa_max(0, fa_min(int(transformed_xy2.m_data[1]), data.pimg->img_height - 1));

	int diff = (int)(data.pimg->img[y1 * data.pimg->img_width + x1])
		- (int)(data.pimg->img[y2 * data.pimg->img_width + x2]);

	if (diff < thr)
		return -1;
	else
		return 1;
}