#include "LBFTrainer.h"
#include <io.h>
#include <stdio.h>
#include "atlstr.h"
#include "LBFCommon.h"
#include "Util.h"
#include "cv.h"
#include "highgui.h"
#include "ImageProcessing.h"
#include "ImageProcess.h"
#include "train.h"

#include "omp.h"

#define THREAD_NUM_4_RANDOM_FOREST 4
#define THREAD_NUM_4_REGRESSION 1

struct PDFeature {
	int pd_feature;
	int index;
};

bool comp_feature(struct PDFeature a, struct PDFeature b) {
	return (a.pd_feature < b.pd_feature);
}


// constructer
CLBFTrainer::CLBFTrainer() {
	int i;
	m_forest = new CForest[T];
	for (int i = 0; i < T; i++) {
		m_forest[i].initialize(N_TREES, VCNT);
	}

	m_global_regressor.set_num_stage(T);

	generate_fern_pool();
}

// destructer
CLBFTrainer::~CLBFTrainer() {
	vector<img_info>::iterator iter = training_images.begin();
	for (; iter < training_images.end(); iter++) {
		img_info img = *iter;
		free(img.img);
	}

	delete[]m_forest;
}

/*
*@brief train Local Binary Feature Face Alignment
*/
void CLBFTrainer::train(char *training_data_path, char *output_path) {
	train(&training_data_path, 1, output_path);
}

/*
*@brief train Local Binary Feature Face Alignment
*/
void CLBFTrainer::train(char **training_data_path, int path_cnt, char *output_path) {

	// 1. collect training images
	printf("Start training of face alignment using Local Binary Features.\n");
	printf("--------------------------------------\n");
	printf("1. collect training data\n");
	collect_data(training_data_path, path_cnt);
	printf("   finished to collect %d data\n\n", training_data.size());

	// 2. compute mean
	printf("2. compute mean shape\n");
	compute_mean_shape();
	printf("   mean shape width - %f, height - %f\n", mean_shape_bound.right - mean_shape_bound.left, mean_shape_bound.bottom - mean_shape_bound.top);
	//show_mean_shape();

	// 3. Initialize shape
	Initialize_shape();


	augment_data();
	printf("3. augmented data - %d\n", training_data.size());

	// 4. compute residual
	compute_residual();

	// 5. train random forest
	printf("4. train random forest\n");
	train_random_forest();

	// 6. write parameters
	save_parameters(output_path);
}

/*
@brief �н� data�� ������.
*/
void CLBFTrainer::collect_data(char *data_path)
{
	collect_data(&data_path, 1);
}

/*
@brief �н� data�� ������.
@param[in] "char **" data_paths - double pointer to directories
@param[in] "int"	path_cnt	- number of paths
*/
void CLBFTrainer::collect_data(char **data_paths, int path_cnt)
{
	char temp[256];

	training_data.clear();
	training_images.clear();

	////////////////////////////////////
	// image �б�
	for (int p = 0; p < path_cnt; p++) {
		char *data_path = data_paths[p];
		printf("%s\n", data_path);
		int cnt = 0;
		for (int i = 0; i < 4; i++) {
			if (i == 0)
				sprintf(temp, "%s/*.jpg", data_path);
			else if (i == 1)
				sprintf(temp, "%s/*.png", data_path);
			else if (i == 2)
				sprintf(temp, "%s/*.bmp", data_path);
			else if (i == 3)
				sprintf(temp, "%s/*.tif", data_path);

			_finddata_t fd;
			_finddata_t fd_face;
			intptr_t handle = _findfirst(temp, &fd);

			if (handle != -1) {
				do {
					sprintf(temp, "%s/%s", data_path, fd.name);

					img_info img;
					sprintf(img.name, "%s", fd.name);
					sprintf(img.path, "%s", data_path);
					if (CLBFCommon::load_img_gt(temp, &img)) {
						training_images.push_back(img);
					}

					if (training_images.size() % 100 == 0) {
						printf(".");
					}
					cnt++;
				} while (_findnext(handle, &fd) != -1);
				//} while (0);

			}
			_findclose(handle);
		}
		printf("files - %d\n", cnt);
	}
	printf("\n");

	////////////////////////////////////
	// flip
	vector<img_info> new_training_images = training_images;
	vector<img_info>::iterator iter = training_images.begin();
	for (; iter < training_images.end(); iter++) {
		img_info img = *iter;
		img_info flip_img;
		flip_data(img, flip_img);
		new_training_images.push_back(flip_img);
	}
	training_images = new_training_images;
	////////////////////////////////////



	////////////////////////////////////
	// data�� �����Ŵ
	{
		vector<img_info>::iterator iter = training_images.begin();
		for (; iter < training_images.end(); iter++) {
			data_info data;
			data.pimg = &(*iter);
			training_data.push_back(data);
		}
	}
	////////////////////////////////////
}


/*
@brief compute mean shape of training data using procrustes analysis
*/
void CLBFTrainer::compute_mean_shape()
{
	int i;
	float *dst, *src;
	float center_x, center_y;
	vector<data_info>::iterator iter;

	/////////////////////////////////////////////
	// procrustes analysis
	procrustes(mean_shape, training_data);
	/////////////////////////////////////////////

	/////////////////////////////////////////////
	// centering
	mean_shape_bound = CLBFCommon::compute_bbox(mean_shape);

	int mean_shape_width = mean_shape_bound.right - mean_shape_bound.left;
	int mean_shape_height = mean_shape_bound.bottom - mean_shape_bound.top;

	center_x = (mean_shape_bound.right + mean_shape_bound.left) / 2;
	center_y = (mean_shape_bound.bottom + mean_shape_bound.top) / 2;


	dst = mean_shape.m_data;
	for (i = 0; i < VCNT; i++) {
		*dst -= center_x;
		dst++;
		*dst -= center_y;
		dst++;
	}
	/////////////////////////////////////////////

	/////////////////////////////////////////////
	// correct rotation
	int dis = 0;
	if (mean_shape.Row() == 51)
		dis = 17;

	float left_eye[2] = { 0, }, right_eye[2] = { 0, };
	for (i = 0; i < LEFT_EYE_IDX_CNT; i++) {
		left_eye[0] += mean_shape.m_data[(LEFT_EYE_IDX[i] - dis) * 2];
		left_eye[1] += mean_shape.m_data[(LEFT_EYE_IDX[i] - dis) * 2 + 1];
	}
	left_eye[0] /= LEFT_EYE_IDX_CNT;
	left_eye[1] /= LEFT_EYE_IDX_CNT;

	for (i = 0; i < RIGHT_EYE_IDX_CNT; i++) {
		right_eye[0] += mean_shape.m_data[(RIGHT_EYE_IDX[i] - dis) * 2];
		right_eye[1] += mean_shape.m_data[(RIGHT_EYE_IDX[i] - dis) * 2 + 1];
	}
	right_eye[0] /= RIGHT_EYE_IDX_CNT;
	right_eye[1] /= RIGHT_EYE_IDX_CNT;

	float eye_center[2];
	eye_center[0] = (left_eye[0] + right_eye[0]) / 2;
	eye_center[1] = (left_eye[1] + right_eye[1]) / 2;

	float eye_vector[2];
	eye_vector[0] = right_eye[0] - eye_center[0];
	eye_vector[1] = right_eye[1] - eye_center[1];

	float size = sqrt(eye_vector[0] * eye_vector[0] + eye_vector[1] * eye_vector[1]);
	eye_vector[0] /= size;
	eye_vector[1] /= size;
	float angle = atan2(eye_vector[1], eye_vector[0]);

	for (i = 0; i < VCNT; i++) {
		float x = mean_shape.m_data[i * 2];
		float y = mean_shape.m_data[i * 2 + 1];

		mean_shape.m_data[i * 2] = cos(angle) * x + sin(angle) * y;
		mean_shape.m_data[i * 2 + 1] = -sin(angle) * x + cos(angle) * y;
	}
	/////////////////////////////////////////////



	/////////////////////////////////////////////
	// scaling
	float ratio = TARGET_MEAN_SHAPE_WIDTH / (float)(mean_shape_width);

	mean_shape *= ratio;
	/////////////////////////////////////////////


	mean_shape_bound = CLBFCommon::compute_bbox(mean_shape);

	/////////////////////////////////////////////
	// centering again
	mean_shape_bound = CLBFCommon::compute_bbox(mean_shape);

	mean_shape_width = mean_shape_bound.right - mean_shape_bound.left;
	mean_shape_height = mean_shape_bound.bottom - mean_shape_bound.top;

	center_x = (mean_shape_bound.right + mean_shape_bound.left) / 2;
	center_y = (mean_shape_bound.bottom + mean_shape_bound.top) / 2;


	dst = mean_shape.m_data;
	for (i = 0; i < VCNT; i++) {
		*dst -= center_x;
		dst++;
		*dst -= center_y;
		dst++;
	}
	/////////////////////////////////////////////


	/*double temp[136];
	FILE *fp = fopen("mean_shape.dat", "rb");
	fread(temp, sizeof(double), 136, fp);
	fclose(fp);

	for (i = 0; i < VCNT2; i++) {
	mean_shape.m_data[i] = temp[i];
	}*/
	mean_shape_bound = CLBFCommon::compute_bbox(mean_shape);

}

/*
@brief procrustes analysis for mean shape
*/
void CLBFTrainer::procrustes(CFMat &dst_mean_shape, vector<data_info> &data)
{
	int i;
	float *dst, *src;
	float center_x, center_y;
	CFMat temp_pts;
	CFMat temp_mean_shape;
	CFMat initial_mean_shape;;
	int max_iter, iteration;
	vector<data_info>::iterator iter;

	/////////////////////////////////////////////
	// procrustes analysis
	temp_mean_shape.Create(VCNT, 2);
	temp_pts.Create(VCNT, 2);

	// initial mean shape
	iter = training_data.begin();
	mean_shape.CopyFrom((*iter).pimg->gt);
	initial_mean_shape.CopyFrom((*iter).pimg->gt);

	max_iter = 100;
	iteration = 0;
	while (1)
	{
		///////////////
		// ������ mean shape
		float q2[4], inv_q2[4];

		temp_mean_shape.Fill(0);
		for (iter = training_data.begin(); iter < training_data.end(); iter++) {
			CLBFCommon::compute_similarity(mean_shape, (*iter).pimg->gt, q2);
			CLBFCommon::apply_similarity_transform((*iter).pimg->gt, temp_pts, q2);

			temp_mean_shape += temp_pts;
		}

		temp_mean_shape /= training_data.size();


		///////////////
		// ������ mean shape�� reference�� ����
		CLBFCommon::compute_similarity(initial_mean_shape, temp_mean_shape, q2);
		CLBFCommon::apply_similarity_transform(temp_mean_shape, temp_mean_shape, q2);

		///////////////
		// compute error
		float error = 0;
		for (i = 0; i < VCNT; i++) {
			error += sqrt((temp_mean_shape.m_data[i * 2] - mean_shape.m_data[i * 2]) * (temp_mean_shape.m_data[i * 2] - mean_shape.m_data[i * 2])
				+ (temp_mean_shape.m_data[i * 2 + 1] - mean_shape.m_data[i * 2 + 1]) * (temp_mean_shape.m_data[i * 2 + 1] - mean_shape.m_data[i * 2 + 1]));
		}
		error /= VCNT;

		mean_shape.CopyFrom(temp_mean_shape);

		if (error < 0.05) {
			break;
		}

		if (iteration++ > max_iter)
		{
			break;
		}
	}
}


void CLBFTrainer::show_mean_shape() {
	printf("   show mean shape. input any key to continue.\n");

	// plotting
	CImageProcessing m_ip;
	cv::Mat image;
	image.create(RECT_HEIGHT(mean_shape_bound) * 2, RECT_WIDTH(mean_shape_bound) * 2, CV_8UC3);
	//unsigned char *temp = (unsigned char *)malloc(sizeof(unsigned char) * mean_shape_width * mean_shape_height * 4 * 3);
	//memset(temp, 0, sizeof(unsigned char) * mean_shape_width * mean_shape_height * 4 * 3);
	//m_ip.setraw(image, mean_shape_width * 2, mean_shape_height * 2, 3, temp);
	//free(temp);

	int center_x = RECT_WIDTH(mean_shape_bound);
	int center_y = RECT_HEIGHT(mean_shape_bound);

	for (int i = 0; i < VCNT; i++) {
		m_ip.draw_circle(image, mean_shape.m_data[i * 2] + center_x, mean_shape.m_data[i * 2 + 1] + center_y, 1, 255, 0, 0, 1);
	}

	imshow("mean shape", image);
	cvWaitKey(0);
	cvDestroyWindow("mean shape");

}

/*
@brief �󱼿� bounding box�� �׸��� mean shape�� �� ���� �ø���.
*/
void CLBFTrainer::Initialize_shape() {
	vector<data_info>::iterator iter;

	for (iter = training_data.begin(); iter < training_data.end(); iter++) {
		data_info data = *iter;
		FMyRect bbox = CLBFCommon::compute_bbox(data.pimg->gt);

		float width_scale = float(RECT_WIDTH(bbox) + 1) / float(RECT_WIDTH(mean_shape_bound));
		float height_scale = float(RECT_HEIGHT(bbox) + 1) / float(RECT_HEIGHT(mean_shape_bound));
		float scale = (width_scale + height_scale) / 2;

		data.q[0] = scale;
		data.q[1] = 0;
		data.q[2] = bbox.left - mean_shape_bound.left * scale;
		data.q[3] = bbox.top - mean_shape_bound.top * scale;

		data.current_norm_pts = mean_shape;
		CLBFCommon::apply_similarity_transform(data.current_norm_pts, data.current_pts, data.q);
		CLBFCommon::compute_inverse_similarity(data.q, data.inv_q);
		CLBFCommon::apply_similarity_transform(data.pimg->gt, data.norm_gt, data.inv_q);
		data.initial_pts = data.current_pts;
		*iter = data;
	}
}

/*
* augment the training data
*/
void CLBFTrainer::augment_data()
{
	float *src;
	vector<data_info> new_training_data = training_data;


	int i;
	vector<data_info>::iterator iter = training_data.begin();
	for (i = 0; iter < training_data.end(); iter++, i++) {
		data_info data = *iter;
		data_info new_data = data;

		//////////////////////////////
		// mean shape�� ground truth�� transform
		{
			new_data.current_norm_pts = mean_shape;
			CLBFCommon::compute_similarity(new_data.pimg->gt, mean_shape, new_data.q);
			CLBFCommon::compute_inverse_similarity(new_data.q, new_data.inv_q);
			CLBFCommon::apply_similarity_transform(new_data.current_norm_pts, new_data.current_pts, new_data.q);
			new_data.initial_pts = new_data.current_pts;
			new_training_data.push_back(new_data);
		}

		//////////////////////////////
		// mean shape�� ground truth�� transform -> �߾����� translation
		{
			new_data = data;
			FMyRect bbox = CLBFCommon::compute_bbox(new_data.pimg->gt);
			new_data.current_norm_pts = mean_shape + new_data.norm_gt;
			new_data.current_norm_pts /= 2;
			CLBFCommon::compute_similarity(new_data.pimg->gt, new_data.current_norm_pts, new_data.q);

			FMyRect bbox3 = CLBFCommon::compute_bbox(data.current_norm_pts);
			float ref_cx = (bbox3.left + bbox3.right) / 2;
			float ref_cy = (bbox3.top + bbox3.bottom) / 2;
			new_data.q[2] += (bbox.left + bbox.right) / 2 - (ref_cx * new_data.q[0] + ref_cy * new_data.q[1] + new_data.q[2]);
			new_data.q[3] += (bbox.top + bbox.bottom) / 2 - (ref_cx * -new_data.q[1] + ref_cy * new_data.q[0] + new_data.q[3]);

			CLBFCommon::compute_inverse_similarity(new_data.q, new_data.inv_q);
			CLBFCommon::apply_similarity_transform(new_data.current_norm_pts, new_data.current_pts, new_data.q);
			new_data.initial_pts = new_data.current_pts;
			new_training_data.push_back(new_data);
		}

		//////////////////////////////
		// gt�� 1/2 �߰�
		{
			new_data.current_norm_pts = mean_shape + new_data.norm_gt;
			new_data.current_norm_pts /= 2;
			CLBFCommon::compute_similarity(new_data.current_pts, new_data.current_norm_pts, new_data.q);
			CLBFCommon::compute_inverse_similarity(new_data.q, new_data.inv_q);
			CLBFCommon::apply_similarity_transform(new_data.current_norm_pts, new_data.current_pts, new_data.q);
			new_data.initial_pts = new_data.current_pts;
			new_training_data.push_back(new_data);
		}

		float scale = sqrt(data.q[0] * data.q[0] + data.q[1] * data.q[1]);
		float angle = atan(data.q[1] / data.q[0]);

		///////////////////
		// mean shape�� random scale, rotation, translation �߰�
		for (int a = 0; a < RANDOM_INIT_NUM; a++)
		{
			new_data = data;

			FMyRect bbox = CLBFCommon::compute_bbox(data.current_pts);
			float d_tx = ((double)(rand() % RAND_MAX) / RAND_MAX * (RECT_WIDTH(mean_shape_bound) * scale * 0.2) - (RECT_WIDTH(mean_shape_bound) * scale * 0.2 * 0.5));
			float d_ty = ((double)(rand() % RAND_MAX) / RAND_MAX * (RECT_HEIGHT(mean_shape_bound) * scale * 0.2) - (RECT_HEIGHT(mean_shape_bound) * scale * 0.2 * 0.5));
			float d_scale = (double)(rand() % RAND_MAX) / RAND_MAX * 0.2 - 0.1 + 1;
			float d_angle = ((double)(rand() % RAND_MAX) / RAND_MAX * 20 - 10) * 3.141592 / 180;
			new_data.q[0] = (scale * d_scale) * cos(angle + d_angle);
			new_data.q[1] = (scale * d_scale) * sin(angle + d_angle);
			new_data.q[2] += d_tx;
			new_data.q[3] += d_ty;

			CLBFCommon::apply_similarity_transform(new_data.current_norm_pts, new_data.current_pts, new_data.q);
			CLBFCommon::compute_inverse_similarity(new_data.q, new_data.inv_q);
			new_data.initial_pts = new_data.current_pts;
			new_training_data.push_back(new_data);


		}

		///////////////////
		// mean shape�� ground truth�� transform ��Ų �Ŀ� random scale, rotation, translation �߰�
		// ���ư� ���� ���ư� �󱼿��� random�� ǥ����
		for (int a = 0; a < RANDOM_INIT_NUM; a++)
		{
			new_data = data;
			new_data.current_norm_pts = mean_shape;
			CLBFCommon::compute_similarity(new_data.pimg->gt, mean_shape, new_data.q);
			float scale = sqrt(new_data.q[0] * new_data.q[0] + new_data.q[1] * new_data.q[1]);
			float angle = atan(new_data.q[1] / new_data.q[0]);


			float d_tx = ((double)(rand() % RAND_MAX) / RAND_MAX * (RECT_WIDTH(mean_shape_bound) * scale * 0.2) - (RECT_WIDTH(mean_shape_bound) * scale * 0.2 * 0.5));
			float d_ty = ((double)(rand() % RAND_MAX) / RAND_MAX * (RECT_HEIGHT(mean_shape_bound) * scale * 0.2) - (RECT_HEIGHT(mean_shape_bound) * scale * 0.2 * 0.5));
			float d_scale = (double)(rand() % RAND_MAX) / RAND_MAX * 0.2 - 0.1 + 1;
			float d_angle = ((double)(rand() % RAND_MAX) / RAND_MAX * 10 - 5) * 3.141592 / 180;
			new_data.q[0] = (scale * d_scale) * cos(angle + d_angle);
			new_data.q[1] = (scale * d_scale) * sin(angle + d_angle);
			new_data.q[2] += d_tx;
			new_data.q[3] += d_ty;

			CLBFCommon::apply_similarity_transform(new_data.current_norm_pts, new_data.current_pts, new_data.q);
			CLBFCommon::compute_inverse_similarity(new_data.q, new_data.inv_q);
			new_data.initial_pts = new_data.current_pts;
			new_training_data.push_back(new_data);
		}

		///////////////////
		// (mean shape+gt)/2�� ground truth�� transform ��Ų �Ŀ� random scale, rotation, translation �߰�
		// ���ư� ���� ���ư� �󱼿��� random�� ǥ����
		for (int a = 0; a < RANDOM_INIT_NUM; a++)
		{
			new_data = data;
			new_data.current_norm_pts = mean_shape + new_data.norm_gt;
			new_data.current_norm_pts /= 2;
			CLBFCommon::compute_similarity(new_data.pimg->gt, mean_shape, new_data.q);
			float scale = sqrt(new_data.q[0] * new_data.q[0] + new_data.q[1] * new_data.q[1]);
			float angle = atan(new_data.q[1] / new_data.q[0]);


			float d_tx = ((double)(rand() % RAND_MAX) / RAND_MAX * (RECT_WIDTH(mean_shape_bound) * scale * 0.2) - (RECT_WIDTH(mean_shape_bound) * scale * 0.2 * 0.5));
			float d_ty = ((double)(rand() % RAND_MAX) / RAND_MAX * (RECT_HEIGHT(mean_shape_bound) * scale * 0.2) - (RECT_HEIGHT(mean_shape_bound) * scale * 0.2 * 0.5));
			float d_scale = (double)(rand() % RAND_MAX) / RAND_MAX * 0.2 - 0.1 + 1;
			float d_angle = ((double)(rand() % RAND_MAX) / RAND_MAX * 10 - 5) * 3.141592 / 180;
			new_data.q[0] = (scale * d_scale) * cos(angle + d_angle);
			new_data.q[1] = (scale * d_scale) * sin(angle + d_angle);
			new_data.q[2] += d_tx;
			new_data.q[3] += d_ty;

			CLBFCommon::apply_similarity_transform(new_data.current_norm_pts, new_data.current_pts, new_data.q);
			CLBFCommon::compute_inverse_similarity(new_data.q, new_data.inv_q);
			new_data.initial_pts = new_data.current_pts;
			new_training_data.push_back(new_data);
		}

		///////////////////
		// ���� �� �Ʒ��� �ø� ��츦 �����
		for (int a = 0; a < 2; a++)
		{
			new_data = data;
			new_data.current_norm_pts = mean_shape + new_data.norm_gt;
			new_data.current_norm_pts /= 2;
			CLBFCommon::compute_similarity(new_data.pimg->gt, mean_shape, new_data.q);
			float scale = sqrt(new_data.q[0] * new_data.q[0] + new_data.q[1] * new_data.q[1]);
			float angle = atan(new_data.q[1] / new_data.q[0]);

			FMyRect o_bbox = CLBFCommon::compute_bbox(new_data.current_norm_pts);
			CFMat temp = new_data.norm_gt;

			if (a == 0) {
				for (int k = 0; k < VCNT; k++) {
					*(temp.RowD(k) + 1) *= 0.3;
				}
			}
			else {
				for (int k = 0; k < VCNT; k++) {
					*(temp.RowD(k) + 1) *= 1.5;
				}
			}
			new_data.current_norm_pts = mean_shape + temp;
			new_data.current_norm_pts /= 2;

			FMyRect s_bbox = CLBFCommon::compute_bbox(new_data.current_norm_pts);

			for (int k = 0; k < VCNT; k++) {
				*(new_data.current_norm_pts.RowD(k)) += o_bbox.left - s_bbox.left;
				*(new_data.current_norm_pts.RowD(k) + 1) += o_bbox.top - s_bbox.top;
			}

			float d_tx = 0;
			float d_ty = 0;
			float d_scale = 1;
			float d_angle = 0;
			new_data.q[0] = (scale * d_scale) * cos(angle + d_angle);
			new_data.q[1] = (scale * d_scale) * sin(angle + d_angle);
			new_data.q[2] += d_tx;
			new_data.q[3] += d_ty;

			CLBFCommon::apply_similarity_transform(new_data.current_norm_pts, new_data.current_pts, new_data.q);
			CLBFCommon::compute_inverse_similarity(new_data.q, new_data.inv_q);
			new_data.initial_pts = new_data.current_pts;
			new_training_data.push_back(new_data);
		}


		///////////////////
		// �ʱ� shape�� random�ϰ� ������ �� intialize ��Ŵ
		for (int a = 0; a < RANDOM_INIT_NUM; a++)
		{
			int random_image_index = (int)((double)(rand() % RAND_MAX) / RAND_MAX * training_data.size());
			data_info random_data = training_data[random_image_index];
			new_data = data;
			new_data.current_norm_pts = random_data.norm_gt + mean_shape;
			new_data.current_norm_pts /= 2;

			CLBFCommon::apply_similarity_transform(new_data.current_norm_pts, new_data.current_pts, new_data.q);
			CLBFCommon::compute_inverse_similarity(new_data.q, new_data.inv_q);
			new_data.initial_pts = new_data.current_pts;
			new_training_data.push_back(new_data);
		}

	}

	training_data = new_training_data;
}

/*
@brief data�� flip �Ѵ�.
*/
void CLBFTrainer::flip_data(img_info &src, img_info &dst) {

	if (src.gt.Row() == 51) {

		int flip_index[] = {
			//16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
			26, 25, 24, 23, 22, 21, 20, 19, 18, 17,
			27, 28, 29, 30,
			35, 34, 33, 32, 31,
			45, 44, 43, 42, 47, 46,
			39, 38, 37, 36, 41, 40,
			54, 53, 52, 51, 50, 49, 48, 59, 58, 57, 56, 55,
			64, 63, 62, 61, 60, 67, 66, 65 };

		dst = src;
		for (int i = 0; i < VCNT; i++) {
			float *dst_pts = dst.gt.RowD(flip_index[i] - 17);
			float *src_pts = src.gt.RowD(i);
			dst_pts[0] = src.img_width - src_pts[0];
			dst_pts[1] = src_pts[1];
		}
	}
	else {
		int flip_index[] = { 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
			26, 25, 24, 23, 22, 21, 20, 19, 18, 17,
			27, 28, 29, 30,
			35, 34, 33, 32, 31,
			45, 44, 43, 42, 47, 46,
			39, 38, 37, 36, 41, 40,
			54, 53, 52, 51, 50, 49, 48, 59, 58, 57, 56, 55,
			64, 63, 62, 61, 60, 67, 66, 65 };

		dst = src;
		for (int i = 0; i < VCNT; i++) {
			float *dst_pts = dst.gt.RowD(flip_index[i]);
			float *src_pts = src.gt.RowD(i);
			dst_pts[0] = src.img_width - src_pts[0];
			dst_pts[1] = src_pts[1];
		}
	}

	dst.img = (unsigned char *)malloc(sizeof(unsigned char) * dst.img_width * dst.img_height);
	FlipX(src.img, src.img_width, src.img_height, dst.img);
	if (dst.fd_enabled == true) {
		FMyRect temp = dst.fd;
		dst.fd.left = src.img_width - temp.right;
		dst.fd.right = src.img_height - temp.left;
	}
}

/*
@brief show data
*/
void CLBFTrainer::show_data(data_info &data) {
	int i;
	CImageProcessing m_ip;
	cv::Mat image, colorImage;
	image.create(data.pimg->img_height, data.pimg->img_width, CV_8UC1);
	m_ip.setraw(image, data.pimg->img_width, data.pimg->img_height, 1, data.pimg->img);
	cv::cvtColor(image, colorImage, CV_GRAY2BGR);

	int thick = 2;
	float *pts = data.current_pts.m_data;
	float *gt = data.pimg->gt.m_data;
	for (i = 0; i < 16; i++) {
		//m_ip.draw_line(colorImage, pts[i * 2], pts[i * 2 + 1], pts[(i + 1) * 2], pts[(i + 1) * 2 + 1], 255, 0, 0, thick);
		m_ip.draw_line(colorImage, gt[i * 2], gt[i * 2 + 1], gt[(i + 1) * 2], gt[(i + 1) * 2 + 1], 255, 0, 0, thick);
	}
	for (i = 17; i < 21; i++) {
		//m_ip.draw_line(colorImage, pts[i * 2], pts[i * 2 + 1], pts[(i + 1) * 2], pts[(i + 1) * 2 + 1], 255, 0, 0, thick);
		m_ip.draw_line(colorImage, gt[i * 2], gt[i * 2 + 1], gt[(i + 1) * 2], gt[(i + 1) * 2 + 1], 255, 0, 0, thick);
	}
	for (i = 22; i < 26; i++) {
		//m_ip.draw_line(colorImage, pts[i * 2], pts[i * 2 + 1], pts[(i + 1) * 2], pts[(i + 1) * 2 + 1], 255, 0, 0, thick);
		m_ip.draw_line(colorImage, gt[i * 2], gt[i * 2 + 1], gt[(i + 1) * 2], gt[(i + 1) * 2 + 1], 255, 0, 0, thick);
	}
	for (i = 27; i < 30; i++) {
		//m_ip.draw_line(colorImage, pts[i * 2], pts[i * 2 + 1], pts[(i + 1) * 2], pts[(i + 1) * 2 + 1], 255, 0, 0, thick);
		m_ip.draw_line(colorImage, gt[i * 2], gt[i * 2 + 1], gt[(i + 1) * 2], gt[(i + 1) * 2 + 1], 255, 0, 0, thick);
	}
	for (i = 31; i < 35; i++) {
		//m_ip.draw_line(colorImage, pts[i * 2], pts[i * 2 + 1], pts[(i + 1) * 2], pts[(i + 1) * 2 + 1], 255, 0, 0, thick);
		m_ip.draw_line(colorImage, gt[i * 2], gt[i * 2 + 1], gt[(i + 1) * 2], gt[(i + 1) * 2 + 1], 255, 0, 0, thick);
	}
	for (i = 36; i < 41; i++) {
		//m_ip.draw_line(colorImage, pts[i * 2], pts[i * 2 + 1], pts[(i + 1) * 2], pts[(i + 1) * 2 + 1], 255, 0, 0, thick);
		m_ip.draw_line(colorImage, gt[i * 2], gt[i * 2 + 1], gt[(i + 1) * 2], gt[(i + 1) * 2 + 1], 255, 0, 0, thick);
	}
	for (i = 42; i < 47; i++) {
		//m_ip.draw_line(colorImage, pts[i * 2], pts[i * 2 + 1], pts[(i + 1) * 2], pts[(i + 1) * 2 + 1], 255, 0, 0, thick);
		m_ip.draw_line(colorImage, gt[i * 2], gt[i * 2 + 1], gt[(i + 1) * 2], gt[(i + 1) * 2 + 1], 255, 0, 0, thick);
	}
	for (i = 48; i < 59; i++) {
		//m_ip.draw_line(colorImage, pts[i * 2], pts[i * 2 + 1], pts[(i + 1) * 2], pts[(i + 1) * 2 + 1], 255, 0, 0, thick);
		m_ip.draw_line(colorImage, gt[i * 2], gt[i * 2 + 1], gt[(i + 1) * 2], gt[(i + 1) * 2 + 1], 255, 0, 0, thick);
	}
	for (i = 60; i < 67; i++) {
		//m_ip.draw_line(colorImage, pts[i * 2], pts[i * 2 + 1], pts[(i + 1) * 2], pts[(i + 1) * 2 + 1], 255, 0, 0, thick);
		m_ip.draw_line(colorImage, gt[i * 2], gt[i * 2 + 1], gt[(i + 1) * 2], gt[(i + 1) * 2 + 1], 255, 0, 0, thick);
	}

	for (i = 0; i < VCNT; i++) {
		m_ip.draw_circle(colorImage, pts[i * 2], pts[i * 2 + 1], thick, 0, 0, 255, 2);
		m_ip.draw_circle(colorImage, gt[i * 2], gt[i * 2 + 1], thick, 0, 0, 255, 2);
		//printf("%02d : %f, %f\n", i, pts[i*2], pts[i*2+1]);
	}

	imshow("current shape", colorImage);
	cvWaitKey(0);
	cvDestroyWindow("current shape");
}



/*
@brief ������ shape�� ground truth�� �Ÿ��� ����Ѵ�.
*/
void CLBFTrainer::compute_residual() {
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

		*iter = data;
	}
}


/*
@brief residual�� �̿��ؼ� random forest�� �н��Ѵ�. feature�� fern
*/
void CLBFTrainer::train_random_forest() {
	int i, j;

	for (i = 0; i < T; i++) { // stage �� �н�
		printf("\tstage : %d\n", i);
		printf("\t\tpoint : ");

		int tic = GetTickCount();

		omp_set_num_threads(THREAD_NUM_4_RANDOM_FOREST);

#pragma omp parallel for
		for (j = 0; j < VCNT; j++) { // Ư¡�� �� �н�
			//printf("\t\tpoint : %d\n", j);
			printf("%d", j);
			for (int k = 0; k < N_TREES; k++) { // tree �� �н�
				printf(".");
				CTree *tree = m_forest[i].GetTree(j, k);
				CNode *root = &(tree->root);

				// sample index �ʱ�ȭ
				// root�� ��� training data�� index�� ���� ��
				// �н��� split ��
				for (int ii = 0; ii < training_data.size(); ii++)
					root->sampleIndexVector.push_back(ii);
				/*for (int ii = 0; ii < 100000; ii++) {
				int index = (double)(rand() % RAND_MAX) / RAND_MAX * (training_data.size() - 1);
				root->sampleIndexVector.push_back(index);
				}*/

				// �н�
				train_tree(*tree, i, j);
			}
		}
		int toc = GetTickCount();
		printf(" - %dms\n", toc - tic);

		// leaf node�� index�� �����Ѵ�.
		set_local_binary_feature_index(i);

		// global regression �н�
		train_global_regression(i);

		//update
		update_by_global_regression(i);

		measure();
	}


}

/*
* @brief landmark_index�� stage�� �ش��ϴ� tree�� �н��Ѵ�.
*/
void CLBFTrainer::train_tree(CTree &tree, int stage, int landmark_index) {
	CNode *root = &(tree.root);

	vector<CNode *> stack;

	// training
	stack.push_back(root);
	int count = stack.size();
	while (count != 0) {
		train_node(stack, stage, landmark_index);
		count = stack.size();
	}
}

/*
@brief �ش� node�� sample���� ������ split������ �н��Ѵ�.
*/
void CLBFTrainer::train_node(vector<CNode *> &stack, int stage, int landmark_index) {
	int i, j;
	// split �н�
	float selected_x1, selected_x2, selected_y1, selected_y2, selected_thr;
	vector<int> selected_left_idx, selected_right_idx;
	bool updated = false;


	CNode *node = stack.front();

	// MAX_DEPTH�� �����ϸ� �н��� ��������
	// node�� stack���� ����
	if (node->depth == MAX_DEPTH) {
		stack.erase(stack.begin());
		return;
	}

	if (node->sampleIndexVector.size() > 1) {

		float _var_curr = node->sampleIndexVector.size() * compute_variance(node->sampleIndexVector, landmark_index);
		float selected_var = INT_MIN;

		///////////////////////////////////////////
		// 1. random pool�� ����
		//srand((unsigned int)time(NULL));
		CFMat rand_xy1, rand_xy2;
		generate_fern_pool(stage, rand_xy1, rand_xy2);
		rand_xy1 *= TARGET_MEAN_SHAPE_WIDTH * MAX_RATIO_RADIUS[stage];
		rand_xy2 *= TARGET_MEAN_SHAPE_WIDTH * MAX_RATIO_RADIUS[stage];

		///////////////////////////////////////////
		// 2. �� ��ǥ�� ������ ��ǥ�� ��ȯ�ؼ� pixel difference�� ��ȯ
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
		// 3. pool�� ���� best location�� ����
		for (i = 0; i < NUM_SAMPLES; i++) {
			CIMat pd_features(node->sampleIndexVector.size(), 1);
			int *pt = all_pd_features.m_data + i;

			//#pragma omp parallel for
			for (j = 0; j < node->sampleIndexVector.size(); j++) {
				*(pd_features[j]) = *(pt);
				pt += NUM_SAMPLES;
			}

			// 4. random threshold ���ϰ�, variance reduction ����
			vector<int> left_idx, right_idx;
			int threshold = 0;

			if (1) {
				CIMat temp_pd_features(node->sampleIndexVector.size(), 1);
				temp_pd_features.CopyFrom(pd_features);
				temp_pd_features.Sort();

				// threshold
				int threshold_index = ceil(node->sampleIndexVector.size()*(0.5 + 0.9*((rand() % 100) / 100.0 - 0.5)));
				//threshold_index = ceil(node->sampleIndexVector.size()*(0.5)) - 1;
				threshold = *temp_pd_features[threshold_index];

				if (node->sampleIndexVector.size() == 2) {
					threshold = (*pd_features[0] + *pd_features[1]) / 2;
				}

				// ū��, ������ ����
				for (j = 0; j < node->sampleIndexVector.size(); j++) {
					if (*(pd_features[j]) < threshold)
						left_idx.push_back(node->sampleIndexVector[j]);
					else
						right_idx.push_back(node->sampleIndexVector[j]);
				}
			}
			else {
				vector <struct PDFeature> feature_vector;
				for (int j = 0; j < pd_features.Row(); j++) {
					struct PDFeature pd;
					pd.pd_feature = pd_features.m_data[j];
					pd.index = j;
					feature_vector.push_back(pd);
				}
				sort(feature_vector.begin(), feature_vector.end(), comp_feature);

				int threshold_index = ceil(node->sampleIndexVector.size()*(0.5 + 0.9*((rand() % 100) / 100.0 - 0.5)));

				while (1) {
					if (threshold_index >= node->sampleIndexVector.size() - 1) {
						threshold_index--;
						break;
					}
					if (feature_vector[threshold_index].pd_feature != feature_vector[threshold_index + 1].pd_feature)
					{
						break;
					}
					threshold_index++;
				}

				if (node->sampleIndexVector.size() == 2) {
					threshold_index = 0;
				}


				for (int j = 0; j <= threshold_index; j++) {
					left_idx.push_back(node->sampleIndexVector[feature_vector[j].index]);
				}
				for (int j = threshold_index + 1; j < node->sampleIndexVector.size(); j++) {
					right_idx.push_back(node->sampleIndexVector[feature_vector[j].index]);
				}
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

/*
@brief sample_index�� �ش��ϴ� ���󿡼� landmark_index�� �ش��ϴ� residual�� variance�� ���Ѵ�.
*/
float CLBFTrainer::compute_variance(vector<int> &sample_idx, int landmark_index)

{
	float avg_x = 0, avg_y = 0;
	float avg_x_2 = 0, avg_y_2 = 0;
	float var = 0, ss = 0;
	int sample_size;
	int idx;
	vector<int>::iterator iter;

	sample_size = sample_idx.size();

	// 1. residual ���
	iter = sample_idx.begin();
	for (; iter < sample_idx.end(); iter++) {
		data_info &data = training_data[*iter];

		float *res = data.shape_norm_residual.RowD(landmark_index);
		avg_x += res[0];
		avg_y += res[1];
		avg_x_2 += res[0] * res[0];
		avg_y_2 += res[1] * res[1];
	}
	avg_x /= sample_size;
	avg_y /= sample_size;
	avg_x_2 /= sample_size;
	avg_y_2 /= sample_size;

	float var_x = 0, var_y = 0;

	var_x = avg_x_2 - avg_x * avg_x;
	var_y = avg_y_2 - avg_y * avg_y;
	var = var_x + var_y;

	return var;
}

/*
@brief fern pool�� �����.
*/
void CLBFTrainer::generate_fern_pool() {
	int i;


	float a = 0;
	while (a < 2 * 3.141952) {
		fern_angles.push_back(a);
		a += 0.1;

		//if (angles.size() > 5) break;
	}

	float r = 0;
	while (r <= 1) {
		fern_radius.push_back(r);
		r += 1 / 30.0;

		//if (radius.size() > 0) break;
	}


	for (i = 0; i < fern_angles.size() * fern_radius.size(); i++) {
		fern_pool_sample_index1.push_back(i);
	}
	fern_pool_sample_index2 = fern_pool_sample_index1;
}

/*
@brief fern pool�� �����. �ΰ��� ������ pair�� �ؼ� NUM_SAMPLES ��ŭ �����.
*/
void CLBFTrainer::generate_fern_pool(int stage, CFMat &xy1, CFMat &xy2) {
	/*int i;
	int r, a;

	xy1.Create(NUM_SAMPLES, 2);
	xy2.Create(NUM_SAMPLES, 2);

	//srand(unsigned(time(0)));
	std::random_shuffle(fern_pool_sample_index1.begin(), fern_pool_sample_index1.end());
	std::random_shuffle(fern_pool_sample_index2.begin(), fern_pool_sample_index2.end());


	for (i = 0; i < NUM_SAMPLES; i++) {
	// ù��° location
	int index = fern_pool_sample_index1[i];
	r = floor(index / fern_angles.size());
	a = index % fern_angles.size();
	float *d = xy1.RowD(i);
	d[0] = cos(fern_angles[a]) * fern_radius[r];
	d[1] = sin(fern_angles[a]) * fern_radius[r];

	// �ι�° location
	index = fern_pool_sample_index2[i];
	r = floor(index / fern_angles.size());
	a = index % fern_angles.size();
	d = xy2.RowD(i);
	d[0] = cos(fern_angles[a]) * fern_radius[r];
	d[1] = sin(fern_angles[a]) * fern_radius[r];
	}*/

	int i;
	vector<float> angles, radius;

	xy1.Create(NUM_SAMPLES, 2);
	xy2.Create(NUM_SAMPLES, 2);

	float a = 0;
	while (a < 2 * 3.141952) {
		angles.push_back(a);
		a += 0.1;

		//if (angles.size() > 5) break;
	}

	float r = 0;
	while (r <= 1) {
		radius.push_back(r);
		r += 1 / 30.0;

		//if (radius.size() > 0) break;
	}

	vector<int> sample_index;
	for (i = 0; i < angles.size() * radius.size(); i++) {
		sample_index.push_back(i);
	}

	//srand(unsigned(time(0)));
	std::random_shuffle(sample_index.begin(), sample_index.end());


	for (i = 0; i < NUM_SAMPLES; i++) {
		int index = sample_index[i];
		r = floor(index / angles.size());
		a = index % angles.size();
		float *d = xy1.RowD(i);
		d[0] = cos(angles[a]) * radius[r];
		d[1] = sin(angles[a]) * radius[r];
	}


	std::random_shuffle(sample_index.begin(), sample_index.end());

	for (i = 0; i < NUM_SAMPLES; i++) {
		int index = sample_index[i];
		r = floor(index / angles.size());
		a = index % angles.size();
		float *d = xy2.RowD(i);
		d[0] = cos(angles[a]) * radius[r];
		d[1] = sin(angles[a]) * radius[r];
	}
}

/*
@brief fern pool�� ���� similarity transform�� ������ ��ǥ�� ���Ѵ�.
*/
void CLBFTrainer::compute_transformed_location(data_info &data, CFMat &xy1, CFMat &xy2, CFMat &t_xy1, CFMat &t_xy2, int landmark_index) {
	CFMat transformed_xy;
	CLBFCommon::apply_similarity_transform(xy1, t_xy1, data.q);
	CLBFCommon::apply_similarity_transform(xy2, t_xy2, data.q);

	float tx = data.current_pts.m_data[landmark_index * 2] - data.q[2];
	float ty = data.current_pts.m_data[landmark_index * 2 + 1] - data.q[3];

	float *pts1 = t_xy1.m_data;
	float *pts2 = t_xy2.m_data;
	//pd_features.Create(xy1.Row(), 1);
	//int *dst = pd_features.m_data;
	for (int i = 0; i < xy1.Row(); i++) {
		*pts1 += tx;
		*(pts1 + 1) += ty;
		*pts2 += tx;
		*(pts2 + 1) += ty;

		*pts1 = fa_max(0, fa_min(int(*pts1), data.pimg->img_width - 1));
		*(pts1 + 1) = fa_max(0, fa_min(int(*(pts1 + 1)), data.pimg->img_height - 1));
		*pts2 = fa_max(0, fa_min(int(*pts2), data.pimg->img_width - 1));
		*(pts2 + 1) = fa_max(0, fa_min(int(*(pts2 + 1)), data.pimg->img_height - 1));

		pts1 += 2;
		pts2 += 2;
	}
}

/*
@brief �� leaf node�� delta�� ����Ѵ�.
*/
void CLBFTrainer::compute_delta(CNode  &node, int landmark_index) {

	vector<int>::iterator iter = node.sampleIndexVector.begin();

	//printf("compute delta\n");
	float sum = 0;
	int idx = 0;
	float sum_x = 0, sum_y = 0;
	for (; iter < node.sampleIndexVector.end(); iter++) {
		data_info &data = training_data[*iter];
		float *res = data.shape_norm_residual.RowD(landmark_index);
		sum_x += res[0];
		sum_y += res[1];
		idx++;
	}
	node.delta_x = sum_x / idx;;
	node.delta_y = sum_y / idx;
}


/*
@brief random forest�� �̿��ؼ� shape�� update �Ѵ�.
*/
void CLBFTrainer::update_by_random_forest(int stage) {
	int i, j;
	vector<data_info>::iterator iter;
	for (iter = training_data.begin(); iter < training_data.end(); iter++) {
		data_info data = *iter;

		for (i = 0; i < VCNT; i++) {
			CFMat delta(1, 2);
			for (j = 0; j < N_TREES; j++) {
				CTree *tree = m_forest[stage].GetTree(i, j);

				CFMat delta_pts = tree->compute_delta(data, i);
				delta += delta_pts;
			}
			delta /= N_TREES;

			data.current_norm_pts.RowD(i)[0] += delta.m_data[0];
			data.current_norm_pts.RowD(i)[1] += delta.m_data[1];
		}

		CFMat temp;
		CLBFCommon::apply_similarity_transform(data.current_norm_pts, temp, data.q);
	}
}

/*
@brief �ش� stage�� forest�� ���� leaf node�� index�� �����Ѵ�.
*/
void CLBFTrainer::set_local_binary_feature_index(int stage) {
	int j, k, ii;
	int index = 0;
	int count;

	for (j = 0; j < VCNT; j++) { // Ư¡�� ��
		for (k = 0; k < N_TREES; k++) { // tree ��
			CTree *tree = m_forest[stage].GetTree(j, k);
			CNode *root = &(tree->root);

			// leaf node�� ���� index ����
			vector<CNode *> stack;
			stack.push_back(root);
			count = stack.size();
			while (count != 0) {
				set_leaf_node_index(stack, index);
				count = stack.size();
			}

			// tree�� ��ü node�� ���� index ����
			int node_index = 0;
			root->index = node_index;
			set_node_index(root, node_index);
		}
	}

	// local binary feature�� dimension ����
	m_forest[stage].set_local_binary_feature_dim(index);
}

/*
@brief leaf node�鿡 ���ؼ� tree�� ���ʺ��� 0���� �����ؼ� �� ������Ŵ
*/
void CLBFTrainer::set_leaf_node_index(vector<CNode *> &stack, int &index)
{
	CNode *node = stack.front();

	if (node->bLeaf == 1) {
		node->leaf_index = index;
		index++;
		stack.erase(stack.begin());

	}
	else {
		stack.erase(stack.begin());
		stack.push_back(node->m_left_node);
		stack.push_back(node->m_right_node);
	}
}

/*
@brief �ش� tree�� node���� root���� 1�� �ؼ� index�� ������
*/
void CLBFTrainer::set_node_index(CNode *node, int &index)
{
	if (node->m_left_node) {
		node->m_left_node->index = ++index;
	}
	if (node->m_right_node) {
		node->m_right_node->index = ++index;
	}

	if (node->m_left_node) {
		set_node_index(node->m_left_node, index);
	}
	if (node->m_right_node) {
		set_node_index(node->m_right_node, index);
	}
}

/*
@brief local binary feature�� global regression�� �н��Ѵ�.
*/
void CLBFTrainer::train_global_regression(int stage) {
	int i, j;
	CForest *stage_forest;

	stage_forest = &m_forest[stage];

	////////////////////////////////////////////////////////
	// training data�� local binary feature
	int *local_binary_features = new int[training_data.size() * VCNT * N_TREES];
	float *residual = new float[VCNT2 * training_data.size()];

	// training data�� ���� local feature mapping function ���
	vector<data_info>::iterator iter = training_data.begin();
	int idx = 0;
	for (; iter < training_data.end(); iter++) {
		data_info &data = *iter;

		// feature mapping function
		compute_local_binary_feature(data, stage_forest, local_binary_features + (idx)* (N_TREES * VCNT));

		// ground gruth
		for (i = 0; i < VCNT; i++) {
			residual[idx * VCNT * 2 + i * 2] = data.shape_norm_residual.m_data[i * 2];
			residual[idx * VCNT * 2 + i * 2 + 1] = data.shape_norm_residual.m_data[i * 2 + 1];
		}

		idx++;
	}

	// linear regression �н�
	CFMat W(VCNT2, m_forest[stage].lbf_dim);

	omp_set_num_threads(THREAD_NUM_4_REGRESSION);

	// �� Ư¡�� �� �н�
#pragma omp parallel for
	for (i = 0; i < VCNT2; i++) {
		float *res = new float[training_data.size()];
		for (int j = 0; j < training_data.size(); j++) {
			res[j] = residual[j * (VCNT2)+i];
		}
		learn_global_linear_regression(local_binary_features, N_TREES * VCNT, m_forest[stage].lbf_dim, training_data.size(), res, W.RowD(i));

		delete res;
	}

	// weight ����
	CFMat Wt;
	Wt.IsTransposed(W);
	m_global_regressor.set_weight(Wt, stage);


	// ����
	delete[] local_binary_features;
	delete residual;
}

/*
@brief ���� ���� local binary feature�� ����Ѵ�.
*/
void CLBFTrainer::compute_local_binary_feature(data_info &data, CForest *stage_forest, int *local_binary_features) {
	int i, j;

	omp_set_num_threads(THREAD_NUM_4_RANDOM_FOREST);

#pragma omp parallel for
	for (i = 0; i < VCNT; i++)
	{
		for (int j = 0; j < N_TREES; j++)
		{
			CTree *tree = stage_forest->GetTree(i, j);
			int idx = tree->compute_local_binary_feature(data, i);
			local_binary_features[i * N_TREES + j] = idx;
		}
	}
}

/*
@brief global regression�� �̿��ؼ� shape�� update �Ѵ�.
*/
void CLBFTrainer::update_by_global_regression(int stage)
{
	int i, j;
	i = 0;


	// training data�� ���� local binary feature 
	vector<data_info>::iterator iter = training_data.begin();
	for (; iter < training_data.end(); iter++, i++) {
		data_info data = *iter;

		update_by_global_regression(stage, data);
		update_shape_residual(stage, data);
		*iter = data;


	}
}

/*
@brief global regression�� �̿��ؼ� shape�� update �Ѵ�.
*/
void CLBFTrainer::update_by_global_regression(int stage, data_info &data) {
	CForest *stage_forest;

	stage_forest = &m_forest[stage];

	CIMat local_binary_features(VCNT * N_TREES, 1);

	// feature mapping function
	compute_local_binary_feature(data, stage_forest, local_binary_features.m_data);

	// update
	CFMat delta = m_global_regressor.update_residual(stage, local_binary_features);

	delta.Reshape(VCNT, 2);

	data.current_norm_pts += delta;

	CFMat temp = data.current_pts;
	CLBFCommon::apply_similarity_transform(data.current_norm_pts, data.current_pts, data.q);
}

/*
@brief �ش� data�� ���� ������ shape�� gt�� ���Ͽ� residual�� ����� �� normalize �Ѵ�.
*/
void CLBFTrainer::update_shape_residual(int stage, data_info &data) {

	float temp_q[4], temp_inv_q[4];
	CLBFCommon::compute_similarity(mean_shape, data.current_pts, data.inv_q);
	CLBFCommon::compute_inverse_similarity(data.inv_q, data.q);
	CLBFCommon::apply_similarity_transform(data.current_pts, data.current_norm_pts, data.inv_q);

	CLBFCommon::apply_similarity_transform(data.current_norm_pts, data.current_pts, data.q);
	CLBFCommon::compute_inverse_similarity(data.q, data.inv_q);
	CLBFCommon::apply_similarity_transform(data.pimg->gt, data.norm_gt, data.inv_q);


	// compute residual
	data.shape_residual = data.pimg->gt - data.current_pts;
	data.shape_norm_residual = data.norm_gt - data.current_norm_pts;
}

/*
@brief training ������ average normalized error�� ������
*/
void CLBFTrainer::measure() {
	int i, j;
	double avg_norm_error = 0;
	vector<data_info>::iterator iter = training_data.begin();
	int idx = 0;
	for (; iter < training_data.end(); iter++, idx++) {
		data_info data = *iter;
		CFMat res = data.current_pts - data.pimg->gt;

		float norm_error = 0;
		for (i = 0; i < res.Row(); i++) {
			float *pt = res.RowD(i);
			norm_error += sqrt(pt[0] * pt[0] + pt[1] * pt[1]);
		}

		///////////////////////////
		// �� ����
		int dis = 0;

		if (data.pimg->gt.Row() == 51) {
			dis = 17;
		}

		float left_eye[2] = { 0, }, right_eye[2] = { 0, };
		for (i = 0; i < LEFT_EYE_IDX_CNT; i++) {
			left_eye[0] += data.pimg->gt.m_data[(LEFT_EYE_IDX[i] - dis) * 2];
			left_eye[1] += data.pimg->gt.m_data[(LEFT_EYE_IDX[i] - dis) * 2 + 1];
		}
		left_eye[0] /= LEFT_EYE_IDX_CNT;
		left_eye[1] /= LEFT_EYE_IDX_CNT;

		for (i = 0; i < RIGHT_EYE_IDX_CNT; i++) {
			right_eye[0] += data.pimg->gt.m_data[(RIGHT_EYE_IDX[i] - dis) * 2];
			right_eye[1] += data.pimg->gt.m_data[(RIGHT_EYE_IDX[i] - dis) * 2 + 1];
		}
		right_eye[0] /= RIGHT_EYE_IDX_CNT;
		right_eye[1] /= RIGHT_EYE_IDX_CNT;

		float D_iod = sqrt((left_eye[0] - right_eye[0]) * (left_eye[0] - right_eye[0]) + (left_eye[1] - right_eye[1]) * (left_eye[1] - right_eye[1]));
		///////////////////////////

		norm_error /= D_iod;
		norm_error /= res.Row();

		avg_norm_error += norm_error;
	}
	avg_norm_error /= training_data.size();

	printf("  average normalized error : %f\n", avg_norm_error * 100);
}

void CLBFTrainer::save_result_images() {
	int i;
	for (i = 0; i < training_data.size(); i++)
	{
		data_info data = training_data[i];
		CImageProcessing m_ip;
		cv::Mat image, colorImage;
		image.create(data.pimg->img_height, data.pimg->img_width, CV_8UC1);
		m_ip.setraw(image, data.pimg->img_width, data.pimg->img_height, 1, data.pimg->img);
		cv::cvtColor(image, colorImage, CV_GRAY2BGR);
		if (data.pimg->fd_enabled == true) {
			m_ip.draw_rect(colorImage, data.pimg->fd.left, data.pimg->fd.top, data.pimg->fd.right - data.pimg->fd.left, data.pimg->fd.bottom - data.pimg->fd.top, 255, 0, 0, 2);
		}
		for (int i = 0; i < VCNT; i++) {
			m_ip.draw_circle(colorImage, data.pimg->gt.m_data[i * 2], data.pimg->gt.m_data[i * 2 + 1], 2, 255, 0, 0, 2);
			m_ip.draw_circle(colorImage, data.current_pts.m_data[i * 2], data.current_pts.m_data[i * 2 + 1], 2, 0, 255, 0, 2);
			m_ip.draw_circle(colorImage, data.initial_pts.m_data[i * 2], data.initial_pts.m_data[i * 2 + 1], 2, 0, 0, 255, 2);
		}

		char write_name[256];
		sprintf(write_name, "result_%05d.jpg", i);
		imwrite(write_name, colorImage);
	}
}

/*
@brief �н��� ����� header ���Ϸ� �����Ѵ�.
*/
void CLBFTrainer::save_parameters(char *output_path) {
	int i, j, k;

	char output_dat_path[512];
	sprintf(output_dat_path, "%s.dat", output_path);

	FILE *fp = fopen(output_path, "wt");
	FILE *fp_dat = fopen(output_dat_path, "wb");

	fprintf(fp, "#ifndef LBF_PARAMETERS_H_\n");
	fprintf(fp, "#define LBF_PARAMETERS_H_\n");

	fprintf(fp, "int NUM_STAGE = %d;\n", T);
	fprintf(fp, "int NUM_TREES = %d;\n", N_TREES);
	fprintf(fp, "int NUM_VERTEX = %d;\n", VCNT);

	fwrite(&T, sizeof(int), 1, fp_dat);
	fwrite(&N_TREES, sizeof(int), 1, fp_dat);
	fwrite(&VCNT, sizeof(int), 1, fp_dat);


	// global regressor
	fprintf(fp, "int REGRESSOR_SIZE[] = {");
	for (i = 0; i < T; i++) {
		CFMat *W = m_global_regressor.get_weight(i);
		if (i == T - 1) {
			fprintf(fp, "%d, %d};\n", W->Row(), W->Col());
		}
		else {
			fprintf(fp, "%d, %d, ", W->Row(), W->Col());
		}

		int w = W->Row();
		int h = W->Col();
		fwrite(&w, sizeof(int), 1, fp_dat);
		fwrite(&h, sizeof(int), 1, fp_dat);
	}
	fprintf(fp, "int GLOBAL_REGRESSOR[] = {");
	int write_cnt = 0;
	for (i = 0; i < T; i++) {
		CFMat *W = m_global_regressor.get_weight(i);
		for (j = 0; j < W->Row() * W->Col(); j++) {
			if (i == T - 1 && j == W->Row() * W->Col() - 1) {
				fprintf(fp, "%d};\n", (int)(W->m_data[j] * (2 << 20)));
				//fprintf(fp, "%f};\n", W->m_data[j]);
			}
			else {
				fprintf(fp, "%d, ", (int)(W->m_data[j] * (2 << 20)));
				//fprintf(fp, "%f, ", W->m_data[j]);
			}

			if (++write_cnt % 100 == 0)
				fprintf(fp, "\n");

			int temp = (int)(W->m_data[j] * (2 << 20));
			fwrite(&temp, sizeof(int), 1, fp_dat);
		}
		//fwrite(W->m_data, sizeof(float), W->Row() * W->Col(), fp_dat);
	}

	// mean shape
	write_cnt = 0;
	fprintf(fp, "float MEAN_SHAPE[] = {");
	for (i = 0; i < VCNT2; i++) {
		if (i == VCNT2 - 1)
			fprintf(fp, "%f};\n", mean_shape.m_data[i]);
		else
			fprintf(fp, "%f, ", mean_shape.m_data[i]);

		if (++write_cnt % 100 == 0)
			fprintf(fp, "\n");
	}

	fwrite(mean_shape.m_data, sizeof(float), VCNT2, fp_dat);



	// random forest
	write_cnt = 0;
	fprintf(fp, "short TOTAL_NUM_NODES[] = {");
	for (i = 0; i < T; i++) {
		for (j = 0; j < VCNT; j++) {
			for (k = 0; k < N_TREES; k++) {
				CTree *tree = m_forest[i].GetTree(j, k);
				CNode *root = &(tree->root);

				int total_node_num = tree->total_number_of_nodes();

				if (i == T - 1 && j == VCNT - 1 && k == N_TREES - 1)
					fprintf(fp, "%d};\n", total_node_num);
				else
					fprintf(fp, "%d, ", total_node_num);

				if (++write_cnt % 100 == 0)
					fprintf(fp, "\n");

				fwrite(&total_node_num, sizeof(int), 1, fp_dat);
			}
		}
	}

	write_cnt = 0;
	fprintf(fp, "short TREE_CHILDREN_INDEX[] = {");
	for (i = 0; i < T; i++) {
		for (j = 0; j < VCNT; j++) {
			for (k = 0; k < N_TREES; k++) {
				CTree *tree = m_forest[i].GetTree(j, k);
				CNode *root = &(tree->root);

				vector <CNode *>node_vector;
				node_vector.push_back(root);
				make_tree_to_vector(node_vector, root);

				for (int ii = 0; ii < node_vector.size(); ii++) {
					CNode *node = node_vector[ii];
					if (node->m_left_node && node->m_right_node) {
						fprintf(fp, "%d, %d", node->m_left_node->index, node->m_right_node->index);
						fwrite(&(node->m_left_node->index), sizeof(int), 1, fp_dat);
						fwrite(&(node->m_right_node->index), sizeof(int), 1, fp_dat);

					}
					else {
						fprintf(fp, "%d, %d", -1, -1);
						int temp = -1;
						fwrite(&temp, sizeof(int), 1, fp_dat);
						fwrite(&temp, sizeof(int), 1, fp_dat);
					}

					if (i == T - 1 && j == VCNT - 1 && k == N_TREES - 1 && ii == node_vector.size() - 1)
						fprintf(fp, "};\n");
					else
						fprintf(fp, ", ");

					if (++write_cnt % 100 == 0)
						fprintf(fp, "\n");

				}
			}
		}
	}

	write_cnt = 0;
	fprintf(fp, "float TREE_FERN_PARAMS[] = {");
	for (i = 0; i < T; i++) {
		for (j = 0; j < VCNT; j++) {
			for (k = 0; k < N_TREES; k++) {
				CTree *tree = m_forest[i].GetTree(j, k);
				CNode *root = &(tree->root);

				vector <CNode *>node_vector;
				node_vector.push_back(root);
				make_tree_to_vector(node_vector, root);

				for (int ii = 0; ii < node_vector.size(); ii++) {
					CNode *node = node_vector[ii];
					fprintf(fp, "%f, %f, %f, %f", node->x1, node->y1, node->x2, node->y2);

					if (i == T - 1 && j == VCNT - 1 && k == N_TREES - 1 && ii == node_vector.size() - 1)
						fprintf(fp, "};\n");
					else
						fprintf(fp, ", ");

					if (++write_cnt % 100 == 0)
						fprintf(fp, "\n");

					fwrite(&(node->x1), sizeof(float), 1, fp_dat);
					fwrite(&(node->y1), sizeof(float), 1, fp_dat);
					fwrite(&(node->x2), sizeof(float), 1, fp_dat);
					fwrite(&(node->y2), sizeof(float), 1, fp_dat);
				}
			}
		}
	}

	write_cnt = 0;
	fprintf(fp, "short TREE_THRESHOLD_PARAMS[] = {");
	for (i = 0; i < T; i++) {
		for (j = 0; j < VCNT; j++) {
			for (k = 0; k < N_TREES; k++) {
				CTree *tree = m_forest[i].GetTree(j, k);
				CNode *root = &(tree->root);

				vector <CNode *>node_vector;
				node_vector.push_back(root);
				make_tree_to_vector(node_vector, root);

				for (int ii = 0; ii < node_vector.size(); ii++) {
					CNode *node = node_vector[ii];
					fprintf(fp, "%d", (int)node->thr);

					if (i == T - 1 && j == VCNT - 1 && k == N_TREES - 1 && ii == node_vector.size() - 1)
						fprintf(fp, "};\n");
					else
						fprintf(fp, ", ");

					if (++write_cnt % 100 == 0)
						fprintf(fp, "\n");

					int thr = (int)node->thr;
					fwrite(&thr, sizeof(int), 1, fp_dat);
				}
			}
		}
	}

	write_cnt = 0;
	fprintf(fp, "short TREE_LEAF_INDEX[] = {");
	for (i = 0; i < T; i++) {
		for (j = 0; j < VCNT; j++) {
			for (k = 0; k < N_TREES; k++) {
				CTree *tree = m_forest[i].GetTree(j, k);
				CNode *root = &(tree->root);

				vector <CNode *>node_vector;
				node_vector.push_back(root);
				make_tree_to_vector(node_vector, root);

				for (int ii = 0; ii < node_vector.size(); ii++) {
					CNode *node = node_vector[ii];
					fprintf(fp, "%d", node->leaf_index);

					if (i == T - 1 && j == VCNT - 1 && k == N_TREES - 1 && ii == node_vector.size() - 1)
						fprintf(fp, "};\n");
					else
						fprintf(fp, ", ");

					if (++write_cnt % 100 == 0)
						fprintf(fp, "\n");

					fwrite(&(node->leaf_index), sizeof(int), 1, fp_dat);
				}
			}
		}
	}


	fprintf(fp, "#endif");

	fclose(fp);
	fclose(fp_dat);
}

void CLBFTrainer::make_tree_to_vector(vector <CNode *> &node_vector, CNode *node) {
	if (node->m_left_node) {
		node_vector.push_back(node->m_left_node);
	}
	if (node->m_right_node) {
		node_vector.push_back(node->m_right_node);
	}
	if (node->m_left_node) {
		make_tree_to_vector(node_vector, node->m_left_node);
	}
	if (node->m_right_node) {
		make_tree_to_vector(node_vector, node->m_right_node);
	}
}





