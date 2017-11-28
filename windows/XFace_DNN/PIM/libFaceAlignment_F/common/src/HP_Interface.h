#pragma once
//#include "HP_Defines.h"

#ifndef WIN32
#include "FacePlatformInterface.h"
#endif

typedef struct
{
	int left, top, right, bottom;
} HP_RECT;

typedef struct
{
	float x, y;
} HP_POINTF;

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

	void HP_Init();
	void HP_Release();
	void HP_TimeAverage(HP_RECT rect, float yaw, float rip, float *n_yaw,  float *n_rip);
	void HP_EstimatePose(unsigned char *rawdata, int src_w, int src_h, HP_RECT face_rect, int fd_rip, int fd_rop, int image_direction, float *rip_angle, float *rop_angle, HP_POINTF *estimated_shape);

#ifdef __cplusplus
}
#endif /* __cplusplus */
	
