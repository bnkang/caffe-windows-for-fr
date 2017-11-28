#ifndef _FR_PARAMS_H_
#define _FR_PARAMS_H_

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define IMAGE_WIDTH					190
#define IMAGE_HEIGHT				190
#define ALIGN_IMG_WIDTH				95
#define ALIGN_IMG_HEIGHT			95
#define BLOCK_WIDTH					40
#define BLOCK_HEIGHT				40
#define CELL_WIDTH					10
#define CELL_HEIGHT					10
#define LANDMARK_NUM				70
#define IMAGE_SCALE					4
#define LANDMARK_PTS				27

#define FR_FACE_BIN_SIZE_ULBP		59
#define FR_FEATURE_MULTI_ULBP_SIZE	(BLOCK_WIDTH*BLOCK_HEIGHT/CELL_WIDTH/CELL_HEIGHT*FR_FACE_BIN_SIZE_ULBP*LANDMARK_PTS*IMAGE_SCALE)
#define FR_FEATURE_DNN_ENSEMBLE		8192
#define PCA_FEATURE_DIM				1024/*2000*/


#define FR_ENROLL_IMAGE_PER_PERSON 10
#define	FR_ENROLL_IMAGE_PER_PERSON_4_IMAGE	1

#define FR_ENROLL_MAX_SIZE			200
	//#define FR_ENROLL_MAX_SIZE			160
#define FR_TL_LBP_FEATURE_SIZE		PCA_FEATURE_DIM
#define FR_FACE_WIDTH				190
#define FR_FACE_HEIGHT				190

#define NUM_THREADS					12


	// Verifier Threshold
#define FR_VERIFIER_THR				0.0f

	// Identifier Threshold
	//#define FR_IDENTIFIER_THR			40.0f
#define FR_IDENTIFIER_THR			685.0f//666.0f/*719.0f*/


#define _TRACKING_MODE_

// #define TRANS_MAT_PATH				"TransMatrix_DNN.dat"
// #define LBP_TL_MAT_PATH				"SQRTLBP_Matrix_TL_DEEP_1024.dat"//*"SQRTLBP_Matrix_1000.dat"*//*"SQRTLBP_Matrix_1000.dat"*///"SQRTLBP_Matrix_2000.dat"//"SQRTLBP_Matrix_500.dat"//"SQRTLBP_Matrix_2000.dat"//"SQRTLBP_Matrix_LFW_300.dat"
// #define FR_GALLERY_DB_PATH			"enroll/enroll_dnn.dat"//"enroll_JB_1000_1120_2p.dat"//"enroll/enroll_JB_1000_201601.dat"/*"enroll/enroll_JB_1000_1218_2.dat"*//*"enroll/enroll_1000_2.dat"*/
//
// #define MODEL_95_1 "./model/95x95_1_4a.caffemodel"
// #define MODEL_95_2 "./model/95x95_2_4a.caffemodel"
// #define MODEL_67 "./model/67x67_4a.caffemodel"
// #define MODEL_47 "./model/47x47_4a.caffemodel"
// #define MODEL_T "./model/Model_Net_Integrated.caffemodel"
// 
// #define MODEL_NET_95_1 "./model/Model_Net_95_1.prototxt"
// #define MODEL_NET_95_2 "./model/Model_Net_95_2.prototxt"
// #define MODEL_NET_67 "./model/Model_Net_67.prototxt"
// #define MODEL_NET_47 "./model/Model_Net_47.prototxt"
// #define MODEL_NET_T "./model/Model_Net_Integrated.prototxt"


#define DNN_TRANS_MAT_PATH			"STFC_Data_01.dat"
#define DNN_TL_MAT_PATH				"STFC_DATA_02.dat"//"SQRT_Matrix_TL_Deep_1024.dat"
#define MODEL_95_1					"./model/STFC_4a1.dat"
#define MODEL_95_2					"./model/STFC_4a2.dat"
#define MODEL_67					"./model/STFC_4a3.dat"
#define MODEL_47					"./model/STFC_4a4.dat"

#define MODEL_NET_95_1				"./model/STFC_4a1N.dat"
#define MODEL_NET_95_2				"./model/STFC_4a2N.dat"
#define MODEL_NET_67				"./model/STFC_4a3N.dat"
#define MODEL_NET_47				"./model/STFC_4a4N.dat"

#define FR_GALLERY_DB_PATH			"enroll/enroll_hyu.dat"//"enroll/enroll_dnn_Robot_high_school_Enroll_Final.dat"//"enroll/enroll_dnn.dat"
//#define FR_GALLERY_DB_PATH			"enroll/enroll_ytf.dat"//"enroll/enroll_dnn.dat"

#ifdef __cplusplus
}
#endif

#endif // #ifndef _FR_PARAMS_H_