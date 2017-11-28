#pragma once
#ifndef WIN32
#include "FacePlatformInterface.h"
#endif
#define ROTATE_TOP_UP		0
#define ROTATE_LEFT_UP		1
#define ROTATE_RIGHT_UP		2
#define ROTATE_BOTTOM_UP	3
#define ROTATE_FULL_SEARCH	4
#define ROTATE_180_SEARCH	5
#define ROTATE_SDIC_SEARCH	6

#define DEFAULT_SIZE		-1
typedef struct _FD_RECT
{
	int left, top, right, bottom;
} FD_RECT;

typedef struct _ONE_FACE
{
	FD_RECT rect;
	
	/////////////////// for Pose Estimation ////////////////////
	float rip_angle;
	float rop_angle;
	////////////////////////////////////////////////////////////

	int pose;
	int img_direction;
	int intersect_cnt;
	int likelihood;
} ONE_FACE;

typedef struct _FD_RESULT
{
	ONE_FACE *face;
	int capacity;
	int face_num;
	bool overwrite;
} FD_RESULT;

typedef struct _FD_CONTEXT FD_CONTEXT;
#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#ifndef WIN32
	void MVFD_Init(PResFileIO pResFileIO);
#endif
	void MVFD_Init_Header();
	void MVFD_Release();

	FD_CONTEXT *MVFD_CreateContext();
	void MVFD_DestroyContext(FD_CONTEXT *pContext);

	int  MVFD_DetectFace_Video(FD_CONTEXT *pContext, unsigned char *src, int src_w, int src_h, FD_RECT roi, int min_face_sz, int max_face_sz, int rotation);
	int MVFD_DetectFace_Image(FD_CONTEXT *pContext, unsigned char *src, int src_w, int src_h, FD_RECT roi, int min_face_sz, int max_face_sz, int img_orientation);


	void MVFD_GetResults(FD_CONTEXT *pContext, FD_RESULT *results);
	void MVFD_SetPoseWeight(float weights[]);
	void MVFD_SetInSensitivity(float insensitivity);
	void MVFD_AllocResult(FD_RESULT *result, int max_face_num);
	void MVFD_FreeResult(FD_RESULT *result);
	
	void MVFD_Resize_NN(unsigned char * __restrict src_data, int src_w, int src_h, unsigned char *__restrict des_data, int des_w, int des_h);
#ifdef __cplusplus
}
#endif /* __cplusplus */
