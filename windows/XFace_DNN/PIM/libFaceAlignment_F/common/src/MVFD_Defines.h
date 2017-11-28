#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include "MVFD_Interface.h"
#include "MVFD_Thread.h"
using namespace std;

// 1.0~ 1.1사이의 값에 따라 정확도 vs. 속도 trade-off
// 1.0 : 97% Face Acceptance, 84% Bkgnd Rejection
// 1.1 : 93% Face Acceptance, 92% Bkgnd Rejection
#ifdef WIN32
	//#define USE_SSE
#else
	#define USE_NEON
#endif

#ifdef USE_NEON
#include "Neon.h"
#endif
#ifdef ANDROID
#include <android/log.h>
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, "libnav", __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG  , "libnav", __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO   , "libnav", __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN   , "libnav", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR  , "libnav", __VA_ARGS__)
#else
#define LOGV(...) printf( __VA_ARGS__)              
#define LOGD(...) printf( __VA_ARGS__)
#define LOGI(...) printf( __VA_ARGS__)
#define LOGW(...) printf( __VA_ARGS__)
#define LOGE(...) printf( __VA_ARGS__)
#endif // #ifndef USE_SSE

//Optimizations

#ifdef USE_NEON

//MCT Value is doubled to save some instructions during MCT Confidence Computation
#define DOUBLE_MCT_AND_FASTPOSE	1

//Resize Optimization
#define RESIZE_NN_OPT				1

#define USE_FEATURE_DEP_OPT		1

#define MVFD_ENABLE_SECOND_POSE		1
#endif

#define FD_FACE_HEIGHT 24
#define FD_POSE_NUM	9
#define FD_IMAGE_SCALE_RATE 0.87
#define FD_REJECTION_RATE 1.03
#define FD_TRACKING_REJECTION_RATE 1.03
#define FD_NO_REJECTOR_TO_FRONTAL_FACE
#define FD_MAX_CLASS_NUM		22
#define FD_MAX_STAGE_NUM		10
#define FD_FACE_CANDI_HASH_SIZE 100000
#define FD_MAX_FIND		1000
#define FD_FACE_WIDTH 24
#define FD_VIDEO_SCALE_RATE 0.84
#define FD_FEATURE_TYPE_NUM 5
#define FD_MAX_THREADS_NUM	4
#define FEATURE_TRANSFORM_XFLIP	0
#define FEATURE_TRANSFORM_YFLIP	1
#define FEATURE_TRANSFORM_ROT_90CW	2
#define FEATURE_TRANSFORM_ROT_90CCW	3
#define FD_MAX_FACE_CANDIDATES_PER_THREAD 3000
#define FD_LIKELIHOOD_BIN_NUM 100
#define FD_NEW_LIKELIHOOD_BIN_NUM	1000
#define FD_MIN_INTERSECTION_CNT	1
//#define FD_TRACKING_TRY_3SCALES
//#define USE_ONLY_HIGH_CONFIDENCES_FOR_BOX_CLUSTERING

#define MVFD_LOWERBOUND_THRESHOLD	5000
#define MVFD_DEFAULT_THRESHOLD		25000
#define MVFD_UPPERBOUND_THRESHOLD	50000
typedef struct _FD_POINT
{
	int x, y;
} FD_POINT;

typedef struct _FD_CLASSIFIER
{
	int class_num;
	unsigned int *likelihood_new;
	unsigned int fv_lower_new;
	unsigned int fv_upper_new;
	unsigned int likelihood[FD_MAX_CLASS_NUM][100];
	unsigned int fv_lower[FD_MAX_CLASS_NUM];
	unsigned int fv_upper[FD_MAX_CLASS_NUM];
	unsigned short thresholds[24];
	unsigned int threshold;
	double minv;
	double double2short;
	int total_feature_num;
	int feature_type_num;
	int feature_type[FD_FEATURE_TYPE_NUM];
	int feature_num[FD_FEATURE_TYPE_NUM];	
	char *feature_pos[FD_FEATURE_TYPE_NUM];
	int pose;
	int from_header;
	unsigned short *lookup_tbl[FD_FEATURE_TYPE_NUM];
	_FD_CLASSIFIER *next;
} FD_CLASSIFIER;

typedef struct _FEATURE
{
	unsigned char *img;
	unsigned char *rot;
	unsigned char *irmb;
	unsigned short *mct;
	int width, height;
} FEATURE;

typedef struct
{
	FD_RECT rect;
	char pose;
	int img_direction;
	int likelihood;
	int intersect_cnt;
	int pose_likelihood[FD_POSE_NUM];
	float rip_angle;
	float rop_angle;
} FD_INTERNAL_RESULT;

typedef struct _FACE_HASH_ELEMENT
{
	int pose;
	int pos;
	int next;
} FACE_HASH_ELEMENT;

typedef struct _FACE_STORAGE
{
	int *FaceCandiHash;
	FACE_HASH_ELEMENT *FaceCandiHashIdxs;
	FD_INTERNAL_RESULT *FaceCandidates;
	int nHashIdxNum;
	int nHashIdxCapacity;
	int nFaceCandidateNum;
	int nFaceCandidateCapacity;
} FACE_ENROLL_STORAGE;


#define FEATURE_IDX_MCT		0
#define FEATURE_IDX_IRMB2	1
#define FEATURE_IDX_SIMD8P	2
#define FEATURE_IDX_LGP		3
#define FEATURE_IDX_MCT5x5	4

#define MAX_FACE_CANDIDATES	1000
typedef struct _POINT_FEATURE_POS
{
	int x, y;
} POINT_FEATURE_POS;

typedef struct _IRMB2_FEATURE_POS
{
	int scale[6];
	FD_POINT p[6];
} IRMB2_FEATURE_POS;

typedef struct _SIMD8P_FEATURE_POS
{
	struct
	{
		int type;
		int x;
		int y;
	} p[2];
	char mask[8];
} SIMD8P_FEATURE_POS;

typedef struct _FD_THREAD_PARAM
{
	int thread_idx;
	int total_thread_num;
	void *pArg;
} FD_THREAD_PARAM;

typedef struct _FD_CONTEXT
{
	FD_INTERNAL_RESULT m_FinalResults[FD_MAX_FIND];
	int m_nFinalResultCnt;

	FACE_ENROLL_STORAGE detect_enroll_faces;
	FACE_ENROLL_STORAGE track_enroll_faces;
	
	bool wait_for_exit;
	
	typedef struct
	{
		int *face[FD_MAX_CLASS_NUM][FD_MAX_STAGE_NUM][FD_FEATURE_TYPE_NUM];
		int *pose_map[FD_FEATURE_TYPE_NUM];
	} FFPOS;

	FFPOS det_ffpos, trk_ffpos;

	//for tracking in video mode
	typedef struct
	{
		unsigned char *video_frame;
		int frame_width, frame_height;
		bool frame_complete;
		int rip_idx;
		int orientation;
		int min_face_sz, max_face_sz;
		FD_THREAD video_detection_thread;
		FD_MUTEX frame_recv_mutex;
	} VIDEO_INFO;
	VIDEO_INFO video_info;

	//for detection worker threads
	unsigned char *subImage[FD_MAX_THREADS_NUM];
	int subImage_width, subImage_height;
	int *face_candi_xy_buf[FD_MAX_THREADS_NUM];
	int found_face_cnt[FD_MAX_THREADS_NUM];
	FD_THREAD worker_threads[FD_MAX_THREADS_NUM];
	int worker_num;

	int finish_count;
	FD_MUTEX finish_count_mutex;
	FD_MUTEX wakeup_mutex[FD_MAX_THREADS_NUM];
	FD_MUTEX finish_wait_mutex;
	FD_MUTEX mem_section_mutex;
	map< unsigned long long, pair<void *, size_t> > mem_sections;
} FD_CONTEXT;

#define FD_MAX(a,b)        ((a)>(b)?(a):(b)) 
#define FD_MIN(a,b)        ((a)<(b)?(a):(b))

#define IRMB2_ALIGNED_WIDTH(x) (((x+8)/8)*8)
#define ALIGN16(x) (((x+15)>>4)<<4)

#if defined _DEBUG && WIN32
#include <stdio.h>
#define FDASSERT(x) if(!(x)) {printf("Error in file:%s, line:%d func:%s\n\n", __FILE__, __LINE__, __FUNCTION__);__debugbreak();}
#else

#if defined WIN32
#include <stdio.h>
#define FDASSERT(x) if(!(x)) {printf("Error in file:%s, line:%d func:%s\n\n", __FILE__, __LINE__, __FUNCTION__);exit(-1);}
#elif defined ANDROID
#define FDASSERT(x) if(!(x)) {LOGE("Error in file:%s, line:%d func:%s\n\n", __FILE__, __LINE__, __FUNCTION__);exit(-1);}
#else
#define FDASSERT(x)
#endif
#endif
