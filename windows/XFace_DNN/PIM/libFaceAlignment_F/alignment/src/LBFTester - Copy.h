#ifndef _LBF_TESTER_H_
#define _LBF_TESTER_H_

#include "FMat.h"
#include "Forest.h"
#include "IGlobalRegressor.h"
#include "FGlobalRegressor.h"
#include "FaceRejector.h"

#include "../../common_opencv_2.1/include/cv.h"

class CLBFTester {
public:
	CLBFTester();
	~CLBFTester();

public:
	int img_detect(img_info &img);

	int detect_img(img_info &img);
	int detect(img_info &img);
	int detect3(img_info &img);
		
	int track(img_info &img);
	int track3(img_info &img);

	void get_fd_result(int idx, SMyRect &rect);
	bool get_alignment_result(int idx, CFMat &result);
	void reinitialize();
	bool get_pose(float &pitch, float &yaw, float &roll);

private:
	void create_child_nodes(CNode *root, int node_cnt, short *child_index, float *tree_fern_params, short *threshold_params, short *leaf_index);
	void create_child_nodes(CTree *tree, int node_cnt, short *child_index, float *tree_fern_params, short *threshold_params, short *leaf_index);
	int DetectFaceEye(unsigned char *src, int width, int height);
	void initialize_data(data_info &data, int idx);
	bool align(data_info &data);
	bool verify(img_info &img);
	void compute_local_binary_feature(data_info &data, CForest *stage_forest, int *local_binary_features);
	void compute_pose(CFMat &pts);
	void resize_face_rect(data_info &data, int idx);
	void back_face_rect(data_info &data, int idx);
	int CorrectEyeInfo(FD_INFO &fd_info);

private:
	CFMat mean_shape;
	FMyRect mean_shape_bound;
	CForest *m_forest;
	CIGlobalRegressor m_global_regressor;
	CFaceRejector m_face_rejector;

	CvPOSITObject *positObject;
	float m_rotation_matrix[9];
	float m_translation_vector[3];


	int TRACKING_STATUS = 0;

	int frame_count = 0;

	bool b_img_resized;
	float img_resized_ratio;

public:
	data_info m_data[MAX_FACE_NUM];
	data_info m_prev_data[MAX_FACE_NUM];

	FD_INFO fd_info;
};

#endif