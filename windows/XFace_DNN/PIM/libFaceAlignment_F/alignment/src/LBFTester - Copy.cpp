#include "LBFTester.h"
#include "LBFCommon.h"
#include "3D_face_shape.h"
#include "define_op.h"
#include "../../common/src/MVFD_Interface.h"
#include "../../common/src/HP_Interface.h"
#include "Util.h"
// #include "ImageProcess.h"
// #include "ImageProcessing.h"
#include "../../common/src/FA_ImageProcess.h"



#include "../../../exp/PIM_Define_op.h"

#define LOAD_BINARY_FILE

#ifndef LOAD_BINARY_FILE
#include "LBF_parameters_ver1.01.h"
#else
#define DAT_FILE "LBF_parameters.h.dat"
#endif



#define IMG_REPEAT_CNT 6
#define ED_ANGLE_CORRECTION 20

int POSE_POINT_INDEX[] = { 21, 22, 30, 31, 33, 35, 36, 39, 42, 45, 48, 54 };
int N_POSE_POINT_INDEX = 12;

using namespace cv;

FD_RESULT result;
FD_CONTEXT *pContext;

int m_face_cnt;

CLBFTester::CLBFTester()
{
#ifndef LOAD_BINARY_FILE

	T = NUM_STAGE;
	N_TREES = NUM_TREES;
	VCNT = NUM_VERTEX;
	VCNT2 = VCNT * 2;

	// global regressor
	int weight_cnt_sum = 0;
	m_global_regressor.set_num_stage(T);
	for (int i = 0; i < T; i++) {
		CIMat W;
		W.Wrap(REGRESSOR_SIZE[i * 2], REGRESSOR_SIZE[i * 2 + 1], GLOBAL_REGRESSOR + weight_cnt_sum);
		weight_cnt_sum += REGRESSOR_SIZE[i * 2] * REGRESSOR_SIZE[i * 2 + 1];
		m_global_regressor.set_weight(W, i);
	}

	// mean shape
	mean_shape.Wrap(VCNT, 2, MEAN_SHAPE);
	mean_shape_bound = CLBFCommon::compute_bbox(mean_shape);

	// random forest
	m_forest = new CForest[T];
	for (int i = 0; i < T; i++) {
		m_forest[i].initialize(N_TREES, VCNT);
	}

	short *child_index = TREE_CHILDREN_INDEX;
	short *total_num_nodes = TOTAL_NUM_NODES;
	float *tree_fern_params = TREE_FERN_PARAMS;
	short *threshold_params = TREE_THRESHOLD_PARAMS;
	short *tree_leaf_index = TREE_LEAF_INDEX;
	int sum_node_cnt = 0;
	int tree_index = 0;
	for (int i = 0; i < T; i++) {
		for (int j = 0; j < VCNT; j++) {
			for (int k = 0; k < N_TREES; k++) {
				CTree *tree = m_forest[i].GetTree(j, k);
				CNode *root = &(tree->root);

				root->index = 0;
				int node_cnt = total_num_nodes[tree_index];
				create_child_nodes(tree, node_cnt, child_index + sum_node_cnt * 2,
													tree_fern_params + sum_node_cnt * 4,
													threshold_params + sum_node_cnt,
													tree_leaf_index + sum_node_cnt);
				tree_index++;
				sum_node_cnt += node_cnt;
			}
		}
	}
#else
	FILE *fp = fopen(DAT_FILE, "rb");
	fread(&T, sizeof(int), 1, fp);
	fread(&N_TREES, sizeof(int), 1, fp);
	fread(&VCNT, sizeof(int), 1, fp);
	VCNT2 = VCNT * 2;

	// global regressor
	int weight_cnt_sum = 0;
	m_global_regressor.set_num_stage(T);
	for (int i = 0; i < T; i++) {
		int w, h;
		fread(&h, sizeof(int), 1, fp);
		fread(&w, sizeof(int), 1, fp);
		CIMat W(h, w);
		m_global_regressor.set_weight(W, i);
	}

	for (int i = 0; i < T; i++) {
		CIMat *W = m_global_regressor.get_weight(i);
		fread(W->m_data, sizeof(int), W->Row() * W->Col(), fp);
	}

	// mean shape
	int w;
	mean_shape.Create(VCNT, 2);
	fread(mean_shape.m_data, sizeof(float), VCNT2, fp);
	mean_shape_bound = CLBFCommon::compute_bbox(mean_shape);

	// random forest
	m_forest = new CForest[T];
	for (int i = 0; i < T; i++) {
		m_forest[i].initialize(N_TREES, VCNT);
	}

	for (int i = 0; i < T; i++) {
		for (int j = 0; j < VCNT; j++) {
			for (int k = 0; k < N_TREES; k++) {
				CTree *tree = m_forest[i].GetTree(j, k);
				int node_cnt = 0;
				fread(&node_cnt, sizeof(int), 1, fp);
				tree->node_cnt = node_cnt;
			}
		}
	}

	for (int i = 0; i < T; i++) {
		for (int j = 0; j < VCNT; j++) {
			for (int k = 0; k < N_TREES; k++) {
				CTree *tree = m_forest[i].GetTree(j, k);
				int node_cnt = tree->node_cnt;

				int *child_list = new int[node_cnt * 2];
				short *short_child_list = new short[node_cnt * 2];
				fread(child_list, sizeof(int), node_cnt * 2, fp);
				for (int ii = 0; ii < node_cnt * 2; ii++) {
					short_child_list[ii] = child_list[ii];
				}
				delete[]child_list;
				tree->tree_children_list = short_child_list;
			}
		}
	}

	for (int i = 0; i < T; i++) {
		for (int j = 0; j < VCNT; j++) {
			for (int k = 0; k < N_TREES; k++) {
				CTree *tree = m_forest[i].GetTree(j, k);
				int node_cnt = tree->node_cnt;

				float *fern_param_list = new float[node_cnt * 4];
				fread(fern_param_list, sizeof(float), node_cnt * 4, fp);
				tree->fern_param_list = fern_param_list;
			}
		}
	}

	for (int i = 0; i < T; i++) {
		for (int j = 0; j < VCNT; j++) {
			for (int k = 0; k < N_TREES; k++) {
				CTree *tree = m_forest[i].GetTree(j, k);
				int node_cnt = tree->node_cnt;

				int *threshold_list = new int[node_cnt];
				short *short_threshold_list = new short[node_cnt];
				fread(threshold_list, sizeof(int), node_cnt, fp);
				for (int ii = 0; ii < node_cnt; ii++) {
					short_threshold_list[ii] = threshold_list[ii];
				}
				delete[]threshold_list;
				tree->threshold_list = short_threshold_list;
			}
		}
	}

	for (int i = 0; i < T; i++) {
		for (int j = 0; j < VCNT; j++) {
			for (int k = 0; k < N_TREES; k++) {
				CTree *tree = m_forest[i].GetTree(j, k);
				int node_cnt = tree->node_cnt;

				int *leaf_index_list = new int[node_cnt];
				short *short_leaf_index_list = new short[node_cnt];
				fread(leaf_index_list, sizeof(int), node_cnt, fp);
				for (int ii = 0; ii < node_cnt; ii++) {
					short_leaf_index_list[ii] = leaf_index_list[ii];
				}
				delete[]leaf_index_list;
				tree->leaf_index_list = short_leaf_index_list;
			}
		}
	}

	fclose(fp);


#endif

	MVFD_Init_Header();
	MVFD_AllocResult(&result, MAX_FACE_NUM);
	pContext = MVFD_CreateContext();
	HP_Init();


	///////////////////////////////

	for( int i = 0; i < MAX_FACE_NUM; i++ )
	{
		printf("Initialize FFT Constructor() ============================================\n");
		m_data[i].current_pts.Clear();
		m_data[i].initial_pts.Clear();
		m_data[i].current_norm_pts.Clear();
		memset(m_data[i].q, 0, sizeof(float) * 4);
		memset(m_data[i].inv_q, 0, sizeof(float) * 4);
		m_data[i].nROP = 0;
		m_data[i].nHitCnt = -10;
		m_data[i].nUnknownHitCnt = -15;
		m_data[i].nRecogID = -2;
		m_data[i].nRankOneID = -2;
		m_data[i].fConfidence = 0.0;
		m_data[i].nGenderType = 0;
		m_data[i].nPermanentID = -1;
		m_data[i].TRACKING_STATUS = 0;
		memset(m_data[i].nRecogIDs, -2, sizeof(int) * 20);
		
		m_prev_data[i].current_pts.Clear();
		m_prev_data[i].initial_pts.Clear();
		m_prev_data[i].current_norm_pts.Clear();
		memset(m_prev_data[i].q, 0, sizeof(float) * 4);
		memset(m_prev_data[i].inv_q, 0, sizeof(float) * 4);
		m_prev_data[i].nROP = 0;
		m_prev_data[i].nHitCnt = -10;
		m_prev_data[i].nUnknownHitCnt = -15;
		m_prev_data[i].nRecogID = -2;
		m_prev_data[i].nRankOneID = -2;
		m_prev_data[i].fConfidence = 0.0;
		m_prev_data[i].nGenderType = 0;
		m_prev_data[i].nPermanentID = -1;
		m_prev_data[i].TRACKING_STATUS = 0;
		memset(m_prev_data[i].nRecogIDs, -2, sizeof(int) * 20);

		fd_info.id[i] = -1;		
		memset(&(fd_info.boundRect[i]), 0, sizeof(FDRECT) * 1);
	}
	fd_info.prev_cnt = 0;
	fd_info.cnt = 0;

	
	//////////////////////////////
	std::vector<CvPoint3D32f> modelPoints;

	float *pt = SHAPE_3D_MODEL_MEAN_SHAPE;
	for (int i = 0; i < N_POSE_POINT_INDEX; i++) {
		modelPoints.push_back(cvPoint3D32f(*(pt + POSE_POINT_INDEX[i] * 3), *(pt + POSE_POINT_INDEX[i] * 3 + 1), *(pt + POSE_POINT_INDEX[i] * 3 + 2)));
	}
	positObject = cvCreatePOSITObject(&modelPoints[0], static_cast<int>(modelPoints.size()));


}

CLBFTester::~CLBFTester() {
	delete[]m_forest;

	MVFD_Release();
	MVFD_FreeResult(&result);
	MVFD_DestroyContext(pContext);
	HP_Release();

	cvReleasePOSITObject(&positObject);
}

/*
@brief 해당 tree의 child node들을 list 형태로 만들어준다. 그냥 link만 연결시킴
*/
void CLBFTester::create_child_nodes(CTree *tree, int node_cnt, short *child_index, float *tree_fern_params, short *threshold_params, short *leaf_index)
{
	tree->leaf_index_list = leaf_index;
	tree->fern_param_list = tree_fern_params;
	tree->threshold_list = threshold_params;
	tree->tree_children_list = child_index;
	tree->node_cnt = node_cnt;
}




int CLBFTester::detect_img(img_info &img)
{
	int detected_face_cnt = DetectFaceEye(img.img, img.img_width, img.img_height);
		
	for( int i = 0; i < fd_info.cnt && i < MAX_FACE_NUM; i++ )
	{
		data_info data, temp_data;
		data.pimg = &img;
		initialize_data(data, i);

		temp_data = data;
		img_info temp_img;
		temp_img = img;
		data.pimg = &temp_img;
		resize_face_rect(data, i);

		align(data);

		back_face_rect(data, i);
		data.pimg = &img;

		m_data[i] = data;

	}

	return m_face_cnt = fa_min(fd_info.cnt, MAX_FACE_NUM);
}



/*
@brief 영상을 입력으로 주고, align을 한다.
*/
int CLBFTester::detect(img_info &img)
{
	int detected_face_cnt = DetectFaceEye(img.img, img.img_width, img.img_height);

// 	if( fd_info.prev_cnt != fd_info.cnt )
// 	{
// 		TRACKING_STATUS = 0;
// 	}

#if 1
	if( detected_face_cnt > 0 )
	{
		int l, t, r, b;
		Rect r1, r2, r3;
		
		for( int i = 0; i < detected_face_cnt; ++i )
		{
			fd_info.id[i] = -1;
			r1.x = fd_info.boundRect[i].left;
			r1.y = fd_info.boundRect[i].top;
			r1.width = fd_info.boundRect[i].width;
			r1.height = fd_info.boundRect[i].height;

			for( int j = 0; j < m_face_cnt; ++j )
			{
				l = m_data[j].current_pts.m_data[0];
				t = m_data[j].current_pts.m_data[1];
				r = m_data[j].current_pts.m_data[16 * 2];
				b = m_data[j].current_pts.m_data[8 * 2 + 1];

				r2.x = l;
				r2.y = t;
				r2.width = abs(r - l);
				r2.height = abs(b - t);

				r3 = r1 & r2;
				if( r3.area() != 0 )
				{
					fd_info.id[i] = j;
				}
			}
		}
		

		memcpy(m_prev_data, m_data, sizeof(data_info)*MAX_FACE_NUM);

		if( TRACKING_STATUS == 0 )
		{
			for( int i = 0; i < fd_info.cnt && i < MAX_FACE_NUM; i++ )
			{
				data_info data, temp_data;
				data.pimg = &img;
				initialize_data(data, i);

				temp_data = data;
				img_info temp_img;
				temp_img = img;
				data.pimg = &temp_img;
				resize_face_rect(data, i);

				align(data);

				back_face_rect(data, i);
				data.pimg = &img;

				m_data[i] = data;

// 				if( fd_info.id[i] != -1 )
// 				{
// 					m_data[i].nRecogID = m_prev_data[fd_info.id[i]].nRecogID;
// 					m_data[i].fConfidence = m_prev_data[fd_info.id[i]].fConfidence;
// 					m_data[i].nHitCnt = m_prev_data[fd_info.id[i]].nHitCnt;
// 					m_data[i].nUnknownHitCnt = m_prev_data[fd_info.id[i]].nUnknownHitCnt;
// 					m_data[i].nPermanentID = m_prev_data[fd_info.id[i]].nPermanentID;
// 					m_data[i].nRankOneID = m_prev_data[fd_info.id[i]].nRankOneID;
// 				}
			}
		}
		else
		{
			for( int i = 0; i < fd_info.cnt && i < MAX_FACE_NUM; i++ )
			{
				if( fd_info.id[i] != -1 )
				{
// 					m_data[i].nRecogID = m_prev_data[fd_info.id[i]].nRecogID;
// 					m_data[i].fConfidence = m_prev_data[fd_info.id[i]].fConfidence;
// 					m_data[i].nHitCnt = m_prev_data[fd_info.id[i]].nHitCnt;
// 					m_data[i].nUnknownHitCnt = m_prev_data[fd_info.id[i]].nUnknownHitCnt;
// 					m_data[i].nPermanentID = m_prev_data[fd_info.id[i]].nPermanentID;
// 					m_data[i].nRankOneID = m_prev_data[fd_info.id[i]].nRankOneID;

					m_data[i] = m_prev_data[fd_info.id[i]];
				}
			}
		}
	}
	
#else
	if( TRACKING_STATUS == 0 )
	{
		for( int i = 0; i < fd_info.cnt && i < MAX_FACE_NUM; i++ ) 
		{
			data_info data, temp_data;
			data.pimg = &img;
			initialize_data(data, i);

			temp_data = data;
			img_info temp_img;
			temp_img = img;
			data.pimg = &temp_img;
			resize_face_rect(data, i);

			align(data);

			back_face_rect(data, i);
			data.pimg = &img;

			m_data[i] = data;
		}
	}
#endif


// 	fd_info.prev_cnt = fa_min(fd_info.cnt, MAX_FACE_NUM);
	return m_face_cnt = fa_min(fd_info.cnt, MAX_FACE_NUM);
}


int CLBFTester::detect3(img_info &img)
{
	int detected_face_cnt = DetectFaceEye(img.img, img.img_width, img.img_height);	

	if( fd_info.prev_cnt != detected_face_cnt )
	{
		int l, t, r, b;
		Rect r1, r2, r3;

		for( int i = 0; i < MAX_FACE_NUM; i++ )
		{
			m_data[i].current_pts.Clear();
			m_data[i].initial_pts.Clear();
			m_data[i].current_norm_pts.Clear();

			memset(m_data[i].q, 0, sizeof(float) * 4);
			memset(m_data[i].inv_q, 0, sizeof(float) * 4);
			m_data[i].nROP = 0;
			m_data[i].nHitCnt = 0;
			m_data[i].nUnknownHitCnt = -1;
			m_data[i].nRecogID = -2;
			m_data[i].nRankOneID = -2;
			m_data[i].fConfidence = 0.0;
			m_data[i].nGenderType = 0;
			m_data[i].nPermanentID = -1;
			m_data[i].TRACKING_STATUS = 0;
			memset(m_data[i].nRecogIDs, -2, sizeof(int) * 20);
		}

		for( int i = 0; i < detected_face_cnt; ++i )
		{
			r1.x = fd_info.boundRect[i].left;
			r1.y = fd_info.boundRect[i].top;
			r1.width = fd_info.boundRect[i].width;
			r1.height = fd_info.boundRect[i].height;

			if( fd_info.prev_cnt > 0 )
			{
				for( int j = 0; j < fd_info.prev_cnt; ++j )
				{
					l = m_prev_data[j].current_pts.m_data[0];
					t = m_prev_data[j].current_pts.m_data[1];
					r = m_prev_data[j].current_pts.m_data[16 * 2];
					b = m_prev_data[j].current_pts.m_data[8 * 2 + 1];

					r2.x = l;
					r2.y = t;
					r2.width = abs(r - l);
					r2.height = abs(b - t);

					r3 = r1 & r2;
					if( r3.area() != 0 )
					{
						fd_info.id[i] = j;
												
						m_data[i].current_pts.CopyFrom(m_prev_data[j].current_pts);
						m_data[i].initial_pts.CopyFrom(m_prev_data[j].initial_pts);
						m_data[i].current_norm_pts.CopyFrom(m_prev_data[j].current_norm_pts);

						memcpy(m_data[i].q, m_prev_data[j].q, sizeof(float) * 4);
						memcpy(m_data[i].inv_q, m_prev_data[j].inv_q, sizeof(float) * 4);
						m_data[i].nROP = fd_info.rop_angle[i]; //////////////////////////////////////////////////////////////////
						m_data[i].nHitCnt = m_prev_data[j].nHitCnt;
						m_data[i].nUnknownHitCnt = m_prev_data[j].nUnknownHitCnt;
						m_data[i].nRecogID = m_prev_data[j].nRecogID;
						m_data[i].nRankOneID = m_prev_data[j].nRankOneID;
						m_data[i].fConfidence = m_prev_data[j].fConfidence;
						m_data[i].nGenderType = m_prev_data[j].nGenderType;
						m_data[i].nPermanentID = m_prev_data[j].nPermanentID;
						m_data[i].TRACKING_STATUS = m_prev_data[j].TRACKING_STATUS;

						memcpy(m_data[i].nRecogIDs, m_prev_data[j].nRecogIDs, sizeof(int) * 20);
						break;
					}
					else
					{
						fd_info.id[i] = fd_info.prev_cnt;
						m_data[i].nROP = fd_info.rop_angle[i];
						m_data[i].TRACKING_STATUS = 0;
					}
				}
			}
			else
			{
				if( i > 0 )
				{
					fd_info.id[i] = fd_info.id[i - 1] + 1;
				}
				else
				{
					fd_info.id[i] = 0;
				}
				m_data[i].TRACKING_STATUS =  0;
				m_data[i].nROP = fd_info.rip_angle[i];
			}
		}
	}
	if( detected_face_cnt == 0 )
	{
		for( int i = 0; i < MAX_FACE_NUM; i++ )
		{
			m_data[i].current_pts.Clear();
			m_data[i].initial_pts.Clear();
			m_data[i].current_norm_pts.Clear();

			memset(m_data[i].q, 0.0, sizeof(float) * 4);
			memset(m_data[i].inv_q, 0.0, sizeof(float) * 4);
			m_data[i].nROP = 0;
			m_data[i].nHitCnt = 0;
			m_data[i].nUnknownHitCnt = -1;
			m_data[i].nRecogID = -2;
			m_data[i].nRankOneID = -2;
			m_data[i].fConfidence = 0.0;
			m_data[i].nGenderType = 0;
			m_data[i].nPermanentID = -1;
			m_data[i].TRACKING_STATUS = 0;
			memset(m_data[i].nRecogIDs, -2, sizeof(int) * 20);

			m_prev_data[i].current_pts.Clear();
			m_prev_data[i].initial_pts.Clear();
			m_prev_data[i].current_norm_pts.Clear();

			memset(m_prev_data[i].q, 0.0, sizeof(float) * 4);
			memset(m_prev_data[i].inv_q, 0.0, sizeof(float) * 4);
			m_prev_data[i].nROP = 0;
			m_prev_data[i].nHitCnt = -1;
			m_prev_data[i].nUnknownHitCnt = -1;
			m_prev_data[i].nRecogID = -2;
			m_prev_data[i].nRankOneID = -2;
			m_prev_data[i].fConfidence = 0.0;
			m_prev_data[i].nGenderType = -1;
			m_prev_data[i].nPermanentID = -1;
			m_prev_data[i].TRACKING_STATUS = 0;	
			memset(m_prev_data[i].nRecogIDs, -2, sizeof(int) * 20);
		}
	}

	for( int i = 0; i < detected_face_cnt && i < MAX_FACE_NUM; i++ )
	{
		if( m_data[i].TRACKING_STATUS != 1 )
		{
			data_info data, temp_data;
			memset(&data, 0, sizeof(data_info));
			memset(&temp_data, 0, sizeof(data_info));
			data.pimg = &img;
			initialize_data(data, i);

			temp_data = data;
			img_info temp_img;
			temp_img = img;
			data.pimg = &temp_img;
			resize_face_rect(data, i);

			bool is_align = align(data);

			back_face_rect(data, i);
			data.pimg = &img;

			m_data[i] = data;
			m_data[i].nHitCnt = -1;
			m_data[i].nUnknownHitCnt = -1;
			m_data[i].nPermanentID = -1;
			m_data[i].fConfidence = 0.0;
			m_data[i].nRecogID = -2;
			m_data[i].nRankOneID = -2;
			memset(m_data[i].nRecogIDs, -2, sizeof(int) * 20);

			if( is_align )
			{
				m_data[i].TRACKING_STATUS = 1;
			}
			else
			{
				m_data[i].TRACKING_STATUS = 0;
			}			
		}
		m_data[i].nROP = fd_info.rop_angle[i];
	}
	
	return m_face_cnt = fa_min(fd_info.cnt, MAX_FACE_NUM);
}


int CLBFTester::track3(img_info &img)
{
	int aligned_face_cnt = 0;

	int fd_count = detect3(img);
	
	for( int i = 0; i < fd_count && i < MAX_FACE_NUM; i++ )
	{
		
		//if( m_data[i].TRACKING_STATUS == 1 )
		{
			data_info data;
			data = m_data[i];
			data.pimg = &img;

			FMyRect bbox = CLBFCommon::compute_bbox(data.current_pts);
			data.pimg->fd = bbox;
			data.current_norm_pts = data.current_norm_pts * 2 + mean_shape;
			data.current_norm_pts /= 3;
			CLBFCommon::compute_similarity(data.current_pts, data.current_norm_pts, data.q);
			CLBFCommon::apply_similarity_transform(data.current_norm_pts, data.current_pts, data.q);

			FMyRect bbox2 = CLBFCommon::compute_bbox(data.current_pts);
			data.q[2] += (bbox.left + bbox.right) / 2 - (bbox2.left + bbox2.right) / 2;
			data.q[3] += (bbox.top + bbox.bottom) / 2 - (bbox2.top + bbox2.bottom) / 2;

			CLBFCommon::compute_inverse_similarity(data.q, data.inv_q);
			CLBFCommon::apply_similarity_transform(data.current_norm_pts, data.current_pts, data.q);

			bool is_face = align(data);

			float pitch = 0, yaw = 0, roll = 0;
			get_pose(pitch, yaw, roll);
			//printf("PITCH= %f, Yaw = %f, Roll = %f\n", pitch, yaw, roll);
			m_data[i] = data;
			m_data[i].nROP = (int)yaw;

			if( is_face )
			{
				aligned_face_cnt++;
				m_data[i].TRACKING_STATUS = 1;
			}
			else
			{
				m_data[i].TRACKING_STATUS = 0;
			}
							
			float lEyeP1 = (data.current_pts.m_data[36 * 2] + data.current_pts.m_data[39*2]) / 2 + 0.5;
			float lEyeP2 = (data.current_pts.m_data[36 * 2 + 1] + data.current_pts.m_data[39 * 2 + 1]) / 2 + 0.5;
			float rEyeP1 = (data.current_pts.m_data[42 * 2] + data.current_pts.m_data[45 * 2 + 1]) / 2 + 0.5;
			float rEyeP2 = (data.current_pts.m_data[42 * 2 + 1] + data.current_pts.m_data[45 * 2 + 1]) / 2 + 0.5;
			if( lEyeP1 < fd_info.boundRect[i].left || rEyeP1 > fd_info.boundRect[i].left + fd_info.boundRect[i].width ||
				lEyeP2 < fd_info.boundRect[i].top || rEyeP2 > fd_info.boundRect[i].top + fd_info.boundRect[i].height )
			{
				m_data[i].TRACKING_STATUS = 0;
			}
			
		}
	}

	
// 	for( int i = 0; /*i < fd_info.prev_cnt &&*/ i < MAX_FACE_NUM; i++ )
// 	{
// 		m_prev_data[i].current_pts.Clear();
// 		m_prev_data[i].initial_pts.Clear();
// 		m_prev_data[i].current_norm_pts.Clear();
// 
// 		memset(m_prev_data[i].q, 0, sizeof(float) * 4);
// 		memset(m_prev_data[i].inv_q, 0, sizeof(float) * 4);
// 		m_prev_data[i].nROP = 0;
// 		m_prev_data[i].nHitCnt = -1;
// 		m_prev_data[i].nUnknownHitCnt = -1;
// 		m_prev_data[i].nRecogID = -1;
// 		m_prev_data[i].nRankOneID = -1;
// 		m_prev_data[i].fConfidence = 0.0;
// 		m_prev_data[i].nGenderType = -1;
// 		m_prev_data[i].nPermanentID = -1;
// 		m_prev_data[i].TRACKING_STATUS = 0;	
// 		memset(m_prev_data[i].nRecogIDs, 0, sizeof(int) * 20);
// 	}

	for( int i = 0; /*i < fd_count &&*/  i < MAX_FACE_NUM; i++ )
	{
		m_prev_data[i].current_pts.CopyFrom(m_data[i].current_pts);
		m_prev_data[i].initial_pts.CopyFrom(m_data[i].initial_pts);
		m_prev_data[i].current_norm_pts.CopyFrom(m_data[i].current_norm_pts);

		memcpy(m_prev_data[i].q, m_data[i].q, sizeof(float) * 4);
		memcpy(m_prev_data[i].inv_q, m_data[i].inv_q, sizeof(float) * 4);
		m_prev_data[i].nROP = m_data[i].nROP;
		m_prev_data[i].nHitCnt = m_data[i].nHitCnt;
		m_prev_data[i].nUnknownHitCnt = m_data[i].nUnknownHitCnt;
		m_prev_data[i].nRecogID = m_data[i].nRecogID;
		m_prev_data[i].nRankOneID = m_data[i].nRankOneID;
		m_prev_data[i].fConfidence = m_data[i].fConfidence;
		m_prev_data[i].nGenderType = m_data[i].nGenderType;
		m_prev_data[i].nPermanentID = m_data[i].nPermanentID;
		m_prev_data[i].TRACKING_STATUS = m_data[i].TRACKING_STATUS;
		
		memcpy(m_prev_data[i].nRecogIDs, m_data[i].nRecogIDs, sizeof(int) * 20);
	}

	fd_info.prev_cnt = fa_min(fd_count, MAX_FACE_NUM);
	
	frame_count++;


	// 얼굴 검출을 이용한 verify
	//bool b_v = true;
	if( frame_count % 100000 == 0 )
	{
		//b_v = verify(img);
		reinitialize();
		return m_face_cnt;
	}

	if( aligned_face_cnt == m_face_cnt )
	{
		return aligned_face_cnt;
	}
	else
	{
		reinitialize();
		return m_face_cnt;
	}
}

/*
@brief 비디오의 영상을 입력으로 주고 특징점을 추적한다.
*/
int CLBFTester::track(img_info &img) 
{
	int fd_count = detect(img);
	if( fd_count > 0 )
	{
		if( fd_info.prev_cnt != fd_info.cnt )
		{
			TRACKING_STATUS = 0;
			frame_count = 1;
		}
		else
		{
			TRACKING_STATUS = 1;
		}
	}
	else
	{
		TRACKING_STATUS = 0;
		frame_count = 0;
	}
	

	int aligned_face_cnt = 0;
	for( int i = 0; i < m_face_cnt; i++ )
	{
		data_info data;
		data = m_data[i];
		data.pimg = &img;

		///////////////////////////////
		// mean shape으로 다시 초기화
		FMyRect bbox = CLBFCommon::compute_bbox(data.current_pts);
		data.pimg->fd = bbox;
		data.current_norm_pts = data.current_norm_pts * 2 + mean_shape;
		data.current_norm_pts /= 3;
		CLBFCommon::compute_similarity(data.current_pts, data.current_norm_pts, data.q);
		CLBFCommon::apply_similarity_transform(data.current_norm_pts, data.current_pts, data.q);

		FMyRect bbox2 = CLBFCommon::compute_bbox(data.current_pts);
		data.q[2] += (bbox.left + bbox.right) / 2 - (bbox2.left + bbox2.right) / 2;
		data.q[3] += (bbox.top + bbox.bottom) / 2 - (bbox2.top + bbox2.bottom) / 2;


		CLBFCommon::compute_inverse_similarity(data.q, data.inv_q);
		CLBFCommon::apply_similarity_transform(data.current_norm_pts, data.current_pts, data.q);


		///////////////////////////////
		bool is_face = align(data);
		m_data[i] = data;

		if( is_face )
		{
			aligned_face_cnt++;
		}
	}

	frame_count++;

	fd_info.prev_cnt = fa_min(fd_count, MAX_FACE_NUM);

	// 얼굴 검출을 이용한 verify
	//bool b_v = true;
	if (frame_count % 10000 == 0) 
	{
		//b_v = verify(img);
		reinitialize();
		return m_face_cnt;
	}

	if (aligned_face_cnt == m_face_cnt) 
	{
		return aligned_face_cnt;
	}
	else
	{
		reinitialize();
		return m_face_cnt;
	}
}


int CLBFTester::img_detect(img_info &img) {

	int fd_count = detect_img(img);

	for (int i = 0; i < m_face_cnt; i++) {
		data_info data;
		data = m_data[i];

		for (int re = 0; re < IMG_REPEAT_CNT; re++) {

			///////////////////////////////
			// mean shape으로 다시 초기화
			FMyRect bbox = CLBFCommon::compute_bbox(data.current_pts);
			data.pimg->fd = bbox;
			//data.current_norm_pts = mean_shape;
			data.current_norm_pts = data.current_norm_pts * 2 + mean_shape;
			data.current_norm_pts /= 3;
			CLBFCommon::compute_similarity(data.current_pts, data.current_norm_pts, data.q);
			CLBFCommon::apply_similarity_transform(data.current_norm_pts, data.current_pts, data.q);

			FMyRect bbox2 = CLBFCommon::compute_bbox(data.current_pts);
			data.q[2] += (bbox.left + bbox.right) / 2 - (bbox2.left + bbox2.right) / 2;
			data.q[3] += (bbox.top + bbox.bottom) / 2 - (bbox2.top + bbox2.bottom) / 2;


			CLBFCommon::compute_inverse_similarity(data.q, data.inv_q);
			CLBFCommon::apply_similarity_transform(data.current_norm_pts, data.current_pts, data.q);

			///////////////////////////////


			
			/*data_info temp_data = data;
			img_info temp_img;
			temp_img = img;
			data.pimg = &temp_img;
			resize_face_rect(data, i);*/

			bool is_face = align(data);

			/*back_face_rect(data, i);
			data.pimg = &img;*/

			m_data[i] = data;

			if (re == IMG_REPEAT_CNT - 1) {
				fd_info.enable[i] = is_face;
			}
		}
	}

	return fd_count;

}

/*
 @brief 얼굴이 큰 경우 resize 한 다음 align 할 수 있도록 한다.
*/
void CLBFTester::resize_face_rect(data_info &data, int idx) {
	FMyRect norm_box = CLBFCommon::compute_bbox(data.current_norm_pts);
	CFMat bound_rect(4, 2), t_bound_rect(4, 2);
	bound_rect.m_data[0 * 2] = norm_box.left;
	bound_rect.m_data[0 * 2 + 1] = norm_box.top;
	bound_rect.m_data[1 * 2] = norm_box.right;
	bound_rect.m_data[1 * 2 + 1] = norm_box.top;
	bound_rect.m_data[2 * 2] = norm_box.left;
	bound_rect.m_data[2 * 2 + 1] = norm_box.bottom;
	bound_rect.m_data[3 * 2] = norm_box.right;
	bound_rect.m_data[3 * 2 + 1] = norm_box.bottom;


	CLBFCommon::apply_similarity_transform(bound_rect, t_bound_rect, data.q);

	// face width
	SMyPoint ptLtop, ptRtop;
	ptLtop.x = t_bound_rect.m_data[0];
	ptLtop.y = t_bound_rect.m_data[1];
	ptRtop.x = t_bound_rect.m_data[2];
	ptRtop.y = t_bound_rect.m_data[3];

	int face_width = sqrt((ptLtop.x - ptRtop.x) * (ptLtop.x - ptRtop.x) + (ptLtop.y - ptRtop.y) * (ptLtop.y - ptRtop.y));

	if (face_width > 100) {
		b_img_resized = true;

		img_resized_ratio = 40.0f / face_width;
		//img_resized_ratio = 0.2;

		int face_width = data.pimg->img_width * img_resized_ratio;
		int face_height = data.pimg->img_height * img_resized_ratio;

		unsigned char *img = new unsigned char[face_width * face_height];
		Resize_NN(data.pimg->img, data.pimg->img_width, data.pimg->img_height, img, face_width, face_height);

		data.pimg->img = img;
		data.pimg->img_width = face_width;
		data.pimg->img_height = face_height;

		data.q[0] *= img_resized_ratio;
		data.q[1] *= img_resized_ratio;
		data.q[2] *= img_resized_ratio;
		data.q[3] *= img_resized_ratio;
		data.current_pts *= img_resized_ratio;
	}
	else {
		b_img_resized = false;
	}
}

/*
@brief 작은 얼굴로 align한 다음 원래 얼굴 크기로 변환
*/
void CLBFTester::back_face_rect(data_info &data, int idx) {
	if (b_img_resized) {
		data.q[0] *= 1 / img_resized_ratio;
		data.q[1] *= 1 / img_resized_ratio;
		data.q[2] *= 1 / img_resized_ratio;
		data.q[3] *= 1 / img_resized_ratio;
		data.current_pts *= 1 / img_resized_ratio;

		delete [](data.pimg->img);
	}
}



void CLBFTester::reinitialize() 
{
	for( int i = 0; i < MAX_FACE_NUM; i++ )
	{
		m_data[i].current_pts.Clear();
		m_data[i].initial_pts.Clear();
		m_data[i].current_norm_pts.Clear();
		memset(m_data[i].q, 0, sizeof(float) * 4);
		memset(m_data[i].inv_q, 0, sizeof(float) * 4);
		m_data[i].nROP = 0;
		m_data[i].nHitCnt = -10;
		m_data[i].nUnknownHitCnt = -15;
		m_data[i].nRecogID = -2;
		m_data[i].nRankOneID = -2;
		m_data[i].fConfidence = 0.0;
		m_data[i].nGenderType = 0;
		m_data[i].nPermanentID = -1;
		m_data[i].TRACKING_STATUS = 0;
		memset(m_data[i].nRecogIDs, -2, sizeof(int) * 20);

		m_prev_data[i].current_pts.Clear();
		m_prev_data[i].initial_pts.Clear();
		m_prev_data[i].current_norm_pts.Clear();
		memset(m_prev_data[i].q, 0, sizeof(float) * 4);
		memset(m_prev_data[i].inv_q, 0, sizeof(float) * 4);
		m_prev_data[i].nROP = 0;
		m_prev_data[i].nHitCnt = -10;
		m_prev_data[i].nUnknownHitCnt = -15;
		m_prev_data[i].nRecogID = -2;
		m_prev_data[i].nRankOneID = -2;
		m_prev_data[i].fConfidence = 0.0;
		m_prev_data[i].nGenderType = 0;
		m_prev_data[i].nPermanentID = -1;
		m_prev_data[i].TRACKING_STATUS = 0;
		memset(m_prev_data[i].nRecogIDs, -2, sizeof(int) * 20);

		fd_info.id[i] = -1;
		memset(&(fd_info.boundRect[i]), 0, sizeof(FDRECT) * 1);
	}
	fd_info.prev_cnt = 0;
	fd_info.cnt = 0;
}

void CLBFTester::get_fd_result(int idx, SMyRect &rect) {
	if (idx < fd_info.cnt) {
		rect.left = fd_info.boundRect[idx].left;
		rect.right = fd_info.boundRect[idx].left + fd_info.boundRect[idx].width;
		rect.top = fd_info.boundRect[idx].top;
		rect.bottom = fd_info.boundRect[idx].top + fd_info.boundRect[idx].height;
	}
}

bool CLBFTester::get_alignment_result(int idx, CFMat &result) {
	if (idx < fd_info.cnt) {
		result.CopyFrom(m_data[idx].current_pts);
		return fd_info.enable[idx];
	}

	return false;
}

/*
@brief idx번째 얼굴검출 결과를 이용하여 data_info를 초기화한다.
*/
void CLBFTester::initialize_data(data_info &data, int idx) {

	FMyRect bbox;
	bbox.left = fd_info.boundRect[idx].left;
	bbox.right = fd_info.boundRect[idx].left + fd_info.boundRect[idx].width;
	bbox.top = fd_info.boundRect[idx].top;
	bbox.bottom = fd_info.boundRect[idx].top + fd_info.boundRect[idx].height;

	float width_scale = float(RECT_WIDTH(bbox) + 1) / float(RECT_WIDTH(mean_shape_bound));
	float height_scale = float(RECT_HEIGHT(bbox) + 1) / float(RECT_HEIGHT(mean_shape_bound));
	float scale = (width_scale + height_scale) / 2 * 0.9;

	data.q[0] = scale * cos(fd_info.rip_angle[idx] * 3.141952 / 180);
	data.q[1] = scale * sin(fd_info.rip_angle[idx] * 3.141952 / 180);
	data.q[2] = bbox.left - mean_shape_bound.left * scale;
	data.q[3] = bbox.top - mean_shape_bound.top * scale;

	if (1) {
		float pitch = 0;
		float yaw = fa_max(fa_min(-fd_info.rop_angle[idx], 30), -30) *  3.141592 / 180;
		float roll = 0;

		float sina = sin(pitch), sinb = sin(yaw), sinc = sin(roll);
		float cosa = cos(pitch), cosb = cos(yaw), cosc = cos(roll);

		CFMat R(3, 3);

		R[0][0] = cosb * cosc;								R[0][1] = -cosb * sinc;								R[0][2] = sinb;
		R[1][0] = cosa * sinc + sina * sinb * cosc;			R[1][1] = cosa * cosc - sina * sinb * sinc;			R[1][2] = -sina * cosb;
		R[2][0] = R[0][1] * R[1][2] - R[0][2] * R[1][1];	R[2][1] = R[0][2] * R[1][0] - R[0][0] * R[1][2];	R[2][2] = R[0][0] * R[1][1] - R[0][1] * R[1][0];

		float r00 = R[0][0];
		float r01 = R[0][1];
		float r02 = R[0][2];
		float r10 = R[1][0];
		float r11 = R[1][1];
		float r12 = R[1][2];
		float *src = SHAPE_3D_MODEL_MEAN_SHAPE;
		CFMat projected_shape(VCNT, 2);
		float *dst = projected_shape.m_data;
		for (int i = 0; i <VCNT; i++) // 2d shape = scale * R33 * vecSp + [tx ty]
		{
			// x
			*dst++ = 2 * (r00 * *src + r01 * *(src + 1) + r02 * *(src + 2));
			// y
			*dst++ = 0.8 * 2 * (r10 * *src + r11 * *(src + 1) + r12 * *(src + 2));
			src += 3;
		}

		data.current_norm_pts = projected_shape;

		CLBFCommon::apply_similarity_transform(data.current_norm_pts, data.current_pts, data.q);
		FMyRect current_box = CLBFCommon::compute_bbox(data.current_pts);
		data.q[2] -= (current_box.left + current_box.right) / 2 - (bbox.left + bbox.right) / 2;
		data.q[3] -= (current_box.top + current_box.bottom) / 2 - (bbox.top + bbox.bottom) / 2;
	}
	else {
		data.current_norm_pts = mean_shape;
	}

	CLBFCommon::apply_similarity_transform(data.current_norm_pts, data.current_pts, data.q);
	CLBFCommon::compute_inverse_similarity(data.q, data.inv_q);
}

/*
@brief 입력 영상에 대해 align을 실행한다.
*/
bool CLBFTester::align(data_info &data)
{
	int i;
	CForest *stage_forest;

	for( i = 0; i < T; i++ ) // stage 별 학습
	{
		stage_forest = &m_forest[i];

		//////////////////////////////////////
		// 1. local binary feature
		CIMat local_binary_features(VCNT * N_TREES, 1);
		compute_local_binary_feature(data, stage_forest, local_binary_features.m_data);

		//////////////////////////////////////
		// 2. global regression
		CFMat delta = m_global_regressor.update_residual(i, local_binary_features);
		delta.Reshape(VCNT, 2);

		data.current_norm_pts += delta;
		CLBFCommon::apply_similarity_transform(data.current_norm_pts, data.current_pts, data.q);

		//////////////////////////////////////
		// 3. shape 정리
		float temp_q[4], temp_inv_q[4];
		CLBFCommon::compute_similarity(mean_shape, data.current_pts, data.inv_q);
		CLBFCommon::compute_inverse_similarity(data.inv_q, data.q);
		CLBFCommon::apply_similarity_transform(data.current_pts, data.current_norm_pts, data.inv_q);
		CLBFCommon::apply_similarity_transform(data.current_norm_pts, data.current_pts, data.q);
	}

	compute_pose(data.current_pts);

	//////////////////////////////////////
	// 4. check face
	bool b_face = m_face_rejector.is_face(data);
	
	return b_face;
}

/*
@brief 영상에 대해 local binary feature를 계산한다.
*/
void CLBFTester::compute_local_binary_feature(data_info &data, CForest *stage_forest, int *local_binary_features) 
{
	int i, j;

	int *dst = local_binary_features;

	for( i = 0; i < VCNT; i++ )
	{
		for( j = 0; j < N_TREES; j++ )
		{
			CTree *tree = stage_forest->GetTree(i, j);
			int idx = tree->compute_local_binary_feature(data, i);
			*dst++ = idx;
		}
	}
}

/**
@brief	detect the faces
@param[in]	"unsigned char"		*src			- gray raw data
@param[in]	"int"				width			- width of input image
@param[in]	"int"				height			- height of input image
@return	the number of detected faces
*/
int CLBFTester::DetectFaceEye(unsigned char *src, int width, int height)
{
	int nFaceCount = 0;

	//////////////////////////////////////////////////////
	// face detection
	FD_RECT roi = { 0, 0, width, height };
	MVFD_DetectFace_Video(pContext, src, width, height, roi, DEFAULT_SIZE, DEFAULT_SIZE, ROTATE_SDIC_SEARCH);
	//MVFD_DetectFace_Image(pContext, src, width, height, roi, DEFAULT_SIZE, DEFAULT_SIZE, ROTATE_FULL_SEARCH);
	MVFD_GetResults(pContext, &result);

	nFaceCount = result.face_num;

	if (nFaceCount == 0) 
	{
		fd_info.cnt = 0;
		return 0;
	}

	//srand(0);
	for (int i = 0; i < result.face_num; ++i)
	{
		float rip_angle, rop_angle;
		HP_POINTF face_pose_shape[4];
		HP_RECT rect = { result.face[i].rect.left, result.face[i].rect.top, result.face[i].rect.right, result.face[i].rect.bottom };		
		HP_EstimatePose(src, width, height, rect, result.face[i].rip_angle, result.face[i].rop_angle, result.face[i].img_direction, &rip_angle, &rop_angle, face_pose_shape);

		fd_info.boundRect[i].left = result.face[i].rect.left;
		fd_info.boundRect[i].top = result.face[i].rect.top;
		fd_info.boundRect[i].width = result.face[i].rect.right - result.face[i].rect.left;
		fd_info.boundRect[i].height = result.face[i].rect.bottom - result.face[i].rect.top;

		fd_info.boundRect[i].enable = 1;	

		fd_info.pose[i] = (int)result.face[i].pose;
		fd_info.rip_angle[i] = (int)rip_angle;
		fd_info.rop_angle[i] = (int)rop_angle;

		fd_info.eyeL[i].enable = 0;
		fd_info.eyeR[i].enable = 0;
	}
	fd_info.cnt = nFaceCount;

	return nFaceCount;
}

int CLBFTester::CorrectEyeInfo(FD_INFO &fd_info)
/**
@brief	detect face and eyes(option)
@param[in]	"unsigned char"		*src			- gray raw data
@param[in]	"int"				width			- width of input image
@param[in]	"int"				height			- height of input image
@return	the number of detected faces
*/
{
	/////////////////////////////////////////////////
	// correct eye detection
	// check the rop_angle by face detector and eye detector
	int i;
	for (i = 0; i < fd_info.cnt; i++) {
		int fd_angle = fd_info.rip_angle[i];

		if (fd_info.eyeL[i].enable == 1 && fd_info.eyeR[i].enable == 1)
		{
			int line1[2], line2[2];

			line1[0] = fd_info.eyeR[i].left + fd_info.eyeR[i].width / 2 -
				(fd_info.eyeL[i].left + fd_info.eyeL[i].width / 2);
			line1[1] = fd_info.eyeR[i].top + fd_info.eyeR[i].height / 2 -
				(fd_info.eyeL[i].top + fd_info.eyeL[i].height / 2);

			line2[0] = line1[0];
			line2[1] = 0;


			double l_line1 = sqrt(double(line1[0] * line1[0] + line1[1] * line1[1]));
			double l_line2 = sqrt(double(line2[0] * line2[0] + line2[1] * line2[1]));

			int ed_angle;
			if (l_line1 / l_line2 < 1) {
				ed_angle = CUtil::radin2angle(acos(l_line1 / l_line2));
			}
			else {
				ed_angle = CUtil::radin2angle(acos(l_line2 / l_line1));
			}

			int diff = ed_angle - abs(fd_angle);

			if (diff > ED_ANGLE_CORRECTION
				|| diff < -ED_ANGLE_CORRECTION) {
				fd_info.eyeL[i].enable = 0;
				fd_info.eyeR[i].enable = 0;
			}

			line1[0] = fd_info.eyeR[i].left + fd_info.eyeR[i].width / 2 -
				(fd_info.eyeL[i].left + fd_info.eyeL[i].width / 2);
			line1[1] = fd_info.eyeR[i].top + fd_info.eyeR[i].height / 2 -
				(fd_info.eyeL[i].top + fd_info.eyeL[i].height / 2);

			l_line1 = sqrt(double(line1[0] * line1[0] + line1[1] * line1[1]));

			if (l_line1 > (fd_info.boundRect[i].width + fd_info.boundRect[i].height) / 2.0 * 0.8) {
				fd_info.eyeL[i].enable = 0;
				fd_info.eyeR[i].enable = 0;
			}
		}
	}

	return 1;
	//return nFaceCount;
}

/*
@brief 얼굴을 검출해서 align된 얼굴과 비슷한 위치에 있는지 확인한다.
*/
bool CLBFTester::verify(img_info &img) {
	int detected_face_cnt = DetectFaceEye(img.img, img.img_width, img.img_height);

	if (detected_face_cnt != m_face_cnt) {
		return false;
	}
	else {
		return true;
	}

	/*int *graph = new int[detected_face_cnt *m_face_cnt];

	for (int i = 0; i < detected_face_cnt; i++) {

		for (int j = 0; j < m_face_cnt; j++) {

		}
	}*/

}

void CLBFTester::compute_pose(CFMat &pts) {


	std::vector<CvPoint2D32f> projectedPoints;

	CFMat temp = pts;
	FMyRect current_box = CLBFCommon::compute_bbox(temp);
	for (int i = 0; i < N_POSE_POINT_INDEX; i++) {
		temp.m_data[POSE_POINT_INDEX[i] * 2] -= (current_box.left + current_box.right) / 2;
		temp.m_data[POSE_POINT_INDEX[i] * 2 + 1] -= (current_box.top + current_box.bottom) / 2;
		projectedPoints.push_back(cvPoint2D32f(temp.m_data[POSE_POINT_INDEX[i] * 2], temp.m_data[POSE_POINT_INDEX[i] * 2 + 1]));
	}

#define FOCAL_LENGTH 1000 

	//CvMatr32f rotation_matrix = new float[9];
	//CvVect32f translation_vector = new float[3];
	CvTermCriteria criteria = cvTermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 100, 1.0e-4f);
	cvPOSIT(positObject, &projectedPoints[0], FOCAL_LENGTH, criteria, m_rotation_matrix, m_translation_vector);

	//printf("%f, %f, %f\n", m_rotation_matrix[0 * 3], m_rotation_matrix[1 * 3 + 1], m_rotation_matrix[2 * 3 + 2]);

	//delete rotation_matrix;
	//delete translation_vector;

	
}

bool CLBFTester::get_pose(float &pitch, float &yaw, float &roll) {
	float q[4], y, r;
	CFMat R;
	R.Wrap(3, 3, &m_rotation_matrix[0]);

	q[0] = sqrt(1 + R[0][0] + R[1][1] + R[2][2]) / (float)2.0;
	q[1] = (R[2][1] - R[1][2]) / (4 * q[0]);
	q[2] = (R[0][2] - R[2][0]) / (4 * q[0]);
	q[3] = (R[1][0] - R[0][1]) / (4 * q[0]);

	pitch = atan2(2 * (q[0] * q[1] - q[2] * q[3]), q[0] * q[0] - q[1] * q[1] - q[2] * q[2] + q[3] * q[3]);
	yaw = asin(2 * (q[0] * q[2] + q[1] * q[3]));
	roll = atan2(2 * (q[0] * q[3] - q[1] * q[2]), q[0] * q[0] + q[1] * q[1] - q[2] * q[2] - q[3] * q[3]);

	pitch *= 180 / 3.141592;
	yaw *= 180 / 3.141592;
	roll *= 180 / 3.141592;


	return true;
}