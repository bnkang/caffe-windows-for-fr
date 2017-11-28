#ifndef _LBF_COMMON_H_
#define _LBF_COMMON_H_

#include "FMat.h"
//#include "define_op.h"
#include "../../../exp/PIM_Define_op.h"


//////////////////////////////////////////////////////////////////////
static int T = 7;								// number of stage (iteration)
static int N_TREES = 5;							// number of trees
static int MAX_DEPTH = 5;						// tree depth
static int NUM_SAMPLES = 100;					// number of sampling
static int TARGET_MEAN_SHAPE_WIDTH = 100;		// mean shape's width
static int RANDOM_INIT_NUM = 3;					// training data augmentation 할 때, random의 횟수
static float MAX_RATIO_RADIUS[] = { 0.4, 0.3, 0.2, 0.15, 0.12, 0.10, 0.08, 0.06, 0.06, 0.05, };



static int VCNT = 68;										// number of facial features
static int VCNT2 = VCNT * 2;
static int LEFT_EYE_IDX[] = { 36, 37, 38, 39, 40, 41 };		// 왼쪽 눈에 대한 index
static int RIGHT_EYE_IDX[] = { 42, 43, 44, 45, 46, 47 };	// 오른쪽 눈에 대한 index
static int LEFT_EYE_IDX_CNT = 6;
static int RIGHT_EYE_IDX_CNT = 6;
//////////////////////////////////////////////////////////////////////

#define NORM_WIDTH 128
#define NORM_HEIGHT 128

// /*
// * @brief structure of image
// */
// struct img_info
// {
// 	unsigned char *img;		// 영상의 pointer
// 	int img_width;			// 영상의 width
// 	int img_height;			// 영상의 height
// 	CFMat gt;				// ground truth 정보
// 	FMyRect fd;				// 얼굴 검출한 경우 fd의 rect 정보
// 	bool fd_enabled;		// fd를 했는지 안 했는지
// 	bool gt_enabled;		// true if gt is enable, false otherwise
// 	char name[128];
// 	char path[256];
// };
// 
// /*
// * @brief structure of data
// */
// struct data_info
// {
// 	CFMat norm_gt;					// normalized ground truth
// 	CFMat current_pts;				// current points
// 	CFMat initial_pts;				// 현재 영상에서 초기의 shape
// 	CFMat current_norm_pts;			// current normalized points
// 	CFMat shape_residual;			// current shape's residual
// 	CFMat shape_norm_residual;		// normalized residual
// 	float q[4];						// similarity transform from mean shape to pts
// 	float inv_q[4];					// similarity transform from pts to mean shape
// 	img_info *pimg;					// img_info structure의 pointer
// 	unsigned char *norm_img;		// rejector에서 사용할 normalized image의 pointer
// };
// 
// /*
// @brief load image and ground truth
// */

class CLBFCommon {

public:
	CLBFCommon();
	~CLBFCommon();


	static void compute_similarity(CFMat& s1, CFMat& s0, float *q);
	static void compute_inverse_similarity(float *q2, float *inv_q2);
	static void apply_similarity_transform(CFMat& s1, CFMat& s0, float *q2);
	static FMyRect compute_bbox(CFMat &pts);
	
};

#endif