#include "ULBPTrainer.h"
#include "cv.h"
#include "highgui.h"
#include "ImageProcess.h"
#include "ImageProcessing.h"
#include "omp.h"
#include <io.h>
#include <stdio.h>
#include "atlstr.h"

#define THREAD_NUM_4_RANDOM_FOREST 8
#define THREAD_NUM_4_REGRESSION 4



CULBPTrainer::CULBPTrainer() {
	CLBFTrainer::CLBFTrainer();
}

CULBPTrainer::~CULBPTrainer() {
	vector<data_info>::iterator iter = training_data.begin();
	for (; iter < training_data.end(); iter++) {
		data_info data = *iter;
		free(data.norm_img);
	}
}


/*
*@brief train Local Binary Feature Face Alignment
*/
void CULBPTrainer::train(char **training_data_path, int path_cnt, char *output_path) {

	// 1. collect training images
	printf("Start training of face alignment using ULBP.\n");
	printf("--------------------------------------\n");
	printf("1. collect training data\n");
	collect_data(training_data_path, path_cnt);
	printf("   finished to collect %d data\n\n", training_data.size());

	// 2. compute mean
	printf("2. compute mean shape\n");
	compute_mean_shape();
	translate_mean_shape();
	printf("   mean shape width - %f, height - %f\n", mean_shape_bound.right - mean_shape_bound.left, mean_shape_bound.bottom - mean_shape_bound.top);
	//show_mean_shape();

	// 3. Initialize shape
	Initialize_shape();

	//augment_data();
	printf("  augmented data - %d\n", training_data.size());

	//
	set_normalized_img_buffer();


	// 4. compute residual
	compute_residual();


	/*// 5. train random forest
	printf("5. train random forest\n");
	train_random_forest();

	// 6. write parameters
	save_parameters(output_path);*/
}

/*
@brief mean shape을 normalized image의 중간으로 이동
*/
void CULBPTrainer::translate_mean_shape() {
	/////////////////////////////////////////////
	// centering again
	mean_shape_bound = CLBFCommon::compute_bbox(mean_shape);

	float mean_shape_width = mean_shape_bound.right - mean_shape_bound.left;
	float mean_shape_height = mean_shape_bound.bottom - mean_shape_bound.top;

	float center_x = (mean_shape_bound.right + mean_shape_bound.left) / 2;
	float center_y = (mean_shape_bound.bottom + mean_shape_bound.top) / 2;


	float *dst = mean_shape.m_data;
	for (int i = 0; i < VCNT; i++) {
		*dst -= (center_x - NORM_WIDTH/2);
		dst++;
		*dst -= (center_y - NORM_HEIGHT/2);
		dst++;
	}
	/////////////////////////////////////////////

	mean_shape_bound = CLBFCommon::compute_bbox(mean_shape);
}

/*
@brief 각 data_info에 대해서 normalized image를 저장할 buffer를 지정한다.
*/
void CULBPTrainer::set_normalized_img_buffer() {
	vector<data_info>::iterator iter;
	for (iter = training_data.begin(); iter < training_data.end(); iter++) {
		data_info data = *iter;
		data.norm_img = new unsigned char[NORM_WIDTH * NORM_HEIGHT];
		*iter = data;
	}
}


/*
@brief 현재의 shape과 ground truth의 거리를 계산한다.
*/
void CULBPTrainer::compute_residual() {

	vector<data_info>::iterator iter;

	int i = 0;
	for (iter = training_data.begin(); iter < training_data.end(); iter++, i++) {
		data_info data = *iter;

		data.shape_residual = data.pimg->gt - data.current_pts;
		//data.shape_norm_residual = data.norm_gt - data.current_norm_pts;
		float zero_inv_q[4];
		zero_inv_q[0] = data.inv_q[0];
		zero_inv_q[1] = data.inv_q[1];
		zero_inv_q[2] = 0;
		zero_inv_q[3] = 0;
		CLBFCommon::apply_similarity_transform(data.shape_residual, data.shape_norm_residual, zero_inv_q);

		// 정규화 과정
		normalize_face(data);

		*iter = data;
	}
}

/*
@brief 현재의 특징점을 기준으로 얼굴을 normalize하여 저장한다.
*/
void CULBPTrainer::normalize_face(data_info &data) {
		memset(data.norm_img, 0, 128 * 128);

		double src_x, src_y;
		double a, b;
		int l, k, src, src_l, src_k, src_lk, src_w_step, des_w_step = 0;
		int src_w = data.pimg->img_width;
		unsigned char *pdes = data.norm_img;
		unsigned char *src_data = data.pimg->img;
		for (int des_y = 0; des_y < NORM_HEIGHT; des_y++) {
			for (int des_x = 0; des_x < NORM_WIDTH; des_x++) {
				src_x = data.q[0] * des_x + data.q[1] * des_y + data.q[2];
				src_y = -data.q[1] * des_x + data.q[0] * des_y + data.q[3];

				if (src_x < 0 || src_x >= data.pimg->img_width - 1 || src_y < 0 || src_y >= data.pimg->img_height - 1)
				{
					*pdes++ = 0;
				}
				else
				{
					l = (int)(src_x);
					k = (int)(src_y);
					a = src_x - l;
					b = src_y - k;
					src_w_step = k*data.pimg->img_width;
					src = src_data[src_w_step + l];
					src_l = src_data[src_w_step + l + 1];
					src_k = src_data[src_w_step + src_w + l];
					src_lk = src_data[src_w_step + src_w + l + 1];

					*pdes++ = (unsigned char)(src + a*(src_l - src) + b*(src_k - src) +
						a*b*(src + src_lk - src_l - src_k));
				}
			}
		}

		/*CImageProcessing m_ip;
		cv::Mat image, colorImage;
		image.create(NORM_HEIGHT, NORM_WIDTH, CV_8UC1);
		m_ip.setraw(image, NORM_WIDTH, NORM_HEIGHT, 1, data.norm_img);
		cv::cvtColor(image, colorImage, CV_GRAY2BGR);
		for (int i = 0; i < VCNT; i++) {
			m_ip.draw_circle(colorImage, data.norm_gt.m_data[i * 2], data.norm_gt.m_data[i * 2 + 1], 2, 255, 0, 0, 2);
			m_ip.draw_circle(colorImage, data.current_norm_pts.m_data[i * 2], data.current_norm_pts.m_data[i * 2 + 1], 2, 0, 0, 255, 2);
		}

		imshow("image", colorImage);
		cvWaitKey(-1);*/
}


/*
@brief residual을 이용해서 random forest를 학습한다. feature는 fern
*/
void CULBPTrainer::train_random_forest() {
	int i, j;

	for (i = 0; i < T; i++) { // stage 별 학습
		printf("\tstage : %d\n", i);
		printf("\t\tpoint : ");

		int tic = GetTickCount();

		omp_set_num_threads(THREAD_NUM_4_RANDOM_FOREST);

#pragma omp parallel for
		for (j = 0; j < VCNT; j++) { // 특징점 별 학습
			//printf("\t\tpoint : %d\n", j);
			printf("%d", j);
			for (int k = 0; k < N_TREES; k++) { // tree 별 학습
				printf(".");
				CTree *tree = m_forest[i].GetTree(j, k);
				CNode *root = &(tree->root);

				// sample index 초기화
				// root에 모든 training data의 index를 넣은 후
				// 학습시 split 됨
				int *ulbp_histograms = new int[training_data.size() * 59];
				for (int ii = 0; ii < training_data.size(); ii++)
					root->sampleIndexVector.push_back(ii);

				// 학습
				compute_ULBP_histogram(root->sampleIndexVector, j, i, ulbp_histograms);
				train_tree(*tree, i, j, ulbp_histograms);

				delete []ulbp_histograms;
			}
		}
		int toc = GetTickCount();
		printf(" - %dms\n", toc - tic);

		// leaf node의 index를 설정한다.
		set_local_binary_feature_index(i);

		// global regression 학습
		train_global_regression(i);

		//update
		update_by_global_regression(i);

		measure();
	}
}

/*
@brief 정규화 영상에서 각 landmark에 대해 ULBP histogram을 그린다.
*/
void CULBPTrainer::compute_ULBP_histogram(vector<int> data_vector, int landmark_index, int stage, int *ulbp) {
	vector<data_info>::iterator iter;

	unsigned char m_nFRULBPIndex[] = {
		1, 2, 3, 4, 5, 0, 6, 7, 8, 0, 0, 0, 9, 0, 10, 11, 12, 0, 0, 0, 0, 0, 0, 0, 13, 0, 0, 0, 14, 0, 15, 16, 17, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18,
		0, 0, 0, 0, 0, 0, 0, 19, 0, 0, 0, 20, 0, 21, 22, 23, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 25, 0, 0, 0, 0, 0, 0, 0, 26, 0, 0, 0, 27, 0, 28, 29, 30, 31, 0, 32, 0, 0, 0, 33, 0, 0, 0, 0, 0, 0, 0, 34, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 35, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 36, 37, 38, 0, 39, 0, 0, 0,
		40, 0, 0, 0, 0, 0, 0, 0, 41, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 42, 43, 44, 0, 45, 0, 0, 0, 46, 0, 0, 0, 0, 0, 0, 0, 47, 48, 49, 0, 50, 0, 0, 0,
		51, 52, 53, 0, 54, 55, 56, 57, 58
	};

	int i = 0;
	for (iter = training_data.begin(); iter < training_data.end(); iter++, i++) {
		data_info data = *iter;

		float *pts = data.current_pts.RowD(landmark_index);

		SMyRect patch_rect;
		int res = 10;
		patch_rect.left = pts[0] - res;
		patch_rect.right = pts[0] + res;
		patch_rect.top = pts[1] - res;
		patch_rect.bottom = pts[1] - res;

		patch_rect.left = fa_max(0, fa_min(patch_rect.left, NORM_WIDTH - 1));
		patch_rect.right = fa_max(0, fa_min(patch_rect.right, NORM_WIDTH - 1));
		patch_rect.top = fa_max(0, fa_min(patch_rect.top, NORM_HEIGHT - 1));
		patch_rect.bottom = fa_max(0, fa_min(patch_rect.bottom, NORM_HEIGHT - 1));

		int patch_width = patch_rect.right - patch_rect.left;
		int patch_height = patch_rect.bottom - patch_rect.top;

		unsigned char *patch = new unsigned char[patch_width * patch_height];
		unsigned char *lbp_patch = new unsigned char[patch_width * patch_height];

		Region(data.norm_img, NORM_WIDTH, NORM_HEIGHT, patch, patch_rect);
		Image2LBP8_1(patch, patch_width, patch_height, lbp_patch);

		int lbp_histogram[59] = { 0, };

		for (int i = 1; i < patch_height - 1; i++) {
			for (int j = 1; j < patch_width - 1; j++) {
				int idx = m_nFRULBPIndex[lbp_patch[i * patch_width + j]];
				lbp_histogram[idx]++;
			}
		}

		memcpy(ulbp + i * 59, lbp_histogram, sizeof(int) * 59);

	}
}

/*
* @brief landmark_index와 stage에 해당하는 tree를 학습한다.
*/
void CULBPTrainer::train_tree(CTree &tree, int stage, int landmark_index, int *ulbp_histogram) {
	CNode *root = &(tree.root);

	vector<CNode *> stack;

	// training
	stack.push_back(root);
	int count = stack.size();
	while (count != 0) {
		train_node(stack, stage, landmark_index, ulbp_histogram);
		count = stack.size();
	}
}



/*
@brief 해당 node의 sample들을 가지고 split할지를 학습한다.
*/
void CULBPTrainer::train_node(vector<CNode *> &stack, int stage, int landmark_index, int *ulbp_histogram) {
	int i, j;
	// split 학습
	float selected_x1, selected_x2, selected_y1, selected_y2, selected_thr;
	vector<int> selected_left_idx, selected_right_idx;
	bool updated = false;


	CNode *node = stack.front();

	// MAX_DEPTH에 도달하면 학습이 끝났으니
	// node를 stack에서 삭제
	if (node->depth == MAX_DEPTH) {
		stack.erase(stack.begin());
		return;
	}

	if (node->sampleIndexVector.size() > 1) {

		float _var_curr = node->sampleIndexVector.size() * compute_variance(node->sampleIndexVector, landmark_index);
		float selected_var = INT_MIN;

		///////////////////////////////////////////
		// 1. random pool을 생성
		//srand((unsigned int)time(NULL));
		CFMat rand_xy1, rand_xy2;
		generate_fern_pool(stage, rand_xy1, rand_xy2);
		rand_xy1 *= TARGET_MEAN_SHAPE_WIDTH * MAX_RATIO_RADIUS[stage];
		rand_xy2 *= TARGET_MEAN_SHAPE_WIDTH * MAX_RATIO_RADIUS[stage];

		///////////////////////////////////////////
		// 2. 각 좌표를 영상내의 좌표로 변환해서 pixel difference로 변환
		CIMat all_pd_features(node->sampleIndexVector.size() * NUM_SAMPLES, 1);

		for (i = 0; i < node->sampleIndexVector.size(); i++) {
			int sample_index = node->sampleIndexVector[i];
			data_info& data = training_data[sample_index];

			CFMat transformed_xy1(rand_xy1.Row(), rand_xy1.Col());
			CFMat transformed_xy2(rand_xy2.Row(), rand_xy2.Col());
			compute_transformed_location(data, rand_xy1, rand_xy2, transformed_xy1, transformed_xy2, landmark_index);
			int *dst = all_pd_features.m_data + i * NUM_SAMPLES;
			float *pts1 = transformed_xy1.m_data;
			float *pts2 = transformed_xy2.m_data;
			for (int j = 0; j < NUM_SAMPLES; j++) {
				*dst++ = (int)(data.pimg->img[(int)((*(pts1 + 1)) * data.pimg->img_width + (*pts1))])
					- (int)(data.pimg->img[(int)((*(pts2 + 1)) * data.pimg->img_width + (*pts2))]);
				pts1 += 2;
				pts2 += 2;
			}
		}

		///////////////////////////////////////////
		// 3. pool에 대해 best location을 선택
		for (i = 0; i < NUM_SAMPLES; i++) {
			CIMat pd_features(node->sampleIndexVector.size(), 1);
			int *pt = all_pd_features.m_data + i;

			//#pragma omp parallel for
			for (j = 0; j < node->sampleIndexVector.size(); j++) {
				*(pd_features[j]) = *(pt);
				pt += NUM_SAMPLES;
			}

			// 4. random threshold 구하고, variance reduction 구함
			CIMat temp_pd_features(node->sampleIndexVector.size(), 1);
			temp_pd_features.CopyFrom(pd_features);
			temp_pd_features.Sort();

			// threshold
			int threshold_index = ceil(node->sampleIndexVector.size()*(0.5 + 0.9*((rand() % 100) / 100.0 - 0.5)));
			//threshold_index = ceil(node->sampleIndexVector.size()*(0.5)) - 1;
			int threshold = *temp_pd_features[threshold_index];

			if (node->sampleIndexVector.size() == 2) {
				threshold = (*pd_features[0] + *pd_features[1]) / 2;
			}

			// 큰거, 작은거 구함
			vector<int> left_idx, right_idx;
			for (j = 0; j < node->sampleIndexVector.size(); j++) {
				if (*(pd_features[j]) < threshold)
					left_idx.push_back(node->sampleIndexVector[j]);
				else
					right_idx.push_back(node->sampleIndexVector[j]);
			}

			float _var_left = left_idx.size() * compute_variance(left_idx, landmark_index);
			float _var_right = right_idx.size() * compute_variance(right_idx, landmark_index);
			float new_var = _var_curr - _var_left - _var_right;

			if (selected_var < new_var && (left_idx.size() > 0 && right_idx.size() > 0)) {
				selected_x1 = rand_xy1.RowD(i)[0];
				selected_x2 = rand_xy2.RowD(i)[0];
				selected_y1 = rand_xy1.RowD(i)[1];
				selected_y2 = rand_xy2.RowD(i)[1];
				selected_thr = threshold;
				selected_var = new_var;
				selected_left_idx = left_idx;
				selected_right_idx = right_idx;
				updated = true;
			}
		}
	}

	// 5. split
	if (updated)
	{
		CNode *left_node = new CNode;
		CNode *right_node = new CNode;
		left_node->bLeaf = 1;
		left_node->depth = node->depth + 1;
		left_node->sampleIndexVector = selected_left_idx;
		left_node->m_upper_node = node;
		right_node->bLeaf = 1;
		right_node->depth = node->depth + 1;
		right_node->sampleIndexVector = selected_right_idx;
		right_node->m_upper_node = node;
		node->bLeaf = 0;
		node->m_left_node = left_node;
		node->m_right_node = right_node;
		node->x1 = selected_x1;
		node->x2 = selected_x2;
		node->y1 = selected_y1;
		node->y2 = selected_y2;
		node->thr = selected_thr;
		node->sampleIndexVector.clear();
		stack.erase(stack.begin());
		stack.push_back(left_node);
		stack.push_back(right_node);

		compute_delta(*left_node, landmark_index);
		compute_delta(*right_node, landmark_index);

		if (selected_left_idx.size() == 0) {
			printf("no sample in left\n");
		}
		if (selected_right_idx.size() == 0) {
			printf("no sample in right\n");
		}
	}
	else {
		node->bLeaf = true;
		stack.erase(stack.begin());
	}
}