#ifndef _LBF_TRAINER_H_
#define _LBF_TRAINER_H_

#include "LBFCommon.h"
#include "FMat.h"
#include "IMat.h"
#include "Forest.h"
#include <vector>
#include "FGlobalRegressor.h"

using namespace std;

class CLBFTrainer {
public:
	CLBFTrainer();
	virtual ~CLBFTrainer();
	void train(char *training_data_path, char *output_path);
	void train(char **training_data_path, int path_cnt, char *output_path);

protected:
	void collect_data(char *data_path);
	void collect_data(char **data_paths, int path_cnt);
	void compute_mean_shape();
	void procrustes(CFMat &dst_mean_shape, vector<data_info> &data);
	void show_mean_shape();
	void Initialize_shape();
	void augment_data();
	void compute_residual();
	void train_random_forest();
	void train_tree(CTree &tree, int stage, int landmark_index);
	void train_node(vector<CNode *> &stack, int stage, int landmark_index);
	float compute_variance(vector<int> &sample_idx, int landmark_index);
	void generate_fern_pool(int stage, CFMat &rand_xy1, CFMat &rand_xy2);
	void generate_fern_pool();
	void compute_transformed_location(data_info &data, CFMat &xy1, CFMat &xy2, CFMat &t_xy1, CFMat &t_xy2, int landmark_index);
	void compute_delta(CNode &node, int landmark_index);
	void update_by_random_forest(int stage);
	void set_local_binary_feature_index(int stage);
	void set_leaf_node_index(vector<CNode *> &stack, int &index);
	void set_node_index(CNode *node, int &index);
	void train_global_regression(int stage);
	void compute_local_binary_feature(data_info &data, CForest *stage_forest, int *local_binary_features);
	void update_by_global_regression(int stage);
	void update_by_global_regression(int stage, data_info &data);
	void update_shape_residual(int stage, data_info &data);
	void save_parameters(char *output_path);
	void measure();
	void make_tree_to_vector(vector <CNode *> &node_vector, CNode *node);
	int  DetectFaceEye(unsigned char *src, int width, int height);
	void save_result_images();
	void flip_data(img_info &src, img_info &dst);
	void show_data(data_info &data);

protected:
	vector<img_info> training_images;
	vector<data_info> training_data;
	vector<float> fern_angles, fern_radius;
	vector<int> fern_pool_sample_index1, fern_pool_sample_index2;

	CFMat mean_shape;
	FMyRect mean_shape_bound;
	CForest *m_forest;
	CFGlobalRegressor m_global_regressor;

};

#endif