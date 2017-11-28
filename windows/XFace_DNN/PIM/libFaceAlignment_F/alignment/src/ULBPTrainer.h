#ifndef _ULBP_TRAINER_H_
#define _ULBP_TRAINER_H_

#include "LBFTrainer.h"
#include "LBFCommon.h"
#include "FMat.h"
#include "IMat.h"
#include "Forest.h"
#include <vector>
#include "FGlobalRegressor.h"

using namespace std;

#include "LBFTrainer.h"

class CULBPTrainer : public CLBFTrainer {
public:
	CULBPTrainer();
	virtual ~CULBPTrainer();

	void train(char **training_data_path, int path_cnt, char *output_path);
private:
	void translate_mean_shape();
	void set_normalized_img_buffer();
	void compute_residual();
	void normalize_face(data_info &data);
	void train_random_forest();
	void train_tree(CTree &tree, int stage, int landmark_index, int *ulbp_histogram);
	void train_node(vector<CNode *> &stack, int stage, int landmark_index, int *ulbp_histogram);
	void compute_ULBP_histogram(vector<int> data_vector, int landmark_index, int stage, int *ulbp);


};

#endif